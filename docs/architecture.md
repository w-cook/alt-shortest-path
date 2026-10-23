# ALT Shortest-Path Library — Architecture

**Project:** `alt-shortest-path`  
**Status:** Design specification; no C algorithm implementation yet  
**Version:** Draft 1.0 — October 23, 2026  
**Companion documents:** [Algorithm specification](algorithm-specification.md), [Public API specification](api-specification.md)

## 1. Purpose and boundaries

Provide a small, reusable **C17 library implementing ALT (A*, Landmarks, Triangle Inequality)** for repeated shortest-path queries on nonnegatively weighted directed graphs. The library implements the algorithm, not a general graph-processing toolkit or a domain-specific navigation system.

The library must be usable by another library as a low-level component. It must not assume what vertices or edges represent, require coordinates, or incorporate game, road-network, or other application-specific rules.

**In scope:** graph ingestion, immutable routing representation, landmark selection/preprocessing, ALT queries, optional route reconstruction, resource ownership, error handling, testing, and performance measurement.

**Out of scope for v1:** mutable graph editing, incremental preprocessing, dynamic obstacle handling, custom application adapters, grid or navigation-mesh models, file-format importers, bidirectional ALT, ALP, and a general-purpose graph API.

## 2. System overview

```text
Caller-owned weighted edge list + landmark options
                     |
                     v
             alt_index_build()
                     |
            validate and copy data
                     |
                     v
         Immutable internal CSR graph
                     |
           landmark selection
                     |
      directed landmark preprocessing
           /                 \
      Dijkstra(G)       Dijkstra(reverse G)
           \                 /
              distance tables
                     |
                     v
              Opaque alt_index
                     |
          +----------+----------+
          |                     |
   alt_distance()      alt_shortest_path()
          |                     |
   cost / no path       allocated path / no path
```

**Ownership model:** One opaque `alt_index` owns the graph snapshot, selected landmarks, and distance tables. The caller's input edge array and explicit landmark list are borrowed only for the duration of `alt_index_build()`. Queries never modify the index.

## 3. Internal graph: compressed sparse row (CSR)

Internal vertex IDs are contiguous and zero-based: `0 .. vertex_count - 1`. An undirected connection is supplied as two directed edges. Parallel edges, self-loops, isolated vertices, and zero-cost edges are supported.

I will use the more intuitive internal identifier **`edge_starts`** for what CSR literature calls *row offsets*:

```text
edge_starts[v]       first CSR edge position for vertex v (inclusive)
edge_starts[v + 1]   end of that range (exclusive)

targets[i]          target vertex of CSR edge at position i
weights[i]          weight of CSR edge at position i
input_edge_ids[i]   original position of this edge in caller's edge list
```

CSR invariants:

- `edge_starts` has `vertex_count + 1` entries; the other three arrays each have `edge_count` entries.
- `edge_starts[0] == 0`, and `edge_starts[vertex_count] == edge_count`.
- `edge_starts[v] <= edge_starts[v + 1]` for every vertex.
- The outgoing edge positions of `v` are `[edge_starts[v], edge_starts[v + 1])`.
- If `edge_starts[v] == edge_starts[v + 1]`, then `v` has no outgoing edges.
- The CSR layout may reorder edges from the caller's input list; `input_edge_ids` preserves their original identity for path results, including parallel edges.

For example, these input edges:

```text
(0 -> 1, 3), (0 -> 2, 7), (1 -> 2, 2), (2 -> 3, 4)
```

produce (assuming input order is retained within each vertex):

```text
edge_starts  = [0, 2, 3, 4, 4]
targets      = [1, 2, 2, 3]
weights      = [3, 7, 2, 4]
input_edge_ids = [0, 1, 2, 3]
```

Construction can count outgoing edges, compute cumulative starts, and fill contiguous edge ranges. All size calculations must be checked for integer overflow before allocating.

## 4. Supporting graphs and search structures

Preprocessing requires distances in both directions for each landmark. The implementation can construct a **temporary reverse CSR graph** with each directed edge reversed, run Dijkstra for each landmark, and release the reverse graph when finished.

The farthest-first landmark strategy in [algorithm-specification.md](algorithm-specification.md) uses a separate **temporary undirected projection** *for selection only*. Its distances must never replace the directed ALT preprocessing distances. That projection may likewise be discarded after selection.

An internal binary min-heap is a sensible starting priority queue for Dijkstra and A*. It may contain duplicate entries for a vertex, with stale entries discarded when extracted; decrease-key is not required. Heap capacity and growth must be checked for overflow and allocation failure.

## 5. Internal responsibilities

| Responsibility | Main role |
| --- | --- |
| Graph construction | Validate and convert caller's weighted edge list into immutable CSR, preserving input edge IDs. |
| Priority queue | Shared internal min-heap operations used by Dijkstra/A*. |
| Dijkstra | Calculate all reachable distances from one source on an abstract graph/CSR view. |
| Landmark selection | Validate explicit landmarks or choose random/farthest-first landmarks. |
| Preprocessing | Build and own `distance_from` / `distance_to` for every landmark. |
| Heuristic | Compute finite-only directed ALT lower bounds from stored tables. |
| Search | Run unidirectional A*, optionally recording predecessors. |
| Results | Return cost or allocated vertex/edge route; distinguish unreachable from failures. |

The graph and numerical interfaces should be shared across internal modules. This is a logical decomposition, **not** a requirement for one source file per row.

### Index contents (conceptual)

```text
alt_index                         (opaque to callers)
  graph
    vertex_count, edge_count
    edge_starts, targets, weights, input_edge_ids
  landmark_count, landmark_vertex_ids
  distance_from[landmark][vertex]
  distance_to[landmark][vertex]
```

The precise C struct layout is undecided.

### Query-local state

Each query owns its own `g` scores, priority queue, and optional predecessor arrays. A distance-only query should not allocate or populate predecessor arrays. Preprocessed index data remain read-only.

This arrangement makes concurrent independent queries architecturally possible without shared mutable search state. **Actual thread-safety claims must wait for implementation and tests.** Public reusable workspaces are deferred.

## 6. Build, lifecycle, and failure behavior

The initial public lifecycle is:

```text
caller input -> alt_index_build() -> immutable alt_index
                                   -> repeated queries
                                   -> alt_index_destroy()
```

- Build is **all-or-nothing**: on failure, release partial allocations and return no index.
- The index owns a copy of the graph data and of explicit landmark IDs; the caller retains ownership of its original buffers.
- A graph change requires construction of a new index; no in-place graph mutation API is provided.
- Query scratch allocations are released before query return; a successful path result has a separate lifetime managed by `alt_path_destroy()`.
- Allocation failure, invalid arguments, invalid graph data, invalid landmark configuration, and detected numeric errors must not be reported as `NO_PATH`.
- A blank graph (zero vertices) is rejected for v1; a nonempty graph with zero edges is valid. Zero landmarks are supported as a zero-heuristic baseline.

## 7. Numeric representation and correctness

The v1 public edge-weight and distance type is **`double`**. Target ordinary IEEE 754 binary64 behavior with a round-to-nearest environment.

- Only finite, nonnegative input edge weights are accepted (`-0.0` is treated as zero).
- Positive infinity represents an unreachable internal distance; subtracting unreachable distances is forbidden.
- Detect and report nonfinite accumulated distances or priorities, integer allocation-size overflow, and other explicitly detectable numeric failures. A positive edge weight that leaves the accumulated distance unchanged after addition also triggers `ALT_NUMERIC_ERROR`.
- Use strict relaxation comparisons, not arbitrary epsilon-based comparisons.
- No fast-math optimizations that undermine NaN/infinity or floating-point semantics.
- Floating-point arithmetic cannot guarantee strict mathematical optimality for every fractional input; the proven ALT bounds presume exact arithmetic. A future strict numerical guarantee would require additional work, not merely a fixed epsilon adjustment.

This positive-weight precision-loss check does **not** catch every rounding problem or prove mathematical optimality; it is one conservative failure rule in the v1 numerical policy.

See [api-specification.md](api-specification.md) for observable statuses and output conventions.

## 8. Performance and verification

Performance is part of the purpose, but **ALT is not always faster** than Dijkstra or ordinary A*.

Verify correctness with known directed graphs (including unreachable vertices, zero-weight cycles, parallel edges, and fractional weights), independent Dijkstra comparisons, and randomized tests. Check returned original edge IDs and costs. Measure separate landmark-selection, preprocessing, and per-query costs, plus memory used by `O(kV)` distance tables, explored vertices, and query break-even points.

A primary benchmarking question is how many repeated queries are required to amortize selection and preprocessing for a given graph and landmark count.

## 9. Source layout

```text
include/alt/alt.h          public API
src/                       internal graph, heap, Dijkstra, landmarks,
                           preprocessing, heuristic, and search
examples/                  minimal caller usage
tests/                     unit and integration/correctness tests
benchmarks/                reproducible performance comparisons
docs/                      specifications and user documentation
CMakeLists.txt              C17/CMake build
```

CMake will gain the library target when implementation files exist; test targets should be added incrementally. Only `include/alt/alt.h` is intended as a public installed header for v1.

## 10. Design boundaries to preserve

1. Implement **ALT**, not a multi-purpose graph toolkit.
2. Keep application-specific data conversion in consuming applications or higher-level libraries.
3. Hide CSR, Dijkstra, landmark tables, and search scratch details behind the small C API.
4. Keep the index immutable and reusable for many independent queries.
5. Do not add dynamic graph support, domain adapters, or bindings without a demonstrated need and an explicit scope decision.
