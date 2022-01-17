/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file circularAllocator.cxx
 * @author rdb
 * @date 2022-01-16
 */

#include "circularAllocator.h"

/**
 * Advances the head.  Returns the new offset, or -1 if there was no space for
 * a contiguous allocation of this size and alignment.
 */
ssize_t CircularAllocator::
alloc(size_t size, size_t alignment) {
  if (size > _capacity) {
    return -1;
  }

  // If the head pointer changes while we calculate the new head pointer,
  // repeat the process.
  size_t prev_head;
  size_t this_head;
  size_t next_head;
  do {
    prev_head = get_head();

    // Align.
    this_head = prev_head;
    if (alignment > 1) {
      this_head = (prev_head + alignment - 1) / alignment * alignment;
    }
    next_head = this_head + size;

    if ((size_t)next_head > _capacity) {
      // Write to the beginning (we want a continuous block).
      this_head = 0;
      next_head = size;

      // Add padding to ensure that the next allocation will be aligned.
      size_t min_alignment = _min_alignment;
      if (min_alignment > 0) {
        next_head = (next_head + min_alignment - 1) / min_alignment * min_alignment;
      }

      size_t this_tail = get_tail();
      if (next_head >= this_tail) {
        return -1;
      }
    }
    else if ((size_t)next_head == _capacity) {
      // Write until the end.
      // Set next_head to _capacity (instead of 0) so that we can fill
      // up the buffer if the read pointer is at 0.  This is the only
      // situation in which the buffer can be fully filled up.
      next_head = _capacity;
      size_t this_tail = get_tail();
      if (this_tail > prev_head) {
        return -1;
      }
    }
    else {
      // Add padding to ensure that the next allocation will be aligned to the
      // minimum alignment.
      size_t min_alignment = _min_alignment;
      if (min_alignment > 1) {
        // It's possible that the next head now falls past the capacity (if
        // the capacity isn't a multiple of the alignment) so we clamp it.
        next_head = std::min(_capacity, (next_head + min_alignment - 1) / min_alignment * min_alignment);
      }

      // Don't pass the read pointer.
      size_t this_tail = get_tail();
      if ((prev_head >= this_tail) != (next_head >= this_tail)) {
        return -1;
      }
    }
  }
  while ((size_t)AtomicAdjust::compare_and_exchange(_head, prev_head, next_head) != prev_head);

  return (ssize_t)this_head;
}
