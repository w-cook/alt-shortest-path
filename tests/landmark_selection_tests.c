#include "../src/landmark_selection.h"
#include "alt/alt.h"
#include <assert.h>
#include <stdlib.h>

/* ==========================================================================
 * Explicit landmark-selection tests
 * ========================================================================== */

// Verify multiple valid landmark IDs are returned in their requested order,
// including non-sorted IDs
void test_explicit_multiple_landmarks() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Construct the five-vertex graph
  assert(graph_build(5, NULL, 0, &graph) == ALT_OK);

  // Prepare the alt_landmark_options
  alt_vertex_id landmark_vertices[] = {4, 1, 3};
  alt_landmark_options options = {ALT_LANDMARK_EXPLICIT, 3, landmark_vertices,
                                  0, 0};

  // Initialize a pointer to receive the selected landmarks
  alt_vertex_id *selected_landmarks = NULL;

  // Select landmarks and verify success
  assert(landmark_select(&graph, &options, &selected_landmarks) == ALT_OK);

  // Verify expected pointer and returned values
  assert(selected_landmarks != NULL);
  assert(selected_landmarks[0] == 4);
  assert(selected_landmarks[1] == 1);
  assert(selected_landmarks[2] == 3);

  // Verify input landmark IDs remain unchanged
  assert(landmark_vertices[0] == 4);
  assert(landmark_vertices[1] == 1);
  assert(landmark_vertices[2] == 3);

  // Cleanup test allocations
  free(selected_landmarks);
  graph_destroy(&graph);
}

// Verify a single valid landmark is selected correctly
void test_explicit_single_landmark() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Construct the five-vertex graph
  assert(graph_build(5, NULL, 0, &graph) == ALT_OK);

  // Prepare the alt_landmark_options
  alt_vertex_id landmark_vertices[] = {3};
  alt_landmark_options options = {ALT_LANDMARK_EXPLICIT, 1, landmark_vertices,
                                  0, 0};

  // Initialize a pointer to receive the selected landmarks
  alt_vertex_id *selected_landmarks = NULL;

  // Select landmarks and verify success
  assert(landmark_select(&graph, &options, &selected_landmarks) == ALT_OK);

  // Verify expected pointer and returned values
  assert(selected_landmarks != NULL);
  assert(selected_landmarks[0] == 3);

  // Verify input landmark ID remains unchanged
  assert(landmark_vertices[0] == 3);

  // Cleanup test allocations
  free(selected_landmarks);
  graph_destroy(&graph);
}

// Verify selecting every graph vertex succeeds
void test_explicit_all_vertices() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Construct the five-vertex graph
  assert(graph_build(5, NULL, 0, &graph) == ALT_OK);

  // Prepare the alt_landmark_options
  alt_vertex_id landmark_vertices[] = {4, 2, 0, 3, 1};
  alt_landmark_options options = {ALT_LANDMARK_EXPLICIT, 5, landmark_vertices,
                                  0, 0};

  // Initialize a pointer to receive the selected landmarks
  alt_vertex_id *selected_landmarks = NULL;

  // Select landmarks and verify success
  assert(landmark_select(&graph, &options, &selected_landmarks) == ALT_OK);

  // Verify expected pointer and returned values
  assert(selected_landmarks != NULL);
  assert(selected_landmarks[0] == 4);
  assert(selected_landmarks[1] == 2);
  assert(selected_landmarks[2] == 0);
  assert(selected_landmarks[3] == 3);
  assert(selected_landmarks[4] == 1);

  // Verify input landmark IDs remain unchanged
  assert(landmark_vertices[0] == 4);
  assert(landmark_vertices[1] == 2);
  assert(landmark_vertices[2] == 0);
  assert(landmark_vertices[3] == 3);
  assert(landmark_vertices[4] == 1);

  // Cleanup test allocations
  free(selected_landmarks);
  graph_destroy(&graph);
}

// Verify the returned array is independent of the caller's input, and modifying
// one doesn't change the other
void test_explicit_independent_copy() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Construct the five-vertex graph
  assert(graph_build(5, NULL, 0, &graph) == ALT_OK);

  // Prepare the alt_landmark_options
  alt_vertex_id landmark_vertices[] = {4, 1, 3};
  alt_landmark_options options = {ALT_LANDMARK_EXPLICIT, 3, landmark_vertices,
                                  0, 0};

  // Initialize a pointer to receive the selected landmarks
  alt_vertex_id *selected_landmarks = NULL;

  // Select landmarks and verify success
  assert(landmark_select(&graph, &options, &selected_landmarks) == ALT_OK);

  // Verify expected pointer and returned values
  assert(selected_landmarks != NULL);
  assert(selected_landmarks[0] == 4);
  assert(selected_landmarks[1] == 1);
  assert(selected_landmarks[2] == 3);

  // Verify input landmark IDs remain unchanged
  assert(landmark_vertices[0] == 4);
  assert(landmark_vertices[1] == 1);
  assert(landmark_vertices[2] == 3);

  // Verify the returned pointer is different from the original array's address
  assert(selected_landmarks != landmark_vertices);

  // Modify the original array and verify the returned array has not changed
  landmark_vertices[0] = 2;
  assert(selected_landmarks[0] == 4);

  // Modify the returned array and verify the original array has not changed
  selected_landmarks[1] = 0;
  assert(landmark_vertices[1] == 1);

  // Cleanup test allocations
  free(selected_landmarks);
  graph_destroy(&graph);
}

// Verify a NULL vertex array with positive count returns
// ALT_INVALID_CONFIGURATION
void test_explicit_null_vertices() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Construct the five-vertex graph
  assert(graph_build(5, NULL, 0, &graph) == ALT_OK);

  // Prepare the alt_landmark_options
  alt_landmark_options options = {ALT_LANDMARK_EXPLICIT, 3, NULL, 0, 0};

  // Initialize a pointer to receive the selected landmarks
  alt_vertex_id initial_array[] = {1};
  alt_vertex_id *selected_landmarks = initial_array;

  // Select landmarks and verify expected failure
  assert(landmark_select(&graph, &options, &selected_landmarks) ==
         ALT_INVALID_CONFIGURATION);

  // Verify expected pointer
  assert(selected_landmarks == NULL);

  // Cleanup test allocations
  graph_destroy(&graph);
}

// Verify a vertex ID equal to or greater than vertex_count is rejected
void test_explicit_out_of_range_vertex() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Construct the five-vertex graph
  assert(graph_build(5, NULL, 0, &graph) == ALT_OK);

  // Prepare the alt_landmark_options
  alt_vertex_id landmark_vertices[] = {0, 5, 3};
  alt_landmark_options options = {ALT_LANDMARK_EXPLICIT, 3, landmark_vertices,
                                  0, 0};

  // Initialize a pointer to receive the selected landmarks
  alt_vertex_id *selected_landmarks = NULL;

  // Select landmarks and verify expected failure
  assert(landmark_select(&graph, &options, &selected_landmarks) ==
         ALT_INVALID_CONFIGURATION);

  // Verify expected pointer
  assert(selected_landmarks == NULL);

  // Verify input landmark IDs remain unchanged
  assert(landmark_vertices[0] == 0);
  assert(landmark_vertices[1] == 5);
  assert(landmark_vertices[2] == 3);

  // Cleanup test allocations
  graph_destroy(&graph);
}

// Verify consecutive duplicate landmark IDs are rejected
void test_explicit_duplicate_adjacent() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Construct the five-vertex graph
  assert(graph_build(5, NULL, 0, &graph) == ALT_OK);

  // Prepare the alt_landmark_options
  alt_vertex_id landmark_vertices[] = {1, 1, 4};
  alt_landmark_options options = {ALT_LANDMARK_EXPLICIT, 3, landmark_vertices,
                                  0, 0};

  // Initialize a pointer to receive the selected landmarks
  alt_vertex_id *selected_landmarks = NULL;

  // Select landmarks and verify expected failure
  assert(landmark_select(&graph, &options, &selected_landmarks) ==
         ALT_INVALID_CONFIGURATION);

  // Verify expected pointer
  assert(selected_landmarks == NULL);

  // Verify input landmark IDs remain unchanged
  assert(landmark_vertices[0] == 1);
  assert(landmark_vertices[1] == 1);
  assert(landmark_vertices[2] == 4);

  // Cleanup test allocations
  graph_destroy(&graph);
}

// Verify duplicate IDs separated by other IDs are rejected
void test_explicit_duplicate_nonadjacent() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Construct the five-vertex graph
  assert(graph_build(5, NULL, 0, &graph) == ALT_OK);

  // Prepare the alt_landmark_options
  alt_vertex_id landmark_vertices[] = {2, 4, 1, 2};
  alt_landmark_options options = {ALT_LANDMARK_EXPLICIT, 4, landmark_vertices,
                                  0, 0};

  // Initialize a pointer to receive the selected landmarks
  alt_vertex_id *selected_landmarks = NULL;

  // Select landmarks and verify expected failure
  assert(landmark_select(&graph, &options, &selected_landmarks) ==
         ALT_INVALID_CONFIGURATION);

  // Verify expected pointer
  assert(selected_landmarks == NULL);

  // Verify input landmark IDs remain unchanged
  assert(landmark_vertices[0] == 2);
  assert(landmark_vertices[1] == 4);
  assert(landmark_vertices[2] == 1);
  assert(landmark_vertices[3] == 2);

  // Cleanup test allocations
  graph_destroy(&graph);
}

/* ==========================================================================
 * Shared validation and edge-case tests
 * ========================================================================== */

// Verify explicit selection with count zero succeeds, returning NULL without
// allocation
void test_zero_landmarks_explicit() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Construct the five-vertex graph
  assert(graph_build(5, NULL, 0, &graph) == ALT_OK);

  // Prepare the alt_landmark_options
  alt_landmark_options options = {ALT_LANDMARK_EXPLICIT, 0, NULL, 0, 0};

  // Initialize a pointer to receive the selected landmarks
  alt_vertex_id initial_array[] = {1};
  alt_vertex_id *selected_landmarks = initial_array;

  // Select landmarks and verify success
  assert(landmark_select(&graph, &options, &selected_landmarks) == ALT_OK);

  // Verify expected pointer
  assert(selected_landmarks == NULL);

  // Cleanup test allocations
  graph_destroy(&graph);
}

// Verify random selection with count zero succeeds, returning NULL
void test_zero_landmarks_random() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Construct the five-vertex graph
  assert(graph_build(5, NULL, 0, &graph) == ALT_OK);

  // Prepare the alt_landmark_options
  alt_landmark_options options = {ALT_LANDMARK_RANDOM, 0, NULL, 42, 0};

  // Initialize a pointer to receive the selected landmarks
  alt_vertex_id initial_array[] = {1};
  alt_vertex_id *selected_landmarks = initial_array;

  // Select landmarks and verify success
  assert(landmark_select(&graph, &options, &selected_landmarks) == ALT_OK);

  // Verify expected pointer
  assert(selected_landmarks == NULL);

  // Cleanup test allocations
  graph_destroy(&graph);
}

// Verify farthest-first selection with count zero succeeds, returning NULL
void test_zero_landmarks_farthest() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Construct the five-vertex graph
  assert(graph_build(5, NULL, 0, &graph) == ALT_OK);

  // Prepare the alt_landmark_options
  alt_landmark_options options = {ALT_LANDMARK_FARTHEST, 0, NULL, 0, 99};

  // Initialize a pointer to receive the selected landmarks
  alt_vertex_id initial_array[] = {1};
  alt_vertex_id *selected_landmarks = initial_array;

  // Select landmarks and verify success
  assert(landmark_select(&graph, &options, &selected_landmarks) == ALT_OK);

  // Verify expected pointer
  assert(selected_landmarks == NULL);

  // Cleanup test allocations
  graph_destroy(&graph);
}

// Verify NULL out_landmarks returns ALT_INVALID_ARGUMENT
void test_null_output_pointer() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Construct the five-vertex graph
  assert(graph_build(5, NULL, 0, &graph) == ALT_OK);

  // Prepare the alt_landmark_options
  alt_vertex_id landmark_vertices[] = {1, 3};
  alt_landmark_options options = {ALT_LANDMARK_EXPLICIT, 2, landmark_vertices,
                                  0, 0};

  // Select landmarks and verify expected failure
  assert(landmark_select(&graph, &options, NULL) == ALT_INVALID_ARGUMENT);

  // Cleanup test allocations
  graph_destroy(&graph);
}

// Verify NULL graph returns ALT_INVALID_ARGUMENT and clears a valid output
// destination
void test_null_graph() {
  // Prepare the alt_landmark_options
  alt_vertex_id landmark_vertices[] = {1, 3};
  alt_landmark_options options = {ALT_LANDMARK_EXPLICIT, 2, landmark_vertices,
                                  0, 0};

  // Initialize a pointer to receive the selected landmarks
  alt_vertex_id initial_array[] = {1};
  alt_vertex_id *selected_landmarks = initial_array;

  // Select landmarks and verify expected failure
  assert(landmark_select(NULL, &options, &selected_landmarks) ==
         ALT_INVALID_ARGUMENT);

  // Verify expected pointer
  assert(selected_landmarks == NULL);

  // Verify input landmark IDs remain unchanged
  assert(landmark_vertices[0] == 1);
  assert(landmark_vertices[1] == 3);
}

// Verify NULL options returns ALT_INVALID_ARGUMENT and clears a valid output
// destination
void test_null_options() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Construct the five-vertex graph
  assert(graph_build(5, NULL, 0, &graph) == ALT_OK);

  // Initialize a pointer to receive the selected landmarks
  alt_vertex_id initial_array[] = {1};
  alt_vertex_id *selected_landmarks = initial_array;

  // Select landmarks and verify expected failure
  assert(landmark_select(&graph, NULL, &selected_landmarks) ==
         ALT_INVALID_ARGUMENT);

  // Verify expected pointer
  assert(selected_landmarks == NULL);

  // Cleanup test allocations
  graph_destroy(&graph);
}

// Verify an unrecognized selection method returns ALT_INVALID_CONFIGURATION,
// including when count is zero
void test_invalid_method() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Construct the five-vertex graph
  assert(graph_build(5, NULL, 0, &graph) == ALT_OK);

  // Prepare the alt_landmark_options for case a
  alt_vertex_id landmark_vertices[] = {1, 3};
  alt_landmark_options options = {(alt_landmark_method)99, 2, landmark_vertices,
                                  0, 0};

  // Initialize a pointer to receive the selected landmarks
  alt_vertex_id initial_array[] = {1};
  alt_vertex_id *selected_landmarks = initial_array;

  // Select landmarks and verify expected failure
  assert(landmark_select(&graph, &options, &selected_landmarks) ==
         ALT_INVALID_CONFIGURATION);

  // Verify expected pointer
  assert(selected_landmarks == NULL);

  // Verify input landmark IDs remain unchanged
  assert(landmark_vertices[0] == 1);
  assert(landmark_vertices[1] == 3);

  // Prepare the alt_landmark_options for case b
  options = (alt_landmark_options){(alt_landmark_method)99, 0, NULL, 0, 0};

  // Reset the selected landmarks pointer
  selected_landmarks = initial_array;

  // Select landmarks and verify expected failure
  assert(landmark_select(&graph, &options, &selected_landmarks) ==
         ALT_INVALID_CONFIGURATION);

  // Verify expected pointer
  assert(selected_landmarks == NULL);

  // Cleanup test allocations
  graph_destroy(&graph);
}

// Verify requesting more landmarks than graph vertices returns
// ALT_INVALID_CONFIGURATION
void test_count_exceeds_vertex_count() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Construct the five-vertex graph
  assert(graph_build(5, NULL, 0, &graph) == ALT_OK);

  // Prepare the alt_landmark_options
  alt_vertex_id landmark_vertices[] = {0, 1, 2, 3, 4, 0};
  alt_landmark_options options = {ALT_LANDMARK_EXPLICIT, 6, landmark_vertices,
                                  0, 0};

  // Initialize a pointer to receive the selected landmarks
  alt_vertex_id initial_array[] = {1};
  alt_vertex_id *selected_landmarks = initial_array;

  // Select landmarks and verify expected failure
  assert(landmark_select(&graph, &options, &selected_landmarks) ==
         ALT_INVALID_CONFIGURATION);

  // Verify expected pointer
  assert(selected_landmarks == NULL);

  // Verify input landmark IDs remain unchanged
  assert(landmark_vertices[0] == 0);
  assert(landmark_vertices[1] == 1);
  assert(landmark_vertices[2] == 2);
  assert(landmark_vertices[3] == 3);
  assert(landmark_vertices[4] == 4);
  assert(landmark_vertices[5] == 0);

  // Cleanup test allocations
  graph_destroy(&graph);
}

// Verify a method-specific validation failure sets a valid output destination
// to NULL
void test_failure_clears_output() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Construct the five-vertex graph
  assert(graph_build(5, NULL, 0, &graph) == ALT_OK);

  // Prepare the alt_landmark_options
  alt_vertex_id landmark_vertices[] = {2, 4, 2};
  alt_landmark_options options = {ALT_LANDMARK_EXPLICIT, 3, landmark_vertices,
                                  0, 0};

  // Initialize a pointer to receive the selected landmarks
  alt_vertex_id initial_array[] = {1};
  alt_vertex_id *selected_landmarks = initial_array;

  // Select landmarks and verify expected failure
  assert(landmark_select(&graph, &options, &selected_landmarks) ==
         ALT_INVALID_CONFIGURATION);

  // Verify expected pointer
  assert(selected_landmarks == NULL);

  // Verify input landmark IDs remain unchanged
  assert(landmark_vertices[0] == 2);
  assert(landmark_vertices[1] == 4);
  assert(landmark_vertices[2] == 2);

  // Cleanup test allocations
  graph_destroy(&graph);
}

/* ==========================================================================
 * Random selection tests
 * ========================================================================== */

// Verify produces the requested number of valid, distinct vertices
void test_random_valid_selection() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Construct the ten-vertex graph
  assert(graph_build(10, NULL, 0, &graph) == ALT_OK);

  // Prepare the alt_landmark_options
  alt_landmark_options options = {ALT_LANDMARK_RANDOM, 4, NULL, 42, 0};

  // Initialize a pointer to receive the selected landmarks
  alt_vertex_id *selected_landmarks = NULL;

  // Select landmarks and verify success
  assert(landmark_select(&graph, &options, &selected_landmarks) == ALT_OK);

  // Verify expected pointer and returned values
  assert(selected_landmarks != NULL);
  for (size_t landmark_i = 0; landmark_i < options.count; landmark_i++) {
    assert(selected_landmarks[landmark_i] < graph.vertex_count);

    for (size_t compare_landmark_i = 0; compare_landmark_i < landmark_i;
         compare_landmark_i++) {
      assert(selected_landmarks[landmark_i] !=
             selected_landmarks[compare_landmark_i]);
    }
  }

  // Cleanup test allocations
  free(selected_landmarks);
  graph_destroy(&graph);
}

// Verify identical seeds and graph configuration produce identical results
void test_random_reproducible_seed() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Construct the ten-vertex graph
  assert(graph_build(10, NULL, 0, &graph) == ALT_OK);

  // Prepare the alt_landmark_options
  alt_landmark_options options = {ALT_LANDMARK_RANDOM, 4, NULL, 42, 0};

  // Initialize two pointers to receive the selected landmarks
  alt_vertex_id *selected_landmarks_one = NULL;
  alt_vertex_id *selected_landmarks_two = NULL;

  // Select landmarks twice with the same graph and options and verify success
  assert(landmark_select(&graph, &options, &selected_landmarks_one) == ALT_OK);
  assert(landmark_select(&graph, &options, &selected_landmarks_two) == ALT_OK);

  // Verify expected pointer and returned values
  assert(selected_landmarks_one != NULL);
  assert(selected_landmarks_two != NULL);
  for (size_t landmark_i = 0; landmark_i < options.count; landmark_i++) {
    assert(selected_landmarks_one[landmark_i] ==
           selected_landmarks_two[landmark_i]);
  }

  // Cleanup test allocations
  free(selected_landmarks_one);
  free(selected_landmarks_two);
  graph_destroy(&graph);
}

// Verify selecting all vertices produces a permutation without duplicates or
// omissions
void test_random_all_vertices() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Construct the seven-vertex graph
  assert(graph_build(7, NULL, 0, &graph) == ALT_OK);

  // Prepare the alt_landmark_options
  alt_landmark_options options = {ALT_LANDMARK_RANDOM, 7, NULL, 12345, 0};

  // Initialize a pointer to receive the selected landmarks
  alt_vertex_id *selected_landmarks = NULL;

  // Select landmarks and verify success
  assert(landmark_select(&graph, &options, &selected_landmarks) == ALT_OK);

  // Verify expected pointer and returned values
  // If the output contains seven distinct, valid IDs drawn from a seven-vertex
  // graph, that proves no vertex is missing
  assert(selected_landmarks != NULL);
  for (size_t landmark_i = 0; landmark_i < options.count; landmark_i++) {
    assert(selected_landmarks[landmark_i] < graph.vertex_count);

    for (size_t compare_landmark_i = 0; compare_landmark_i < landmark_i;
         compare_landmark_i++) {
      assert(selected_landmarks[landmark_i] !=
             selected_landmarks[compare_landmark_i]);
    }
  }

  // Cleanup test allocations
  free(selected_landmarks);
  graph_destroy(&graph);
}

// Verify correct behavior with count one
void test_random_single_landmark() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Construct the five-vertex graph
  assert(graph_build(5, NULL, 0, &graph) == ALT_OK);

  // Prepare the alt_landmark_options
  alt_landmark_options options = {ALT_LANDMARK_RANDOM, 1, NULL, 0, 0};

  // Initialize a pointer to receive the selected landmarks
  alt_vertex_id *selected_landmarks = NULL;

  // Select landmarks and verify success
  assert(landmark_select(&graph, &options, &selected_landmarks) == ALT_OK);

  // Verify expected pointer and returned values
  assert(selected_landmarks != NULL);
  assert(selected_landmarks[0] < graph.vertex_count);

  // Cleanup test allocations
  free(selected_landmarks);
  graph_destroy(&graph);
}

// Verify returned arrays have independent ownership
void test_random_independent_results() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Construct the five-vertex graph
  assert(graph_build(5, NULL, 0, &graph) == ALT_OK);

  // Prepare the alt_landmark_options
  alt_landmark_options options = {ALT_LANDMARK_RANDOM, 3, NULL, 42, 0};

  // Initialize two pointers to receive the selected landmarks from two
  // different calls
  alt_vertex_id *selected_landmarks_one = NULL;
  alt_vertex_id *selected_landmarks_two = NULL;

  // Select landmarks and verify success
  assert(landmark_select(&graph, &options, &selected_landmarks_one) == ALT_OK);
  assert(landmark_select(&graph, &options, &selected_landmarks_two) == ALT_OK);

  // Verify expected pointers and identical values
  assert(selected_landmarks_one != NULL);
  assert(selected_landmarks_two != NULL);
  assert(selected_landmarks_one != selected_landmarks_two);
  for (size_t landmark_i = 0; landmark_i < options.count; landmark_i++) {
    assert(selected_landmarks_one[landmark_i] ==
           selected_landmarks_two[landmark_i]);
  }

  // Verify modifying one element of the first array does not change the
  // corresponding element in the second array
  alt_vertex_id unmodified_first_landmark = selected_landmarks_two[0];
  selected_landmarks_one[0]++;
  assert(selected_landmarks_two[0] == unmodified_first_landmark);

  // Cleanup test allocations
  free(selected_landmarks_one);
  free(selected_landmarks_two);
  graph_destroy(&graph);
}

/* ==========================================================================
 * Farthest-first selection tests
 * ========================================================================== */

// Verify correct selection on a small weighted graph with known distances
void test_farthest_basic_selection() {}

// Verify correct initial landmark selection based on the specified anchor
void test_farthest_anchor_behavior() {}

// Verify equal-distance candidates are resolved by stable vertex ordering
void test_farthest_deterministic_ties() {}

// Verify uncovered connected components receive the specified priority
void test_farthest_disconnected_graph() {}

// Verify landmark placement uses the undirected projection, even for asymmetric
// directed edges
void test_farthest_undirected_projection() {}

// Verify an out-of-range anchor is rejected for positive count
void test_farthest_invalid_anchor() {}

// Verify every vertex can be selected exactly once
void test_farthest_all_vertices() {}

// Executes tests
int main() {
  // Execute tests
  // Explicit landmark-selection tests
  test_explicit_multiple_landmarks();
  test_explicit_single_landmark();
  test_explicit_all_vertices();
  test_explicit_independent_copy();
  test_explicit_null_vertices();
  test_explicit_out_of_range_vertex();
  test_explicit_duplicate_adjacent();
  test_explicit_duplicate_nonadjacent();
  // Shared validation and edge-case tests
  test_zero_landmarks_explicit();
  test_zero_landmarks_random();
  test_zero_landmarks_farthest();
  test_null_output_pointer();
  test_null_graph();
  test_null_options();
  test_invalid_method();
  test_count_exceeds_vertex_count();
  test_failure_clears_output();
  // Random selection tests
  test_random_valid_selection();
  test_random_reproducible_seed();
  test_random_all_vertices();
  test_random_single_landmark();
  test_random_independent_results();
  // Farthest-first selection tests
  test_farthest_basic_selection();
  test_farthest_anchor_behavior();
  test_farthest_deterministic_ties();
  test_farthest_disconnected_graph();
  test_farthest_undirected_projection();
  test_farthest_invalid_anchor();
  test_farthest_all_vertices();

  // All tests passed
  return 0;
}
