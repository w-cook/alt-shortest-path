#ifndef ALT_GRAPH_H
#define ALT_GRAPH_H

#include "alt/alt.h"
#include <stddef.h>

typedef struct {
  // Number of vertices
  size_t vertex_count;
  // Number of directed edges
  size_t edge_count;

  // Starting index of each vertex's outgoing edges
  // Commonly referenced in CSR literature as "row offsets"
  size_t *edge_starts;
  // Destination vertex for each stored edge
  size_t *targets;
  // Weight of each stored edge
  double *weights;

  // Mapping to the caller's original edge-list positions
  size_t *original_edge_ids;
} alt_graph;

// Constructor function for alt_graph
alt_status graph_build(size_t vertex_count, const alt_edge *edges,
                       size_t edge_count, alt_graph *out_graph);

// Destructor function for alt_graph
void graph_destroy(alt_graph *graph);

#endif // ALT_GRAPH_H
