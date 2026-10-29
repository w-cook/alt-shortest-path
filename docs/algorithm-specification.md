# ALT Shortest-Path Library — Algorithm Specification

**Project:** `alt-shortest-path`\
**Status:** Algorithm design/reference specification. Internal CSR graph, binary min-heap priority queue, and Dijkstra shortest-distance implementation are implemented and tested. Landmark selection/preprocessing and ALT queries remain planned.\
**Version:** Draft 1.0 — Updated October 29, 2026\
**Algorithm:** ALT — A*, Landmarks, Triangle Inequality\
**Companion documents:** [Architecture](architecture.md), [Public API specification](api-specification.md)

This document translates ALT into implementation-oriented, language-independent pseudocode. It is an original explanatory formulation informed by Goldberg and Harrelson's work, **not** a verbatim reproduction of the paper or a third-party implementation. It defines the algorithm, not the public C API or physical storage layout.

## 1. Scope and graph assumptions

The initial implementation supports a **finite weighted directed graph** `G = (V, E)` with:

- Unique vertex identities; a directed edge `e` has a source, target, and **finite nonnegative** weight.
- Parallel edges, self-loops, zero-weight edges, cycles, isolated vertices, and disconnected components.
- A graph that does not change between preprocessing and queries. An undirected connection can be represented by two equal-weight edges pointing in opposite directions.
- Unidirectional A* search, with **exact shortest-path guarantees in the mathematical model (exact arithmetic)**.
- Multiple point-to-point queries using the same preprocessed landmark index.

Pseudocode assumes ideal non-overflowing arithmetic. `∞` is a sentinel meaning *unreachable/not yet discovered*; `NIL` means *absent*. A real `double`-based C implementation must enforce the separate numerical contract: finite input weights, checked accumulation, explicit failure outcomes, and documented rounding limitations. Floating-point calculation does not automatically preserve the mathematical optimality proof for arbitrary fractional values.

### Abstract graph operations

```text
VERTICES(G)                  // Iterable of all vertices
EDGES(G)                     // Iterable of all edges
OUTGOING_EDGES(G, u)         // Iterable of edges leaving u
SOURCE(e), TARGET(e)         // Endpoints of e
WEIGHT(e)                    // Nonnegative cost of e
ADD_EDGE(G, u, v, w, id)     // Internal graph construction operation
```

### Algorithm Specification — Edge Identity

In this pseudocode, `id(e)` and `original_id` refer to an edge's original position in the caller's input edge list, rather than its position in an internal graph representation. The implemented CSR graph preserves this mapping in its `original_edge_ids` array, ensuring that edge identities remain consistent when edges are reordered.

### Priority queue convention

The implemented internal binary min-heap (`src/priority_queue.c`) stores entries containing `vertex`, `recorded_distance`, and `priority`. It orders entries **only by priority**, allows duplicate entries for the same vertex, and does not guarantee stable ordering for equal priorities. Its operations support insertion, minimum extraction, an empty check, and destruction. The pseudocode below represents these through `INSERT(entry, priority)`, `EXTRACT_MIN()`, and `IS_EMPTY()` rather than literal C function signatures. The implemented Dijkstra search discards an extracted stale entry when its recorded distance exceeds the best currently known distance; the planned A* search must follow the same rule. **The queue itself does not perform stale-entry detection**.

### Data in the ALT index

For every selected landmark `l` and graph vertex `v`:

```text
distance_from[l][v] = d(l, v)    // Shortest distance FROM landmark l
distance_to[l][v]   = d(v, l)    // Shortest distance TO landmark l
```

In a directed graph these can differ. Unreachable distances are `∞`. With `k` landmarks and `|V|` vertices, the distance tables have `2k|V|` entries before any undirected-graph optimization.

## 2. Dijkstra — shortest distances to all vertices

**Input:** Graph `G`, source vertex `source`.  
**Output:** An array `distance[v] = d(source, v)`, or `∞` if unreachable.

```text
function DIJKSTRA_ALL(G, source)
    for each vertex v in VERTICES(G)
        distance[v] ← ∞

    distance[source] ← 0
    Q ← empty min-priority queue
    Q.INSERT((source, recorded_distance = 0), priority = 0)

    while not Q.IS_EMPTY()
        (u, recorded_distance) ← Q.EXTRACT_MIN()

        if recorded_distance > distance[u]
            continue                       // Obsolete queue entry

        for each edge e in OUTGOING_EDGES(G, u)
            v ← TARGET(e)
            w ← WEIGHT(e)
            alternative ← distance[u] + w  // Checked in C implementation

            if alternative < distance[v]
                distance[v] ← alternative
                Q.INSERT((v, recorded_distance = alternative),
                         priority = alternative)

    return distance
```

There is no early exit at a destination: preprocessing needs distances to *all* reachable vertices. No predecessor array is required here.

**Implemented internal component:** `dijkstra_all()` in `src/dijkstra.c` accepts a valid CSR graph, a source vertex, and a caller-allocated array with at least one `double` per graph vertex. On success, unreachable vertices retain positive infinity. The function returns `ALT_OK`, `ALT_INVALID_ARGUMENT`, `ALT_OUT_OF_MEMORY`, or `ALT_NUMERIC_ERROR` as applicable, and its output distances must not be relied upon after failure. The C implementation checks each accumulated distance for nonfinite results and detects when a positive edge weight is lost through rounding. It also inserts an improved queue entry before updating the destination distance so an allocation failure does not record an unscheduled improvement. The pseudocode above intentionally omits C-specific status handling and resource cleanup.

## 3. Reverse graph

For directed ALT, shortest distances **to** each landmark is also needed. Reversing every edge allows the same Dijkstra procedure to calculate them.

```text
function REVERSE_GRAPH(G)
    R ← new graph containing all vertices of G

    for each edge e in EDGES(G)
        u ← SOURCE(e)
        v ← TARGET(e)
        R.ADD_EDGE(v, u, WEIGHT(e), original_id = id(e))

    return R
```

The key identity is:

```text
distance_in_REVERSE_GRAPH(G)(l, v) = distance_in_G(v, l)
```

The reversed representation may be temporary and released once preprocessing completes.

## 4. Landmark selection

The landmark set `L` contains **distinct** valid vertices. Any such set is correct; selection quality affects speed, not the mathematical validity of ALT.

### 4.1 Explicit selection

Use the caller's list after validating that all IDs exist in `G` and no vertex occurs twice. An empty list is permitted as a Dijkstra-equivalent baseline.

### 4.2 Random selection without replacement

**Input:** `G`, number `k`, an unbiased random integer generator `R`.  
**Output:** `k` distinct vertices. The caller/library can choose a fixed seed for reproducibility.

```text
function SELECT_RANDOM_LANDMARKS(G, k, R)
    candidates ← array containing VERTICES(G) in a stable order
    n ← length(candidates)
    require 0 ≤ k ≤ n

    for i ← 0 to k - 1
        j ← R.UNIFORM_INTEGER(i, n - 1)     // Both bounds inclusive
        SWAP(candidates[i], candidates[j])

    return candidates[0 .. k-1]
```

This is a partial Fisher–Yates shuffle. The abstract random generator's exact implementation and reproducibility contract belong to the C design.

### 4.3 Farthest-first selection

The original paper describes repeatedly choosing a vertex farthest from the existing landmarks. **For this library**, an explicit distance policy is defined for directed and disconnected graphs: selection uses a temporary **undirected projection** of the graph, not its directed distances. This is a selection-design choice, **not a prescribed requirement of ALT**.

For each directed edge `(u, v, w)`, the projection includes `(u, v, w)` and `(v, u, w)`. Parallel edges are allowed. Projection distances are used *only for landmark placement* and never substituted for directed ALT preprocessing distances.

```text
function BUILD_SELECTION_GRAPH(G)
    U ← new graph containing all vertices of G
    for each edge e in EDGES(G)
        u ← SOURCE(e)
        v ← TARGET(e)
        w ← WEIGHT(e)
        U.ADD_EDGE(u, v, w)
        U.ADD_EDGE(v, u, w)
    return U
```

**Input:** `G`, landmark count `k`, valid anchor vertex `anchor` when `k > 0`.  
**Output:** `k` distinct landmarks.

```text
function SELECT_FARTHEST_LANDMARKS(G, k, anchor)
    n ← number of vertices in G
    require 0 ≤ k ≤ n
    if k = 0
        return []
    require anchor exists in G

    U ← BUILD_SELECTION_GRAPH(G)
    anchor_distance ← DIJKSTRA_ALL(U, anchor)

    first ← anchor
    for each vertex v in VERTICES(U) in stable order
        if anchor_distance[v] ≠ ∞
           and anchor_distance[v] > anchor_distance[first]
            first ← v

    landmarks ← [first]
    for each vertex v in VERTICES(U)
        selected[v] ← false
        nearest_distance[v] ← ∞
    selected[first] ← true
    last_selected ← first

    while length(landmarks) < k
        distances ← DIJKSTRA_ALL(U, last_selected)

        for each vertex v in VERTICES(U)
            nearest_distance[v] ← MIN(nearest_distance[v], distances[v])

        next_landmark ← NIL
        for each vertex v in VERTICES(U) in stable order
            if selected[v]
                continue
            if next_landmark = NIL
               or nearest_distance[v] > nearest_distance[next_landmark]
                next_landmark ← v

        landmarks.APPEND(next_landmark)
        selected[next_landmark] ← true
        last_selected ← next_landmark

    return landmarks
```

For selection only, `∞` compares greater than any finite distance. Therefore an as-yet-uncovered connected component is favored. Stable vertex ordering resolves ties. If `k` is smaller than the number of components, not every component necessarily receives a landmark.

**Design note:** This heuristic is deliberately simple and testable; it is not necessarily the best-performing landmark placement strategy for every graph. Selection costs also contribute to total preprocessing time.

## 5. ALT preprocessing

**Input:** Graph `G`, validated distinct landmarks `L`.  
**Output:** An ALT index containing the landmark list and both distance tables.

```text
function ALT_PREPROCESS(G, L)
    require every l in L exists in G
    require no duplicate landmarks

    index ← new ALT index
    index.landmarks ← copy of L
    R ← REVERSE_GRAPH(G)

    for each landmark l in L
        index.distance_from[l] ← DIJKSTRA_ALL(G, l)
        index.distance_to[l] ← DIJKSTRA_ALL(R, l)

    discard R
    return index
```

For `k` landmarks, directed preprocessing performs `2k` Dijkstra runs (plus any Dijkstra runs needed by landmark selection). On an undirected graph, from/to distances are equal, so one run per landmark is sufficient as a possible later optimization.

## 6. ALT heuristic

For current vertex `v`, target `t`, and landmark `l`, the triangle inequality yields two lower bounds:

```text
forward bound = d(l, t) - d(l, v)
reverse bound = d(v, l) - d(t, l)
```

The heuristic is the maximum nonnegative **finite** bound across all landmarks:

```text
h(v, t) = MAX(0,
              d(l, t) - d(l, v),
              d(v, l) - d(t, l)
              for each l in L where the operands are finite)
```

**Input:** ALT index, current vertex `v`, destination `target`.  
**Output:** Nonnegative estimate `h(v, target)`.

```text
function ALT_HEURISTIC(index, v, target)
    best_estimate ← 0

    for each landmark l in index.landmarks
        a ← index.distance_from[l][target]  // d(l, target)
        b ← index.distance_from[l][v]       // d(l, v)

        if a ≠ ∞ and b ≠ ∞
            bound ← a - b
            if bound > best_estimate
                best_estimate ← bound

        c ← index.distance_to[l][v]        // d(v, l)
        d ← index.distance_to[l][target]   // d(target, l)

        if c ≠ ∞ and d ≠ ∞
            bound ← c - d
            if bound > best_estimate
                best_estimate ← bound

    return best_estimate
```

**Skip** bounds containing `∞`; never calculate `∞ - ∞`. This is conservative and preserves admissibility in the exact-arithmetic model. With an empty landmark set or no usable bounds, `h = 0`.

For vertices that can reach the target, the key property is:

```text
0 ≤ h(v, target) ≤ true_shortest_distance(v, target)
```

With this finite-only handling, do not assume the heuristic is consistent in every directed/disconnected graph. The A* implementation below allows vertices to be reopened.

## 7. ALT shortest-path query

**Input:** A valid ALT index containing an immutable graph snapshot, and start and target vertices.
**Output:** `FOUND(path, total_cost)`, `NO_PATH`, or an explicit failure status from the eventual C implementation.

**Search-state convention:** `g[v]` is the best *discovered* source-to-`v` cost. The heap priority is `g[v] + h(v,target)`, not `g[v]` alone. Each entry stores the tentative `g` that was current when inserted, enabling stale-entry detection.

```text
function ALT_SHORTEST_PATH(index, start, target)
    G ← index.graph
    require start and target are valid vertices of G

    if start = target
        return FOUND(vertices = [start], edges = [], total_cost = 0)

    for each vertex v in VERTICES(G)
        g[v] ← ∞
        predecessor[v] ← NIL
        predecessor_edge[v] ← NIL

    g[start] ← 0
    Q ← empty min-priority queue
    Q.INSERT((start, recorded_g = 0),
             priority = ALT_HEURISTIC(index, start, target))

    while not Q.IS_EMPTY()
        (u, recorded_g) ← Q.EXTRACT_MIN()

        if recorded_g > g[u]
            continue                       // Obsolete entry

        if u = target
            path ← RECONSTRUCT_PATH(predecessor, predecessor_edge,
                                    start, target)
            return FOUND(path, g[target])

        for each edge e in OUTGOING_EDGES(G, u)
            v ← TARGET(e)
            tentative_g ← g[u] + WEIGHT(e) // Checked in C implementation

            if tentative_g < g[v]
                g[v] ← tentative_g
                predecessor[v] ← u
                predecessor_edge[v] ← id(e)

                h ← ALT_HEURISTIC(index, v, target)
                priority ← tentative_g + h // Checked in C implementation
                Q.INSERT((v, recorded_g = tentative_g), priority)

    return NO_PATH
```

The destination is accepted **when extracted**, not merely when discovered. No permanent closed set is used: if a better route is discovered, a vertex can be inserted again. Strict `<` relaxations prevent gratuitous updates on zero-cost cycles.

**Distance-only query:** Run the same search without predecessor allocation/tracking; return `g[target]` at the valid target extraction. It should not be a separate algorithm.

## 8. Path reconstruction

```text
function RECONSTRUCT_PATH(predecessor, predecessor_edge, start, target)
    vertices ← empty list
    edges ← empty list
    current ← target

    while current ≠ start
        vertices.APPEND(current)
        if predecessor[current] = NIL
            return FAILURE  // Broken internal predecessor chain
        edges.APPEND(predecessor_edge[current])
        current ← predecessor[current]

    vertices.APPEND(start)
    REVERSE(vertices)
    REVERSE(edges)
    return PATH(vertices, edges)
```

For `n` returned vertices, there are `n - 1` edges. The original input edge identity must be preserved if the internal graph reorders edges (for example, when building CSR), especially for parallel edges.

`FAILURE` represents an internal path-reconstruction failure, not a public `alt_status` value. A missing predecessor before reaching the starting vertex indicates a violation of the algorithm's internal invariants. The implementation must discard any partially reconstructed path and propagate the failure without returning an invalid result. The precise mapping to a public error status will be determined when path reconstruction is implemented. Such a failure must not be reported as `ALT_NO_PATH`.

## 9. Full algorithm workflow

```text
function ALT_BUILD_INDEX(G, landmark_method, selection_options)
    VALIDATE_GRAPH(G)

    if landmark_method = EXPLICIT
        L ← VALIDATE_AND_COPY_EXPLICIT_LANDMARKS(G, selection_options)
    else if landmark_method = RANDOM
        L ← SELECT_RANDOM_LANDMARKS(G, count, random_generator)
    else if landmark_method = FARTHEST
        L ← SELECT_FARTHEST_LANDMARKS(G, count, anchor)
    else
        return INVALID_CONFIGURATION

    index ← ALT_PREPROCESS(G, L)
    return index

function ALT_QUERY(index, start, target)
    return ALT_SHORTEST_PATH(index, start, target)
```

`VALIDATE_GRAPH` and configuration validation are **public-input validation responsibilities**, not additional shortest-path algorithms: check valid endpoints, permitted finite weights, valid landmark IDs/counts, and arithmetic/allocation failures. The eventual C API will specify exact statuses and ownership.

## 10. Essential correctness and numerical requirements

- **Preprocessing correctness:** `distance_from[l][v] = d(l,v)` and `distance_to[l][v] = d(v,l)`.
- **Admissibility under exact arithmetic:** For every reachable `(v,t)`, `0 ≤ h(v,t) ≤ d(v,t)`.
- **Query optimality under exact arithmetic:** On success, `total_cost = d(start,target)`, with a connected path whose edge costs sum to that value.
- **No-path correctness:** If `t` cannot be reached from `s`, return `NO_PATH`, not a memory or arithmetic error.
- **Snapshot validity:** A preprocessed index is for the graph from which it was built. The initial API rejects/avoids changes rather than attempting incremental maintenance.
- **Floating-point caveat:** Real `double` operations can round; a slightly overestimated heuristic may violate strict admissibility. The v1 numerical contract documents this limitation. The implemented Dijkstra function reports nonfinite accumulated distances and positive-weight rounding stagnation; the planned ALT operations must also report detected numerical failures. Unconditional mathematical exactness for all fractional weights cannot be claimed.
- **Error propagation:** A failed allocation or detected unsupported numeric operation must terminate the relevant build/query cleanly, not turn into `NO_PATH` or a partially valid result.

### Edge cases to verify

| Case | Expected property |
| --- | --- |
| Source equals target | One-vertex, zero-edge, zero-cost path |
| Disconnected graph | Unreachable distances remain `∞`; queries report `NO_PATH` |
| One-way edge | Reverse traversal is not silently permitted |
| Zero-weight edges/cycles | Search terminates with correct cost |
| Parallel edges | Correct edge and its source input ID are returned |
| Multiple equal-cost paths | Any optimal route is valid |
| Empty landmark list | Zero heuristic; Dijkstra-equivalent search |
| Landmarks unreachable from/to some vertices | Unsafe infinity subtractions skipped |
| Invalid endpoints / duplicate landmarks | Explicit configuration or graph error |
| Negative / NaN / infinite weights | Invalid graph |
| Arithmetic overflow / severe precision loss | Detected where possible and reported distinctly |
| Caller-owned graph input modified or released | The index retains its independent graph snapshot, and subsequent queries remain unaffected |

The implemented CSR graph has 12 test functions covering graph construction, supported edge cases, invalid inputs, and destruction. The internal heap has 17 test functions covering priority ordering, ties, duplicate vertices, growth, invalid values, and lifecycle cases. The implemented Dijkstra algorithm has 16 test functions covering directed shortest distances, supported graph edge cases, repeated runs, heap growth, invalid arguments, fractional and large finite distances, overflow, and positive-weight rounding stagnation. All three CTest executables, containing **45 test functions in total**, passed local Debug, Release (with assertions enabled), ASan, and UBSan validation using GCC 15.2.0 on October 29, 2026. These results validate the tested internal components, **not** landmark preprocessing or ALT queries. Once implemented, ALT queries should be compared with the independent Dijkstra baseline over hand-checked and randomized directed graphs, including strongly connected and disconnected cases. Benchmark landmark selection, preprocessing, memory, query times, queue work, and the break-even query count. Performance superiority is not assumed.

## 11. Approximate algorithm costs

For `V` vertices, `E` edges, and `k` landmarks, with a binary min-heap allowing duplicate entries:

| Operation | Approximate cost |
| --- | --- |
| Reverse graph | `O(V + E)` time/space |
| One Dijkstra | `O((V + E) log(V + E + 1))` time |
| Random selection | `O(V + k)` time |
| Farthest-first selection | `O(k (V + E) log(V + E + 1))` time |
| Directed ALT preprocessing | `2k` Dijkstra runs; `O(kV)` table space |
| ALT heuristic | `O(k)` per evaluation |
| Path reconstruction | `O(path length)` |

Actual query cost depends on heuristic quality, graph structure, and any reopenings. ALT's benefit is practical search reduction across repeated queries, not an improved universal worst-case guarantee.

## 12. Deliberately outside the initial algorithm

Bidirectional ALT, ALP, query-specific landmark subsets, dynamic graph updates, incremental search repair, application-specific adapters/importers, parallel preprocessing, and game-specific traversal mechanics are excluded from this first implementation.

## References

- Andrew V. Goldberg and Chris Harrelson, **Computing the Shortest Path: A* Search Meets Graph Theory**, SODA 2005. Microsoft Research: https://www.microsoft.com/en-us/research/publication/computing-the-shortest-path-a-search-meets-graph-theory/
- Original research paper PDF: https://www.microsoft.com/en-us/research/wp-content/uploads/2005/01/soda05.pdf
