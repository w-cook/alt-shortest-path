#include "../src/alt_index.h"
#include "alt/alt.h"
#include <assert.h>
#include <float.h>
#include <math.h>

/* ==========================================================================
 * Successful construction tests
 * ========================================================================== */

// Verify graph ownership, selected landmark IDs, and precomputed distances
void test_explicit_landmarks() {
  // Construct the index and verify success and non-null result
  alt_edge edges[] = {
      {0, 1, 2.0}, {1, 2, 3.0}, {2, 3, 4.0}, {3, 1, 1.0}, {4, 0, 5.0}};
  alt_vertex_id landmarks[] = {4, 1};
  alt_landmark_options options = {ALT_LANDMARK_EXPLICIT, 2, landmarks, 0, 0};

  alt_index *index = NULL;

  assert(alt_index_build(5, edges, 5, &options, &index) == ALT_OK);
  assert(index != NULL);

  // Verify the results match expected values
  assert(index->graph.vertex_count == 5);
  assert(index->graph.edge_count == 5);
  assert(index->landmarks != NULL);
  assert(index->landmarks[0] == 4 && index->landmarks[1] == 1);
  assert(index->distances.vertex_count == 5);
  assert(index->distances.landmark_count == 2);
  assert(index->distances.distance_from != NULL);
  assert(index->distances.distance_to != NULL);

  const double expected_distance_from[][5] = {
      {5.0, 7.0, 10.0, 14.0, 0.0}, {INFINITY, 0.0, 3.0, 7.0, INFINITY}};
  const double expected_distance_to[][5] = {
      {INFINITY, INFINITY, INFINITY, INFINITY, 0.0}, {2.0, 0.0, 5.0, 1.0, 7.0}};

  for (size_t row_i = 0; row_i < index->distances.landmark_count; row_i++) {
    for (size_t col_i = 0; col_i < index->distances.vertex_count; col_i++) {
      assert(
          index->distances
              .distance_from[row_i * index->distances.vertex_count + col_i] ==
          expected_distance_from[row_i][col_i]);
      assert(index->distances
                 .distance_to[row_i * index->distances.vertex_count + col_i] ==
             expected_distance_to[row_i][col_i]);
    }
  }

  // Cleanup test allocations
  alt_index_destroy(index);
}

// Verify integration with seeded random selection
void test_random_landmarks() {
  // Construct two indices with the same inputs and verify success and non-null
  // result
  alt_edge edges[] = {
      {0, 1, 1.0}, {1, 2, 1.0}, {2, 3, 1.0}, {3, 4, 1.0}, {4, 0, 1.0}};
  alt_landmark_options options = {ALT_LANDMARK_RANDOM, 3, NULL, 42, 0};

  alt_index *index_one = NULL;
  alt_index *index_two = NULL;

  assert(alt_index_build(5, edges, 5, &options, &index_one) == ALT_OK);
  assert(index_one != NULL);
  assert(alt_index_build(5, edges, 5, &options, &index_two) == ALT_OK);
  assert(index_two != NULL);

  // Verify the results match expected values
  assert(index_one->graph.vertex_count == 5);
  assert(index_two->graph.vertex_count == 5);
  assert(index_one->graph.edge_count == 5);
  assert(index_two->graph.edge_count == 5);

  assert(index_one->landmarks != NULL);
  assert(index_two->landmarks != NULL);
  for (size_t landmark_i = 0; landmark_i < options.count; landmark_i++) {
    assert(index_one->landmarks[landmark_i] ==
           index_two->landmarks[landmark_i]);
    assert(index_one->landmarks[landmark_i] < index_one->graph.vertex_count);

    for (size_t compare_landmark_i = 0; compare_landmark_i < landmark_i;
         compare_landmark_i++) {
      assert(index_one->landmarks[landmark_i] !=
             index_one->landmarks[compare_landmark_i]);
    }
  }

  assert(index_one->distances.vertex_count == 5);
  assert(index_two->distances.vertex_count == 5);
  assert(index_one->distances.landmark_count == 3);
  assert(index_two->distances.landmark_count == 3);
  assert(index_one->distances.distance_from != NULL);
  assert(index_two->distances.distance_from != NULL);
  assert(index_one->distances.distance_to != NULL);
  assert(index_two->distances.distance_to != NULL);

  for (size_t row_i = 0; row_i < index_one->distances.landmark_count; row_i++) {
    alt_vertex_id landmark_vertex = index_one->landmarks[row_i];

    for (size_t col_i = 0; col_i < index_one->distances.vertex_count; col_i++) {
      assert(index_one->distances
                 .distance_from[row_i * index_one->distances.vertex_count +
                                col_i] ==
             index_two->distances
                 .distance_from[row_i * index_two->distances.vertex_count +
                                col_i]);
      assert(
          index_one->distances
              .distance_to[row_i * index_one->distances.vertex_count + col_i] ==
          index_two->distances
              .distance_to[row_i * index_two->distances.vertex_count + col_i]);

      alt_vertex_id target_vertex = col_i;

      double expected_distance_from;
      if (landmark_vertex <= target_vertex) {
        expected_distance_from = target_vertex - landmark_vertex;
      } else {
        expected_distance_from =
            index_one->distances.vertex_count - landmark_vertex + target_vertex;
      }
      assert(index_one->distances
                 .distance_from[row_i * index_one->distances.vertex_count +
                                col_i] == expected_distance_from);

      double expected_distance_to;
      if (target_vertex <= landmark_vertex) {
        expected_distance_to = landmark_vertex - target_vertex;
      } else {
        expected_distance_to =
            index_one->distances.vertex_count - target_vertex + landmark_vertex;
      }
      assert(
          index_one->distances
              .distance_to[row_i * index_one->distances.vertex_count + col_i] ==
          expected_distance_to);
    }
  }

  // Cleanup test allocations
  alt_index_destroy(index_one);
  alt_index_destroy(index_two);
}

// Verify integration with farthest-first selection
void test_farthest_landmarks() {
  // Construct the index and verify success and non-null result
  alt_edge edges[] = {{0, 1, 1.0}, {1, 2, 2.0}, {2, 3, 3.0}, {3, 4, 4.0}};
  alt_landmark_options options = {ALT_LANDMARK_FARTHEST, 3, NULL, 0, 0};

  alt_index *index = NULL;

  assert(alt_index_build(5, edges, 4, &options, &index) == ALT_OK);
  assert(index != NULL);

  // Verify the results match expected values
  assert(index->graph.vertex_count == 5);
  assert(index->graph.edge_count == 4);
  assert(index->landmarks != NULL);
  assert(index->landmarks[0] == 4 && index->landmarks[1] == 0 &&
         index->landmarks[2] == 3);
  assert(index->distances.vertex_count == 5);
  assert(index->distances.landmark_count == 3);
  assert(index->distances.distance_from != NULL);
  assert(index->distances.distance_to != NULL);

  const double expected_distance_from[][5] = {
      {INFINITY, INFINITY, INFINITY, INFINITY, 0.0},
      {0.0, 1.0, 3.0, 6.0, 10.0},
      {INFINITY, INFINITY, INFINITY, 0, 4}};
  const double expected_distance_to[][5] = {
      {10.0, 9.0, 7.0, 4.0, 0.0},
      {0.0, INFINITY, INFINITY, INFINITY, INFINITY},
      {6.0, 5.0, 3.0, 0.0, INFINITY}};

  for (size_t row_i = 0; row_i < index->distances.landmark_count; row_i++) {
    for (size_t col_i = 0; col_i < index->distances.vertex_count; col_i++) {
      assert(
          index->distances
              .distance_from[row_i * index->distances.vertex_count + col_i] ==
          expected_distance_from[row_i][col_i]);
      assert(index->distances
                 .distance_to[row_i * index->distances.vertex_count + col_i] ==
             expected_distance_to[row_i][col_i]);
    }
  }

  // Cleanup test allocations
  alt_index_destroy(index);
}

// Verify successful construction without landmark allocations
void test_zero_landmarks() {
  // Construct the index and verify success and non-null result
  alt_edge edges[] = {{0, 1, 2.0}, {1, 2, 3.0}};
  alt_landmark_options options = {ALT_LANDMARK_EXPLICIT, 0, NULL, 0, 0};

  alt_index *index = NULL;

  assert(alt_index_build(3, edges, 2, &options, &index) == ALT_OK);
  assert(index != NULL);

  // Verify the results match expected values
  assert(index->graph.vertex_count == 3);
  assert(index->graph.edge_count == 2);
  assert(index->landmarks == NULL);
  assert(index->distances.vertex_count == 3);
  assert(index->distances.landmark_count == 0);
  assert(index->distances.distance_from == NULL);
  assert(index->distances.distance_to == NULL);

  // Cleanup test allocations
  alt_index_destroy(index);
}

// Verify an index can represent a graph with isolated vertices
void test_zero_edge_graph() {
  // Construct the index and verify success and non-null result
  alt_vertex_id landmarks[] = {3, 1};
  alt_landmark_options options = {ALT_LANDMARK_EXPLICIT, 2, landmarks, 0, 0};

  alt_index *index = NULL;

  assert(alt_index_build(4, NULL, 0, &options, &index) == ALT_OK);
  assert(index != NULL);

  // Verify the results match expected values
  assert(index->graph.vertex_count == 4);
  assert(index->graph.edge_count == 0);

  assert(index->graph.edge_starts != NULL);
  for (size_t vertex_i = 0; vertex_i <= index->graph.vertex_count; vertex_i++) {
    assert(index->graph.edge_starts[vertex_i] == 0);
  }

  assert(index->graph.targets == NULL);
  assert(index->graph.weights == NULL);
  assert(index->graph.original_edge_ids == NULL);

  assert(index->landmarks != NULL);
  assert(index->landmarks[0] == 3 && index->landmarks[1] == 1);

  assert(index->distances.vertex_count == 4);
  assert(index->distances.landmark_count == 2);
  assert(index->distances.distance_from != NULL);
  assert(index->distances.distance_to != NULL);

  const double expected_distances[][4] = {{INFINITY, INFINITY, INFINITY, 0.0},
                                          {INFINITY, 0.0, INFINITY, INFINITY}};

  for (size_t row_i = 0; row_i < index->distances.landmark_count; row_i++) {
    for (size_t col_i = 0; col_i < index->distances.vertex_count; col_i++) {
      assert(
          index->distances
              .distance_from[row_i * index->distances.vertex_count + col_i] ==
          expected_distances[row_i][col_i]);
      assert(index->distances
                 .distance_to[row_i * index->distances.vertex_count + col_i] ==
             expected_distances[row_i][col_i]);
    }
  }

  // Cleanup test allocations
  alt_index_destroy(index);
}

// Verify modifying caller-owned input arrays doesn't affect the constructed
// index
void test_input_independence() {
  // Construct the index and verify success and non-null result
  alt_edge edges[] = {{0, 1, 2.0}, {1, 2, 3.0}};
  alt_vertex_id landmarks[] = {2, 0};
  alt_landmark_options options = {ALT_LANDMARK_EXPLICIT, 2, landmarks, 0, 0};

  alt_index *index = NULL;

  assert(alt_index_build(3, edges, 2, &options, &index) == ALT_OK);
  assert(index != NULL);

  // Modify the caller-owned arrays
  edges[0] = (alt_edge){2, 1, 1.0};
  edges[1] = (alt_edge){0, 2, 5.0};
  landmarks[0] = 1;
  landmarks[1] = 2;

  // Verify the index still matches expected values
  assert(index->graph.vertex_count == 3);
  assert(index->graph.edge_count == 2);

  assert(index->graph.edge_starts != NULL);
  const size_t expected_edge_starts[] = {0, 1, 2, 2};
  for (size_t vertex_i = 0; vertex_i <= index->graph.vertex_count; vertex_i++) {
    assert(index->graph.edge_starts[vertex_i] ==
           expected_edge_starts[vertex_i]);
  }

  assert(index->graph.targets != NULL);
  assert(index->graph.targets[0] == 1 && index->graph.targets[1] == 2);
  assert(index->graph.weights != NULL);
  assert(index->graph.weights[0] == 2.0 && index->graph.weights[1] == 3.0);
  assert(index->graph.original_edge_ids != NULL);
  assert(index->graph.original_edge_ids[0] == 0 &&
         index->graph.original_edge_ids[1] == 1);

  assert(index->landmarks != NULL);
  assert(index->landmarks[0] == 2 && index->landmarks[1] == 0);

  assert(index->distances.vertex_count == 3);
  assert(index->distances.landmark_count == 2);
  assert(index->distances.distance_from != NULL);
  assert(index->distances.distance_to != NULL);

  const double expected_distance_from[][3] = {{INFINITY, INFINITY, 0.0},
                                              {0.0, 2.0, 5.0}};
  const double expected_distance_to[][3] = {{5.0, 3.0, 0.0},
                                            {0.0, INFINITY, INFINITY}};

  for (size_t row_i = 0; row_i < index->distances.landmark_count; row_i++) {
    for (size_t col_i = 0; col_i < index->distances.vertex_count; col_i++) {
      assert(
          index->distances
              .distance_from[row_i * index->distances.vertex_count + col_i] ==
          expected_distance_from[row_i][col_i]);
      assert(index->distances
                 .distance_to[row_i * index->distances.vertex_count + col_i] ==
             expected_distance_to[row_i][col_i]);
    }
  }

  // Cleanup test allocations
  alt_index_destroy(index);
}

/* ==========================================================================
 * Argument and configuration validation tests
 * ========================================================================== */

// Verify null output pointers are rejected
void test_null_output() {
  // Construct the index and verify failure
  alt_edge edges[] = {{0, 1, 2.0}};
  alt_vertex_id landmarks[] = {0};
  alt_landmark_options options = {ALT_LANDMARK_EXPLICIT, 1, landmarks, 0, 0};

  assert(alt_index_build(2, edges, 1, &options, NULL) == ALT_INVALID_ARGUMENT);
}

// Verify null configuration pointers are rejected and clear the output
void test_null_options() {
  // Construct the index and verify failure and cleared output
  alt_edge edges[] = {{0, 1, 2.0}};
  alt_index index_val = {0};
  alt_index *index = &index_val;

  assert(alt_index_build(2, edges, 1, NULL, &index) == ALT_INVALID_ARGUMENT);
  assert(index == NULL);
}

// Verify invalid graph errors are propagated
void test_invalid_graph() {
  // Construct the index and verify failure and output cleared
  alt_edge edges[] = {{0, 1, 2.0}, {1, 3, 3.0}};
  alt_vertex_id landmarks[] = {0};
  alt_landmark_options options = {ALT_LANDMARK_EXPLICIT, 1, landmarks, 0, 0};
  alt_index index_val = {0};
  alt_index *index = &index_val;

  assert(alt_index_build(3, edges, 2, &options, &index) == ALT_INVALID_GRAPH);
  assert(index == NULL);
}

// Verify invalid landmark configuration errors are propagated
void test_invalid_configuration() {
  // Construct the index and verify failure and output cleared
  alt_edge edges[] = {{0, 1, 2.0}, {1, 2, 3.0}};
  alt_vertex_id landmarks[] = {1, 1};
  alt_landmark_options options = {ALT_LANDMARK_EXPLICIT, 2, landmarks, 0, 0};
  alt_index index_val = {0};
  alt_index *index = &index_val;

  assert(alt_index_build(3, edges, 2, &options, &index) ==
         ALT_INVALID_CONFIGURATION);
  assert(index == NULL);
}

/* ==========================================================================
 * Failure handling tests
 * ========================================================================== */

// Verify numerical failure errors during preprocessing are propagated
void test_numeric_error() {
  // Construct the index and verify failure and output cleared
  alt_edge edges[] = {{0, 1, DBL_MAX}, {1, 2, DBL_MAX}};
  alt_vertex_id landmarks[] = {3, 0};
  alt_landmark_options options = {ALT_LANDMARK_EXPLICIT, 2, landmarks, 0, 0};
  alt_index index_val = {0};
  alt_index *index = &index_val;

  assert(alt_index_build(4, edges, 2, &options, &index) == ALT_NUMERIC_ERROR);
  assert(index == NULL);
}

// Verify failure after partial construction leaves no index or leaked resources
void test_failed_construction_cleanup() {
  // Construct the index and verify failure and output cleared
  alt_edge edges[] = {{0, 1, DBL_MAX}, {1, 2, DBL_MAX}};
  alt_vertex_id landmarks[] = {3, 2};
  alt_landmark_options options = {ALT_LANDMARK_EXPLICIT, 2, landmarks, 0, 0};
  alt_index index_val = {0};
  alt_index *index = &index_val;

  assert(alt_index_build(4, edges, 2, &options, &index) == ALT_NUMERIC_ERROR);
  assert(index == NULL);
}

/* ==========================================================================
 * Lifecycle tests
 * ========================================================================== */

// Verify null destruction is safe
void test_destroy_null() { alt_index_destroy(NULL); }

// Verify independently constructed indexes coexist and can be destroyed
// separately
void test_repeated_construction() {
  // Construct the indices using the same inputs and verify success
  alt_edge edges[] = {{0, 1, 2.0}, {1, 2, 3.0}};
  alt_vertex_id landmarks[] = {0};
  alt_landmark_options options = {ALT_LANDMARK_EXPLICIT, 1, landmarks, 0, 0};
  alt_index *index_one = NULL;
  alt_index *index_two = NULL;

  assert(alt_index_build(3, edges, 2, &options, &index_one) == ALT_OK);
  assert(alt_index_build(3, edges, 2, &options, &index_two) == ALT_OK);

  // Verify independent allocations and contents
  assert(index_one != NULL);
  assert(index_two != NULL);
  assert(index_one != index_two);

  assert(index_one->graph.edge_starts != NULL);
  assert(index_two->graph.edge_starts != NULL);
  assert(index_one->graph.edge_starts != index_two->graph.edge_starts);

  assert(index_one->graph.targets != NULL);
  assert(index_two->graph.targets != NULL);
  assert(index_one->graph.targets != index_two->graph.targets);

  assert(index_one->graph.weights != NULL);
  assert(index_two->graph.weights != NULL);
  assert(index_one->graph.weights != index_two->graph.weights);

  assert(index_one->graph.original_edge_ids != NULL);
  assert(index_two->graph.original_edge_ids != NULL);
  assert(index_one->graph.original_edge_ids !=
         index_two->graph.original_edge_ids);

  assert(index_one->landmarks != NULL);
  assert(index_two->landmarks != NULL);
  assert(index_one->landmarks != index_two->landmarks);

  assert(index_one->distances.distance_from != NULL);
  assert(index_two->distances.distance_from != NULL);
  assert(index_one->distances.distance_from !=
         index_two->distances.distance_from);

  assert(index_one->distances.distance_to != NULL);
  assert(index_two->distances.distance_to != NULL);
  assert(index_one->distances.distance_to != index_two->distances.distance_to);

  assert(index_one->graph.vertex_count == 3);
  assert(index_two->graph.vertex_count == 3);

  assert(index_one->graph.edge_count == 2);
  assert(index_two->graph.edge_count == 2);

  assert(index_one->landmarks[0] == 0);
  assert(index_two->landmarks[0] == 0);

  assert(index_one->distances.vertex_count == 3);
  assert(index_two->distances.vertex_count == 3);

  assert(index_one->distances.landmark_count == 1);
  assert(index_two->distances.landmark_count == 1);

  const double expected_distance_from[] = {0, 2, 5};
  const double expected_distance_to[] = {0, INFINITY, INFINITY};

  for (size_t vertex_i = 0; vertex_i < index_one->distances.vertex_count;
       vertex_i++) {
    assert(index_one->distances.distance_from[vertex_i] ==
           expected_distance_from[vertex_i]);
    assert(index_two->distances.distance_from[vertex_i] ==
           expected_distance_from[vertex_i]);
    assert(index_one->distances.distance_to[vertex_i] ==
           expected_distance_to[vertex_i]);
    assert(index_two->distances.distance_to[vertex_i] ==
           expected_distance_to[vertex_i]);
  }

  // Destroy the first index and verify the second index's contents are
  // unaffected
  alt_index_destroy(index_one);

  assert(index_two != NULL);
  assert(index_two->graph.edge_starts != NULL);
  assert(index_two->graph.targets != NULL);
  assert(index_two->graph.weights != NULL);
  assert(index_two->graph.original_edge_ids != NULL);
  assert(index_two->landmarks != NULL);
  assert(index_two->distances.distance_from != NULL);
  assert(index_two->distances.distance_to != NULL);
  assert(index_two->graph.vertex_count == 3);
  assert(index_two->graph.edge_count == 2);
  assert(index_two->landmarks[0] == 0);
  assert(index_two->distances.vertex_count == 3);
  assert(index_two->distances.landmark_count == 1);

  for (size_t vertex_i = 0; vertex_i < index_two->distances.vertex_count;
       vertex_i++) {
    assert(index_two->distances.distance_from[vertex_i] ==
           expected_distance_from[vertex_i]);
    assert(index_two->distances.distance_to[vertex_i] ==
           expected_distance_to[vertex_i]);
  }

  // Finish test allocation cleanup
  alt_index_destroy(index_two);
}

// Executes tests
int main() {
  // Execute tests
  // Successful construction tests
  test_explicit_landmarks();
  test_random_landmarks();
  test_farthest_landmarks();
  test_zero_landmarks();
  test_zero_edge_graph();
  test_input_independence();
  // Argument and configuration validation tests
  test_null_output();
  test_null_options();
  test_invalid_graph();
  test_invalid_configuration();
  // Failure handling tests
  test_numeric_error();
  test_failed_construction_cleanup();
  // Lifecycle tests
  test_destroy_null();
  test_repeated_construction();

  // All tests passed
  return 0;
}
