# ALT Shortest-Path Library

A standalone C library implementing ALT (A*, Landmarks, Triangle
Inequality) for shortest-path queries on weighted graphs.

The library is under development, with only the CSR foundation implemented so far. The shortest-path algorithm implementations remain planned work.

## Project Status

CSR graph foundation implemented and tested. Algorithms have not been implemented yet.

## Planned Capabilities

- Directed graphs with nonnegative edge weights
- Landmark selection and distance preprocessing
- Unidirectional ALT shortest-path search
- Distance-only and complete-path queries
- Compact CSR graph representation
- Automated correctness tests and performance benchmarks

## Technology

- C17
- CMake

## References

Goldberg, A. V., and Harrelson, C. (2005).
*Computing the Shortest Path: A* Search Meets Graph Theory.*
