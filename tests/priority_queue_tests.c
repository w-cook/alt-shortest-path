#include "../src/priority_queue.h"
#include "alt/alt.h"
#include <assert.h>
#include <math.h>
#include <stddef.h>

// Verify that a zero-initialized queue is empty, popping returns
// false, and a failed pop leaves the output unchanged
void test_empty_queue() {
  // Create a zero-initialized queue
  alt_priority_queue queue = {0};

  // Verify queue is empty
  assert(priority_queue_is_empty(&queue));

  // Create queue out_entry with initial distinct values
  alt_queue_entry entry = {1, 2.0, 3.0};

  // Verify popping empty queue returns false
  assert(!priority_queue_pop(&queue, &entry));

  // Verify failed pop leaves the output unchanged
  assert(entry.vertex == 1);
  assert(entry.recorded_distance == 2.0);
  assert(entry.priority == 3.0);

  // Verify queue's internal state is unchanged
  assert(queue.entries == NULL);
  assert(queue.size == 0);
  assert(queue.capacity == 0);
}

// Verify null-pointer behavior for all four operations, including
// attempting to pop with a null output pointer
void test_null_arguments() {
  // Create a valid queue entry
  alt_queue_entry entry = {1, 2.0, 3.0};

  // Verify null queue push returns invalid
  assert(priority_queue_push(NULL, entry) == ALT_INVALID_ARGUMENT);

  // Verify null queue pop fails
  assert(!priority_queue_pop(NULL, &entry));

  // Verify failed pop leaves the output unchanged
  assert(entry.vertex == 1);
  assert(entry.recorded_distance == 2.0);
  assert(entry.priority == 3.0);

  // Create zero-initialized queue
  alt_priority_queue queue = {0};

  // Add valid entry to queue and make sure it succeeded
  assert(priority_queue_push(&queue, entry) == ALT_OK);

  // Record queue's current state
  alt_queue_entry *entries = queue.entries;
  size_t size = queue.size;
  size_t capacity = queue.capacity;

  // Verify null output pointer pop fails
  assert(!priority_queue_pop(&queue, NULL));

  // Verify queue's internal state is unchanged
  assert(queue.entries == entries);
  assert(queue.size == size);
  assert(queue.capacity == capacity);

  // Verify the stored entry is unchanged
  assert(queue.entries[0].vertex == entry.vertex);
  assert(queue.entries[0].recorded_distance == entry.recorded_distance);
  assert(queue.entries[0].priority == entry.priority);

  // Verify empty check on null queue pointer returns true
  assert(priority_queue_is_empty(NULL));

  // Verify calling destroy on a null queue pointer returns safely without
  // crashing
  priority_queue_destroy(NULL);

  // Cleanup test allocations
  priority_queue_destroy(&queue);
}

// Push one entry, verify the queue is nonempty, extract that entry
// with all fields preserved, and confirm the queue is empty afterward
void test_single_entry() {
  // Create a zero-initialized queue
  alt_priority_queue queue = {0};

  // Create a distinct valid queue entry
  alt_queue_entry entry = {1, 2.0, 3.0};

  // Push the entry to the queue and verify it succeeds
  assert(priority_queue_push(&queue, entry) == ALT_OK);

  // Verify that the queue is no longer empty
  assert(!priority_queue_is_empty(&queue));

  // Create a zero-initialized output entry
  alt_queue_entry out_entry = {0};

  // Pop the entry and verify success
  assert(priority_queue_pop(&queue, &out_entry));

  // Verify that popped entry matches original
  assert(out_entry.vertex == entry.vertex);
  assert(out_entry.recorded_distance == entry.recorded_distance);
  assert(out_entry.priority == entry.priority);

  // Verify that the queue is empty
  assert(priority_queue_is_empty(&queue));

  // Verify that another pop fails
  assert(!priority_queue_pop(&queue, &out_entry));

  // Cleanup test allocations
  priority_queue_destroy(&queue);
}

// Insert entries in ascending priority order and verify they're
// extracted in that order. Exercises ordinary sift-down behavior
void test_ascending_priorities() {
  // Create a zero-initialized queue
  alt_priority_queue queue = {0};

  // Create several entries with increasing priorities
  alt_queue_entry entry_one = {1, 1.0, 1.0};
  alt_queue_entry entry_two = {2, 2.0, 2.0};
  alt_queue_entry entry_three = {3, 3.0, 3.0};
  alt_queue_entry entry_four = {4, 4.0, 4.0};
  alt_queue_entry entry_five = {5, 5.0, 5.0};

  // Insert entries in order and verify success
  assert(priority_queue_push(&queue, entry_one) == ALT_OK);
  assert(priority_queue_push(&queue, entry_two) == ALT_OK);
  assert(priority_queue_push(&queue, entry_three) == ALT_OK);
  assert(priority_queue_push(&queue, entry_four) == ALT_OK);
  assert(priority_queue_push(&queue, entry_five) == ALT_OK);

  // Create zero-initialized output entry
  alt_queue_entry out_entry = {0};

  // Pop first entry and verify success
  assert(priority_queue_pop(&queue, &out_entry));

  // Verify output entry matches expected values of entry_one
  assert(out_entry.vertex == entry_one.vertex);
  assert(out_entry.recorded_distance == entry_one.recorded_distance);
  assert(out_entry.priority == entry_one.priority);

  // Pop second entry and verify success
  assert(priority_queue_pop(&queue, &out_entry));

  // Verify output entry matches expected values of entry_two
  assert(out_entry.vertex == entry_two.vertex);
  assert(out_entry.recorded_distance == entry_two.recorded_distance);
  assert(out_entry.priority == entry_two.priority);

  // Pop third entry and verify success
  assert(priority_queue_pop(&queue, &out_entry));

  // Verify output entry matches expected values of entry_three
  assert(out_entry.vertex == entry_three.vertex);
  assert(out_entry.recorded_distance == entry_three.recorded_distance);
  assert(out_entry.priority == entry_three.priority);

  // Pop fourth entry and verify success
  assert(priority_queue_pop(&queue, &out_entry));

  // Verify output entry matches expected values of entry_four
  assert(out_entry.vertex == entry_four.vertex);
  assert(out_entry.recorded_distance == entry_four.recorded_distance);
  assert(out_entry.priority == entry_four.priority);

  // Pop fifth entry and verify success
  assert(priority_queue_pop(&queue, &out_entry));

  // Verify output entry matches expected values of entry_five
  assert(out_entry.vertex == entry_five.vertex);
  assert(out_entry.recorded_distance == entry_five.recorded_distance);
  assert(out_entry.priority == entry_five.priority);

  // Verify that the queue is now empty
  assert(priority_queue_is_empty(&queue));

  // Cleanup test allocations
  priority_queue_destroy(&queue);
}

// Insert entries in descending priority order and verify ascending
// extraction. Exercises repeated sift-up during insertion
void test_descending_priorities() {
  // Create a zero-initialized queue
  alt_priority_queue queue = {0};

  // Create several entries with decreasing priorities
  alt_queue_entry entry_one = {1, 1.0, 5.0};
  alt_queue_entry entry_two = {2, 2.0, 4.0};
  alt_queue_entry entry_three = {3, 3.0, 3.0};
  alt_queue_entry entry_four = {4, 4.0, 2.0};
  alt_queue_entry entry_five = {5, 5.0, 1.0};

  // Insert entries in order and verify success
  assert(priority_queue_push(&queue, entry_one) == ALT_OK);
  assert(priority_queue_push(&queue, entry_two) == ALT_OK);
  assert(priority_queue_push(&queue, entry_three) == ALT_OK);
  assert(priority_queue_push(&queue, entry_four) == ALT_OK);
  assert(priority_queue_push(&queue, entry_five) == ALT_OK);

  // Create zero-initialized output entry
  alt_queue_entry out_entry = {0};

  // Pop first entry and verify success
  assert(priority_queue_pop(&queue, &out_entry));

  // Verify output entry matches expected values of entry_five
  assert(out_entry.vertex == entry_five.vertex);
  assert(out_entry.recorded_distance == entry_five.recorded_distance);
  assert(out_entry.priority == entry_five.priority);

  // Pop second entry and verify success
  assert(priority_queue_pop(&queue, &out_entry));

  // Verify output entry matches expected values of entry_four
  assert(out_entry.vertex == entry_four.vertex);
  assert(out_entry.recorded_distance == entry_four.recorded_distance);
  assert(out_entry.priority == entry_four.priority);

  // Pop third entry and verify success
  assert(priority_queue_pop(&queue, &out_entry));

  // Verify output entry matches expected values of entry_three
  assert(out_entry.vertex == entry_three.vertex);
  assert(out_entry.recorded_distance == entry_three.recorded_distance);
  assert(out_entry.priority == entry_three.priority);

  // Pop fourth entry and verify success
  assert(priority_queue_pop(&queue, &out_entry));

  // Verify output entry matches expected values of entry_two
  assert(out_entry.vertex == entry_two.vertex);
  assert(out_entry.recorded_distance == entry_two.recorded_distance);
  assert(out_entry.priority == entry_two.priority);

  // Pop fifth entry and verify success
  assert(priority_queue_pop(&queue, &out_entry));

  // Verify output entry matches expected values of entry_one
  assert(out_entry.vertex == entry_one.vertex);
  assert(out_entry.recorded_distance == entry_one.recorded_distance);
  assert(out_entry.priority == entry_one.priority);

  // Verify that the queue is now empty
  assert(priority_queue_is_empty(&queue));

  // Cleanup test allocations
  priority_queue_destroy(&queue);
}

// Insert entries with deliberately unordered priorities and verify
// that every extraction returns the remaining minimum
void test_mixed_priorities() {
  // Create a zero-initialized queue
  alt_priority_queue queue = {0};

  // Create several entries with unordered priorities
  alt_queue_entry entry_one = {1, 1.0, 4.0};
  alt_queue_entry entry_two = {2, 2.0, 1.0};
  alt_queue_entry entry_three = {3, 3.0, 7.0};
  alt_queue_entry entry_four = {4, 4.0, 3.0};
  alt_queue_entry entry_five = {5, 5.0, 6.0};
  alt_queue_entry entry_six = {6, 6.0, 2.0};
  alt_queue_entry entry_seven = {7, 7.0, 5.0};

  // Insert entries in order and verify success
  assert(priority_queue_push(&queue, entry_one) == ALT_OK);
  assert(priority_queue_push(&queue, entry_two) == ALT_OK);
  assert(priority_queue_push(&queue, entry_three) == ALT_OK);
  assert(priority_queue_push(&queue, entry_four) == ALT_OK);
  assert(priority_queue_push(&queue, entry_five) == ALT_OK);
  assert(priority_queue_push(&queue, entry_six) == ALT_OK);
  assert(priority_queue_push(&queue, entry_seven) == ALT_OK);

  // Create zero-initialized output entry
  alt_queue_entry out_entry = {0};

  // Pop first entry and verify success
  assert(priority_queue_pop(&queue, &out_entry));

  // Verify output entry matches expected values of entry_two
  assert(out_entry.vertex == entry_two.vertex);
  assert(out_entry.recorded_distance == entry_two.recorded_distance);
  assert(out_entry.priority == entry_two.priority);

  // Pop second entry and verify success
  assert(priority_queue_pop(&queue, &out_entry));

  // Verify output entry matches expected values of entry_six
  assert(out_entry.vertex == entry_six.vertex);
  assert(out_entry.recorded_distance == entry_six.recorded_distance);
  assert(out_entry.priority == entry_six.priority);

  // Pop third entry and verify success
  assert(priority_queue_pop(&queue, &out_entry));

  // Verify output entry matches expected values of entry_four
  assert(out_entry.vertex == entry_four.vertex);
  assert(out_entry.recorded_distance == entry_four.recorded_distance);
  assert(out_entry.priority == entry_four.priority);

  // Pop fourth entry and verify success
  assert(priority_queue_pop(&queue, &out_entry));

  // Verify output entry matches expected values of entry_one
  assert(out_entry.vertex == entry_one.vertex);
  assert(out_entry.recorded_distance == entry_one.recorded_distance);
  assert(out_entry.priority == entry_one.priority);

  // Pop fifth entry and verify success
  assert(priority_queue_pop(&queue, &out_entry));

  // Verify output entry matches expected values of entry_seven
  assert(out_entry.vertex == entry_seven.vertex);
  assert(out_entry.recorded_distance == entry_seven.recorded_distance);
  assert(out_entry.priority == entry_seven.priority);

  // Pop sixth entry and verify success
  assert(priority_queue_pop(&queue, &out_entry));

  // Verify output entry matches expected values of entry_five
  assert(out_entry.vertex == entry_five.vertex);
  assert(out_entry.recorded_distance == entry_five.recorded_distance);
  assert(out_entry.priority == entry_five.priority);

  // Pop seventh entry and verify success
  assert(priority_queue_pop(&queue, &out_entry));

  // Verify output entry matches expected values of entry_three
  assert(out_entry.vertex == entry_three.vertex);
  assert(out_entry.recorded_distance == entry_three.recorded_distance);
  assert(out_entry.priority == entry_three.priority);

  // Verify that the queue is now empty
  assert(priority_queue_is_empty(&queue));

  // Cleanup test allocations
  priority_queue_destroy(&queue);
}

// Construct a heap where restoration must swap with a node's only
// child. Tests the case where a node has no right child
void test_single_child_restoration() {
  // Create a zero-initialized queue
  alt_priority_queue queue = {0};

  // Create several entries with priorities that force a single-child swap
  alt_queue_entry entry_one = {1, 1.0, 2.0};
  alt_queue_entry entry_two = {2, 2.0, 1.0};
  alt_queue_entry entry_three = {3, 3.0, 3.0};

  // Insert entries in order and verify success
  assert(priority_queue_push(&queue, entry_one) == ALT_OK);
  assert(priority_queue_push(&queue, entry_two) == ALT_OK);
  assert(priority_queue_push(&queue, entry_three) == ALT_OK);

  // Create zero-initialized output entry
  alt_queue_entry out_entry = {0};

  // Pop first entry and verify success
  assert(priority_queue_pop(&queue, &out_entry));

  // Verify output entry matches expected values of entry_two
  assert(out_entry.vertex == entry_two.vertex);
  assert(out_entry.recorded_distance == entry_two.recorded_distance);
  assert(out_entry.priority == entry_two.priority);

  // Pop second entry and verify success
  assert(priority_queue_pop(&queue, &out_entry));

  // Verify output entry matches expected values of entry_one
  assert(out_entry.vertex == entry_one.vertex);
  assert(out_entry.recorded_distance == entry_one.recorded_distance);
  assert(out_entry.priority == entry_one.priority);

  // Pop third entry and verify success
  assert(priority_queue_pop(&queue, &out_entry));

  // Verify output entry matches expected values of entry_three
  assert(out_entry.vertex == entry_three.vertex);
  assert(out_entry.recorded_distance == entry_three.recorded_distance);
  assert(out_entry.priority == entry_three.priority);

  // Verify that the queue is now empty
  assert(priority_queue_is_empty(&queue));

  // Cleanup test allocations
  priority_queue_destroy(&queue);
}

// Construct a heap where the right child has the lower priority and
// must be selected during sift-down
void test_right_child_restoration() {
  // Create a zero-initialized queue
  alt_priority_queue queue = {0};

  // Create several entries with priorities that will produce a right-child swap
  alt_queue_entry entry_one = {1, 1.0, 1.0};
  alt_queue_entry entry_two = {2, 2.0, 4.0};
  alt_queue_entry entry_three = {3, 3.0, 2.0};
  alt_queue_entry entry_four = {4, 4.0, 5.0};

  // Insert entries in order and verify success
  assert(priority_queue_push(&queue, entry_one) == ALT_OK);
  assert(priority_queue_push(&queue, entry_two) == ALT_OK);
  assert(priority_queue_push(&queue, entry_three) == ALT_OK);
  assert(priority_queue_push(&queue, entry_four) == ALT_OK);

  // Create zero-initialized output entry
  alt_queue_entry out_entry = {0};

  // Pop first entry and verify success
  assert(priority_queue_pop(&queue, &out_entry));

  // Verify output entry matches expected values of entry_one
  assert(out_entry.vertex == entry_one.vertex);
  assert(out_entry.recorded_distance == entry_one.recorded_distance);
  assert(out_entry.priority == entry_one.priority);

  // Pop second entry and verify success
  assert(priority_queue_pop(&queue, &out_entry));

  // Verify output entry matches expected values of entry_three
  assert(out_entry.vertex == entry_three.vertex);
  assert(out_entry.recorded_distance == entry_three.recorded_distance);
  assert(out_entry.priority == entry_three.priority);

  // Pop third entry and verify success
  assert(priority_queue_pop(&queue, &out_entry));

  // Verify output entry matches expected values of entry_two
  assert(out_entry.vertex == entry_two.vertex);
  assert(out_entry.recorded_distance == entry_two.recorded_distance);
  assert(out_entry.priority == entry_two.priority);

  // Pop fourth entry and verify success
  assert(priority_queue_pop(&queue, &out_entry));

  // Verify output entry matches expected values of entry_four
  assert(out_entry.vertex == entry_four.vertex);
  assert(out_entry.recorded_distance == entry_four.recorded_distance);
  assert(out_entry.priority == entry_four.priority);

  // Verify that the queue is now empty
  assert(priority_queue_is_empty(&queue));

  // Cleanup test allocations
  priority_queue_destroy(&queue);
}

// Insert multiple entries with identical priorities. Verify that
// every entry is eventually extracted exactly once, without assuming
// stable ordering
void test_equal_priorities() {
  // Create a zero-initialized queue
  alt_priority_queue queue = {0};

  // Create several entries including equal priorities
  alt_queue_entry entry_one = {1, 1.0, 2.0};
  alt_queue_entry entry_two = {2, 2.0, 1.0};
  alt_queue_entry entry_three = {3, 3.0, 2.0};
  alt_queue_entry entry_four = {4, 4.0, 3.0};
  alt_queue_entry entry_five = {5, 5.0, 2.0};

  // Insert entries in order and verify success
  assert(priority_queue_push(&queue, entry_one) == ALT_OK);
  assert(priority_queue_push(&queue, entry_two) == ALT_OK);
  assert(priority_queue_push(&queue, entry_three) == ALT_OK);
  assert(priority_queue_push(&queue, entry_four) == ALT_OK);
  assert(priority_queue_push(&queue, entry_five) == ALT_OK);

  // Create zero-initialized output entry
  alt_queue_entry out_entry = {0};

  // Create array of flags indicating which entries have been retrieved
  // Indexes of entries are entry # - 1
  bool retrieved_entry[] = {false, false, false, false, false};

  // Pop first entry and verify success
  assert(priority_queue_pop(&queue, &out_entry));

  // Verify output entry matches expected values of entry_two
  assert(out_entry.vertex == entry_two.vertex);
  assert(out_entry.recorded_distance == entry_two.recorded_distance);
  assert(out_entry.priority == entry_two.priority);

  // Mark the entry as retrieved
  retrieved_entry[1] = true;

  // Create a temporary entry to hold comparison values
  alt_queue_entry temp_entry;

  // Next three popped entries will have same priority
  for (size_t i = 0; i < 3; i++) {
    // Pop next entry and verify success
    assert(priority_queue_pop(&queue, &out_entry));

    // Determine which entry was retrieved via its vertex, which
    // correlates to its entry number for this test.
    // Make sure the retrieved entry is either entry_one, entry_three, or
    // entry_five
    assert(out_entry.vertex == 1 || out_entry.vertex == 3 ||
           out_entry.vertex == 5);

    // Ensure the entry has not been seen before and mark it as retrieved
    assert(!retrieved_entry[out_entry.vertex - 1]);
    retrieved_entry[out_entry.vertex - 1] = true;

    // Assign the appropriate entry values to temp_entry
    switch (out_entry.vertex) {
    case 1:
      temp_entry = entry_one;
      break;
    case 3:
      temp_entry = entry_three;
      break;
    case 5:
      temp_entry = entry_five;
    }

    // Verify output entry matches expected values of temp entry
    assert(out_entry.vertex == temp_entry.vertex);
    assert(out_entry.recorded_distance == temp_entry.recorded_distance);
    assert(out_entry.priority == temp_entry.priority);
  }

  // Pop fifth entry and verify success
  assert(priority_queue_pop(&queue, &out_entry));

  // Verify output entry matches expected values of entry_four
  assert(out_entry.vertex == entry_four.vertex);
  assert(out_entry.recorded_distance == entry_four.recorded_distance);
  assert(out_entry.priority == entry_four.priority);

  // Mark the entry as retrieved
  retrieved_entry[3] = true;

  // Verify that all five entries were seen
  for (size_t i = 0; i < 5; i++) {
    assert(retrieved_entry[i]);
  }

  // Verify that the queue is now empty
  assert(priority_queue_is_empty(&queue));

  // Cleanup test allocations
  priority_queue_destroy(&queue);
}

// Verify correct ordering with negative, zero, and positive priorities
void test_negative_and_zero_priorities() {
  // Create a zero-initialized queue
  alt_priority_queue queue = {0};

  // Create several entries with negative, zero, and positive priorities
  alt_queue_entry entry_one = {1, 1.0, 1.0};
  alt_queue_entry entry_two = {2, 2.0, -1.0};
  alt_queue_entry entry_three = {3, 3.0, 0.0};

  // Insert entries in order and verify success
  assert(priority_queue_push(&queue, entry_one) == ALT_OK);
  assert(priority_queue_push(&queue, entry_two) == ALT_OK);
  assert(priority_queue_push(&queue, entry_three) == ALT_OK);

  // Create zero-initialized output entry
  alt_queue_entry out_entry = {0};

  // Pop first entry and verify success
  assert(priority_queue_pop(&queue, &out_entry));

  // Verify output entry matches expected values of entry_two
  assert(out_entry.vertex == entry_two.vertex);
  assert(out_entry.recorded_distance == entry_two.recorded_distance);
  assert(out_entry.priority == entry_two.priority);

  // Pop second entry and verify success
  assert(priority_queue_pop(&queue, &out_entry));

  // Verify output entry matches expected values of entry_three
  assert(out_entry.vertex == entry_three.vertex);
  assert(out_entry.recorded_distance == entry_three.recorded_distance);
  assert(out_entry.priority == entry_three.priority);

  // Pop third entry and verify success
  assert(priority_queue_pop(&queue, &out_entry));

  // Verify output entry matches expected values of entry_one
  assert(out_entry.vertex == entry_one.vertex);
  assert(out_entry.recorded_distance == entry_one.recorded_distance);
  assert(out_entry.priority == entry_one.priority);

  // Verify that the queue is now empty
  assert(priority_queue_is_empty(&queue));

  // Cleanup test allocations
  priority_queue_destroy(&queue);
}

// Verify that ordering depends on priority, not recorded_distance,
// and that both values are preserved during extraction
void test_priority_independent_of_distance() {
  // Create a zero-initialized queue
  alt_priority_queue queue = {0};

  // Create several entries with distances that would order differently than
  // priority
  alt_queue_entry entry_one = {1, 2.0, 10.0};
  alt_queue_entry entry_two = {2, 20.0, 1.0};
  alt_queue_entry entry_three = {3, 5.0, 5.0};

  // Insert entries in order and verify success
  assert(priority_queue_push(&queue, entry_one) == ALT_OK);
  assert(priority_queue_push(&queue, entry_two) == ALT_OK);
  assert(priority_queue_push(&queue, entry_three) == ALT_OK);

  // Create zero-initialized output entry
  alt_queue_entry out_entry = {0};

  // Pop first entry and verify success
  assert(priority_queue_pop(&queue, &out_entry));

  // Verify output entry matches expected values of entry_two
  assert(out_entry.vertex == entry_two.vertex);
  assert(out_entry.recorded_distance == entry_two.recorded_distance);
  assert(out_entry.priority == entry_two.priority);

  // Pop second entry and verify success
  assert(priority_queue_pop(&queue, &out_entry));

  // Verify output entry matches expected values of entry_three
  assert(out_entry.vertex == entry_three.vertex);
  assert(out_entry.recorded_distance == entry_three.recorded_distance);
  assert(out_entry.priority == entry_three.priority);

  // Pop third entry and verify success
  assert(priority_queue_pop(&queue, &out_entry));

  // Verify output entry matches expected values of entry_one
  assert(out_entry.vertex == entry_one.vertex);
  assert(out_entry.recorded_distance == entry_one.recorded_distance);
  assert(out_entry.priority == entry_one.priority);

  // Verify that the queue is now empty
  assert(priority_queue_is_empty(&queue));

  // Cleanup test allocations
  priority_queue_destroy(&queue);
}

// Insert multiple entries for the same vertex with different priorities
// and recorded distances. Verify that each remains an independent queue entry
void test_duplicate_vertices() {
  // Create a zero-initialized queue
  alt_priority_queue queue = {0};

  // Create several entries including two sharing a vertex
  alt_queue_entry entry_one = {1, 2.0, 10.0};
  alt_queue_entry entry_two = {1, 20.0, 1.0};
  alt_queue_entry entry_three = {2, 5.0, 5.0};

  // Insert entries in order and verify success
  assert(priority_queue_push(&queue, entry_one) == ALT_OK);
  assert(priority_queue_push(&queue, entry_two) == ALT_OK);
  assert(priority_queue_push(&queue, entry_three) == ALT_OK);

  // Create zero-initialized output entry
  alt_queue_entry out_entry = {0};

  // Pop first entry and verify success
  assert(priority_queue_pop(&queue, &out_entry));

  // Verify output entry matches expected values of entry_two
  assert(out_entry.vertex == entry_two.vertex);
  assert(out_entry.recorded_distance == entry_two.recorded_distance);
  assert(out_entry.priority == entry_two.priority);

  // Pop second entry and verify success
  assert(priority_queue_pop(&queue, &out_entry));

  // Verify output entry matches expected values of entry_three
  assert(out_entry.vertex == entry_three.vertex);
  assert(out_entry.recorded_distance == entry_three.recorded_distance);
  assert(out_entry.priority == entry_three.priority);

  // Pop third entry and verify success
  assert(priority_queue_pop(&queue, &out_entry));

  // Verify output entry matches expected values of entry_one
  assert(out_entry.vertex == entry_one.vertex);
  assert(out_entry.recorded_distance == entry_one.recorded_distance);
  assert(out_entry.priority == entry_one.priority);

  // Verify that the queue is now empty
  assert(priority_queue_is_empty(&queue));

  // Cleanup test allocations
  priority_queue_destroy(&queue);
}

// Insert enough entries to force multiple reallocations, checking queue
// size, capacity, and extraction correctness
void test_capacity_growth() {
  // Create a zero-initialized queue
  alt_priority_queue queue = {0};

  // Create and insert enough entries to trigger two reallocations
  for (size_t i = 0; i < 20; i++) {
    // Create a queue entry
    alt_queue_entry entry = {i, i * 1.0, 20.0 - i};

    // Insert the entry and ensure success
    assert(priority_queue_push(&queue, entry) == ALT_OK);
  }

  // Make sure the queue's size has grown to twenty
  assert(queue.size == 20);

  // Make sure the queue's capacity has grown to at least twenty
  assert(queue.capacity >= 20);

  // Make sure the queue has a valid entries pointer
  assert(queue.entries != NULL);

  // Create zero-initialized output entry
  alt_queue_entry out_entry = {0};

  // Pop and verify all entries
  for (size_t i = 0; i < 20; i++) {
    // Pop next entry and verify success
    assert(priority_queue_pop(&queue, &out_entry));

    // Verify output entry matches expected values
    assert(out_entry.vertex == 19 - i);
    assert(out_entry.recorded_distance == 19.0 - i);
    assert(out_entry.priority == i + 1);
  }

  // Verify that the queue is now empty
  assert(priority_queue_is_empty(&queue));

  // Cleanup test allocations
  priority_queue_destroy(&queue);
}

// Alternate pushes and pops, including inserting a new minimum after
// earlier extractions. Verify ordering throughout
void test_interleaved_operations() {
  // Create a zero-initialized queue
  alt_priority_queue queue = {0};

  // Create several entries including a new minimum that will occur after
  // earlier extractions
  alt_queue_entry entry_one = {1, 1.0, 5.0};
  alt_queue_entry entry_two = {2, 2.0, 2.0};
  alt_queue_entry entry_three = {3, 3.0, 1.0};
  alt_queue_entry entry_four = {4, 4.0, 4.0};
  alt_queue_entry entry_five = {5, 5.0, 3.0};

  // Insert entry_one and ensure success
  assert(priority_queue_push(&queue, entry_one) == ALT_OK);

  // Insert entry_two and ensure success
  assert(priority_queue_push(&queue, entry_two) == ALT_OK);

  // Create zero-initialized output entry
  alt_queue_entry out_entry = {0};

  // Pop entry and verify success
  assert(priority_queue_pop(&queue, &out_entry));

  // Verify output entry matches expected values of entry_two
  assert(out_entry.vertex == entry_two.vertex);
  assert(out_entry.recorded_distance == entry_two.recorded_distance);
  assert(out_entry.priority == entry_two.priority);

  // Insert entry_three and ensure success
  assert(priority_queue_push(&queue, entry_three) == ALT_OK);

  // Insert entry_four and ensure success
  assert(priority_queue_push(&queue, entry_four) == ALT_OK);

  // Pop entry and verify success
  assert(priority_queue_pop(&queue, &out_entry));

  // Verify output entry matches expected values of entry_three
  assert(out_entry.vertex == entry_three.vertex);
  assert(out_entry.recorded_distance == entry_three.recorded_distance);
  assert(out_entry.priority == entry_three.priority);

  // Insert entry_five and ensure success
  assert(priority_queue_push(&queue, entry_five) == ALT_OK);

  // Pop entry and verify success
  assert(priority_queue_pop(&queue, &out_entry));

  // Verify output entry matches expected values of entry_five
  assert(out_entry.vertex == entry_five.vertex);
  assert(out_entry.recorded_distance == entry_five.recorded_distance);
  assert(out_entry.priority == entry_five.priority);

  // Pop entry and verify success
  assert(priority_queue_pop(&queue, &out_entry));

  // Verify output entry matches expected values of entry_four
  assert(out_entry.vertex == entry_four.vertex);
  assert(out_entry.recorded_distance == entry_four.recorded_distance);
  assert(out_entry.priority == entry_four.priority);

  // Pop entry and verify success
  assert(priority_queue_pop(&queue, &out_entry));

  // Verify output entry matches expected values of entry_one
  assert(out_entry.vertex == entry_one.vertex);
  assert(out_entry.recorded_distance == entry_one.recorded_distance);
  assert(out_entry.priority == entry_one.priority);

  // Verify that the queue is now empty
  assert(priority_queue_is_empty(&queue));

  // Cleanup test allocations
  priority_queue_destroy(&queue);
}

// Verify that NaN, positive infinity, and negative infinity are rejected
// in both priority and recorded_distance, without changing the queue
void test_invalid_numeric_entries() {
  // Create a zero-initialized queue
  alt_priority_queue queue = {0};

  // Create two valid entries
  alt_queue_entry entry_one = {1, 1.0, 1.0};
  alt_queue_entry entry_two = {2, 2.0, 2.0};

  // Insert entries in order and verify success
  assert(priority_queue_push(&queue, entry_one) == ALT_OK);
  assert(priority_queue_push(&queue, entry_two) == ALT_OK);

  // Record the queue's state
  alt_queue_entry *entries = queue.entries;
  size_t size = queue.size;
  size_t capacity = queue.capacity;

  // Create invalid entry with NaN recorded_distance
  alt_queue_entry invalid_entry = {3, NAN, 1.0};

  // Attempt invalid entry insertion and verify failure
  assert(priority_queue_push(&queue, invalid_entry) == ALT_NUMERIC_ERROR);

  // Update invalid entry with positive infinity recorded_distance
  invalid_entry = (alt_queue_entry){3, INFINITY, 1.0};

  // Attempt invalid entry insertion and verify failure
  assert(priority_queue_push(&queue, invalid_entry) == ALT_NUMERIC_ERROR);

  // Update invalid entry with negative infinity recorded_distance
  invalid_entry = (alt_queue_entry){3, -INFINITY, 1.0};

  // Attempt invalid entry insertion and verify failure
  assert(priority_queue_push(&queue, invalid_entry) == ALT_NUMERIC_ERROR);

  // Update invalid entry with NaN priority
  invalid_entry = (alt_queue_entry){3, 1.0, NAN};

  // Attempt invalid entry insertion and verify failure
  assert(priority_queue_push(&queue, invalid_entry) == ALT_NUMERIC_ERROR);

  // Update invalid entry with positive infinity priority
  invalid_entry = (alt_queue_entry){3, 1.0, INFINITY};

  // Attempt invalid entry insertion and verify failure
  assert(priority_queue_push(&queue, invalid_entry) == ALT_NUMERIC_ERROR);

  // Update invalid entry with negative infinity priority
  invalid_entry = (alt_queue_entry){3, 1.0, -INFINITY};

  // Attempt invalid entry insertion and verify failure
  assert(priority_queue_push(&queue, invalid_entry) == ALT_NUMERIC_ERROR);

  // Verify queue's state was preserved
  assert(queue.entries == entries);
  assert(queue.size == size);
  assert(queue.capacity == capacity);

  // Create zero-initialized output entry
  alt_queue_entry out_entry = {0};

  // Pop entry and verify success
  assert(priority_queue_pop(&queue, &out_entry));

  // Verify output entry matches expected values of entry_one
  assert(out_entry.vertex == entry_one.vertex);
  assert(out_entry.recorded_distance == entry_one.recorded_distance);
  assert(out_entry.priority == entry_one.priority);

  // Pop entry and verify success
  assert(priority_queue_pop(&queue, &out_entry));

  // Verify output entry matches expected values of entry_two
  assert(out_entry.vertex == entry_two.vertex);
  assert(out_entry.recorded_distance == entry_two.recorded_distance);
  assert(out_entry.priority == entry_two.priority);

  // Verify that the queue is now empty
  assert(priority_queue_is_empty(&queue));

  // Cleanup test allocations
  priority_queue_destroy(&queue);
}

// Verify destruction releases storage, resets all fields, supports
// repeated destruction, and allows the queue to be reused
void test_destroy_and_reuse() {
  // Create a zero-initialized queue
  alt_priority_queue queue = {0};

  // Create two valid entries
  alt_queue_entry entry_one = {1, 1.0, 1.0};
  alt_queue_entry entry_two = {2, 2.0, 2.0};

  // Insert entries in order and verify success
  assert(priority_queue_push(&queue, entry_one) == ALT_OK);
  assert(priority_queue_push(&queue, entry_two) == ALT_OK);

  // Verify that the queue is not empty
  assert(!priority_queue_is_empty(&queue));

  // Destroy the queue
  priority_queue_destroy(&queue);

  // Destroy the queue again to verify repeated destruction is safe
  priority_queue_destroy(&queue);

  // Verify the queue has been reset to a zero-initialized state
  assert(queue.entries == NULL);
  assert(queue.size == 0);
  assert(queue.capacity == 0);

  // Verify that the queue now reports itself as empty
  assert(priority_queue_is_empty(&queue));

  // Insert an entry and verify success
  assert(priority_queue_push(&queue, entry_one) == ALT_OK);

  // Create zero-initialized output entry
  alt_queue_entry out_entry = {0};

  // Pop entry and verify success
  assert(priority_queue_pop(&queue, &out_entry));

  // Verify output entry matches expected values of entry_one
  assert(out_entry.vertex == entry_one.vertex);
  assert(out_entry.recorded_distance == entry_one.recorded_distance);
  assert(out_entry.priority == entry_one.priority);

  // Verify that the queue is now empty
  assert(priority_queue_is_empty(&queue));

  // Cleanup test allocations
  priority_queue_destroy(&queue);
}

// Operate on two separate queues and verify that modifying or destroying
// one doesn't affect the other
void test_independent_queues() {
  // Create two zero-initialized queues
  alt_priority_queue queue_one = {0};
  alt_priority_queue queue_two = {0};

  // Create several valid entries
  alt_queue_entry entry_one = {1, 1.0, 5.0};
  alt_queue_entry entry_two = {2, 2.0, 2.0};
  alt_queue_entry entry_three = {3, 3.0, 4.0};
  alt_queue_entry entry_four = {4, 4.0, 1.0};
  alt_queue_entry entry_five = {5, 5.0, 3.0};

  // Insert entry_one and entry_two to queue_one and ensure success
  assert(priority_queue_push(&queue_one, entry_one) == ALT_OK);
  assert(priority_queue_push(&queue_one, entry_two) == ALT_OK);

  // Insert entry_three and entry_four to queue_two and ensure success
  assert(priority_queue_push(&queue_two, entry_three) == ALT_OK);
  assert(priority_queue_push(&queue_two, entry_four) == ALT_OK);

  // Record queue_two's state
  alt_queue_entry *queue_two_entries = queue_two.entries;
  size_t queue_two_size = queue_two.size;
  size_t queue_two_capacity = queue_two.capacity;

  // Create zero-initialized output entry
  alt_queue_entry out_entry = {0};

  // Pop entry from queue_one and verify success
  assert(priority_queue_pop(&queue_one, &out_entry));

  // Verify output entry matches expected values of entry_two
  assert(out_entry.vertex == entry_two.vertex);
  assert(out_entry.recorded_distance == entry_two.recorded_distance);
  assert(out_entry.priority == entry_two.priority);

  // Verify queue_two's state is unchanged
  assert(queue_two.entries == queue_two_entries);
  assert(queue_two.size == queue_two_size);
  assert(queue_two.capacity == queue_two_capacity);

  // Record queue_one's state
  alt_queue_entry *queue_one_entries = queue_one.entries;
  size_t queue_one_size = queue_one.size;
  size_t queue_one_capacity = queue_one.capacity;

  // Pop entry from queue_two and verify success
  assert(priority_queue_pop(&queue_two, &out_entry));

  // Verify output entry matches expected values of entry_four
  assert(out_entry.vertex == entry_four.vertex);
  assert(out_entry.recorded_distance == entry_four.recorded_distance);
  assert(out_entry.priority == entry_four.priority);

  // Verify queue_one's state is unchanged
  assert(queue_one.entries == queue_one_entries);
  assert(queue_one.size == queue_one_size);
  assert(queue_one.capacity == queue_one_capacity);

  // Verify queue_one's remaining entry matches expected values of entry_one
  assert(queue_one.entries[0].vertex == entry_one.vertex);
  assert(queue_one.entries[0].recorded_distance == entry_one.recorded_distance);
  assert(queue_one.entries[0].priority == entry_one.priority);

  // Destroy queue_one and verify queue_two is still non-empty
  priority_queue_destroy(&queue_one);
  assert(!priority_queue_is_empty(&queue_two));

  // Insert entry_five to queue_two and ensure success
  assert(priority_queue_push(&queue_two, entry_five) == ALT_OK);

  // Pop entry from queue_two and verify success
  assert(priority_queue_pop(&queue_two, &out_entry));

  // Verify output entry matches expected values of entry_five
  assert(out_entry.vertex == entry_five.vertex);
  assert(out_entry.recorded_distance == entry_five.recorded_distance);
  assert(out_entry.priority == entry_five.priority);

  // Pop entry from queue_two and verify success
  assert(priority_queue_pop(&queue_two, &out_entry));

  // Verify output entry matches expected values of entry_three
  assert(out_entry.vertex == entry_three.vertex);
  assert(out_entry.recorded_distance == entry_three.recorded_distance);
  assert(out_entry.priority == entry_three.priority);

  // Verify that queue_two is now empty
  assert(priority_queue_is_empty(&queue_two));

  // Cleanup test allocations
  priority_queue_destroy(&queue_two);
}

// Executes tests
int main() {
  // Execute tests
  test_empty_queue();
  test_null_arguments();
  test_single_entry();
  test_ascending_priorities();
  test_descending_priorities();
  test_mixed_priorities();
  test_single_child_restoration();
  test_right_child_restoration();
  test_equal_priorities();
  test_negative_and_zero_priorities();
  test_priority_independent_of_distance();
  test_duplicate_vertices();
  test_capacity_growth();
  test_interleaved_operations();
  test_invalid_numeric_entries();
  test_destroy_and_reuse();
  test_independent_queues();

  // All tests passed
  return 0;
}
