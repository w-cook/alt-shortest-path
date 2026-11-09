#include "../src/alt_heuristic.h"
#include "alt/alt.h"
#include <assert.h>
#include <math.h>

// Verify correct positive forward bound
void test_forward_bound() {
  // Build a manually constructed distances structure
  alt_landmark_distances distances = {0};
  distances.landmark_count = 1;
  distances.vertex_count = 4;
  double distance_from[] = {0.0, 3.0, 8.0, 10.0};
  distances.distance_from = distance_from;
  double distance_to[] = {0.0, 3.0, 8.0, 10.0};
  distances.distance_to = distance_to;

  // Run the heuristic and ensure success
  double estimate = 0.0;
  assert(alt_heuristic_evaluate(&distances, 1, 2, &estimate) == ALT_OK);

  // Verify the expected output
  assert(estimate == 5.0);
}

// Verify correct positive reverse bound
void test_reverse_bound() {
  // Build a manually constructed distances structure
  alt_landmark_distances distances = {0};
  distances.landmark_count = 1;
  distances.vertex_count = 3;
  double distance_from[] = {0.0, 8.0, 3.0};
  distances.distance_from = distance_from;
  double distance_to[] = {0.0, 8.0, 2.0};
  distances.distance_to = distance_to;

  // Run the heuristic and ensure success
  double estimate = 0.0;
  assert(alt_heuristic_evaluate(&distances, 1, 2, &estimate) == ALT_OK);

  // Verify the expected output
  assert(estimate == 6.0);
}

// Verify selecting maximum across landmarks and row indexing
void test_multiple_landmarks_maximum() {
  // Build a manually constructed distances structure
  alt_landmark_distances distances = {0};
  distances.landmark_count = 2;
  distances.vertex_count = 5;
  double distance_from[] = {INFINITY, 0.0, 3.0,  7.0,  INFINITY,
                            5.0,      7.0, 10.0, 14.0, 0.0};
  distances.distance_from = distance_from;
  double distance_to[] = {2.0,      0.0,      5.0,      1.0,      7.0,
                          INFINITY, INFINITY, INFINITY, INFINITY, 0.0};
  distances.distance_to = distance_to;

  // Run the heuristic and ensure success
  double estimate = 0.0;
  assert(alt_heuristic_evaluate(&distances, 0, 3, &estimate) == ALT_OK);

  // Verify the expected output
  assert(estimate == 9.0);
}

// Verify negative bounds cannot reduce estimate below zero
void test_negative_bounds_clamped() {
  // Build a manually constructed distances structure
  alt_landmark_distances distances = {0};
  distances.landmark_count = 1;
  distances.vertex_count = 3;
  double distance_from[] = {0.0, 7.0, 3.0};
  distances.distance_from = distance_from;
  double distance_to[] = {0.0, 2.0, 8.0};
  distances.distance_to = distance_to;

  // Run the heuristic and ensure success
  double estimate = -1.0;
  assert(alt_heuristic_evaluate(&distances, 1, 2, &estimate) == ALT_OK);

  // Verify the expected output
  assert(estimate == 0.0);
}

// Verify evaluating a vertex against itself gives zero
void test_same_vertex() {
  // Build a manually constructed distances structure
  alt_landmark_distances distances = {0};
  distances.landmark_count = 1;
  distances.vertex_count = 4;
  double distance_from[] = {0.0, 4.0, 9.0, 12.0};
  distances.distance_from = distance_from;
  double distance_to[] = {0.0, 7.0, 3.0, 8.0};
  distances.distance_to = distance_to;

  // Run the heuristic and ensure success
  double estimate = -1.0;
  assert(alt_heuristic_evaluate(&distances, 2, 2, &estimate) == ALT_OK);

  // Verify the expected output
  assert(estimate == 0.0);
}

// Verify zero landmarks and null tables produce zero
void test_zero_landmarks() {
  // Build a manually constructed distances structure
  alt_landmark_distances distances = {0};
  distances.vertex_count = 4;

  // Run the heuristic and ensure success
  double estimate = -1.0;
  assert(alt_heuristic_evaluate(&distances, 1, 3, &estimate) == ALT_OK);

  // Verify the expected output
  assert(estimate == 0.0);
}

// Verify skipping unusable forward bound and preserving valid reverse bound
void test_forward_unreachable() {
  // Build a manually constructed distances structure
  alt_landmark_distances distances = {0};
  distances.landmark_count = 1;
  distances.vertex_count = 3;
  double distance_from[] = {0.0, INFINITY, 4.0};
  distances.distance_from = distance_from;
  double distance_to[] = {0.0, 5.0, 2.0};
  distances.distance_to = distance_to;

  // Run the heuristic and ensure success
  double estimate = 0.0;
  assert(alt_heuristic_evaluate(&distances, 1, 2, &estimate) == ALT_OK);

  // Verify the expected output
  assert(estimate == 3.0);
}

// Verify skipping unusable reverse bound and preserving valid forward bound
void test_reverse_unreachable() {
  // Build a manually constructed distances structure
  alt_landmark_distances distances = {0};
  distances.landmark_count = 1;
  distances.vertex_count = 3;
  double distance_from[] = {0.0, 2.0, 7.0};
  distances.distance_from = distance_from;
  double distance_to[] = {0.0, 4.0, INFINITY};
  distances.distance_to = distance_to;

  // Run the heuristic and ensure success
  double estimate = 0.0;
  assert(alt_heuristic_evaluate(&distances, 1, 2, &estimate) == ALT_OK);

  // Verify the expected output
  assert(estimate == 5.0);
}

// Verify returns zero when every candidate is unusable
void test_no_usable_bounds() {
  // Build a manually constructed distances structure
  alt_landmark_distances distances = {0};
  distances.landmark_count = 1;
  distances.vertex_count = 3;
  double distance_from[] = {0.0, INFINITY, INFINITY};
  distances.distance_from = distance_from;
  double distance_to[] = {0.0, INFINITY, INFINITY};
  distances.distance_to = distance_to;

  // Run the heuristic and ensure success
  double estimate = -1.0;
  assert(alt_heuristic_evaluate(&distances, 1, 2, &estimate) == ALT_OK);

  // Verify the expected output
  assert(estimate == 0.0);
}

// Verify correct behavior with exactly representable fractional distances
void test_fractional_distances() {
  // Build a manually constructed distances structure
  alt_landmark_distances distances = {0};
  distances.landmark_count = 1;
  distances.vertex_count = 3;
  double distance_from[] = {0.0, 1.25, 3.75};
  distances.distance_from = distance_from;
  double distance_to[] = {0.0, 2.5, 0.75};
  distances.distance_to = distance_to;

  // Run the heuristic and ensure success
  double estimate = 0.0;
  assert(alt_heuristic_evaluate(&distances, 1, 2, &estimate) == ALT_OK);

  // Verify the expected output
  assert(estimate == 2.5);
}

// Verify rejection of null distance structure
void test_null_distances() {
  // Run the heuristic with null distance argument and ensure failure
  double estimate = 0.0;
  assert(alt_heuristic_evaluate(NULL, 0, 1, &estimate) == ALT_INVALID_ARGUMENT);
}

// Verify rejection of null output pointer
void test_null_output() {
  // Build a manually constructed distances structure
  alt_landmark_distances distances = {0};
  distances.vertex_count = 3;

  // Run the heuristic with null output pointer and ensure failure
  assert(alt_heuristic_evaluate(&distances, 0, 1, NULL) ==
         ALT_INVALID_ARGUMENT);
}

// Verify rejection of out-of-range current vertex
void test_invalid_current_vertex() {
  // Build a manually constructed distances structure
  alt_landmark_distances distances = {0};
  distances.vertex_count = 3;

  // Run the heuristic with out-of-range current vertex and ensure failure
  double estimate = 0.0;
  assert(alt_heuristic_evaluate(&distances, 3, 1, &estimate) ==
         ALT_INVALID_ARGUMENT);
  assert(alt_heuristic_evaluate(&distances, 4, 1, &estimate) ==
         ALT_INVALID_ARGUMENT);
}

// Verify rejection of out-of-range destination vertex
void test_invalid_target_vertex() {
  // Build a manually constructed distances structure
  alt_landmark_distances distances = {0};
  distances.vertex_count = 3;

  // Run the heuristic with out-of-range target vertex and ensure failure
  double estimate = 0.0;
  assert(alt_heuristic_evaluate(&distances, 1, 3, &estimate) ==
         ALT_INVALID_ARGUMENT);
  assert(alt_heuristic_evaluate(&distances, 1, 4, &estimate) ==
         ALT_INVALID_ARGUMENT);
}

// Verify correct heuristic results using actual preprocessing output
void test_preprocessed_graph_integration() {
  // Build a valid five-vertex graph and select landmarks
  alt_graph graph = {0};
  alt_edge edges[] = {
      {0, 1, 2.0}, {1, 2, 3.0}, {2, 3, 4.0}, {3, 1, 1.0}, {4, 0, 5.0}};
  assert(graph_build(5, edges, 5, &graph) == ALT_OK);
  alt_vertex_id landmarks[] = {4, 1};

  // Preprocess landmarks and verify success
  alt_landmark_distances distances = {0};
  assert(landmark_preprocess(&graph, landmarks, 2, &distances) == ALT_OK);

  // Evaluate heuristics query A and verify success and output
  double estimate = 0.0;
  assert(alt_heuristic_evaluate(&distances, 0, 3, &estimate) == ALT_OK);
  assert(estimate == 9.0);

  // Evaluate heuristics query B and verify success and output
  assert(alt_heuristic_evaluate(&distances, 3, 1, &estimate) == ALT_OK);
  assert(estimate == 1.0);

  // Cleanup test allocations
  landmark_distances_destroy(&distances);
  graph_destroy(&graph);
}

// Executes tests
int main() {
  // Execute tests
  test_forward_bound();
  test_reverse_bound();
  test_multiple_landmarks_maximum();
  test_negative_bounds_clamped();
  test_same_vertex();
  test_zero_landmarks();
  test_forward_unreachable();
  test_reverse_unreachable();
  test_no_usable_bounds();
  test_fractional_distances();
  test_null_distances();
  test_null_output();
  test_invalid_current_vertex();
  test_invalid_target_vertex();
  test_preprocessed_graph_integration();

  // All tests passed
  return 0;
}
