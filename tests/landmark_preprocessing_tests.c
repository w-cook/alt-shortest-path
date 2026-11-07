#include "../src/landmark_preprocessing.h"
#include "alt/alt.h"
#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdlib.h>

/* ==========================================================================
 * Distance calculation and graph behavior tests
 * ========================================================================== */

// Verify that for multiple landmarks on a directed graph, all entries match the
// expected distances. Catch landmark-position versus vertex-ID mistakes
void test_directed_landmark_distances() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Create the edge array
  alt_edge edges[] = {
      {0, 1, 2.0}, {1, 2, 3.0}, {2, 3, 4.0}, {3, 1, 1.0}, {4, 0, 5.0}};

  // Construct the five-vertex graph and declare its landmarks
  assert(graph_build(5, edges, 5, &graph) == ALT_OK);
  alt_vertex_id landmarks[] = {4, 1};

  // Preprocess landmarks and ensure success
  alt_landmark_distances distances = {0};
  assert(landmark_preprocess(&graph, landmarks, 2, &distances) == ALT_OK);

  // Verify distances result
  const double expected_distance_from[][5] = {
      {5.0, 7.0, 10.0, 14.0, 0.0}, {INFINITY, 0.0, 3.0, 7.0, INFINITY}};
  const double expected_distance_to[][5] = {
      {INFINITY, INFINITY, INFINITY, INFINITY, 0.0}, {2.0, 0.0, 5.0, 1.0, 7.0}};

  assert(distances.landmark_count == 2);
  assert(distances.vertex_count == 5);
  assert(distances.distance_from != NULL);
  assert(distances.distance_to != NULL);

  for (size_t row_i = 0; row_i < distances.landmark_count; row_i++) {
    for (size_t col_i = 0; col_i < distances.vertex_count; col_i++) {
      assert(distances.distance_from[row_i * distances.vertex_count + col_i] ==
             expected_distance_from[row_i][col_i]);
      assert(distances.distance_to[row_i * distances.vertex_count + col_i] ==
             expected_distance_to[row_i][col_i]);
    }
  }

  // Cleanup test allocations
  landmark_distances_destroy(&distances);
  graph_destroy(&graph);
}

// Verify correct dimensions and forward/reverse distances with exactly one
// landmark, using a nonzero vertex ID
void test_single_landmark() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Create the edge array
  alt_edge edges[] = {{0, 1, 2.0}, {1, 2, 3.0}, {2, 3, 4.0}, {3, 1, 1.0}};

  // Construct the four-vertex graph and declare its landmarks
  assert(graph_build(4, edges, 4, &graph) == ALT_OK);
  alt_vertex_id landmarks[] = {2};

  // Preprocess landmarks and ensure success
  alt_landmark_distances distances = {0};
  assert(landmark_preprocess(&graph, landmarks, 1, &distances) == ALT_OK);

  // Verify distances result
  const double expected_distance_from[] = {INFINITY, 5.0, 0.0, 4.0};
  const double expected_distance_to[] = {5.0, 3.0, 0.0, 4.0};

  assert(distances.landmark_count == 1);
  assert(distances.vertex_count == 4);
  assert(distances.distance_from != NULL);
  assert(distances.distance_to != NULL);

  for (size_t vertex_i = 0; vertex_i < distances.vertex_count; vertex_i++) {
    assert(distances.distance_from[vertex_i] ==
           expected_distance_from[vertex_i]);
    assert(distances.distance_to[vertex_i] == expected_distance_to[vertex_i]);
  }

  // Cleanup test allocations
  landmark_distances_destroy(&distances);
  graph_destroy(&graph);
}

// Verify that when every graph vertex is a landmark, in a deliberately
// nonascending order, all rows map to their proper source vertices
void test_all_vertices_as_landmarks() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Create the edge array
  alt_edge edges[] = {{0, 1, 2.0}, {1, 2, 3.0}, {2, 0, 4.0}, {0, 2, 10.0}};

  // Construct the three-vertex graph and declare its landmarks
  assert(graph_build(3, edges, 4, &graph) == ALT_OK);
  alt_vertex_id landmarks[] = {2, 0, 1};

  // Preprocess landmarks and ensure success
  alt_landmark_distances distances = {0};
  assert(landmark_preprocess(&graph, landmarks, 3, &distances) == ALT_OK);

  // Verify distances result
  const double expected_distance_from[][3] = {
      {4.0, 6.0, 0.0}, {0.0, 2.0, 5.0}, {7.0, 0.0, 3.0}};
  const double expected_distance_to[][3] = {
      {5.0, 3.0, 0.0}, {0.0, 7.0, 4.0}, {2.0, 0.0, 6.0}};

  assert(distances.landmark_count == 3);
  assert(distances.vertex_count == 3);
  assert(distances.distance_from != NULL);
  assert(distances.distance_to != NULL);

  for (size_t row_i = 0; row_i < distances.landmark_count; row_i++) {
    for (size_t col_i = 0; col_i < distances.vertex_count; col_i++) {
      assert(distances.distance_from[row_i * distances.vertex_count + col_i] ==
             expected_distance_from[row_i][col_i]);
      assert(distances.distance_to[row_i * distances.vertex_count + col_i] ==
             expected_distance_to[row_i][col_i]);
    }
  }

  // Cleanup test allocations
  landmark_distances_destroy(&distances);
  graph_destroy(&graph);
}

// Verify that separate components and isolated vertices produce positive
// infinity where routes are unreachable, independently in each direction
void test_disconnected_graph() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Create the edge array
  alt_edge edges[] = {{0, 1, 2.0}, {1, 2, 3.0}, {3, 4, 5.0}, {4, 3, 1.0}};

  // Construct the six-vertex graph and declare its landmarks
  assert(graph_build(6, edges, 4, &graph) == ALT_OK);
  alt_vertex_id landmarks[] = {1, 3, 5};

  // Preprocess landmarks and ensure success
  alt_landmark_distances distances = {0};
  assert(landmark_preprocess(&graph, landmarks, 3, &distances) == ALT_OK);

  // Verify distances result
  const double expected_distance_from[][6] = {
      {INFINITY, 0.0, 3.0, INFINITY, INFINITY, INFINITY},
      {INFINITY, INFINITY, INFINITY, 0.0, 5.0, INFINITY},
      {INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, 0.0}};
  const double expected_distance_to[][6] = {
      {2.0, 0.0, INFINITY, INFINITY, INFINITY, INFINITY},
      {INFINITY, INFINITY, INFINITY, 0.0, 1.0, INFINITY},
      {INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, 0.0}};

  assert(distances.landmark_count == 3);
  assert(distances.vertex_count == 6);
  assert(distances.distance_from != NULL);
  assert(distances.distance_to != NULL);

  for (size_t row_i = 0; row_i < distances.landmark_count; row_i++) {
    for (size_t col_i = 0; col_i < distances.vertex_count; col_i++) {
      assert(distances.distance_from[row_i * distances.vertex_count + col_i] ==
             expected_distance_from[row_i][col_i]);
      assert(distances.distance_to[row_i * distances.vertex_count + col_i] ==
             expected_distance_to[row_i][col_i]);
    }
  }

  // Cleanup test allocations
  landmark_distances_destroy(&distances);
  graph_destroy(&graph);
}

// Verify that a graph with vertices but no edges produces zero distances from
// each landmark to itself and infinity everywhere else
void test_zero_edge_graph() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Construct the four-vertex graph and declare its landmarks
  assert(graph_build(4, NULL, 0, &graph) == ALT_OK);
  alt_vertex_id landmarks[] = {3, 1};

  // Preprocess landmarks and ensure success
  alt_landmark_distances distances = {0};
  assert(landmark_preprocess(&graph, landmarks, 2, &distances) == ALT_OK);

  // Verify distances result
  const double expected_distance_from[][4] = {
      {INFINITY, INFINITY, INFINITY, 0.0}, {INFINITY, 0.0, INFINITY, INFINITY}};
  const double expected_distance_to[][4] = {
      {INFINITY, INFINITY, INFINITY, 0.0}, {INFINITY, 0.0, INFINITY, INFINITY}};

  assert(distances.landmark_count == 2);
  assert(distances.vertex_count == 4);
  assert(distances.distance_from != NULL);
  assert(distances.distance_to != NULL);

  for (size_t row_i = 0; row_i < distances.landmark_count; row_i++) {
    for (size_t col_i = 0; col_i < distances.vertex_count; col_i++) {
      assert(distances.distance_from[row_i * distances.vertex_count + col_i] ==
             expected_distance_from[row_i][col_i]);
      assert(distances.distance_to[row_i * distances.vertex_count + col_i] ==
             expected_distance_to[row_i][col_i]);
    }
  }

  // Cleanup test allocations
  landmark_distances_destroy(&distances);
  graph_destroy(&graph);
}

// Verify that zero-weight edges and cycles produce correct zero-cost shortest
// distances
void test_zero_weight_cycle() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Create the edge array
  alt_edge edges[] = {
      {0, 1, 0.0}, {1, 2, 0.0}, {2, 0, 0.0}, {2, 3, 2.0}, {3, 1, 1.0}};

  // Construct the four-vertex graph and declare its landmarks
  assert(graph_build(4, edges, 5, &graph) == ALT_OK);
  alt_vertex_id landmarks[] = {2, 3};

  // Preprocess landmarks and ensure success
  alt_landmark_distances distances = {0};
  assert(landmark_preprocess(&graph, landmarks, 2, &distances) == ALT_OK);

  // Verify distances result
  const double expected_distance_from[][4] = {{0.0, 0.0, 0.0, 2.0},
                                              {1.0, 1.0, 1.0, 0.0}};
  const double expected_distance_to[][4] = {{0.0, 0.0, 0.0, 1.0},
                                            {2.0, 2.0, 2.0, 0.0}};

  assert(distances.landmark_count == 2);
  assert(distances.vertex_count == 4);
  assert(distances.distance_from != NULL);
  assert(distances.distance_to != NULL);

  for (size_t row_i = 0; row_i < distances.landmark_count; row_i++) {
    for (size_t col_i = 0; col_i < distances.vertex_count; col_i++) {
      assert(distances.distance_from[row_i * distances.vertex_count + col_i] ==
             expected_distance_from[row_i][col_i]);
      assert(distances.distance_to[row_i * distances.vertex_count + col_i] ==
             expected_distance_to[row_i][col_i]);
    }
  }

  // Cleanup test allocations
  landmark_distances_destroy(&distances);
  graph_destroy(&graph);
}

// Verify that parallel edges use the cheapest available route and self-loops do
// not corrupt distances
void test_parallel_edges_and_self_loops() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Create the edge array
  alt_edge edges[] = {{0, 1, 10.0}, {0, 1, 2.0}, {1, 2, 3.0}, {2, 3, 4.0},
                      {3, 0, 5.0},  {1, 1, 0.0}, {2, 2, 7.0}};

  // Construct the four-vertex graph and declare its landmarks
  assert(graph_build(4, edges, 7, &graph) == ALT_OK);
  alt_vertex_id landmarks[] = {2, 0};

  // Preprocess landmarks and ensure success
  alt_landmark_distances distances = {0};
  assert(landmark_preprocess(&graph, landmarks, 2, &distances) == ALT_OK);

  // Verify distances result
  const double expected_distance_from[][4] = {{9.0, 11.0, 0.0, 4.0},
                                              {0.0, 2.0, 5.0, 9.0}};
  const double expected_distance_to[][4] = {{5.0, 3.0, 0.0, 10.0},
                                            {0.0, 12.0, 9.0, 5.0}};

  assert(distances.landmark_count == 2);
  assert(distances.vertex_count == 4);
  assert(distances.distance_from != NULL);
  assert(distances.distance_to != NULL);

  for (size_t row_i = 0; row_i < distances.landmark_count; row_i++) {
    for (size_t col_i = 0; col_i < distances.vertex_count; col_i++) {
      assert(distances.distance_from[row_i * distances.vertex_count + col_i] ==
             expected_distance_from[row_i][col_i]);
      assert(distances.distance_to[row_i * distances.vertex_count + col_i] ==
             expected_distance_to[row_i][col_i]);
    }
  }

  // Cleanup test allocations
  landmark_distances_destroy(&distances);
  graph_destroy(&graph);
}

// Verify that count zero succeeds with NULL landmark input, no distance
// allocations, and correctly initialized dimensions
void test_zero_landmarks() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Create the edge array
  alt_edge edges[] = {{0, 1, 2.0}, {1, 2, 3.0}};

  // Construct the three-vertex graph
  assert(graph_build(3, edges, 2, &graph) == ALT_OK);

  // Preprocess landmarks and ensure success
  alt_landmark_distances distances = {0};
  assert(landmark_preprocess(&graph, NULL, 0, &distances) == ALT_OK);

  // Verify distances result
  assert(distances.landmark_count == 0);
  assert(distances.vertex_count == 3);
  assert(distances.distance_from == NULL);
  assert(distances.distance_to == NULL);

  // Cleanup test allocations
  landmark_distances_destroy(&distances);
  graph_destroy(&graph);
}

/* ==========================================================================
 * Argument validation tests
 * ========================================================================== */

// Verify null output pointer returns ALT_INVALID_ARGUMENT
void test_null_output_pointer() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Create the edge array
  alt_edge edges[] = {{0, 1, 2.0}, {1, 2, 3.0}};

  // Construct the three-vertex graph and declare its landmarks
  assert(graph_build(3, edges, 2, &graph) == ALT_OK);
  alt_vertex_id landmarks[] = {1};

  // Preprocess landmarks with null output pointer and ensure failure
  assert(landmark_preprocess(&graph, landmarks, 1, NULL) ==
         ALT_INVALID_ARGUMENT);

  // Cleanup test allocations
  graph_destroy(&graph);
}

// Verify null graph pointer returns ALT_INVALID_ARGUMENT and leaves output
// structure unchanged
void test_null_graph() {
  // Preprocess landmarks with a null graph and ensure failure
  alt_vertex_id landmarks[] = {1};
  alt_landmark_distances distances = {0};
  assert(landmark_preprocess(NULL, landmarks, 1, &distances) ==
         ALT_INVALID_ARGUMENT);

  // Verify output structure unchanged
  assert(distances.landmark_count == 0);
  assert(distances.vertex_count == 0);
  assert(distances.distance_from == NULL);
  assert(distances.distance_to == NULL);

  // Cleanup test allocations
  landmark_distances_destroy(&distances);
}

// Verify null landmarks pointer with a positive count returns
// ALT_INVALID_ARGUMENT and leaves output structure unchanged
void test_null_landmarks() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Create the edge array
  alt_edge edges[] = {{0, 1, 2.0}, {1, 2, 3.0}};

  // Construct the three-vertex graph
  assert(graph_build(3, edges, 2, &graph) == ALT_OK);

  // Preprocess landmarks with null landmarks pointer and ensure failure
  alt_landmark_distances distances = {0};
  assert(landmark_preprocess(&graph, NULL, 1, &distances) ==
         ALT_INVALID_ARGUMENT);

  // Verify output structure unchanged
  assert(distances.landmark_count == 0);
  assert(distances.vertex_count == 0);
  assert(distances.distance_from == NULL);
  assert(distances.distance_to == NULL);

  // Cleanup test allocations
  landmark_distances_destroy(&distances);
  graph_destroy(&graph);
}

// Verify behavior when landmark count exceeds graph's total vertex count
void test_count_exceeds_vertex_count() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Create the edge array
  alt_edge edges[] = {{0, 1, 2.0}, {1, 2, 3.0}};

  // Construct the three-vertex graph and declare its landmarks
  assert(graph_build(3, edges, 2, &graph) == ALT_OK);
  alt_vertex_id landmarks[] = {0, 1, 2, 0};

  // Preprocess landmarks and ensure failure
  alt_landmark_distances distances = {0};
  assert(landmark_preprocess(&graph, landmarks, 4, &distances) ==
         ALT_INVALID_ARGUMENT);

  // Verify output structure unchanged
  assert(distances.landmark_count == 0);
  assert(distances.vertex_count == 0);
  assert(distances.distance_from == NULL);
  assert(distances.distance_to == NULL);

  // Cleanup test allocations
  landmark_distances_destroy(&distances);
  graph_destroy(&graph);
}

// Verify behavior when a landmark provided is not present on the provided graph
void test_out_of_range_landmark() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Create the edge array
  alt_edge edges[] = {{0, 1, 2.0}, {1, 2, 3.0}};

  // Construct the three-vertex graph and declare its landmarks
  assert(graph_build(3, edges, 2, &graph) == ALT_OK);
  alt_vertex_id landmarks[] = {1, 3};

  // Preprocess landmarks and ensure failure
  alt_landmark_distances distances = {0};
  assert(landmark_preprocess(&graph, landmarks, 2, &distances) ==
         ALT_INVALID_ARGUMENT);

  // Verify output structure unchanged
  assert(distances.landmark_count == 0);
  assert(distances.vertex_count == 0);
  assert(distances.distance_from == NULL);
  assert(distances.distance_to == NULL);

  // Cleanup test allocations
  landmark_distances_destroy(&distances);
  graph_destroy(&graph);
}

/* ==========================================================================
 * Numerical errors and cleanup tests
 * ========================================================================== */

// Verify that a reachable path whose accumulated distance overflows to infinity
// returns ALT_NUMERIC_ERROR and output structure remains unchanged
void test_numeric_overflow_cleanup() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Create the edge array
  alt_edge edges[] = {{0, 1, DBL_MAX}, {1, 2, DBL_MAX}};

  // Construct the four-vertex graph and declare its landmarks
  assert(graph_build(4, edges, 2, &graph) == ALT_OK);
  alt_vertex_id landmarks[] = {3, 0};

  // Preprocess landmarks and ensure failure
  alt_landmark_distances distances = {0};
  assert(landmark_preprocess(&graph, landmarks, 2, &distances) ==
         ALT_NUMERIC_ERROR);

  // Verify output structure unchanged
  assert(distances.landmark_count == 0);
  assert(distances.vertex_count == 0);
  assert(distances.distance_from == NULL);
  assert(distances.distance_to == NULL);

  // Cleanup test allocations
  landmark_distances_destroy(&distances);
  graph_destroy(&graph);
}

// Verify that adding a positive weight without changing the accumulated
// floating-point distance returns ALT_NUMERIC_ERROR and output structure
// remains unchanged
void test_numeric_stagnation_cleanup() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Create the edge array
  alt_edge edges[] = {{0, 1, 9007199254740992.0}, {1, 2, 1.0}};

  // Construct the four-vertex graph and declare its landmarks
  assert(graph_build(4, edges, 2, &graph) == ALT_OK);
  alt_vertex_id landmarks[] = {3, 0};

  // Preprocess landmarks and ensure failure
  alt_landmark_distances distances = {0};
  assert(landmark_preprocess(&graph, landmarks, 2, &distances) ==
         ALT_NUMERIC_ERROR);

  // Verify output structure unchanged
  assert(distances.landmark_count == 0);
  assert(distances.vertex_count == 0);
  assert(distances.distance_from == NULL);
  assert(distances.distance_to == NULL);

  // Cleanup test allocations
  landmark_distances_destroy(&distances);
  graph_destroy(&graph);
}

/* ==========================================================================
 * Ownership and destruction tests
 * ========================================================================== */

// Verify two preprocessing calls produce independently allocated tables, and
// destroying one result doesn't invalidate the other
void test_independent_results() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Create the edge array
  alt_edge edges[] = {{0, 1, 2.0}, {1, 2, 3.0}, {2, 0, 4.0}};

  // Construct the three-vertex graph and declare its landmarks
  assert(graph_build(3, edges, 3, &graph) == ALT_OK);
  alt_vertex_id landmarks[] = {2};

  // Preprocess landmarks twice with separate output structures and ensure
  // success
  alt_landmark_distances distances_one = {0};
  alt_landmark_distances distances_two = {0};
  assert(landmark_preprocess(&graph, landmarks, 1, &distances_one) == ALT_OK);
  assert(landmark_preprocess(&graph, landmarks, 1, &distances_two) == ALT_OK);

  // Verify distances results for both runs
  const double expected_distance_from[] = {4.0, 6.0, 0.0};
  const double expected_distance_to[] = {5.0, 3.0, 0.0};

  assert(distances_one.landmark_count == 1);
  assert(distances_one.vertex_count == 3);
  assert(distances_one.distance_from != NULL);
  assert(distances_one.distance_to != NULL);

  for (size_t vertex_i = 0; vertex_i < distances_one.vertex_count; vertex_i++) {
    assert(distances_one.distance_from[vertex_i] ==
           expected_distance_from[vertex_i]);
    assert(distances_one.distance_to[vertex_i] ==
           expected_distance_to[vertex_i]);
  }

  assert(distances_two.landmark_count == 1);
  assert(distances_two.vertex_count == 3);
  assert(distances_two.distance_from != NULL);
  assert(distances_two.distance_to != NULL);

  for (size_t vertex_i = 0; vertex_i < distances_two.vertex_count; vertex_i++) {
    assert(distances_two.distance_from[vertex_i] ==
           expected_distance_from[vertex_i]);
    assert(distances_two.distance_to[vertex_i] ==
           expected_distance_to[vertex_i]);
  }

  // Verify that both distances structures pointers point to different
  // allocations
  assert(distances_one.distance_from != distances_two.distance_from);
  assert(distances_one.distance_to != distances_two.distance_to);

  // Verify that changing values in one structure's arrays does not change them
  // in the other's
  distances_one.distance_from[0]++;
  distances_one.distance_to[0]++;

  assert(distances_two.distance_from[0] == expected_distance_from[0]);
  assert(distances_two.distance_to[0] == expected_distance_to[0]);

  // Verify that after destroying one structure, the other still retains its
  // expected values
  landmark_distances_destroy(&distances_one);

  assert(distances_two.landmark_count == 1);
  assert(distances_two.vertex_count == 3);
  assert(distances_two.distance_from != NULL);
  assert(distances_two.distance_to != NULL);

  for (size_t vertex_i = 0; vertex_i < distances_two.vertex_count; vertex_i++) {
    assert(distances_two.distance_from[vertex_i] ==
           expected_distance_from[vertex_i]);
    assert(distances_two.distance_to[vertex_i] ==
           expected_distance_to[vertex_i]);
  }

  // Cleanup test allocations
  landmark_distances_destroy(&distances_two);
  graph_destroy(&graph);
}

// Verify that passing NULL to the destructor is safe
void test_destroy_null() { landmark_distances_destroy(NULL); }

// Verify that destroying an empty structure is safe and leaves it
// zero-initialized
void test_destroy_zero_initialized() {
  // Create a zero-initialized distances structure and destroy it
  alt_landmark_distances distances = {0};
  landmark_distances_destroy(&distances);

  // Verify the structure remains in a zero-initialized state
  assert(distances.landmark_count == 0);
  assert(distances.vertex_count == 0);
  assert(distances.distance_from == NULL);
  assert(distances.distance_to == NULL);
}

// Verify that a partially initialized structure is cleaned up correctly, and
// repeated destruction is safe
void test_destroy_partial_and_repeated() {
  // Create a manually populated distances structure that reflects a partially
  // initialized state
  alt_landmark_distances distances = {0};
  distances.landmark_count = 1;
  distances.vertex_count = 3;
  distances.distance_from = malloc(3 * sizeof(double));
  assert(distances.distance_from != NULL);

  // Destroy the structure and verify that it has been returned to a
  // zero-initialized state
  landmark_distances_destroy(&distances);

  assert(distances.landmark_count == 0);
  assert(distances.vertex_count == 0);
  assert(distances.distance_from == NULL);
  assert(distances.distance_to == NULL);

  // Destroy the structure again and verify that it is still in a
  // zero-initialized state
  landmark_distances_destroy(&distances);

  assert(distances.landmark_count == 0);
  assert(distances.vertex_count == 0);
  assert(distances.distance_from == NULL);
  assert(distances.distance_to == NULL);
}

// Executes tests
int main() {
  // Execute tests
  // Distance calculation and graph behavior
  test_directed_landmark_distances();
  test_single_landmark();
  test_all_vertices_as_landmarks();
  test_disconnected_graph();
  test_zero_edge_graph();
  test_zero_weight_cycle();
  test_parallel_edges_and_self_loops();
  test_zero_landmarks();
  // Argument validation
  test_null_output_pointer();
  test_null_graph();
  test_null_landmarks();
  test_count_exceeds_vertex_count();
  test_out_of_range_landmark();
  // Numerical errors and cleanup
  test_numeric_overflow_cleanup();
  test_numeric_stagnation_cleanup();
  // Ownership and destruction
  test_independent_results();
  test_destroy_null();
  test_destroy_zero_initialized();
  test_destroy_partial_and_repeated();

  // All tests passed
  return 0;
}
