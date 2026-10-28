# ALT Shortest-Path Library — Architecture

**Project:** `alt-shortest-path`\
**Status:** Architecture specification; internal CSR graph and binary min-heap priority queue implemented and tested. Dijkstra, landmark preprocessing, ALT queries, and the public index lifecycle remain planned.\
**Version:** Draft 1.0 — Updated October 28, 2026\
**Companion documents:** [Algorithm specification](algorithm-specification.md), [Public API specification](api-specification.md)

## 1. Purpose and boundaries

Provide a small, reusable **C17 library implementing ALT (A*, Landmarks, Triangle Inequality)** for repeated shortest-path queries on nonnegatively weighted directed graphs. The library will implement the algorithm, not a general graph-processing toolkit or a domain-specific navigation system.

The library must be usable by another library as a low-level component. It must not assume what vertices or edges represent, require coordinates, or incorporate game, road-network, or other application-specific rules.

**In scope:** graph ingestion, immutable routing representation, landmark selection/preprocessing, ALT queries, optional route reconstruction, resource ownership, error handling, testing, and performance measurement.

**Out of scope for v1:** mutable graph editing, incremental preprocessing, dynamic obstacle handling, custom application adapters, grid or navigation-mesh models, file-format importers, bidirectional ALT, ALP, and a general-purpose graph API.

## 2. System overview (planned)

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

The internal CSR representation uses **`edge_starts`** for what CSR literature calls *row offsets*:

```text
edge_starts[v]       first CSR edge position for vertex v (inclusive)
edge_starts[v + 1]   end of that range (exclusive)

targets[i]          target vertex of CSR edge at position i
weights[i]          weight of CSR edge at position i
original_edge_ids[i]   original position of this edge in caller's edge list
```

CSR invariants:

- `edge_starts` has `vertex_count + 1` entries; the other three arrays each have `edge_count` entries.
- `edge_starts[0] == 0`, and `edge_starts[vertex_count] == edge_count`.
- `edge_starts[v] <= edge_starts[v + 1]` for every vertex.
- The outgoing edge positions of `v` are `[edge_starts[v], edge_starts[v + 1])`.
- If `edge_starts[v] == edge_starts[v + 1]`, then `v` has no outgoing edges.
- The CSR layout may reorder edges from the caller's input list; `original_edge_ids` preserves their original identity for path results, including parallel edges.

For example, these input edges:

```text
(0 -> 1, 3), (0 -> 2, 7), (1 -> 2, 2), (2 -> 3, 4)
```

produce (assuming input order is retained within each vertex):

```text
edge_starts  = [0, 2, 3, 4, 4]
targets      = [1, 2, 2, 3]
weights      = [3, 7, 2, 4]
original_edge_ids = [0, 1, 2, 3]
```

Construction counts outgoing edges, computes cumulative starts, and fills contiguous edge ranges. All size calculations are checked for integer overflow before allocating.

## 4. Supporting graphs and search structures

Preprocessing requires distances in both directions for each landmark. The implementation can construct a **temporary reverse CSR graph** with each directed edge reversed, run Dijkstra for each landmark, and release the reverse graph when finished.

The farthest-first landmark strategy in [algorithm-specification.md](algorithm-specification.md) uses a separate **temporary undirected projection** *for selection only*. Its distances must never replace the directed ALT preprocessing distances. That projection may likewise be discarded after selection.

An internal **binary min-heap** is implemented in `src/priority_queue.c` and `src/priority_queue.h` for eventual Dijkstra and A* use. Each entry stores a `vertex`, `recorded_distance`, and `priority`; heap ordering uses **only `priority`**. Duplicate entries for a vertex are allowed, equal-priority extraction order is unspecified, and no decrease-key operation is provided. The future search algorithms—not the heap—must identify and discard stale entries after extraction. Insertion grows storage as needed with allocation-size overflow checks, reports allocation failure, and rejects nonfinite priorities or recorded distances.

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
  alt_graph
    vertex_count, edge_count
    edge_starts, targets, weights, original_edge_ids
  landmark_count, landmark_vertex_ids
  distance_from[landmark][vertex]
  distance_to[landmark][vertex]
```

`alt_graph` and the separate internal priority queue are implemented. The opaque index layout remains undecided.

### Query-local state

Each query owns its own `g` scores, priority queue, and optional predecessor arrays. A distance-only query should not allocate or populate predecessor arrays. Preprocessed index data remain read-only.

This arrangement makes concurrent independent queries architecturally possible without shared mutable search state. **Actual thread-safety claims must wait for implementation and tests.** Public reusable workspaces are deferred.

## 6. Build, lifecycle, and failure behavior

The planned public lifecycle is:

```text
caller input -> alt_index_build() -> immutable alt_index
                                   -> repeated queries
                                   -> alt_index_destroy()
```

The internal CSR graph lifecycle is already implemented through `graph_build()` and `graph_destroy()`. Construction requires a zero-initialized destination that does not already own allocated memory. On success, the destination takes ownership of the constructed graph's arrays; on failure, the destination remains unchanged and temporary allocations are released. `graph_destroy()` releases owned memory and resets the structure to its zero-initialized state.

The internal priority queue supports zero initialization and allocation on insertion. `priority_queue_push()` reports invalid pointers, nonfinite numeric entries, and allocation failures through `alt_status`; rejected insertions preserve the existing queue state. `priority_queue_pop()` returns `false` for an empty queue or invalid pointers without changing the output entry. `priority_queue_destroy()` releases allocated entries, resets the queue to zero-initialized state, tolerates repeated calls, and allows the same queue structure to be reused. These are **internal** operations, not public ALT API functions.

The public `alt_index_build()` and `alt_index_destroy()` lifecycle remains planned and will incorporate the internal graph and priority queue as appropriate.

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

The internal CSR graph has **12 test functions** covering construction, supported edge cases, invalid inputs, and destruction. The priority queue has **17 test functions** covering heap ordering, duplicate vertex entries, allocation growth, invalid arguments and nonfinite numbers, and lifecycle. Together, these are 29 test functions in two CTest-registered executables. Both executables passed local Debug, Release (with test assertions enabled), AddressSanitizer, and UndefinedBehaviorSanitizer validation on October 28, 2026. These results cover the implemented foundations only; they are not ALT algorithm validation.

Once the algorithms are implemented, verify correctness with known directed graphs (including unreachable vertices, zero-weight cycles, parallel edges, and fractional weights), independent Dijkstra comparisons, and randomized tests. Check returned original edge IDs and costs. Measure separate landmark-selection, preprocessing, and per-query costs, plus memory used by `O(kV)` distance tables, explored vertices, and query break-even points.

A primary benchmarking question is how many repeated queries are required to amortize selection and preprocessing for a given graph and landmark count.

## 9. Source layout

Current repository layout:

```text
include/alt/alt.h               draft public API declarations (not yet implemented)
src/graph.c, src/graph.h         implemented internal CSR graph
src/priority_queue.c, .h        implemented internal binary min-heap
tests/graph_tests.c             12 CSR graph test functions
tests/priority_queue_tests.c    17 priority queue test functions
docs/                          architecture, algorithm, and API specifications
CMakeLists.txt                   C17/CMake static library and CTest targets
```

Dijkstra, landmark preprocessing, the heuristic, ALT search, public API
implementations, examples, and benchmarks remain planned; they are **not**
existing modules or directories in this snapshot. CMake currently builds the
`alt_shortest_path` static library and registers both `graph_tests` and
`priority_queue_tests` with CTest. Only `include/alt/alt.h` is intended as a
public header for v1; no public API implementation or installable release is
claimed yet.

## 10. Design boundaries to preserve

1. Implement **ALT**, not a multi-purpose graph toolkit.
2. Keep application-specific data conversion in consuming applications or higher-level libraries.
3. Hide CSR, Dijkstra, landmark tables, and search scratch details behind the small C API.
4. Keep the index immutable and reusable for many independent queries.
5. Do not add dynamic graph support, domain adapters, or bindings without a demonstrated need and an explicit scope decision.
