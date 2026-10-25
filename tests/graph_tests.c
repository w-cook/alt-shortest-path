#include "../src/graph.h"
#include <assert.h>
#include <stddef.h>

// Verify CSR construction from unordered input edges
void test_unordered_edges() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Construct the input edge array
  alt_edge edges[] = {{2, 3, 4.0}, {0, 1, 3.0}, {1, 2, 2.0}, {0, 2, 7.0}};

  // Build the graph
  alt_status result = graph_build(4, edges, 4, &graph);

  // Make sure the result status OK
  assert(result == ALT_OK);

  // Verify graph counts
  assert(graph.vertex_count == 4);
  assert(graph.edge_count == 4);

  // Verify graph arrays
  const size_t edge_starts[] = {0, 2, 3, 4, 4};
  const size_t targets[] = {1, 2, 2, 3};
  const double weights[] = {3.0, 7.0, 2.0, 4.0};
  const size_t original_edge_ids[] = {1, 3, 2, 0};
  for (size_t i = 0; i < graph.vertex_count + 1; i++) {
    assert(graph.edge_starts[i] == edge_starts[i]);
  }
  for (size_t i = 0; i < graph.edge_count; i++) {
    assert(graph.targets[i] == targets[i]);
    assert(graph.weights[i] == weights[i]);
    assert(graph.original_edge_ids[i] == original_edge_ids[i]);
  }

  // Destroy the graph
  graph_destroy(&graph);

  // Verify graph's counts and pointers reset
  assert(graph.vertex_count == 0);
  assert(graph.edge_count == 0);
  assert(graph.edge_starts == NULL);
  assert(graph.targets == NULL);
  assert(graph.weights == NULL);
  assert(graph.original_edge_ids == NULL);
}

// Verify construction with vertices but no edges
void test_zero_edges() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Build the graph
  alt_status result = graph_build(5, NULL, 0, &graph);

  // Verify that construction succeeded
  assert(result == ALT_OK);

  // Verify the graph counts
  assert(graph.vertex_count == 5);
  assert(graph.edge_count == 0);

  // Verify that edge_starts was allocated
  assert(graph.edge_starts != NULL);

  // Verify all six CSR offsets are zero
  for (size_t i = 0; i < graph.vertex_count + 1; i++) {
    assert(graph.edge_starts[i] == 0);
  }

  // Verify the three edge-data pointers remain null
  assert(graph.targets == NULL);
  assert(graph.weights == NULL);
  assert(graph.original_edge_ids == NULL);

  // Destroy the graph
  graph_destroy(&graph);

  // Verify the graph has been reset
  assert(graph.vertex_count == 0);
  assert(graph.edge_count == 0);
  assert(graph.edge_starts == NULL);
  assert(graph.targets == NULL);
  assert(graph.weights == NULL);
  assert(graph.original_edge_ids == NULL);
}

// Verify vertices without outgoing edges
void test_isolated_vertices() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Construct the input edge array
  alt_edge edges[] = {
      {4, 5, 2.5}, {1, 2, 3.0}, {5, 4, 1.5}, {2, 1, 4.0}, {1, 4, 6.0}};

  // Build the graph
  alt_status result = graph_build(7, edges, 5, &graph);

  // Make sure the result status OK
  assert(result == ALT_OK);

  // Verify graph counts
  assert(graph.vertex_count == 7);
  assert(graph.edge_count == 5);

  // Verify graph arrays
  const size_t edge_starts[] = {0, 0, 2, 3, 3, 4, 5, 5};
  const size_t targets[] = {2, 4, 1, 5, 4};
  const double weights[] = {3.0, 6.0, 4.0, 2.5, 1.5};
  const size_t original_edge_ids[] = {1, 4, 3, 0, 2};
  for (size_t i = 0; i < graph.vertex_count + 1; i++) {
    assert(graph.edge_starts[i] == edge_starts[i]);
  }
  for (size_t i = 0; i < graph.edge_count; i++) {
    assert(graph.targets[i] == targets[i]);
    assert(graph.weights[i] == weights[i]);
    assert(graph.original_edge_ids[i] == original_edge_ids[i]);
  }

  // Destroy the graph
  graph_destroy(&graph);

  // Verify graph's counts and pointers reset
  assert(graph.vertex_count == 0);
  assert(graph.edge_count == 0);
  assert(graph.edge_starts == NULL);
  assert(graph.targets == NULL);
  assert(graph.weights == NULL);
  assert(graph.original_edge_ids == NULL);
}

// Verify edges whose source and destination are identical
void test_self_loops() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Construct the input edge array
  alt_edge edges[] = {{3, 3, 2.5}, {1, 4, 3.0},  {0, 0, 1.0},
                      {3, 1, 4.0}, {4, 4, 0.75}, {1, 1, 2.0}};

  // Build the graph
  alt_status result = graph_build(5, edges, 6, &graph);

  // Make sure the result status OK
  assert(result == ALT_OK);

  // Verify graph counts
  assert(graph.vertex_count == 5);
  assert(graph.edge_count == 6);

  // Verify graph arrays
  const size_t edge_starts[] = {0, 1, 3, 3, 5, 6};
  const size_t targets[] = {0, 4, 1, 3, 1, 4};
  const double weights[] = {1.0, 3.0, 2.0, 2.5, 4.0, 0.75};
  const size_t original_edge_ids[] = {2, 1, 5, 0, 3, 4};
  for (size_t i = 0; i < graph.vertex_count + 1; i++) {
    assert(graph.edge_starts[i] == edge_starts[i]);
  }
  for (size_t i = 0; i < graph.edge_count; i++) {
    assert(graph.targets[i] == targets[i]);
    assert(graph.weights[i] == weights[i]);
    assert(graph.original_edge_ids[i] == original_edge_ids[i]);
  }

  // Destroy the graph
  graph_destroy(&graph);

  // Verify graph's counts and pointers reset
  assert(graph.vertex_count == 0);
  assert(graph.edge_count == 0);
  assert(graph.edge_starts == NULL);
  assert(graph.targets == NULL);
  assert(graph.weights == NULL);
  assert(graph.original_edge_ids == NULL);
}

// Verify multiple edges connecting the same vertices
void test_parallel_edges() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Construct the input edge array
  alt_edge edges[] = {{2, 3, 8.0}, {0, 1, 3.0}, {2, 3, 1.5}, {4, 0, 2.0},
                      {0, 1, 7.0}, {2, 1, 4.0}, {2, 3, 6.25}};

  // Build the graph
  alt_status result = graph_build(5, edges, 7, &graph);

  // Make sure the result status OK
  assert(result == ALT_OK);

  // Verify graph counts
  assert(graph.vertex_count == 5);
  assert(graph.edge_count == 7);

  // Verify graph arrays
  const size_t edge_starts[] = {0, 2, 2, 6, 6, 7};
  const size_t targets[] = {1, 1, 3, 3, 1, 3, 0};
  const double weights[] = {3.0, 7.0, 8.0, 1.5, 4.0, 6.25, 2.0};
  const size_t original_edge_ids[] = {1, 4, 0, 2, 5, 6, 3};
  for (size_t i = 0; i < graph.vertex_count + 1; i++) {
    assert(graph.edge_starts[i] == edge_starts[i]);
  }
  for (size_t i = 0; i < graph.edge_count; i++) {
    assert(graph.targets[i] == targets[i]);
    assert(graph.weights[i] == weights[i]);
    assert(graph.original_edge_ids[i] == original_edge_ids[i]);
  }

  // Destroy the graph
  graph_destroy(&graph);

  // Verify graph's counts and pointers reset
  assert(graph.vertex_count == 0);
  assert(graph.edge_count == 0);
  assert(graph.edge_starts == NULL);
  assert(graph.targets == NULL);
  assert(graph.weights == NULL);
  assert(graph.original_edge_ids == NULL);
}

// Verify that zero-weight edges are accepted and preserved
void test_zero_weight_edges() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Construct the input edge array
  alt_edge edges[] = {{3, 4, 0.0}, {0, 2, 5.0},  {2, 1, 0.0}, {0, 1, 0.0},
                      {4, 0, 2.5}, {2, 3, 1.25}, {1, 4, 0.0}};

  // Build the graph
  alt_status result = graph_build(5, edges, 7, &graph);

  // Make sure the result status OK
  assert(result == ALT_OK);

  // Verify graph counts
  assert(graph.vertex_count == 5);
  assert(graph.edge_count == 7);

  // Verify graph arrays
  const size_t edge_starts[] = {0, 2, 3, 5, 6, 7};
  const size_t targets[] = {2, 1, 4, 1, 3, 4, 0};
  const double weights[] = {5.0, 0.0, 0.0, 0.0, 1.25, 0.0, 2.5};
  const size_t original_edge_ids[] = {1, 3, 6, 2, 5, 0, 4};
  for (size_t i = 0; i < graph.vertex_count + 1; i++) {
    assert(graph.edge_starts[i] == edge_starts[i]);
  }
  for (size_t i = 0; i < graph.edge_count; i++) {
    assert(graph.targets[i] == targets[i]);
    assert(graph.weights[i] == weights[i]);
    assert(graph.original_edge_ids[i] == original_edge_ids[i]);
  }

  // Destroy the graph
  graph_destroy(&graph);

  // Verify graph's counts and pointers reset
  assert(graph.vertex_count == 0);
  assert(graph.edge_count == 0);
  assert(graph.edge_starts == NULL);
  assert(graph.targets == NULL);
  assert(graph.weights == NULL);
  assert(graph.original_edge_ids == NULL);
}

// Reject source and target IDs outside the graph
void test_invalid_vertex_ids() {}

// Reject negative, NaN, and infinite weights
void test_invalid_weights() {}

// Reject null pointers where required
void test_invalid_arguments() {}

// Reject graphs containing zero vertices
void test_invalid_vertex_count() {}

// Reject vertex counts that would overflow allocation sizes
void test_oversized_dimensions() {}

// Verify null-safe and repeated destruction
void test_destroy_safety() {}

// Executes tests
int main() {
  // Execute tests
  test_unordered_edges();
  test_zero_edges();
  test_isolated_vertices();
  test_self_loops();
  test_parallel_edges();
  test_zero_weight_edges();
  test_invalid_vertex_ids();
  test_invalid_weights();
  test_invalid_arguments();
  test_invalid_vertex_count();
  test_oversized_dimensions();
  test_destroy_safety();

  // All tests passed
  return 0;
}
