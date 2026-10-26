#ifndef ALT_PRIORITY_QUEUE_H
#define ALT_PRIORITY_QUEUE_H

#include "alt/alt.h"
#include <stdbool.h>
#include <stddef.h>

// Priority queue entry
typedef struct {
  // Identifies the vertex
  alt_vertex_id vertex;
  // Distance when the entry was inserted
  double recorded_distance;
  // Determines extraction order
  double priority;
} alt_queue_entry;

// Priority queue structure
typedef struct {
  // Dynamically allocated array representing a binary min-heap of queue entries
  alt_queue_entry *entries;
  // Current queue size
  size_t size;
  // Current allocated capacity
  size_t capacity;
} alt_priority_queue;

// Insert an entry, growing storage if necessary
alt_status priority_queue_push(alt_priority_queue *queue,
                               alt_queue_entry entry);

// Extract the minimum-priority entry
bool priority_queue_pop(alt_priority_queue *queue, alt_queue_entry *out_entry);

// Check whether the queue contains entries
bool priority_queue_is_empty(const alt_priority_queue *queue);

// Release storage and reset the structure
void priority_queue_destroy(alt_priority_queue *queue);

#endif // ALT_PRIORITY_QUEUE_H
