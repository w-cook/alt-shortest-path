#include "../src/graph.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>

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

  return;
}

// Executes tests
int main() {
  // Execute tests
  test_unordered_edges();

  // Output test success
  printf("graph_tests executed and passed.\n");

  // All tests passed
  return 0;
}
