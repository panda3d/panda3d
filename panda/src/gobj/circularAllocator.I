/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file circularAllocator.I
 * @author rdb
 * @date 2022-01-16
 */

/**
 * Creates an allocator for a buffer with the given size.  If an alignment is
 * given, all allocations will satisfy the given alignment, even if a lower
 * alignment value is given in the call to alloc().
 */
INLINE CircularAllocator::
CircularAllocator(size_t capacity, size_t min_alignment) :
  _capacity(capacity),
  _min_alignment(min_alignment)
{
}

/**
 * Frees all memory.
 */
INLINE void CircularAllocator::
reset() {
  _head = 0;
  _tail = 0;
}

/**
 * Frees all memory and changes the capacity.
 */
INLINE void CircularAllocator::
reset(size_t capacity) {
  _head = 0;
  _tail = 0;
  _capacity = capacity;
}

/**
 * Returns the total capacity of the buffer.
 */
INLINE size_t CircularAllocator::
get_capacity() const {
  return _capacity;
}

/**
 * Returns the total size of all allocations.
 */
INLINE size_t CircularAllocator::
get_size() const {
  return (get_head() - get_tail()) % _capacity;
}

/**
 * Returns the offset to the head of the buffer, or the "write pointer".
 */
INLINE size_t CircularAllocator::
get_head() const {
  return (size_t)AtomicAdjust::get(_head);
}

/**
 * Returns the offset to the tail of the buffer, or the "read pointer".
 */
INLINE size_t CircularAllocator::
get_tail() const {
  return (size_t)AtomicAdjust::get(_tail);
}

/**
 * Sets the tail of the buffer, or the "read pointer", freeng up the memory
 * up to the given offset.
 */
INLINE void CircularAllocator::
set_tail(size_t tail) {
  AtomicAdjust::set(_tail, tail);
}
