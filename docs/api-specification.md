# ALT Shortest-Path Library — Public C API Specification

**Project:** `alt-shortest-path`\
**Status:** Draft public API contract. The public header is present at `include/alt/alt.h`; internal CSR graph and reverse CSR construction, priority queue, Dijkstra shortest-distance function, landmark selection, directed landmark-distance preprocessing, and directed ALT heuristic evaluation are implemented and tested. `alt_index_build()` and `alt_index_destroy()` are implemented and tested; `alt_distance()`, `alt_shortest_path()`, and `alt_path_destroy()` remain declared but unimplemented.\
**Version:** Draft 1.0 — Updated November 17, 2026\
**Companion documents:** [Architecture](architecture.md), [Algorithm specification](algorithm-specification.md)

## 1. Interface goals

Offer a small, standalone **C17** interface for: (1) creating a preprocessed ALT index from a weighted directed graph, (2) querying a shortest-path **distance**, (3) querying a shortest **path**, and (4) destroying allocated results. The caller must not need to interact with internal graph storage, landmark-distance tables, Dijkstra, or a priority queue.

The header declares five public functions. **Index construction and destruction are implemented** in `src/alt_index.c`; the two shortest-path query operations and path-result destructor are not yet implemented. The CSR graph and its reverse constructor, binary min-heap, `dijkstra_all()`, `landmark_select()`, `landmark_preprocess()`, and `alt_heuristic_evaluate()` remain **internal** components, not separate callable public ALT operations. Dijkstra currently computes source-to-all-vertices distances for a caller-allocated array; it does not reconstruct routes or provide the planned public distance-query interface. The internal preprocessing module produces directed distance tables, which the public index builder now combines with its owned graph snapshot and selected landmark array. The internal heuristic evaluates finite-only directed lower bounds from those tables; ALT/A* query integration remains planned.

## 2. Public header (draft API)

Path: `include/alt/alt.h`.

```c
#ifndef ALT_H
#define ALT_H

#include <stddef.h>
#include <stdint.h>

typedef struct alt_index alt_index;

typedef size_t alt_vertex_id;
typedef size_t alt_edge_id;

typedef struct {
    alt_vertex_id source;
    alt_vertex_id target;
    double weight;
} alt_edge;

typedef enum {
    ALT_LANDMARK_EXPLICIT,
    ALT_LANDMARK_RANDOM,
    ALT_LANDMARK_FARTHEST
} alt_landmark_method;

typedef struct {
    alt_landmark_method method;
    size_t count;
    const alt_vertex_id *vertices;
    uint64_t seed;
    alt_vertex_id anchor;
} alt_landmark_options;

typedef enum {
    ALT_OK,
    ALT_NO_PATH,
    ALT_INVALID_ARGUMENT,
    ALT_INVALID_GRAPH,
    ALT_INVALID_CONFIGURATION,
    ALT_OUT_OF_MEMORY,
    ALT_NUMERIC_ERROR
} alt_status;

typedef struct {
    alt_vertex_id *vertices;
    alt_edge_id *edges;
    size_t vertex_count;
    size_t edge_count;
    double total_cost;
} alt_path;

alt_status alt_index_build(
    size_t vertex_count,
    const alt_edge *edges,
    size_t edge_count,
    const alt_landmark_options *options,
    alt_index **out_index
);

void alt_index_destroy(alt_index *index);

alt_status alt_distance(
    const alt_index *index,
    alt_vertex_id start,
    alt_vertex_id target,
    double *out_distance
);

alt_status alt_shortest_path(
    const alt_index *index,
    alt_vertex_id start,
    alt_vertex_id target,
    alt_path **out_path
);

void alt_path_destroy(alt_path *path);

#endif
```

## 3. Graph input and identity

- `vertex_count` specifies vertices with IDs in `[0, vertex_count)`, including vertices with no edges.
- Every input `alt_edge` supplies a `source`, `target`, and finite nonnegative `weight`.
- Parallel edges, self-loops, zero-weight edges, cycles, and disconnected graphs are accepted.
- Undirected edges can be supplied as a pair of oppositely directed edges of equal cost.
- Input edge positions `0 .. edge_count-1` are **public edge IDs** in returned paths; internal CSR positions may be different.
- For v1, `vertex_count == 0` is rejected as `ALT_INVALID_GRAPH`. When `edge_count == 0`, `edges == NULL` is valid if `vertex_count > 0`.
- For `edge_count > 0`, `edges` must address at least that many readable `alt_edge` values; out-of-range endpoints or invalid weights return `ALT_INVALID_GRAPH`.
- The library copies the necessary edge-list data while building its immutable graph. After the build call returns, the application may reuse or free the original edge array.

No string/object mapping, coordinate handling, custom traversal callbacks, mutation API, or domain-specific adapter is included. Consumers handle their own mapping to/from numeric vertex IDs.

## 4. Landmark configuration

The caller supplies an `alt_landmark_options` configuration. Unused fields are ignored according to the selected method.

| Method | Effective fields | Contract |
| --- | --- | --- |
| `ALT_LANDMARK_EXPLICIT` | `count`, `vertices` | IDs exist, are distinct, and are copied during construction. |
| `ALT_LANDMARK_RANDOM` | `count`, `seed` | Select `count` distinct vertices using a library-local pseudorandom generator; do not mutate C's global `rand()` state. |
| `ALT_LANDMARK_FARTHEST` | `count`, `anchor` | Use the selection algorithm specified in `algorithm-specification.md`, including its undirected projection and deterministic tie-breaking. |

`count` must not exceed `vertex_count`. For any method, `count == 0` is allowed and produces a zero heuristic (a Dijkstra-equivalent baseline). For explicit selection with nonzero `count`, `vertices` must not be `NULL`; with zero count it may be `NULL`. For farthest-first with nonzero `count`, `anchor` must be a valid vertex; with zero count it is ignored. The implemented public index builder supports a zero-landmark configuration without allocating a landmark array or distance tables, while preserving the owned graph and the table structure's vertex count. The internal `alt_heuristic_evaluate()` returns zero for zero landmarks; public zero-heuristic query behavior remains planned.

Random selection uses a library-local SplitMix64 generator, unbiased bounded sampling, and partial Fisher–Yates selection without replacement. Identical seeds and graph vertex ordering produce the same result for the same library implementation; the sequence is not guaranteed across incompatible future versions.

Unrecognized selection-method values, duplicate landmarks, invalid landmark IDs/counts, and invalid anchors return `ALT_INVALID_CONFIGURATION`.

## 5. Index construction and destruction

### `alt_index_build(...)`

**Implemented:** Validates inputs, constructs independently owned internal CSR (including original edge ID mapping), selects landmarks, runs directed landmark preprocessing, and returns a complete **preprocessed index**. The index contains the graph snapshot, selected landmark IDs in selection order, and two contiguous row-major directed distance tables. The private representation is defined in `src/alt_index.h`; the public header keeps `alt_index` opaque. Index construction is usable now, but public shortest-path query functions are still unimplemented, so it is not yet a usable end-to-end query API.

**Success:** Returns `ALT_OK` and stores a newly allocated non-`NULL` `alt_index*` in `*out_index`. The caller supplies an **address of a pointer variable** as `out_index`; that variable may initially hold `NULL`. The builder allocates the index itself, and ownership transfers to the caller only on success.

**Failure:** Returns the appropriate `alt_status`, releases any graph, landmark, table, and enclosing-index allocations acquired during construction, and sets `*out_index = NULL` when `out_index` is valid. Passing `out_index == NULL` is `ALT_INVALID_ARGUMENT` because there is no pointer destination; passing `options == NULL` is also `ALT_INVALID_ARGUMENT`. Graph, landmark-configuration, allocation, and detected numeric failures propagate from the internal components.

The index owns independent graph, selected-landmark, and preprocessing data. It is immutable after construction; modifying or releasing the caller's original edge or explicit-landmark buffers does not affect it. The selected landmark order is preserved, and its positions index the distance-table rows. There is no update or rebuild-in-place operation: create a new index to represent a modified graph.

### `alt_index_destroy(index)`

**Implemented:** Releases the owned CSR graph arrays, selected landmark array, both directed distance arrays, and the enclosing index object. Calling it with `NULL` is permitted and has no effect. The caller must destroy each successfully constructed index **exactly once**. After destruction, its pointer is dangling and must not be dereferenced or passed to the destructor again; the library cannot detect use-after-free. Multiple independently built indexes own separate allocations and can be destroyed separately.

## 6. Distance query

### `alt_distance(index, start, target, out_distance)`

Computes the minimum source-to-target cost without allocating/returning a path or tracking predecessor arrays.

- `ALT_OK`: Writes a finite, calculated distance to `*out_distance`.
- `ALT_NO_PATH`: Writes positive infinity to `*out_distance`.
- Other failures: The output is **not a valid result** and must not be read as a distance; callers must check the returned status.
- `start == target`: Returns `ALT_OK` with distance `0`.
- Invalid source/target, null index, or null output pointer: `ALT_INVALID_ARGUMENT`.

The implementation should initialize a valid output pointer to a documented failure value at entry (for example, `NAN`), before significant work, then assign a valid distance or infinity only for `ALT_OK`/`ALT_NO_PATH`. This avoids exposing partially computed distances as successes.

## 7. Path query and result ownership

### `alt_shortest_path(index, start, target, out_path)`

Performs the same underlying ALT search while recording the predecessor vertex and input edge identifier needed for reconstruction.

- `ALT_OK`: Allocates and returns an `alt_path*` in `*out_path`.
- `ALT_NO_PATH`: Sets `*out_path = NULL`.
- Other failures: Sets `*out_path = NULL` (when the output pointer is valid); no partial path escapes.
- `start == target`: Returns one vertex (`start`), zero edges, and total cost `0`.
- A path with `n` vertices has `n - 1` edges, in forward traversal order.
- Edge IDs refer to **caller input edge positions**, not CSR array positions; this distinguishes parallel edges.
- All reported edges must be directed connections between successive returned vertices; reported total cost must match the search's accumulated cost subject to the numeric contract.

The library allocates the `alt_path` object and its arrays. The caller may read their contents but must not replace/free their pointer members. The caller eventually calls `alt_path_destroy()` exactly once for each successful result. Returned arrays are independent of the index's internal buffers.

### `alt_path_destroy(path)`

Releases the path structure and its owned arrays. Calling it with `NULL` is permitted and has no effect. A path result remains valid independently of the index that produced it (until the path itself is destroyed).

## 8. Status codes

| Status | Meaning | Typical origin |
| --- | --- | --- |
| `ALT_OK` | Operation succeeded. | Build or query. |
| `ALT_NO_PATH` | A valid query found no route. | Query only; **not** an error. |
| `ALT_INVALID_ARGUMENT` | Null required pointer or invalid query vertex ID. | Any applicable operation. |
| `ALT_INVALID_GRAPH` | Invalid graph size, endpoint, or weight. | Build. |
| `ALT_INVALID_CONFIGURATION` | Invalid landmark settings. | Build. |
| `ALT_OUT_OF_MEMORY` | A required allocation failed, or an allocation size exceeds representable limits. | Build or query. |
| `ALT_NUMERIC_ERROR` | Detected unsupported/overflowing numerical operation. | Build or query. |

A `NO_PATH` result must never conceal allocation failure or detected overflow. Errors are returned as statuses, not via global mutable error strings, callbacks, or exceptions.

## 9. Numeric contract

- Public edge weights and shortest-path costs use `double`.
- v1 targets conventional IEEE 754 binary64 `double`, standard round-to-nearest evaluation, and no fast-math.
- Reject negative, NaN, and infinite **input weights**. Treat negative zero as zero.
- Positive infinity is an *internal unreachable-distance sentinel*, never a permissible input edge cost.
- When computing Dijkstra/ALT distances or A* priorities, guard against nonfinite results and report detected numeric failures as `ALT_NUMERIC_ERROR`.
- The implemented internal heuristic skips each bound independently when either stored operand is nonfinite; it never subtracts `∞ - ∞`, and returns a nonnegative estimate (zero when no bound applies).
- Use strict `<` comparisons for cost improvement; do not use an arbitrary epsilon for relaxation.
- Under exact arithmetic, ALT is optimal. With fractional floating-point inputs, rounding can slightly violate lower-bound admissibility and potentially affect optimality. **The library must not promise absolute mathematical optimality for every fractional input.**
- For integer-valued inputs whose all relevant intermediate sums and differences are exactly representable in binary64 (notably within the integer-exact range through `2^53`), testing can establish exact arithmetic behavior within the stated constraints.
- If adding a strictly positive edge weight leaves the accumulated distance unchanged due to rounding, return `ALT_NUMERIC_ERROR`. This detects one loss-of-precision case but does not establish numerical exactness more generally.

The implementation must check array-size multiplications and conversions separately from floating-point cost arithmetic. Query behavior for overflow must be consistent and documented.

## 10. Concurrency and performance boundaries

Query state (priority queue, costs, and optional predecessors) is private to each call; the shared index remains immutable. This permits a future thread-safety guarantee, but such a guarantee should **not** be advertised before concurrent testing and an implementation audit.

Separate distance and path queries should share the same internal search implementation. A distance-only query should avoid predecessor allocation. A reusable query workspace, advanced allocation hooks, query statistics, and other performance-facing API features are **not** part of v1.

## 11. Validation checklist before public release

- Build a nonempty directed graph and query valid vertex pairs.
- Handle no-edge graphs, no-path results, zero-weight cycles, self-loops, and parallel edges.
- Preserve original input edge IDs after CSR construction.
- Verify explicit/random/farthest landmark selection, directed preprocessing, zero-landmark construction, and index ownership through the **implemented public builder**; verify query behavior separately once the query functions exist.
- Reject invalid weights, endpoints, configurations, and required null pointers.
- Verify index-build failure clears the output, returns a specific error status, and cleans up partial allocations; the implemented index destructor accepts `NULL`. Verify the future path-result destructor separately once implemented.
- Verify `alt_distance()` and `alt_shortest_path()` agree on returned costs.
- Compare ALT results against independent Dijkstra on test graphs and randomized directed graphs.
- Test numeric edge cases and document double-precision limitations.
- Record preprocessing cost, memory overhead, query time, and when ALT amortizes its setup cost.

**Index-lifecycle validation status (November 17, 2026):** Fourteen `alt_index_tests` functions cover successful construction with all selection strategies, zero landmarks, zero-edge graphs, independent input and index ownership, argument/configuration rejection, numerical and partial-construction failures, and `NULL` destruction. Together with 118 internal-component tests, **132 test functions across seven CTest executables** passed local Debug, Release (assertions enabled), AddressSanitizer with leak detection, and UndefinedBehaviorSanitizer checks (28/28 CTest executions across four configurations). These results do not establish the correctness or availability of the unimplemented query operations.

**Scope reminder:** Only the five public operations above belong to v1 unless implementation evidence motivates a deliberate API revision. This is an ALT library, not a general-purpose graph toolkit.
