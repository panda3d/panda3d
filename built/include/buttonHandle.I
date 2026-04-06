/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file buttonHandle.I
 * @author drose
 * @date 2000-03-01
 */

/**
 * Constructs a ButtonHandle with the corresponding index number, which may
 * have been returned by an earlier call to ButtonHandle::get_index().
 */
constexpr ButtonHandle::
ButtonHandle(int index) : _index(index) {
}

/**
 *
 */
INLINE bool ButtonHandle::
operator == (const ButtonHandle &other) const {
  return (_index == other._index);
}

/**
 *
 */
INLINE bool ButtonHandle::
operator != (const ButtonHandle &other) const {
  return (_index != other._index);
}

/**
 *
 */
INLINE bool ButtonHandle::
operator < (const ButtonHandle &other) const {
  return (_index < other._index);
}

/**
 *
 */
INLINE bool ButtonHandle::
operator <= (const ButtonHandle &other) const {
  return (_index <= other._index);
}

/**
 *
 */
INLINE bool ButtonHandle::
operator > (const ButtonHandle &other) const {
  return (_index > other._index);
}

/**
 *
 */
INLINE bool ButtonHandle::
operator >= (const ButtonHandle &other) const {
  return (_index >= other._index);
}

/**
 * Sorts ButtonHandles arbitrarily (according to <, >, etc.).  Returns a
 * number less than 0 if this type sorts before the other one, greater than
 * zero if it sorts after, 0 if they are equivalent.
 */
INLINE int ButtonHandle::
compare_to(const ButtonHandle &other) const {
  return _index - other._index;
}

/**
 * Returns a hash code suitable for phash_map.
 */
INLINE size_t ButtonHandle::
get_hash() const {
  return (size_t)_index;
}

/**
 * Returns true if the button was created with an ASCII equivalent code (e.g.
 * for a standard keyboard button).
 */
INLINE bool ButtonHandle::
has_ascii_equivalent() const {
  return (_index > 0 && _index < 128);
}

/**
 * Returns the character code associated with the button, or '\0' if no ASCII
 * code was associated.
 */
INLINE char ButtonHandle::
get_ascii_equivalent() const {
  return has_ascii_equivalent() ? (char)_index : '\0';
}

/**
 * Returns true if this ButtonHandle is the same as the other one, or if the
 * other one is an alias for this one.  (Does not return true if this button
 * is an alias for the other one, however.)
 *
 * This is a more general comparison than operator ==.
 */
INLINE bool ButtonHandle::
matches(const ButtonHandle &other) const {
  return ((*this) == other ||
          (other != ButtonHandle::none() &&
           get_alias() == other));
}

/**
 * Returns the integer index associated with this ButtonHandle.  Each
 * different ButtonHandle will have a different index.  However, you probably
 * shouldn't be using this method; you should just treat the ButtonHandles as
 * opaque classes.  This is provided for the convenience of non-C++ scripting
 * languages to build a hashtable of ButtonHandles.
 */
constexpr int ButtonHandle::
get_index() const {
  return _index;
}

/**
 *
 */
INLINE void ButtonHandle::
output(std::ostream &out) const {
  out << get_name();
}

/**
 * ButtonHandle::none() evaluates to false, everything else evaluates to true.
 */
INLINE ButtonHandle::
operator bool () const {
  return (_index != 0);
}
