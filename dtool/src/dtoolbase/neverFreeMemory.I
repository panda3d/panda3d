/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file neverFreeMemory.I
 * @author drose
 * @date 2007-06-14
 */

/**
 * Returns a pointer to a newly-allocated block of memory of the indicated
 * size.
 *
 * Please note that the resulting pointer is not aligned to any boundary.
 */
INLINE void *NeverFreeMemory::
alloc(size_t size) {
  return get_global_ptr()->ns_alloc(size);
}

/**
 * Returns the total number of bytes consumed by all the pages allocated
 * internally by this object.
 */
INLINE size_t NeverFreeMemory::
get_total_alloc() {
  return get_global_ptr()->_total_alloc;
}

/**
 * Returns the total number of bytes requested by the application in calls to
 * NeverFreeMemory::alloc().
 */
INLINE size_t NeverFreeMemory::
get_total_used() {
  return get_global_ptr()->_total_used;
}

/**
 * Returns the difference between get_total_alloc() and get_total_used().
 * This represents bytes in allocated pages that have not (yet) been used by
 * the application.
 */
INLINE size_t NeverFreeMemory::
get_total_unused() {
  NeverFreeMemory *global_ptr = get_global_ptr();
  global_ptr->_lock.lock();
  size_t total_unused = global_ptr->_total_alloc - global_ptr->_total_used;
  global_ptr->_lock.unlock();
  return total_unused;
}

/**
 *
 */
INLINE NeverFreeMemory *NeverFreeMemory::
get_global_ptr() {
  if (_global_ptr == nullptr) {
    make_global_ptr();
  }
  return _global_ptr;
}

/**
 *
 */
INLINE NeverFreeMemory::Page::
Page(void *start, size_t size) :
  _next((unsigned char *)start),
  _remaining(size)
{
}

/**
 *
 */
INLINE bool NeverFreeMemory::Page::
operator < (const NeverFreeMemory::Page &other) const {
  return _remaining < other._remaining;
}

/**
 *
 */
INLINE void *NeverFreeMemory::Page::
alloc(size_t size) {
  assert(size <= _remaining);
  void *result = _next;
  _next += size;
  _remaining -= size;
  return result;
}
