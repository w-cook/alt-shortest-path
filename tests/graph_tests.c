#include "../src/graph.h"
#include "alt/alt.h"
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* ==========================================================================
 * graph_build tests
 * ========================================================================== */

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
void test_invalid_vertex_ids() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Construct the input edge array where source = vertex count
  alt_edge edges[] = {{4, 1, 2.0}};

  // Build the graph
  alt_status result = graph_build(4, edges, 1, &graph);

  // Make sure the result status ALT_INVALID_GRAPH
  assert(result == ALT_INVALID_GRAPH);

  // Verify graph's counts and pointers are still zero-initialized
  assert(graph.vertex_count == 0);
  assert(graph.edge_count == 0);
  assert(graph.edge_starts == NULL);
  assert(graph.targets == NULL);
  assert(graph.weights == NULL);
  assert(graph.original_edge_ids == NULL);

  // Redefine the input edge array where source > vertex count
  edges[0] = (alt_edge){7, 1, 2.0};

  // Build the graph
  result = graph_build(4, edges, 1, &graph);

  // Make sure the result status ALT_INVALID_GRAPH
  assert(result == ALT_INVALID_GRAPH);

  // Verify graph's counts and pointers are still zero-initialized
  assert(graph.vertex_count == 0);
  assert(graph.edge_count == 0);
  assert(graph.edge_starts == NULL);
  assert(graph.targets == NULL);
  assert(graph.weights == NULL);
  assert(graph.original_edge_ids == NULL);

  // Redefine the input edge array where target = vertex count
  edges[0] = (alt_edge){1, 4, 2.0};

  // Build the graph
  result = graph_build(4, edges, 1, &graph);

  // Make sure the result status ALT_INVALID_GRAPH
  assert(result == ALT_INVALID_GRAPH);

  // Verify graph's counts and pointers are still zero-initialized
  assert(graph.vertex_count == 0);
  assert(graph.edge_count == 0);
  assert(graph.edge_starts == NULL);
  assert(graph.targets == NULL);
  assert(graph.weights == NULL);
  assert(graph.original_edge_ids == NULL);

  // Redefine the input edge array where target > vertex count
  edges[0] = (alt_edge){1, 7, 2.0};

  // Build the graph
  result = graph_build(4, edges, 1, &graph);

  // Make sure the result status ALT_INVALID_GRAPH
  assert(result == ALT_INVALID_GRAPH);

  // Verify graph's counts and pointers are still zero-initialized
  assert(graph.vertex_count == 0);
  assert(graph.edge_count == 0);
  assert(graph.edge_starts == NULL);
  assert(graph.targets == NULL);
  assert(graph.weights == NULL);
  assert(graph.original_edge_ids == NULL);
}

// Reject negative, NaN, and infinite weights
void test_invalid_weights() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Construct the input edge array where edge has negative weight
  alt_edge edges[] = {{0, 1, -2.0}};

  // Build the graph
  alt_status result = graph_build(4, edges, 1, &graph);

  // Make sure the result status ALT_INVALID_GRAPH
  assert(result == ALT_INVALID_GRAPH);

  // Verify graph's counts and pointers are still zero-initialized
  assert(graph.vertex_count == 0);
  assert(graph.edge_count == 0);
  assert(graph.edge_starts == NULL);
  assert(graph.targets == NULL);
  assert(graph.weights == NULL);
  assert(graph.original_edge_ids == NULL);

  // Redefine the input edge array where edge weight is not a number
  edges[0] = (alt_edge){0, 1, NAN};

  // Build the graph
  result = graph_build(4, edges, 1, &graph);

  // Make sure the result status ALT_INVALID_GRAPH
  assert(result == ALT_INVALID_GRAPH);

  // Verify graph's counts and pointers are still zero-initialized
  assert(graph.vertex_count == 0);
  assert(graph.edge_count == 0);
  assert(graph.edge_starts == NULL);
  assert(graph.targets == NULL);
  assert(graph.weights == NULL);
  assert(graph.original_edge_ids == NULL);

  // Redefine the input edge array where edge weight is positive infinity
  edges[0] = (alt_edge){0, 1, INFINITY};

  // Build the graph
  result = graph_build(4, edges, 1, &graph);

  // Make sure the result status ALT_INVALID_GRAPH
  assert(result == ALT_INVALID_GRAPH);

  // Verify graph's counts and pointers are still zero-initialized
  assert(graph.vertex_count == 0);
  assert(graph.edge_count == 0);
  assert(graph.edge_starts == NULL);
  assert(graph.targets == NULL);
  assert(graph.weights == NULL);
  assert(graph.original_edge_ids == NULL);

  // Redefine the input edge array where edge weight is negative infinity
  edges[0] = (alt_edge){0, 1, -INFINITY};

  // Build the graph
  result = graph_build(4, edges, 1, &graph);

  // Make sure the result status ALT_INVALID_GRAPH
  assert(result == ALT_INVALID_GRAPH);

  // Verify graph's counts and pointers are still zero-initialized
  assert(graph.vertex_count == 0);
  assert(graph.edge_count == 0);
  assert(graph.edge_starts == NULL);
  assert(graph.targets == NULL);
  assert(graph.weights == NULL);
  assert(graph.original_edge_ids == NULL);
}

// Reject null pointers where required
void test_invalid_arguments() {
  // Construct the input edge array
  alt_edge edges[] = {{0, 1, 2.0}};

  // Build the graph with a null output pointer
  alt_status result = graph_build(4, edges, 1, NULL);

  // Make sure the result status ALT_INVALID_ARGUMENT
  assert(result == ALT_INVALID_ARGUMENT);

  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Build the graph with a null edge pointer and an edge_count of one
  result = graph_build(4, NULL, 1, &graph);

  // Make sure the result status ALT_INVALID_ARGUMENT
  assert(result == ALT_INVALID_ARGUMENT);

  // Verify graph's counts and pointers are still zero-initialized
  assert(graph.vertex_count == 0);
  assert(graph.edge_count == 0);
  assert(graph.edge_starts == NULL);
  assert(graph.targets == NULL);
  assert(graph.weights == NULL);
  assert(graph.original_edge_ids == NULL);
}

// Reject graphs containing zero vertices
void test_invalid_vertex_count() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Build the graph with zero vertices
  alt_status result = graph_build(0, NULL, 0, &graph);

  // Make sure the result status ALT_INVALID_GRAPH
  assert(result == ALT_INVALID_GRAPH);

  // Verify graph's counts and pointers are still zero-initialized
  assert(graph.vertex_count == 0);
  assert(graph.edge_count == 0);
  assert(graph.edge_starts == NULL);
  assert(graph.targets == NULL);
  assert(graph.weights == NULL);
  assert(graph.original_edge_ids == NULL);
}

// Reject graph dimensions that would overflow allocation sizes
void test_oversized_dimensions() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Build the graph with maximum representable vertex count
  alt_status result = graph_build(SIZE_MAX, NULL, 0, &graph);

  // Make sure the result status ALT_INVALID_GRAPH
  assert(result == ALT_INVALID_GRAPH);

  // Verify graph's counts and pointers are still zero-initialized
  assert(graph.vertex_count == 0);
  assert(graph.edge_count == 0);
  assert(graph.edge_starts == NULL);
  assert(graph.targets == NULL);
  assert(graph.weights == NULL);
  assert(graph.original_edge_ids == NULL);

  // Build the graph with vertex count that would cause allocation overflow
  result = graph_build(SIZE_MAX / sizeof(size_t), NULL, 0, &graph);

  // Make sure the result status ALT_OUT_OF_MEMORY
  assert(result == ALT_OUT_OF_MEMORY);

  // Verify graph's counts and pointers are still zero-initialized
  assert(graph.vertex_count == 0);
  assert(graph.edge_count == 0);
  assert(graph.edge_starts == NULL);
  assert(graph.targets == NULL);
  assert(graph.weights == NULL);
  assert(graph.original_edge_ids == NULL);

  // Construct the input edge array
  alt_edge edges[] = {{0, 1, 2.0}};

  // Build the graph with edge count that would cause allocation overflow
  result = graph_build(4, edges, SIZE_MAX, &graph);

  // Make sure the result status ALT_OUT_OF_MEMORY
  assert(result == ALT_OUT_OF_MEMORY);

  // Verify graph's counts and pointers are still zero-initialized
  assert(graph.vertex_count == 0);
  assert(graph.edge_count == 0);
  assert(graph.edge_starts == NULL);
  assert(graph.targets == NULL);
  assert(graph.weights == NULL);
  assert(graph.original_edge_ids == NULL);
}

/* ==========================================================================
 * graph_build_reverse tests
 * ========================================================================== */

// Verify correct reversal of an unordered directed graph, including exact CSR
// arrays and original edge IDs
void test_reverse_basic() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Construct the input edge array
  alt_edge edges[] = {{2, 3, 4.0}, {0, 3, 3.0}, {1, 2, 2.0}, {0, 2, 7.0}};

  // Build the graph
  alt_status result = graph_build(4, edges, 4, &graph);

  // Make sure the result status OK
  assert(result == ALT_OK);

  // Initialize a second empty alt_graph to receive the reverse graph
  alt_graph graph_reverse = {0};

  // Construct the reverse graph
  result = graph_build_reverse(&graph, &graph_reverse);

  // Make sure the result status OK
  assert(result == ALT_OK);

  // Verify reverse graph's counts
  assert(graph_reverse.vertex_count == 4);
  assert(graph_reverse.edge_count == 4);

  // Verify reverse graph's arrays
  const size_t reverse_edge_starts[] = {0, 0, 0, 2, 4};
  const size_t reverse_targets[] = {0, 1, 0, 2};
  const double reverse_weights[] = {7.0, 2.0, 3.0, 4.0};
  const size_t reverse_original_edge_ids[] = {3, 2, 1, 0};
  for (size_t i = 0; i < graph_reverse.vertex_count + 1; i++) {
    assert(graph_reverse.edge_starts[i] == reverse_edge_starts[i]);
  }
  for (size_t i = 0; i < graph_reverse.edge_count; i++) {
    assert(graph_reverse.targets[i] == reverse_targets[i]);
    assert(graph_reverse.weights[i] == reverse_weights[i]);
    assert(graph_reverse.original_edge_ids[i] == reverse_original_edge_ids[i]);
  }

  // Verify original graph's counts
  assert(graph.vertex_count == 4);
  assert(graph.edge_count == 4);

  // Verify original graph's arrays
  const size_t original_edge_starts[] = {0, 2, 3, 4, 4};
  const size_t original_targets[] = {3, 2, 2, 3};
  const double original_weights[] = {3.0, 7.0, 2.0, 4.0};
  const size_t expected_original_edge_ids[] = {1, 3, 2, 0};
  for (size_t i = 0; i < graph.vertex_count + 1; i++) {
    assert(graph.edge_starts[i] == original_edge_starts[i]);
  }
  for (size_t i = 0; i < graph.edge_count; i++) {
    assert(graph.targets[i] == original_targets[i]);
    assert(graph.weights[i] == original_weights[i]);
    assert(graph.original_edge_ids[i] == expected_original_edge_ids[i]);
  }

  // Destroy both graphs
  graph_destroy(&graph);
  graph_destroy(&graph_reverse);

  // Verify both graphs' counts and pointers reset
  assert(graph.vertex_count == 0);
  assert(graph.edge_count == 0);
  assert(graph.edge_starts == NULL);
  assert(graph.targets == NULL);
  assert(graph.weights == NULL);
  assert(graph.original_edge_ids == NULL);
  assert(graph_reverse.vertex_count == 0);
  assert(graph_reverse.edge_count == 0);
  assert(graph_reverse.edge_starts == NULL);
  assert(graph_reverse.targets == NULL);
  assert(graph_reverse.weights == NULL);
  assert(graph_reverse.original_edge_ids == NULL);
}

// Verify reversal of graphs containing vertices but no edges, including a
// single-vertex graph
void test_reverse_zero_edges() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Build the graph with one vertex and no edges
  alt_status result = graph_build(1, NULL, 0, &graph);

  // Make sure the result status OK
  assert(result == ALT_OK);

  // Initialize a second empty alt_graph to receive the reverse graph
  alt_graph graph_reverse = {0};

  // Construct the reverse graph
  result = graph_build_reverse(&graph, &graph_reverse);

  // Make sure the result status OK
  assert(result == ALT_OK);

  // Verify reverse graph's counts
  assert(graph_reverse.vertex_count == 1);
  assert(graph_reverse.edge_count == 0);

  // Verify reverse graph's arrays
  assert(graph_reverse.edge_starts != NULL);
  for (size_t i = 0; i < graph_reverse.vertex_count + 1; i++) {
    assert(graph_reverse.edge_starts[i] == 0);
  }
  assert(graph_reverse.targets == NULL);
  assert(graph_reverse.weights == NULL);
  assert(graph_reverse.original_edge_ids == NULL);

  // Verify original graph's counts
  assert(graph.vertex_count == 1);
  assert(graph.edge_count == 0);

  // Verify original graph's arrays
  assert(graph.edge_starts != NULL);
  for (size_t i = 0; i < graph.vertex_count + 1; i++) {
    assert(graph.edge_starts[i] == 0);
  }
  assert(graph.targets == NULL);
  assert(graph.weights == NULL);
  assert(graph.original_edge_ids == NULL);

  // Destroy both graphs
  graph_destroy(&graph);
  graph_destroy(&graph_reverse);

  // Verify both graphs' counts and pointers reset
  assert(graph.vertex_count == 0);
  assert(graph.edge_count == 0);
  assert(graph.edge_starts == NULL);
  assert(graph.targets == NULL);
  assert(graph.weights == NULL);
  assert(graph.original_edge_ids == NULL);
  assert(graph_reverse.vertex_count == 0);
  assert(graph_reverse.edge_count == 0);
  assert(graph_reverse.edge_starts == NULL);
  assert(graph_reverse.targets == NULL);
  assert(graph_reverse.weights == NULL);
  assert(graph_reverse.original_edge_ids == NULL);

  // Build the graph with five vertices and no edges
  result = graph_build(5, NULL, 0, &graph);

  // Make sure the result status OK
  assert(result == ALT_OK);

  // Construct the reverse graph
  result = graph_build_reverse(&graph, &graph_reverse);

  // Make sure the result status OK
  assert(result == ALT_OK);

  // Verify reverse graph's counts
  assert(graph_reverse.vertex_count == 5);
  assert(graph_reverse.edge_count == 0);

  // Verify reverse graph's arrays
  assert(graph_reverse.edge_starts != NULL);
  for (size_t i = 0; i < graph_reverse.vertex_count + 1; i++) {
    assert(graph_reverse.edge_starts[i] == 0);
  }
  assert(graph_reverse.targets == NULL);
  assert(graph_reverse.weights == NULL);
  assert(graph_reverse.original_edge_ids == NULL);

  // Verify original graph's counts
  assert(graph.vertex_count == 5);
  assert(graph.edge_count == 0);

  // Verify original graph's arrays
  assert(graph.edge_starts != NULL);
  for (size_t i = 0; i < graph.vertex_count + 1; i++) {
    assert(graph.edge_starts[i] == 0);
  }
  assert(graph.targets == NULL);
  assert(graph.weights == NULL);
  assert(graph.original_edge_ids == NULL);

  // Destroy both graphs
  graph_destroy(&graph);
  graph_destroy(&graph_reverse);

  // Verify both graphs' counts and pointers reset
  assert(graph.vertex_count == 0);
  assert(graph.edge_count == 0);
  assert(graph.edge_starts == NULL);
  assert(graph.targets == NULL);
  assert(graph.weights == NULL);
  assert(graph.original_edge_ids == NULL);
  assert(graph_reverse.vertex_count == 0);
  assert(graph_reverse.edge_count == 0);
  assert(graph_reverse.edge_starts == NULL);
  assert(graph_reverse.targets == NULL);
  assert(graph_reverse.weights == NULL);
  assert(graph_reverse.original_edge_ids == NULL);
}

// Verify reversal of vertices with no incoming edges, no outgoing edges, and
// disconnected components
void test_reverse_isolated_vertices() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Construct the input edge array
  alt_edge edges[] = {
      {4, 5, 5.5}, {0, 3, 2.5}, {2, 3, 4.0}, {5, 4, 0.75}, {0, 2, 1.5}};

  // Build the graph
  alt_status result = graph_build(7, edges, 5, &graph);

  // Make sure the result status OK
  assert(result == ALT_OK);

  // Initialize a second empty alt_graph to receive the reverse graph
  alt_graph graph_reverse = {0};

  // Construct the reverse graph
  result = graph_build_reverse(&graph, &graph_reverse);

  // Make sure the result status OK
  assert(result == ALT_OK);

  // Verify reverse graph's counts
  assert(graph_reverse.vertex_count == 7);
  assert(graph_reverse.edge_count == 5);

  // Verify reverse graph's arrays
  const size_t reverse_edge_starts[] = {0, 0, 0, 1, 3, 4, 5, 5};
  const size_t reverse_targets[] = {0, 0, 2, 5, 4};
  const double reverse_weights[] = {1.5, 2.5, 4.0, 0.75, 5.5};
  const size_t reverse_original_edge_ids[] = {4, 1, 2, 3, 0};
  for (size_t i = 0; i < graph_reverse.vertex_count + 1; i++) {
    assert(graph_reverse.edge_starts[i] == reverse_edge_starts[i]);
  }
  for (size_t i = 0; i < graph_reverse.edge_count; i++) {
    assert(graph_reverse.targets[i] == reverse_targets[i]);
    assert(graph_reverse.weights[i] == reverse_weights[i]);
    assert(graph_reverse.original_edge_ids[i] == reverse_original_edge_ids[i]);
  }

  // Verify original graph's counts
  assert(graph.vertex_count == 7);
  assert(graph.edge_count == 5);

  // Verify original graph's arrays
  const size_t original_edge_starts[] = {0, 2, 2, 3, 3, 4, 5, 5};
  const size_t original_targets[] = {3, 2, 3, 5, 4};
  const double original_weights[] = {2.5, 1.5, 4.0, 5.5, 0.75};
  const size_t expected_original_edge_ids[] = {1, 4, 2, 0, 3};
  for (size_t i = 0; i < graph.vertex_count + 1; i++) {
    assert(graph.edge_starts[i] == original_edge_starts[i]);
  }
  for (size_t i = 0; i < graph.edge_count; i++) {
    assert(graph.targets[i] == original_targets[i]);
    assert(graph.weights[i] == original_weights[i]);
    assert(graph.original_edge_ids[i] == expected_original_edge_ids[i]);
  }

  // Destroy both graphs
  graph_destroy(&graph);
  graph_destroy(&graph_reverse);

  // Verify both graphs' counts and pointers reset
  assert(graph.vertex_count == 0);
  assert(graph.edge_count == 0);
  assert(graph.edge_starts == NULL);
  assert(graph.targets == NULL);
  assert(graph.weights == NULL);
  assert(graph.original_edge_ids == NULL);
  assert(graph_reverse.vertex_count == 0);
  assert(graph_reverse.edge_count == 0);
  assert(graph_reverse.edge_starts == NULL);
  assert(graph_reverse.targets == NULL);
  assert(graph_reverse.weights == NULL);
  assert(graph_reverse.original_edge_ids == NULL);
}

// Verify upon reversal that self-loops remain self-loops, with weights and
// identities preserved
void test_reverse_self_loops() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Construct the input edge array
  alt_edge edges[] = {{3, 3, 2.5}, {1, 4, 3.0},  {0, 0, 1.0},
                      {3, 1, 4.0}, {4, 4, 0.75}, {1, 1, 2.0}};

  // Build the graph
  alt_status result = graph_build(5, edges, 6, &graph);

  // Make sure the result status OK
  assert(result == ALT_OK);

  // Initialize a second empty alt_graph to receive the reverse graph
  alt_graph graph_reverse = {0};

  // Construct the reverse graph
  result = graph_build_reverse(&graph, &graph_reverse);

  // Make sure the result status OK
  assert(result == ALT_OK);

  // Verify reverse graph's counts
  assert(graph_reverse.vertex_count == 5);
  assert(graph_reverse.edge_count == 6);

  // Verify reverse graph's arrays
  const size_t reverse_edge_starts[] = {0, 1, 3, 3, 4, 6};
  const size_t reverse_targets[] = {0, 1, 3, 3, 1, 4};
  const double reverse_weights[] = {1.0, 2.0, 4.0, 2.5, 3.0, 0.75};
  const size_t reverse_original_edge_ids[] = {2, 5, 3, 0, 1, 4};
  for (size_t i = 0; i < graph_reverse.vertex_count + 1; i++) {
    assert(graph_reverse.edge_starts[i] == reverse_edge_starts[i]);
  }
  for (size_t i = 0; i < graph_reverse.edge_count; i++) {
    assert(graph_reverse.targets[i] == reverse_targets[i]);
    assert(graph_reverse.weights[i] == reverse_weights[i]);
    assert(graph_reverse.original_edge_ids[i] == reverse_original_edge_ids[i]);
  }

  // Verify original graph's counts
  assert(graph.vertex_count == 5);
  assert(graph.edge_count == 6);

  // Verify original graph's arrays
  const size_t original_edge_starts[] = {0, 1, 3, 3, 5, 6};
  const size_t original_targets[] = {0, 4, 1, 3, 1, 4};
  const double original_weights[] = {1.0, 3.0, 2.0, 2.5, 4.0, 0.75};
  const size_t expected_original_edge_ids[] = {2, 1, 5, 0, 3, 4};
  for (size_t i = 0; i < graph.vertex_count + 1; i++) {
    assert(graph.edge_starts[i] == original_edge_starts[i]);
  }
  for (size_t i = 0; i < graph.edge_count; i++) {
    assert(graph.targets[i] == original_targets[i]);
    assert(graph.weights[i] == original_weights[i]);
    assert(graph.original_edge_ids[i] == expected_original_edge_ids[i]);
  }

  // Destroy both graphs
  graph_destroy(&graph);
  graph_destroy(&graph_reverse);

  // Verify both graphs' counts and pointers reset
  assert(graph.vertex_count == 0);
  assert(graph.edge_count == 0);
  assert(graph.edge_starts == NULL);
  assert(graph.targets == NULL);
  assert(graph.weights == NULL);
  assert(graph.original_edge_ids == NULL);
  assert(graph_reverse.vertex_count == 0);
  assert(graph_reverse.edge_count == 0);
  assert(graph_reverse.edge_starts == NULL);
  assert(graph_reverse.targets == NULL);
  assert(graph_reverse.weights == NULL);
  assert(graph_reverse.original_edge_ids == NULL);
}

// Verify upon reversal that parallel edges remain distinct and retain their
// individual weights and original IDs
void test_reverse_parallel_edges() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Construct the input edge array
  alt_edge edges[] = {{2, 3, 8.0}, {0, 1, 3.0}, {2, 3, 1.5}, {4, 0, 2.0},
                      {0, 1, 7.0}, {2, 1, 4.0}, {2, 3, 6.25}};

  // Build the graph
  alt_status result = graph_build(5, edges, 7, &graph);

  // Make sure the result status OK
  assert(result == ALT_OK);

  // Initialize a second empty alt_graph to receive the reverse graph
  alt_graph graph_reverse = {0};

  // Construct the reverse graph
  result = graph_build_reverse(&graph, &graph_reverse);

  // Make sure the result status OK
  assert(result == ALT_OK);

  // Verify reverse graph's counts
  assert(graph_reverse.vertex_count == 5);
  assert(graph_reverse.edge_count == 7);

  // Verify reverse graph's arrays
  const size_t reverse_edge_starts[] = {0, 1, 4, 4, 7, 7};
  const size_t reverse_targets[] = {4, 0, 0, 2, 2, 2, 2};
  const double reverse_weights[] = {2.0, 3.0, 7.0, 4.0, 8.0, 1.5, 6.25};
  const size_t reverse_original_edge_ids[] = {3, 1, 4, 5, 0, 2, 6};
  for (size_t i = 0; i < graph_reverse.vertex_count + 1; i++) {
    assert(graph_reverse.edge_starts[i] == reverse_edge_starts[i]);
  }
  for (size_t i = 0; i < graph_reverse.edge_count; i++) {
    assert(graph_reverse.targets[i] == reverse_targets[i]);
    assert(graph_reverse.weights[i] == reverse_weights[i]);
    assert(graph_reverse.original_edge_ids[i] == reverse_original_edge_ids[i]);
  }

  // Verify original graph's counts
  assert(graph.vertex_count == 5);
  assert(graph.edge_count == 7);

  // Verify original graph's arrays
  const size_t original_edge_starts[] = {0, 2, 2, 6, 6, 7};
  const size_t original_targets[] = {1, 1, 3, 3, 1, 3, 0};
  const double original_weights[] = {3.0, 7.0, 8.0, 1.5, 4.0, 6.25, 2.0};
  const size_t expected_original_edge_ids[] = {1, 4, 0, 2, 5, 6, 3};
  for (size_t i = 0; i < graph.vertex_count + 1; i++) {
    assert(graph.edge_starts[i] == original_edge_starts[i]);
  }
  for (size_t i = 0; i < graph.edge_count; i++) {
    assert(graph.targets[i] == original_targets[i]);
    assert(graph.weights[i] == original_weights[i]);
    assert(graph.original_edge_ids[i] == expected_original_edge_ids[i]);
  }

  // Destroy both graphs
  graph_destroy(&graph);
  graph_destroy(&graph_reverse);

  // Verify both graphs' counts and pointers reset
  assert(graph.vertex_count == 0);
  assert(graph.edge_count == 0);
  assert(graph.edge_starts == NULL);
  assert(graph.targets == NULL);
  assert(graph.weights == NULL);
  assert(graph.original_edge_ids == NULL);
  assert(graph_reverse.vertex_count == 0);
  assert(graph_reverse.edge_count == 0);
  assert(graph_reverse.edge_starts == NULL);
  assert(graph_reverse.targets == NULL);
  assert(graph_reverse.weights == NULL);
  assert(graph_reverse.original_edge_ids == NULL);
}

// Verify correct reversal preservation of zero-weight edges, including edges
// participating in cycles
void test_reverse_zero_weight_edges() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Construct the input edge array
  alt_edge edges[] = {{3, 4, 0.0}, {0, 2, 5.0},  {2, 1, 0.0}, {0, 1, 0.0},
                      {4, 0, 2.5}, {2, 3, 1.25}, {1, 4, 0.0}};

  // Build the graph
  alt_status result = graph_build(5, edges, 7, &graph);

  // Make sure the result status OK
  assert(result == ALT_OK);

  // Initialize a second empty alt_graph to receive the reverse graph
  alt_graph graph_reverse = {0};

  // Construct the reverse graph
  result = graph_build_reverse(&graph, &graph_reverse);

  // Make sure the result status OK
  assert(result == ALT_OK);

  // Verify reverse graph's counts
  assert(graph_reverse.vertex_count == 5);
  assert(graph_reverse.edge_count == 7);

  // Verify reverse graph's arrays
  const size_t reverse_edge_starts[] = {0, 1, 3, 4, 5, 7};
  const size_t reverse_targets[] = {4, 0, 2, 0, 2, 1, 3};
  const double reverse_weights[] = {2.5, 0.0, 0.0, 5.0, 1.25, 0.0, 0.0};
  const size_t reverse_original_edge_ids[] = {4, 3, 2, 1, 5, 6, 0};
  for (size_t i = 0; i < graph_reverse.vertex_count + 1; i++) {
    assert(graph_reverse.edge_starts[i] == reverse_edge_starts[i]);
  }
  for (size_t i = 0; i < graph_reverse.edge_count; i++) {
    assert(graph_reverse.targets[i] == reverse_targets[i]);
    assert(graph_reverse.weights[i] == reverse_weights[i]);
    assert(graph_reverse.original_edge_ids[i] == reverse_original_edge_ids[i]);
  }

  // Verify original graph's counts
  assert(graph.vertex_count == 5);
  assert(graph.edge_count == 7);

  // Verify original graph's arrays
  const size_t original_edge_starts[] = {0, 2, 3, 5, 6, 7};
  const size_t original_targets[] = {2, 1, 4, 1, 3, 4, 0};
  const double original_weights[] = {5.0, 0.0, 0.0, 0.0, 1.25, 0.0, 2.5};
  const size_t expected_original_edge_ids[] = {1, 3, 6, 2, 5, 0, 4};
  for (size_t i = 0; i < graph.vertex_count + 1; i++) {
    assert(graph.edge_starts[i] == original_edge_starts[i]);
  }
  for (size_t i = 0; i < graph.edge_count; i++) {
    assert(graph.targets[i] == original_targets[i]);
    assert(graph.weights[i] == original_weights[i]);
    assert(graph.original_edge_ids[i] == expected_original_edge_ids[i]);
  }

  // Destroy both graphs
  graph_destroy(&graph);
  graph_destroy(&graph_reverse);

  // Verify both graphs' counts and pointers reset
  assert(graph.vertex_count == 0);
  assert(graph.edge_count == 0);
  assert(graph.edge_starts == NULL);
  assert(graph.targets == NULL);
  assert(graph.weights == NULL);
  assert(graph.original_edge_ids == NULL);
  assert(graph_reverse.vertex_count == 0);
  assert(graph_reverse.edge_count == 0);
  assert(graph_reverse.edge_starts == NULL);
  assert(graph_reverse.targets == NULL);
  assert(graph_reverse.weights == NULL);
  assert(graph_reverse.original_edge_ids == NULL);
}

// Verify correct reversal null-pointer rejection, source/destination alias
// rejection, and unchanged output after failure
void test_reverse_invalid_arguments() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Construct the input edge array
  alt_edge edges[] = {{0, 1, 2.0}};

  // Build the graph
  alt_status result = graph_build(3, edges, 1, &graph);

  // Make sure the result status OK
  assert(result == ALT_OK);

  // Verify graph's counts
  assert(graph.vertex_count == 3);
  assert(graph.edge_count == 1);

  // Verify graph's arrays
  const size_t original_edge_starts[] = {0, 1, 1, 1};
  const size_t original_targets[] = {1};
  const double original_weights[] = {2.0};
  const size_t expected_original_edge_ids[] = {0};
  for (size_t i = 0; i < graph.vertex_count + 1; i++) {
    assert(graph.edge_starts[i] == original_edge_starts[i]);
  }
  assert(graph.targets[0] == original_targets[0]);
  assert(graph.weights[0] == original_weights[0]);
  assert(graph.original_edge_ids[0] == expected_original_edge_ids[0]);

  // Initialize a second empty alt_graph to receive the reverse graph
  alt_graph graph_reverse = {0};

  // Construct the reverse graph with null source graph
  result = graph_build_reverse(NULL, &graph_reverse);

  // Make sure the result is failure
  assert(result == ALT_INVALID_ARGUMENT);

  // Make sure destination graph is still zero-initialized
  assert(graph_reverse.vertex_count == 0);
  assert(graph_reverse.edge_count == 0);
  assert(graph_reverse.edge_starts == NULL);
  assert(graph_reverse.targets == NULL);
  assert(graph_reverse.weights == NULL);
  assert(graph_reverse.original_edge_ids == NULL);

  // Construct the reverse graph with null destination graph
  result = graph_build_reverse(&graph, NULL);

  // Make sure the result is failure
  assert(result == ALT_INVALID_ARGUMENT);

  // Verify source graph's counts unchanged
  assert(graph.vertex_count == 3);
  assert(graph.edge_count == 1);

  // Verify source graph's arrays unchanged
  for (size_t i = 0; i < graph.vertex_count + 1; i++) {
    assert(graph.edge_starts[i] == original_edge_starts[i]);
  }
  assert(graph.targets[0] == original_targets[0]);
  assert(graph.weights[0] == original_weights[0]);
  assert(graph.original_edge_ids[0] == expected_original_edge_ids[0]);

  // Construct the reverse graph with null source graph and null destination
  // graph
  result = graph_build_reverse(NULL, NULL);

  // Make sure the result is failure
  assert(result == ALT_INVALID_ARGUMENT);

  // Construct the reverse graph with same source and destination graph
  result = graph_build_reverse(&graph, &graph);

  // Make sure the result is failure
  assert(result == ALT_INVALID_ARGUMENT);

  // Verify graph's counts unchanged
  assert(graph.vertex_count == 3);
  assert(graph.edge_count == 1);

  // Verify graph's arrays unchanged
  for (size_t i = 0; i < graph.vertex_count + 1; i++) {
    assert(graph.edge_starts[i] == original_edge_starts[i]);
  }
  assert(graph.targets[0] == original_targets[0]);
  assert(graph.weights[0] == original_weights[0]);
  assert(graph.original_edge_ids[0] == expected_original_edge_ids[0]);

  // Destroy the source graph
  graph_destroy(&graph);

  // Verify source graphs' counts and pointers reset
  assert(graph.vertex_count == 0);
  assert(graph.edge_count == 0);
  assert(graph.edge_starts == NULL);
  assert(graph.targets == NULL);
  assert(graph.weights == NULL);
  assert(graph.original_edge_ids == NULL);
}

// Verify correct reversal independent storage ownership, destruction, and
// preservation of the surviving graph
void test_reverse_independent_lifecycle() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Construct the input edge array
  alt_edge edges[] = {{0, 2, 5.0}, {2, 1, 7.0}};

  // Build the source graph
  alt_status result = graph_build(3, edges, 2, &graph);

  // Make sure the result status OK
  assert(result == ALT_OK);

  // Initialize a second empty alt_graph to receive the reverse graph
  alt_graph graph_reverse = {0};

  // Construct the reverse graph
  result = graph_build_reverse(&graph, &graph_reverse);

  // Make sure the result status OK
  assert(result == ALT_OK);

  // Verify reverse graph's counts
  assert(graph_reverse.vertex_count == 3);
  assert(graph_reverse.edge_count == 2);

  // Verify reverse graph's arrays
  const size_t reverse_edge_starts[] = {0, 0, 1, 2};
  const size_t reverse_targets[] = {2, 0};
  const double reverse_weights[] = {7.0, 5.0};
  const size_t reverse_original_edge_ids[] = {1, 0};
  for (size_t i = 0; i < graph_reverse.vertex_count + 1; i++) {
    assert(graph_reverse.edge_starts[i] == reverse_edge_starts[i]);
  }
  for (size_t i = 0; i < graph_reverse.edge_count; i++) {
    assert(graph_reverse.targets[i] == reverse_targets[i]);
    assert(graph_reverse.weights[i] == reverse_weights[i]);
    assert(graph_reverse.original_edge_ids[i] == reverse_original_edge_ids[i]);
  }

  // Verify original graph's counts
  assert(graph.vertex_count == 3);
  assert(graph.edge_count == 2);

  // Verify original graph's arrays
  const size_t original_edge_starts[] = {0, 1, 1, 2};
  const size_t original_targets[] = {2, 1};
  const double original_weights[] = {5.0, 7.0};
  const size_t expected_original_edge_ids[] = {0, 1};
  for (size_t i = 0; i < graph.vertex_count + 1; i++) {
    assert(graph.edge_starts[i] == original_edge_starts[i]);
  }
  for (size_t i = 0; i < graph.edge_count; i++) {
    assert(graph.targets[i] == original_targets[i]);
    assert(graph.weights[i] == original_weights[i]);
    assert(graph.original_edge_ids[i] == expected_original_edge_ids[i]);
  }

  // Verify all four graph array pointer pairs are different
  assert(graph.edge_starts != graph_reverse.edge_starts);
  assert(graph.targets != graph_reverse.targets);
  assert(graph.weights != graph_reverse.weights);
  assert(graph.original_edge_ids != graph_reverse.original_edge_ids);

  // Destroy original graph
  graph_destroy(&graph);

  // Verify original graph's counts and pointers reset
  assert(graph.vertex_count == 0);
  assert(graph.edge_count == 0);
  assert(graph.edge_starts == NULL);
  assert(graph.targets == NULL);
  assert(graph.weights == NULL);
  assert(graph.original_edge_ids == NULL);

  // Verify reverse graph's counts unchanged
  assert(graph_reverse.vertex_count == 3);
  assert(graph_reverse.edge_count == 2);

  // Verify reverse graph's arrays unchanged
  for (size_t i = 0; i < graph_reverse.vertex_count + 1; i++) {
    assert(graph_reverse.edge_starts[i] == reverse_edge_starts[i]);
  }
  for (size_t i = 0; i < graph_reverse.edge_count; i++) {
    assert(graph_reverse.targets[i] == reverse_targets[i]);
    assert(graph_reverse.weights[i] == reverse_weights[i]);
    assert(graph_reverse.original_edge_ids[i] == reverse_original_edge_ids[i]);
  }

  // Destroy reverse graph
  graph_destroy(&graph_reverse);

  // Verify reverse graph's counts and pointers reset
  assert(graph_reverse.vertex_count == 0);
  assert(graph_reverse.edge_count == 0);
  assert(graph_reverse.edge_starts == NULL);
  assert(graph_reverse.targets == NULL);
  assert(graph_reverse.weights == NULL);
  assert(graph_reverse.original_edge_ids == NULL);
}

// Verify reversing a graph and then reversing its result returns a graph
// representing the same collection of directed edges as the original, including
// weights and original edge identities. The final CSR arrays do not require
// identical ordering
void test_reverse_round_trip() {
  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Construct the input edge array
  alt_edge edges[] = {{2, 3, 4.0}, {0, 3, 3.0}, {1, 2, 2.0}, {0, 2, 7.0}};

  // Build the source graph
  alt_status result = graph_build(4, edges, 4, &graph);

  // Make sure the result status OK
  assert(result == ALT_OK);

  // Initialize a second empty alt_graph to receive the reverse graph
  alt_graph graph_reverse = {0};

  // Construct the reverse graph
  result = graph_build_reverse(&graph, &graph_reverse);

  // Make sure the result status OK
  assert(result == ALT_OK);

  // Initialize a third empty alt_graph to receive the twice reversed graph
  alt_graph graph_twice_reversed = {0};

  // Construct the twice-reversed graph
  result = graph_build_reverse(&graph_reverse, &graph_twice_reversed);

  // Make sure the result status OK
  assert(result == ALT_OK);

  // Verify all three graphs' counts
  assert(graph.vertex_count == 4);
  assert(graph.edge_count == 4);
  assert(graph_reverse.vertex_count == 4);
  assert(graph_reverse.edge_count == 4);
  assert(graph_twice_reversed.vertex_count == 4);
  assert(graph_twice_reversed.edge_count == 4);

  // Verify original graph's CSR arrays
  const size_t original_edge_starts[] = {0, 2, 3, 4, 4};
  const size_t original_targets[] = {3, 2, 2, 3};
  const double original_weights[] = {3.0, 7.0, 2.0, 4.0};
  const size_t expected_original_edge_ids[] = {1, 3, 2, 0};
  for (size_t i = 0; i < graph.vertex_count + 1; i++) {
    assert(graph.edge_starts[i] == original_edge_starts[i]);
  }
  for (size_t i = 0; i < graph.edge_count; i++) {
    assert(graph.targets[i] == original_targets[i]);
    assert(graph.weights[i] == original_weights[i]);
    assert(graph.original_edge_ids[i] == expected_original_edge_ids[i]);
  }

  // Verify reverse graph's CSR arrays
  const size_t reverse_edge_starts[] = {0, 0, 0, 2, 4};
  const size_t reverse_targets[] = {0, 1, 0, 2};
  const double reverse_weights[] = {7.0, 2.0, 3.0, 4.0};
  const size_t reverse_original_edge_ids[] = {3, 2, 1, 0};
  for (size_t i = 0; i < graph_reverse.vertex_count + 1; i++) {
    assert(graph_reverse.edge_starts[i] == reverse_edge_starts[i]);
  }
  for (size_t i = 0; i < graph_reverse.edge_count; i++) {
    assert(graph_reverse.targets[i] == reverse_targets[i]);
    assert(graph_reverse.weights[i] == reverse_weights[i]);
    assert(graph_reverse.original_edge_ids[i] == reverse_original_edge_ids[i]);
  }

  // Verify twice-reversed graph's edge_starts match original's
  for (size_t i = 0; i < graph_twice_reversed.vertex_count + 1; i++) {
    assert(graph_twice_reversed.edge_starts[i] == original_edge_starts[i]);
  }

  // Create a temporary array of flags corresponding to seen original edge ids
  bool seen_original_edge_ids[] = {false, false, false, false};

  // Set flags indicating twice-reversed graph contains original edge identities
  // exactly once
  for (size_t i = 0; i < graph_twice_reversed.edge_count; i++) {
    size_t original_edge_id = graph_twice_reversed.original_edge_ids[i];

    assert(original_edge_id < graph.edge_count);
    assert(!seen_original_edge_ids[original_edge_id]);
    seen_original_edge_ids[original_edge_id] = true;
  }

  // Verify twice-reversed graph preserved all original edge identities
  for (size_t i = 0; i < graph_twice_reversed.edge_count; i++) {
    assert(seen_original_edge_ids[i]);
  }

  // Verify edge correspondence by original id
  for (size_t edge_i = 0; edge_i < graph_twice_reversed.edge_count; edge_i++) {
    size_t original_edge_id = graph_twice_reversed.original_edge_ids[edge_i];

    // Set undetermined source to an impossible value to indicate it has not
    // been identified
    size_t source = SIZE_MAX;

    // Determine edge source
    for (size_t vertex_i = 1; vertex_i < graph_twice_reversed.vertex_count + 1;
         vertex_i++) {
      if (graph_twice_reversed.edge_starts[vertex_i] > edge_i) {
        // Source identified
        source = vertex_i - 1;

        break;
      }
    }
    // Make sure source was successfully identified
    assert(source != SIZE_MAX);

    // Verify twice-reversed graph's edge values correspond with their original
    // values
    assert(source == edges[original_edge_id].source);
    assert(graph_twice_reversed.targets[edge_i] ==
           edges[original_edge_id].target);
    assert(graph_twice_reversed.weights[edge_i] ==
           edges[original_edge_id].weight);
  }

  // Destroy all three graphs
  graph_destroy(&graph);
  graph_destroy(&graph_reverse);
  graph_destroy(&graph_twice_reversed);

  // Verify all graphs' counts and pointers reset
  assert(graph.vertex_count == 0);
  assert(graph.edge_count == 0);
  assert(graph.edge_starts == NULL);
  assert(graph.targets == NULL);
  assert(graph.weights == NULL);
  assert(graph.original_edge_ids == NULL);
  assert(graph_reverse.vertex_count == 0);
  assert(graph_reverse.edge_count == 0);
  assert(graph_reverse.edge_starts == NULL);
  assert(graph_reverse.targets == NULL);
  assert(graph_reverse.weights == NULL);
  assert(graph_reverse.original_edge_ids == NULL);
  assert(graph_twice_reversed.vertex_count == 0);
  assert(graph_twice_reversed.edge_count == 0);
  assert(graph_twice_reversed.edge_starts == NULL);
  assert(graph_twice_reversed.targets == NULL);
  assert(graph_twice_reversed.weights == NULL);
  assert(graph_twice_reversed.original_edge_ids == NULL);
}

/* ==========================================================================
 * graph_destroy tests
 * ========================================================================== */

// Verify null-safe and repeated destruction
void test_destroy_safety() {
  // Call graph_destroy on a null pointer
  graph_destroy(NULL);

  // Initialize an empty alt_graph
  alt_graph graph = {0};

  // Call graph_destroy on a zero-initialized alt_graph
  graph_destroy(&graph);

  // Verify graph's counts and pointers are still zero-initialized
  assert(graph.vertex_count == 0);
  assert(graph.edge_count == 0);
  assert(graph.edge_starts == NULL);
  assert(graph.targets == NULL);
  assert(graph.weights == NULL);
  assert(graph.original_edge_ids == NULL);

  // Construct the input edge array
  alt_edge edges[] = {{0, 1, 2.0}, {1, 2, 3.0}};

  // Build a valid graph
  alt_status result = graph_build(3, edges, 2, &graph);

  // Make sure the result status OK
  assert(result == ALT_OK);

  // Verify graph arrays are non-null
  assert(graph.edge_starts != NULL);
  assert(graph.targets != NULL);
  assert(graph.weights != NULL);
  assert(graph.original_edge_ids != NULL);

  // Destroy the graph
  graph_destroy(&graph);

  // Verify graph's counts and pointers reset
  assert(graph.vertex_count == 0);
  assert(graph.edge_count == 0);
  assert(graph.edge_starts == NULL);
  assert(graph.targets == NULL);
  assert(graph.weights == NULL);
  assert(graph.original_edge_ids == NULL);

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

// Executes tests
int main() {
  // Execute tests
  // graph_build tests
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
  // graph_build_reverse tests
  test_reverse_basic();
  test_reverse_zero_edges();
  test_reverse_isolated_vertices();
  test_reverse_self_loops();
  test_reverse_parallel_edges();
  test_reverse_zero_weight_edges();
  test_reverse_invalid_arguments();
  test_reverse_independent_lifecycle();
  test_reverse_round_trip();
  // graph_destroy tests
  test_destroy_safety();

  // All tests passed
  return 0;
}
