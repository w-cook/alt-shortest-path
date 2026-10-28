# ALT Shortest-Path Library

A C17 library in development for ALT (A*, Landmarks, Triangle Inequality)
shortest-path queries on weighted directed graphs. The intended public API
will support reusable landmark preprocessing, distance queries, and path
reconstruction.

**Project status (October 28, 2026):** The internal compressed sparse row
(CSR) graph foundation and binary min-heap priority queue are implemented and
tested. Dijkstra, landmark selection/preprocessing, ALT queries, and the public
index lifecycle remain planned. **The public ALT functions are declared but
not yet implemented; this is not a usable shortest-path library release.**

## Implemented Foundations

- **CSR graph (`src/graph.c`, `src/graph.h`):** validates and copies directed,
  finite, nonnegative-weight edges into a compact representation, preserving
  original input edge IDs. Supports parallel edges, self-loops, isolated
  vertices, and zero-weight edges.
- **Binary min-heap (`src/priority_queue.c`, `src/priority_queue.h`):** orders
  entries by priority, preserves vertex IDs and recorded distances, allows
  duplicate entries, and grows storage dynamically. Rejects nonfinite
  priorities and recorded distances; supports destruction and reuse.
- **Tests:** CTest runs two executables: `graph_tests` with 12 test functions
  covering CSR construction, edge cases, invalid inputs, and destruction;
  and `priority_queue_tests` with 17 test functions covering ordering,
  capacity growth, invalid arguments and numbers, and lifecycle behavior.
  Both passed local Debug, Release, AddressSanitizer, and
  UndefinedBehaviorSanitizer validation on October 28, 2026. These are 29
  test functions across two CTest-registered executables, not 29 separate
  CTest tests. This is not a claim of CI coverage or end-to-end ALT correctness.

## Planned Capabilities

- Landmark selection and directed-distance preprocessing
- Unidirectional ALT/A* search on immutable graph snapshots
- Distance-only queries and complete paths with original input edge IDs
- Correctness testing against independent Dijkstra baselines and performance
  benchmarks measuring preprocessing costs and repeated-query tradeoffs

## Build and Test the Current Foundations

With CMake and a C17 compiler installed, run from the repository root:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

These commands build the internal static library and run the existing
`graph_tests` and `priority_queue_tests` executables. They do **not** produce a
working public ALT query interface yet.

## Technology

- C17
- CMake
- CTest

## Design Documentation

- [Architecture](docs/architecture.md)
- [Algorithm specification](docs/algorithm-specification.md)
- [Public C API specification (draft)](docs/api-specification.md)

## References

Goldberg, A. V., and Harrelson, C. (2005).  
*Computing the Shortest Path: A* Search Meets Graph Theory.*
