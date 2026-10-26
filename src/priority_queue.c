#include "priority_queue.h"
#include "alt/alt.h"
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

// Insert an entry, growing storage if needed
alt_status priority_queue_push(alt_priority_queue *queue,
                               alt_queue_entry entry) {
  // Reject null pointer queues
  if (queue == NULL) {
    return ALT_INVALID_ARGUMENT;
  }

  // Reject entries with non-finite priority or recorded_distance
  if (!isfinite(entry.priority) || !isfinite(entry.recorded_distance)) {
    return ALT_NUMERIC_ERROR;
  }

  // Ensure sufficient capacity
  if (queue->size == queue->capacity) {
    // Temporary new capacity value
    size_t new_capacity = queue->capacity;

    // Uninitiated capacity
    if (queue->capacity == 0) {
      // Set initial capacity to eight
      new_capacity = 8;
    }
    // Expand capacity
    else {
      // Ensure not at max capacity to avoid overflow
      if (queue->capacity == SIZE_MAX / sizeof(alt_queue_entry)) {
        return ALT_OUT_OF_MEMORY;
      }
      // Ensure not to increase capacity beyond max to avoid overflow
      else if (queue->capacity > SIZE_MAX / sizeof(alt_queue_entry) / 2) {
        // Set capacity to max
        new_capacity = SIZE_MAX / sizeof(alt_queue_entry);
      }
      // Set capacity to double current size
      else {
        new_capacity *= 2;
      }
    }

    // Ensure allocation cannot overflow
    if (new_capacity > SIZE_MAX / sizeof(alt_queue_entry)) {
      return ALT_OUT_OF_MEMORY;
    }

    // Allocate memory for queue capacity
    alt_queue_entry *temp_entries;
    temp_entries =
        realloc(queue->entries, new_capacity * sizeof(alt_queue_entry));

    // Ensure allocation succeeded
    if (temp_entries == NULL) {
      return ALT_OUT_OF_MEMORY;
    }
    // Reassign the queue's entries pointer to the new array
    else {
      queue->entries = temp_entries;
    }

    // Set the queue's capacity
    queue->capacity = new_capacity;
  }

  // Insert the new entry
  queue->entries[queue->size] = entry;
  queue->size++;

  // Restore the min-heap property
  for (size_t i = queue->size - 1; i > 0; i = (i - 1) / 2) {
    // New entry has reached its appropriate position in the queue
    if (queue->entries[(i - 1) / 2].priority <= queue->entries[i].priority) {
      break;
    }

    // Swap positions of new entry and its parent
    alt_queue_entry temp_entry = queue->entries[i];
    queue->entries[i] = queue->entries[(i - 1) / 2];
    queue->entries[(i - 1) / 2] = temp_entry;
  }

  return ALT_OK;
}
