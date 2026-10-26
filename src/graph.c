#include "graph.h"
#include "alt/alt.h"
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

// Constructor function for alt_graph
alt_status graph_build(size_t vertex_count, const alt_edge *edges,
                       size_t edge_count, alt_graph *out_graph) {
  // Caller must supply a zero-initialized alt_graph
  // that doesn't already own allocated arrays
  if (out_graph == NULL) {
    return ALT_INVALID_ARGUMENT;
  }

  // API specification explicitly disallows graphs
  // containing zero vertices
  if (vertex_count == 0) {
    return ALT_INVALID_GRAPH;
  }

  // Graph with an edge_count must have an edges array
  if (edge_count > 0 && edges == NULL) {
    return ALT_INVALID_ARGUMENT;
  }

  // Reject sizes that cannot be represented or allocated safely
  if (vertex_count == SIZE_MAX) {
    return ALT_INVALID_GRAPH;
  }

  // edge_starts allocation size must be able to be safely represented
  if (vertex_count + 1 > SIZE_MAX / sizeof(size_t)) {
    return ALT_OUT_OF_MEMORY;
  }

  // array allocation sizes must be able to be safely represented
  if (edge_count > SIZE_MAX / sizeof(size_t) ||
      edge_count > SIZE_MAX / sizeof(double)) {
    return ALT_OUT_OF_MEMORY;
  }

  // Verify each edge's source vertex ID < vertex_count
  // and that its target vertex ID < vertex_count.
  // Edge weights must also be nonnegative and finite
  for (size_t i = 0; i < edge_count; i++) {
    if (edges[i].source >= vertex_count || edges[i].target >= vertex_count ||
        edges[i].weight < 0 || !isfinite(edges[i].weight)) {
      return ALT_INVALID_GRAPH;
    }
  }

  // Create temporary alt_graph for construction to
  // avoid changing caller's alt_graph in case of failure
  alt_graph local_graph = {0};

  // Allocate edge_starts as zero-initialized elements
  local_graph.edge_starts = calloc(vertex_count + 1, sizeof(size_t));

  // Ensure allocation succeeded
  if (local_graph.edge_starts == NULL) {
    return ALT_OUT_OF_MEMORY;
  }

  // Store counts in the temporary alt_graph
  local_graph.vertex_count = vertex_count;
  local_graph.edge_count = edge_count;

  // Count outgoing edges
  for (size_t i = 0; i < edge_count; i++) {
    local_graph.edge_starts[edges[i].source + 1]++;
  }

  // Convert counts into CSR offsets
  for (size_t i = 1; i < vertex_count + 1; i++) {
    local_graph.edge_starts[i] += local_graph.edge_starts[i - 1];
  }

  // Handle zero-edge graphs by skipping allocation and population
  if (edge_count > 0) {
    // Allocate targets array
    local_graph.targets = malloc(edge_count * sizeof(size_t));

    // Ensure allocation succeeded
    if (local_graph.targets == NULL) {
      graph_destroy(&local_graph);

      return ALT_OUT_OF_MEMORY;
    }

    // Allocate weights array
    local_graph.weights = malloc(edge_count * sizeof(double));

    // Ensure allocation succeeded
    if (local_graph.weights == NULL) {
      graph_destroy(&local_graph);

      return ALT_OUT_OF_MEMORY;
    }

    // Allocate original_edge_ids array
    local_graph.original_edge_ids = malloc(edge_count * sizeof(size_t));

    // Ensure allocation succeeded
    if (local_graph.original_edge_ids == NULL) {
      graph_destroy(&local_graph);

      return ALT_OUT_OF_MEMORY;
    }

    // Create a cursor array to keep track of each vertex's insertion position
    size_t *cursors = malloc(vertex_count * sizeof(size_t));

    // Ensure allocation succeeded
    if (cursors == NULL) {
      graph_destroy(&local_graph);

      return ALT_OUT_OF_MEMORY;
    }

    // Copy starting offsets into the cursor array
    for (size_t i = 0; i < vertex_count; i++) {
      cursors[i] = local_graph.edge_starts[i];
    }

    // Populate the CSR edge arrays
    for (size_t i = 0; i < edge_count; i++) {
      // Store the input edge's target, weight, and original array index
      local_graph.targets[cursors[edges[i].source]] = edges[i].target;
      local_graph.weights[cursors[edges[i].source]] = edges[i].weight;
      local_graph.original_edge_ids[cursors[edges[i].source]] = i;

      // Advance the cursor for the source vertex
      cursors[edges[i].source]++;
    }

    // Free the no longer necessary cursor array's memory
    free(cursors);
  }

  // Assign the local structure to the one pointed to by the caller
  *out_graph = local_graph;

  return ALT_OK;
}

// Destructor function for alt_graph
void graph_destroy(alt_graph *graph) {
  // Handle NULL argument safely
  if (graph == NULL) {
    return;
  }

  // Memory cleanup
  free(graph->edge_starts);
  free(graph->targets);
  free(graph->weights);
  free(graph->original_edge_ids);

  // Structure reset
  *graph = (alt_graph){0};
}
