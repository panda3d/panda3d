/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file memoryUsagePointerCounts.I
 * @author drose
 * @date 2001-06-04
 */

/**
 *
 */
INLINE MemoryUsagePointerCounts::
MemoryUsagePointerCounts() {
  _count = 0;
  _unknown_size_count = 0;
  _size = 0;
}

/**
 *
 */
INLINE MemoryUsagePointerCounts::
MemoryUsagePointerCounts(const MemoryUsagePointerCounts &copy) :
  _count(copy._count),
  _unknown_size_count(copy._unknown_size_count),
  _size(copy._size)
{
}

/**
 *
 */
INLINE void MemoryUsagePointerCounts::
operator = (const MemoryUsagePointerCounts &copy) {
  _count = copy._count;
  _unknown_size_count = copy._unknown_size_count;
  _size = copy._size;
}

/**
 * Resets the counter to empty.
 */
INLINE void MemoryUsagePointerCounts::
clear() {
  _count = 0;
  _unknown_size_count = 0;
  _size = 0;
}

/**
 * Returns true if none of the pointers in the count have a known size, or
 * false if at least one of them does.
 */
INLINE bool MemoryUsagePointerCounts::
is_size_unknown() const {
  return _unknown_size_count == _count;
}

/**
 * Returns the total allocated size of all pointers in the count whose size is
 * known.
 */
INLINE size_t MemoryUsagePointerCounts::
get_size() const {
  return _size;
}

/**
 * Returns the total number of pointers in the count.
 */
INLINE int MemoryUsagePointerCounts::
get_count() const {
  return _count;
}

/**
 *
 */
INLINE bool MemoryUsagePointerCounts::
operator < (const MemoryUsagePointerCounts &other) const {
  if (is_size_unknown() != other.is_size_unknown()) {
    return is_size_unknown() > other.is_size_unknown();
  }

  if (get_size() != other.get_size()) {
    return get_size() < other.get_size();
  }

  if (get_count() != other.get_count()) {
    return get_count() != other.get_count();
  }

  return false;
}

INLINE std::ostream &
operator << (std::ostream &out, const MemoryUsagePointerCounts &c) {
  c.output(out);
  return out;
}
