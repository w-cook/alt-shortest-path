#include "../src/dijkstra.h"
#include "alt/alt.h"
#include <assert.h>
#include <float.h>
#include <math.h>
#include <stddef.h>

// Verify distances in a directed graph with competing routes. Exercise
// improvements to previously discovered distances and stale queue entries
void test_basic_shortest_paths() {
  // Construct a zero-initialized graph
  alt_graph graph = {0};

  // Define the edges
  alt_edge edges[] = {{0, 1, 10.0}, {0, 2, 2.0},  {2, 1, 3.0},
                      {1, 3, 1.0},  {2, 3, 10.0}, {3, 4, 2.0}};

  // Construct the CSR graph and verify success
  assert(graph_build(5, edges, 6, &graph) == ALT_OK);

  // Create distance output array
  double distances[5];

  // Run Dijkstra with vertex 0 as the source and verify success
  assert(dijkstra_all(&graph, 0, distances) == ALT_OK);

  // Verify the output distances
  assert(distances[0] == 0.0);
  assert(distances[1] == 5.0);
  assert(distances[2] == 2.0);
  assert(distances[3] == 6.0);
  assert(distances[4] == 8.0);

  // Clean up allocations
  graph_destroy(&graph);
}

// Run Dijkstra from a vertex other than zero. Verify that edges are traversed
// only in their specified direction
void test_nonzero_source_directionality() {
  // Construct a zero-initialized graph
  alt_graph graph = {0};

  // Define the edges
  alt_edge edges[] = {
      {0, 1, 2.0}, {1, 2, 3.0}, {2, 3, 4.0}, {3, 1, 1.0}, {0, 3, 20.0}};

  // Construct the CSR graph and verify success
  assert(graph_build(4, edges, 5, &graph) == ALT_OK);

  // Create distance output array
  double distances[4];

  // Run Dijkstra with vertex 2 as the source and verify success
  assert(dijkstra_all(&graph, 2, distances) == ALT_OK);

  // Verify the output distances
  assert(distances[0] == INFINITY);
  assert(distances[1] == 5.0);
  assert(distances[2] == 0.0);
  assert(distances[3] == 4.0);

  // Clean up allocations
  graph_destroy(&graph);
}

// Verify correct distances when multiple routes have the same total cost
void test_equal_cost_alternatives() {
  // Construct a zero-initialized graph
  alt_graph graph = {0};

  // Define the edges
  alt_edge edges[] = {{0, 1, 2.0}, {0, 2, 1.0}, {2, 1, 1.0},
                      {1, 3, 3.0}, {2, 3, 4.0}, {0, 3, 5.0}};

  // Construct the CSR graph and verify success
  assert(graph_build(4, edges, 6, &graph) == ALT_OK);

  // Create distance output array
  double distances[4];

  // Run Dijkstra with vertex 0 as the source and verify success
  assert(dijkstra_all(&graph, 0, distances) == ALT_OK);

  // Verify the output distances
  assert(distances[0] == 0.0);
  assert(distances[1] == 2.0);
  assert(distances[2] == 1.0);
  assert(distances[3] == 5.0);

  // Clean up allocations
  graph_destroy(&graph);
}

// Run Dijkstra multiple times against the same graph using different sources.
// Verify that results are independent and the supplied output array is fully
// reinitialized
void test_repeated_calls() {
  // Construct a zero-initialized graph
  alt_graph graph = {0};

  // Define the edges
  alt_edge edges[] = {
      {0, 1, 2.0}, {1, 2, 3.0}, {0, 3, 10.0}, {2, 3, 1.0}, {3, 1, 4.0}};

  // Construct the CSR graph and verify success
  assert(graph_build(5, edges, 5, &graph) == ALT_OK);

  // Create distance output array
  double distances[5];

  // Run Dijkstra with vertex 0 as the source and verify success
  assert(dijkstra_all(&graph, 0, distances) == ALT_OK);

  // Verify the output distances
  assert(distances[0] == 0.0);
  assert(distances[1] == 2.0);
  assert(distances[2] == 5.0);
  assert(distances[3] == 6.0);
  assert(distances[4] == INFINITY);

  // Run Dijkstra with vertex 4 as the source and verify success
  assert(dijkstra_all(&graph, 4, distances) == ALT_OK);

  // Verify the output distances
  assert(distances[0] == INFINITY);
  assert(distances[1] == INFINITY);
  assert(distances[2] == INFINITY);
  assert(distances[3] == INFINITY);
  assert(distances[4] == 0.0);

  // Run Dijkstra with vertex 3 as the source and verify success
  assert(dijkstra_all(&graph, 3, distances) == ALT_OK);

  // Verify the output distances
  assert(distances[0] == INFINITY);
  assert(distances[1] == 4.0);
  assert(distances[2] == 7.0);
  assert(distances[3] == 0.0);
  assert(distances[4] == INFINITY);

  // Run Dijkstra with vertex 0 as the source and verify success
  assert(dijkstra_all(&graph, 0, distances) == ALT_OK);

  // Verify the output distances
  assert(distances[0] == 0.0);
  assert(distances[1] == 2.0);
  assert(distances[2] == 5.0);
  assert(distances[3] == 6.0);
  assert(distances[4] == INFINITY);

  // Clean up allocations
  graph_destroy(&graph);
}

// One vertex, no edges. Its distance to itself must be zero
void test_single_vertex() {
  // Construct a zero-initialized graph
  alt_graph graph = {0};

  // Construct the CSR graph and verify success
  assert(graph_build(1, NULL, 0, &graph) == ALT_OK);

  // Create distance output array
  double distances[1];

  // Run Dijkstra with vertex 0 as the source and verify success
  assert(dijkstra_all(&graph, 0, distances) == ALT_OK);

  // Verify the output distance
  assert(distances[0] == 0.0);

  // Clean up allocations
  graph_destroy(&graph);
}

// Multiple vertices, no edges. Source distance is zero; every other distance is
// infinity
void test_no_edges() {
  // Construct a zero-initialized graph
  alt_graph graph = {0};

  // Construct the CSR graph and verify success
  assert(graph_build(5, NULL, 0, &graph) == ALT_OK);

  // Create distance output array
  double distances[5];

  // Run Dijkstra with vertex 2 as the source and verify success
  assert(dijkstra_all(&graph, 2, distances) == ALT_OK);

  // Verify the output distances
  assert(distances[0] == INFINITY);
  assert(distances[1] == INFINITY);
  assert(distances[2] == 0.0);
  assert(distances[3] == INFINITY);
  assert(distances[4] == INFINITY);

  // Clean up allocations
  graph_destroy(&graph);
}

// A graph with multiple disconnected components. Reachable distances must be
// correct, and unreachable vertices must retain infinity
void test_disconnected_components() {
  // Construct a zero-initialized graph
  alt_graph graph = {0};

  // Define the edges
  alt_edge edges[] = {
      {0, 1, 2.0}, {1, 2, 3.0}, {0, 2, 10.0}, {3, 4, 1.0}, {4, 5, 2.0}};

  // Construct the CSR graph and verify success
  assert(graph_build(7, edges, 5, &graph) == ALT_OK);

  // Create distance output array
  double distances[7];

  // Run Dijkstra with vertex 0 as the source and verify success
  assert(dijkstra_all(&graph, 0, distances) == ALT_OK);

  // Verify the output distances
  assert(distances[0] == 0.0);
  assert(distances[1] == 2.0);
  assert(distances[2] == 5.0);
  assert(distances[3] == INFINITY);
  assert(distances[4] == INFINITY);
  assert(distances[5] == INFINITY);
  assert(distances[6] == INFINITY);

  // Clean up allocations
  graph_destroy(&graph);
}

// Verify propagation through zero-weight edges and termination when zero-weight
// cycles exist
void test_zero_weight_edges_and_cycles() {
  // Construct a zero-initialized graph
  alt_graph graph = {0};

  // Define the edges
  alt_edge edges[] = {{0, 1, 0.0}, {1, 2, 0.0}, {2, 1, 0.0}, {2, 3, 0.0},
                      {0, 3, 9.0}, {3, 4, 2.0}, {1, 4, 8.0}};

  // Construct the CSR graph and verify success
  assert(graph_build(5, edges, 7, &graph) == ALT_OK);

  // Create distance output array
  double distances[5];

  // Run Dijkstra with vertex 0 as the source and verify success
  assert(dijkstra_all(&graph, 0, distances) == ALT_OK);

  // Verify the output distances
  assert(distances[0] == 0.0);
  assert(distances[1] == 0.0);
  assert(distances[2] == 0.0);
  assert(distances[3] == 0.0);
  assert(distances[4] == 2.0);

  // Clean up allocations
  graph_destroy(&graph);
}

// Verify that self-loops and positive-weight cycles do not produce incorrect
// distance improvements
void test_self_loops_and_positive_cycles() {
  // Construct a zero-initialized graph
  alt_graph graph = {0};

  // Define the edges
  alt_edge edges[] = {{0, 0, 7.0}, {0, 1, 4.0}, {1, 1, 2.0},  {1, 2, 3.0},
                      {2, 3, 1.0}, {3, 1, 1.0}, {0, 3, 20.0}, {3, 4, 2.0}};

  // Construct the CSR graph and verify success
  assert(graph_build(5, edges, 8, &graph) == ALT_OK);

  // Create distance output array
  double distances[5];

  // Run Dijkstra with vertex 0 as the source and verify success
  assert(dijkstra_all(&graph, 0, distances) == ALT_OK);

  // Verify the output distances
  assert(distances[0] == 0.0);
  assert(distances[1] == 4.0);
  assert(distances[2] == 7.0);
  assert(distances[3] == 8.0);
  assert(distances[4] == 10.0);

  // Clean up allocations
  graph_destroy(&graph);
}

// Multiple edges between the same vertices with different weights. Verify that
// the least-cost route is selected
void test_parallel_edges() {
  // Construct a zero-initialized graph
  alt_graph graph = {0};

  // Define the edges
  alt_edge edges[] = {{0, 1, 9.0},  {0, 1, 2.0}, {1, 2, 3.0},
                      {0, 2, 10.0}, {2, 3, 1.0}, {1, 3, 10.0}};

  // Construct the CSR graph and verify success
  assert(graph_build(4, edges, 6, &graph) == ALT_OK);

  // Create distance output array
  double distances[4];

  // Run Dijkstra with vertex 0 as the source and verify success
  assert(dijkstra_all(&graph, 0, distances) == ALT_OK);

  // Verify the output distances
  assert(distances[0] == 0.0);
  assert(distances[1] == 2.0);
  assert(distances[2] == 5.0);
  assert(distances[3] == 6.0);

  // Clean up allocations
  graph_destroy(&graph);
}

// Use a graph whose source has at least 12 outgoing edges, forcing the priority
// queue to expand beyond its initial capacity of eight
void test_queue_capacity_growth() {
  // Construct a zero-initialized graph
  alt_graph graph = {0};

  // Define the edges
  alt_edge edges[] = {{0, 1, 1.0}, {0, 2, 2.0},   {0, 3, 3.0},   {0, 4, 4.0},
                      {0, 5, 5.0}, {0, 6, 6.0},   {0, 7, 7.0},   {0, 8, 8.0},
                      {0, 9, 9.0}, {0, 10, 10.0}, {0, 11, 11.0}, {0, 12, 12.0}};

  // Construct the CSR graph and verify success
  assert(graph_build(13, edges, 12, &graph) == ALT_OK);

  // Create distance output array
  double distances[13];

  // Run Dijkstra with vertex 0 as the source and verify success
  assert(dijkstra_all(&graph, 0, distances) == ALT_OK);

  // Verify the output distances
  for (size_t i = 0; i < 13; i++) {
    assert(distances[i] == i * 1.0);
  }

  // Clean up allocations
  graph_destroy(&graph);
}

// Verify ALT_INVALID_ARGUMENT for a null graph pointer, null output pointer,
// source equal to vertex_count, and source greater than vertex_count
void test_invalid_arguments() {
  // Construct a zero-initialized graph
  alt_graph graph = {0};

  // Construct the CSR graph and verify success
  assert(graph_build(3, NULL, 0, &graph) == ALT_OK);

  // Create distance output array
  double distances[] = {11.0, 22.0, 33.0};

  // Run Dijkstra with vertex 0 as the source, a null graph pointer, and a valid
  // output pointer and verify failure
  assert(dijkstra_all(NULL, 0, distances) == ALT_INVALID_ARGUMENT);

  // Verify that output remains unchanged
  assert(distances[0] == 11.0 && distances[1] == 22.0 && distances[2] == 33.0);

  // Run Dijkstra with vertex 0 as the source, a valid graph pointer, and a null
  // output pointer and verify failure
  assert(dijkstra_all(&graph, 0, NULL) == ALT_INVALID_ARGUMENT);

  // Run Dijkstra with vertex 3 as the source, a valid graph pointer, and a
  // valid output pointer and verify failure
  assert(dijkstra_all(&graph, 3, distances) == ALT_INVALID_ARGUMENT);

  // Verify that output remains unchanged
  assert(distances[0] == 11.0 && distances[1] == 22.0 && distances[2] == 33.0);

  // Run Dijkstra with vertex 5 as the source, a valid graph pointer, and a
  // valid output pointer and verify failure
  assert(dijkstra_all(&graph, 5, distances) == ALT_INVALID_ARGUMENT);

  // Verify that output remains unchanged
  assert(distances[0] == 11.0 && distances[1] == 22.0 && distances[2] == 33.0);

  // Clean up allocations
  graph_destroy(&graph);
}

// Verify shortest paths using exactly representable fractional weights, such as
// 0.125, 0.25, and 0.5
void test_fractional_weights() {
  // Construct a zero-initialized graph
  alt_graph graph = {0};

  // Define the edges
  alt_edge edges[] = {{0, 1, 0.5},   {0, 2, 0.125}, {2, 1, 0.25},
                      {1, 3, 0.125}, {2, 3, 0.5},   {3, 4, 0.25}};

  // Construct the CSR graph and verify success
  assert(graph_build(5, edges, 6, &graph) == ALT_OK);

  // Create distance output array
  double distances[5];

  // Run Dijkstra with vertex 0 as the source and verify success
  assert(dijkstra_all(&graph, 0, distances) == ALT_OK);

  // Verify the output distances
  assert(distances[0] == 0.0);
  assert(distances[1] == 0.375);
  assert(distances[2] == 0.125);
  assert(distances[3] == 0.5);
  assert(distances[4] == 0.75);

  // Clean up allocations
  graph_destroy(&graph);
}

// Verify that very large, representable distances are calculated successfully
// without being mistaken for infinity
void test_large_finite_distances() {
  // Construct a zero-initialized graph
  alt_graph graph = {0};

  // Define a test value that is an extremely large finite number, with room for
  // several additions without overflowing
  double large_val = DBL_MAX / 16.0;

  // Define the edges
  alt_edge edges[] = {{0, 1, large_val},
                      {1, 2, large_val},
                      {0, 2, large_val * 4},
                      {2, 3, large_val * 2},
                      {0, 3, large_val * 8}};

  // Construct the CSR graph and verify success
  assert(graph_build(4, edges, 5, &graph) == ALT_OK);

  // Create distance output array
  double distances[4];

  // Run Dijkstra with vertex 0 as the source and verify success
  assert(dijkstra_all(&graph, 0, distances) == ALT_OK);

  // Verify the output distances
  assert(distances[0] == 0.0);
  assert(distances[1] == large_val);
  assert(distances[2] == large_val * 2);
  assert(distances[3] == large_val * 4);

  // Clean up allocations
  graph_destroy(&graph);
}

// Construct a valid graph whose reachable path accumulation exceeds the finite
// range of double. Expect ALT_NUMERIC_ERROR
void test_distance_overflow() {
  // Construct a zero-initialized graph
  alt_graph graph = {0};

  // Define the edges
  alt_edge edges[] = {{0, 1, DBL_MAX}, {1, 2, DBL_MAX}};

  // Construct the CSR graph and verify success
  assert(graph_build(3, edges, 2, &graph) == ALT_OK);

  // Create distance output array
  double distances[3];

  // Run Dijkstra with vertex 0 as the source and verify failure
  assert(dijkstra_all(&graph, 0, distances) == ALT_NUMERIC_ERROR);

  // Clean up allocations
  graph_destroy(&graph);
}

// Verify that adding a strictly positive weight that leaves the accumulated
// distance unchanged due to rounding produces ALT_NUMERIC_ERROR
void test_positive_weight_rounding_stagnation() {
  // Construct a zero-initialized graph
  alt_graph graph = {0};

  // Define the edges
  alt_edge edges[] = {{0, 1, 9007199254740992.0}, {1, 2, 1.0}};

  // Construct the CSR graph and verify success
  assert(graph_build(3, edges, 2, &graph) == ALT_OK);

  // Create distance output array
  double distances[3];

  // Run Dijkstra with vertex 0 as the source and verify failure
  assert(dijkstra_all(&graph, 0, distances) == ALT_NUMERIC_ERROR);

  // Clean up allocations
  graph_destroy(&graph);
}

// Verify that Dijkstra on the reversed graph computes distances from each
// vertex to the original source, including asymmetric and unreachable paths
void test_reverse_graph_distances() {
  // Construct a zero-initialized graph
  alt_graph graph = {0};

  // Define the edges
  alt_edge edges[] = {{3, 4, 1.0}, {0, 1, 2.0},  {2, 3, 4.0},
                      {1, 2, 3.0}, {0, 3, 10.0}, {4, 3, 2.0}};

  // Construct the CSR graph and verify success
  assert(graph_build(5, edges, 6, &graph) == ALT_OK);

  // Construct another zero-initialized graph for reversal
  alt_graph graph_reverse = {0};

  // Construct the reversed CSR graph and verify success
  assert(graph_build_reverse(&graph, &graph_reverse) == ALT_OK);

  // Create distance output arrays
  double original_distances[5];
  double reverse_distances[5];

  // Run Dijkstra on both graphs with vertex 2 as the source and verify success
  assert(dijkstra_all(&graph, 2, original_distances) == ALT_OK);
  assert(dijkstra_all(&graph_reverse, 2, reverse_distances) == ALT_OK);

  // Verify the resulting distance arrays
  const double expected_original_distances[] = {INFINITY, INFINITY, 0.0, 4.0,
                                                5.0};
  const double expected_reverse_distances[] = {5.0, 3.0, 0.0, INFINITY,
                                               INFINITY};
  for (size_t i = 0; i < 5; i++) {
    assert(original_distances[i] == expected_original_distances[i]);
    assert(reverse_distances[i] == expected_reverse_distances[i]);
  }

  // Clean up allocations
  graph_destroy(&graph);
  graph_destroy(&graph_reverse);
}

// Executes tests
int main() {
  // Execute tests
  test_basic_shortest_paths();
  test_nonzero_source_directionality();
  test_equal_cost_alternatives();
  test_repeated_calls();
  test_single_vertex();
  test_no_edges();
  test_disconnected_components();
  test_zero_weight_edges_and_cycles();
  test_self_loops_and_positive_cycles();
  test_parallel_edges();
  test_queue_capacity_growth();
  test_invalid_arguments();
  test_fractional_weights();
  test_large_finite_distances();
  test_distance_overflow();
  test_positive_weight_rounding_stagnation();
  test_reverse_graph_distances();

  // All tests passed
  return 0;
}
