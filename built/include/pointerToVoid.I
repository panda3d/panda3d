/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pointerToVoid.I
 * @author drose
 * @date 2004-09-27
 */

/**
 *
 */
//INLINE PointerToVoid::
//~PointerToVoid() {
//  nassertv(_void_ptr == nullptr);
//}

/**
 * Returns true if the PointerTo is a NULL pointer, false otherwise.  (Direct
 * comparison to a NULL pointer also works.)
 */
constexpr bool PointerToVoid::
is_null() const {
  return _void_ptr == nullptr;
}

/**
 *
 */
INLINE size_t PointerToVoid::
get_hash() const {
  return (size_t)_void_ptr;
}

/**
 *
 */
INLINE bool PointerToVoid::
operator < (const void *other) const {
  return _void_ptr < other;
}

/**
 *
 */
INLINE bool PointerToVoid::
operator < (const PointerToVoid &other) const {
  return _void_ptr < other._void_ptr;
}

/**
 *
 */
INLINE bool PointerToVoid::
operator == (const PointerToVoid &other) const {
  return _void_ptr == other._void_ptr;
}

/**
 *
 */
INLINE bool PointerToVoid::
operator != (const PointerToVoid &other) const {
  return _void_ptr != other._void_ptr;
}

/**
 * Swaps the contents of this PointerTo with the other, without touching the
 * reference counts.
 *
 * For internal use only.  Use the global swap() function instead.
 */
INLINE void PointerToVoid::
swap(PointerToVoid &other) noexcept {
  AtomicAdjust::Pointer temp = _void_ptr;
  _void_ptr = other._void_ptr;
  other._void_ptr = temp;
}
