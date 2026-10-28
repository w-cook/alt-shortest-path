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

// Extract the minimum-priority entry
bool priority_queue_pop(alt_priority_queue *queue, alt_queue_entry *out_entry) {
  // Check for valid queue pointer
  if (queue == NULL) {
    return false;
  }

  // Check for valid output pointer
  if (out_entry == NULL) {
    return false;
  }

  // Check for empty queue
  if (priority_queue_is_empty(queue)) {
    return false;
  }

  // Preserve the minimum entry
  alt_queue_entry min_entry = queue->entries[0];

  // Remove the root
  queue->entries[0] = queue->entries[queue->size - 1];
  queue->size--;

  // Restore the heap property. A single entry queue needs no restoration
  if (queue->size > 0) {
    // Store the current entry index
    size_t current_entry_index = 0;

    // Stop if current entry is a leaf
    while (current_entry_index < queue->size / 2) {
      // Index of the child with the lowest priority (initially first)
      size_t lowest_priority_child_index = current_entry_index * 2 + 1;

      // If second child index does not exceed array size, then current entry
      // has second child. If that second child has a lower priority than the
      // first, change the lowest_priority_child_index
      if (lowest_priority_child_index + 1 < queue->size &&
          queue->entries[lowest_priority_child_index].priority >
              queue->entries[lowest_priority_child_index + 1].priority) {
        // Child two has the lowest priority
        lowest_priority_child_index++;
      }

      // If smallest child has a smaller priority than current entry, swap and
      // continue downward
      if (queue->entries[lowest_priority_child_index].priority <
          queue->entries[current_entry_index].priority) {
        // Create temporary entry
        alt_queue_entry temp_entry =
            queue->entries[lowest_priority_child_index];

        // Swap entries
        queue->entries[lowest_priority_child_index] =
            queue->entries[current_entry_index];
        queue->entries[current_entry_index] = temp_entry;

        // Update the current_entry_index
        current_entry_index = lowest_priority_child_index;
      }
      // Heap property is satisfied
      else {
        // Break from the loop
        break;
      }
    }
  }

  // Set the output entry
  *out_entry = min_entry;

  return true;
}

// Check whether the queue contains entries
bool priority_queue_is_empty(const alt_priority_queue *queue) {
  // Handle null queue pointer
  if (queue == NULL) {
    return true;
  }

  // Check queue's size value
  return queue->size == 0;
}

// Release storage and reset the structure
void priority_queue_destroy(alt_priority_queue *queue) {
  // Handle null queue
  if (queue == NULL) {
    return;
  }

  // Memory cleanup
  free(queue->entries);

  // Structure reset
  *queue = (alt_priority_queue){0};
}
