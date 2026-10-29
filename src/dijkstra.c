#include "dijkstra.h"
#include "priority_queue.h"
#include <math.h>

// Calculates shortest distances from a source vertex to every vertex
// in a directed, weighted graph.
alt_status dijkstra_all(const alt_graph *graph, alt_vertex_id source_vertex_id,
                        double *out_distances) {
  // Check for invalid arguments
  if (graph == NULL || out_distances == NULL ||
      source_vertex_id >= graph->vertex_count) {
    return ALT_INVALID_ARGUMENT;
  }

  // Initialize distances
  for (size_t i = 0; i < graph->vertex_count; i++) {
    // Source distance is initialized to zero
    if (i == source_vertex_id) {
      out_distances[i] = 0.0;

      continue;
    }

    // All other distances are initialized to positive infinity
    out_distances[i] = INFINITY;
  }

  // Create a zero-initialized priority queue
  alt_priority_queue queue = {0};

  // Create a queue entry for the source vertex
  alt_queue_entry source_entry = {source_vertex_id, 0.0, 0.0};

  // Insert the source entry into the queue
  alt_status insert_status = priority_queue_push(&queue, source_entry);

  // Ensure insertion succeeded
  if (insert_status != ALT_OK) {
    // Cleanup allocations
    priority_queue_destroy(&queue);

    // Propagate the error
    return insert_status;
  }

  // Create a zero-initialized queue entry to receive output
  alt_queue_entry entry = {0};

  // Process entries until the queue is empty
  while (priority_queue_pop(&queue, &entry)) {
    // Check if the popped entry is stale
    if (entry.recorded_distance > out_distances[entry.vertex]) {
      // Skip the stale entry
      continue;
    }

    // Traverse outgoing edges in the CSR graph
    for (size_t i = graph->edge_starts[entry.vertex];
         i < graph->edge_starts[entry.vertex + 1]; i++) {
      // Retrieve edge's destination vertex and weight
      alt_vertex_id destination_vertex = graph->targets[i];
      double edge_weight = graph->weights[i];

      // Calculate candidate distance
      double alternative_distance = out_distances[entry.vertex] + edge_weight;

      // Validate alternative distance
      if (!isfinite(alternative_distance) ||
          (edge_weight > 0.0 &&
           alternative_distance == out_distances[entry.vertex])) {
        // Cleanup allocations
        priority_queue_destroy(&queue);

        return ALT_NUMERIC_ERROR;
      }

      // Shorter paths trigger an update
      if (alternative_distance < out_distances[destination_vertex]) {
        // Create a new queue entry for destination_vertex
        alt_queue_entry new_entry = {destination_vertex, alternative_distance,
                                     alternative_distance};

        // Insert the new entry
        insert_status = priority_queue_push(&queue, new_entry);

        // Verify that insertion succeeded
        if (insert_status != ALT_OK) {
          // Cleanup allocations
          priority_queue_destroy(&queue);

          // Propagate the error
          return insert_status;
        }

        // Update out_distances to reflect the new distance for the destination
        // vertex
        out_distances[destination_vertex] = alternative_distance;
      }
    }
  }

  // Cleanup allocations
  priority_queue_destroy(&queue);

  return ALT_OK;
}
