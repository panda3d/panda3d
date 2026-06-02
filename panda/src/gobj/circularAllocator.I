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
 * The atomic members force us to spell out the move operations.  Moves only
 * happen during setup, when the source is quiescent, so relaxed suffices.
 */
INLINE CircularAllocator::
CircularAllocator(CircularAllocator &&from) noexcept :
  _head(from._head.load(std::memory_order_relaxed)),
  _tail(from._tail.load(std::memory_order_relaxed)),
  _capacity(from._capacity),
  _min_alignment(from._min_alignment)
{
}

/**
 * See the move constructor.
 */
INLINE CircularAllocator &CircularAllocator::
operator = (CircularAllocator &&from) noexcept {
  _head.store(from._head.load(std::memory_order_relaxed), std::memory_order_relaxed);
  _tail.store(from._tail.load(std::memory_order_relaxed), std::memory_order_relaxed);
  _capacity = from._capacity;
  _min_alignment = from._min_alignment;
  return *this;
}

/**
 * Frees all memory.
 */
INLINE void CircularAllocator::
reset() {
  _head.store(0, std::memory_order_relaxed);
  _tail.store(0, std::memory_order_relaxed);
}

/**
 * Frees all memory and changes the capacity.
 */
INLINE void CircularAllocator::
reset(size_t capacity) {
  _head.store(0, std::memory_order_relaxed);
  _tail.store(0, std::memory_order_relaxed);
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
  return _head.load(std::memory_order_relaxed);
}

/**
 * Returns the offset to the tail of the buffer, or the "read pointer".
 */
INLINE size_t CircularAllocator::
get_tail() const {
  return _tail.load(std::memory_order_acquire);
}

/**
 * Sets the tail of the buffer, or the "read pointer", freeing up the memory
 * up to the given offset.
 */
INLINE void CircularAllocator::
set_tail(size_t tail) {
  _tail.store(tail, std::memory_order_release);
}
