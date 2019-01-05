/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sparseArray.I
 * @author drose
 * @date 2007-02-14
 */

/**
 *
 */
INLINE SparseArray::
SparseArray() : _inverse(false) {
}

/**
 * Returns a SparseArray with an infinite array of bits, all on.
 */
INLINE SparseArray SparseArray::
all_on() {
  SparseArray result;
  result._inverse = true;
  return result;
}

/**
 * Returns a SparseArray whose bits are all off.
 */
INLINE SparseArray SparseArray::
all_off() {
  return SparseArray();
}

/**
 * Returns a SparseArray whose lower on_bits bits are on.
 */
INLINE SparseArray SparseArray::
lower_on(int on_bits) {
  SparseArray result;
  if (on_bits > 0) {
    result._subranges.push_back(Subrange(0, on_bits));
  }
  return result;
}

/**
 * Returns a SparseArray with only the indicated bit on.
 */
INLINE SparseArray SparseArray::
bit(int index) {
  SparseArray result;
  result.set_bit(index);
  return result;
}

/**
 * Returns a SparseArray whose size bits, beginning at low_bit, are on.
 */
INLINE SparseArray SparseArray::
range(int low_bit, int size) {
  SparseArray result;
  result.set_range(low_bit, size);
  return result;
}

/**
 * Returns true if there is a maximum number of bits that may be stored in
 * this structure, false otherwise.  If this returns true, the number may be
 * queried in get_max_num_bits().
 *
 * This method always returns false.  The SparseArray has no maximum number of
 * bits.  This method is defined so generic programming algorithms can use
 * BitMask or SparseArray interchangeably.
 */
INLINE bool SparseArray::
has_max_num_bits() {
  return false;
}

/**
 * If get_max_num_bits() returned true, this method may be called to return
 * the maximum number of bits that may be stored in this structure.  It is an
 * error to call this if get_max_num_bits() return false.
 *
 * It is always an error to call this method.  The SparseArray has no maximum
 * number of bits.  This method is defined so generic programming algorithms
 * can use BitMask or SparseArray interchangeably.
 */
INLINE int SparseArray::
get_max_num_bits() {
  nassert_raise("SparseArray has no maximum bit count");
  return 0;
}

/**
 * Returns the current number of possibly different bits in this array.  There
 * are actually an infinite number of bits, but every bit higher than this bit
 * will have the same value, either 0 or 1 (see get_highest_bits()).
 *
 * This number may grow and/or shrink automatically as needed.
 */
INLINE int SparseArray::
get_num_bits() const {
  if (_subranges.empty()) {
    return 0;
  } else {
    Subranges::const_iterator si = _subranges.begin() + _subranges.size() - 1;
    return (*si)._end;
  }
}

/**
 * Returns true if the nth bit is set, false if it is cleared.  It is valid
 * for n to increase beyond get_num_bits(), but the return value
 * get_num_bits() will always be the same.
 */
INLINE bool SparseArray::
get_bit(int index) const {
  return has_any_of(index, 1);
}

/**
 * Sets the nth bit on.  If n >= get_num_bits(), this automatically extends
 * the array.
 */
INLINE void SparseArray::
set_bit(int index) {
  set_range(index, 1);
}

/**
 * Sets the nth bit off.  If n >= get_num_bits(), this automatically extends
 * the array.
 */
INLINE void SparseArray::
clear_bit(int index) {
  clear_range(index, 1);
}

/**
 * Sets the nth bit either on or off, according to the indicated bool value.
 */
INLINE void SparseArray::
set_bit_to(int index, bool value) {
  if (value) {
    set_bit(index);
  } else {
    clear_bit(index);
  }
}

/**
 * Returns true if the infinite set of bits beyond get_num_bits() are all on,
 * or false of they are all off.
 */
INLINE bool SparseArray::
get_highest_bits() const {
  return _inverse;
}

/**
 * Returns true if the entire bitmask is zero, false otherwise.
 */
INLINE bool SparseArray::
is_zero() const {
  if (_inverse) {
    return false;
  } else {
    return _subranges.empty();
  }
}

/**
 * Returns true if the entire bitmask is one, false otherwise.
 */
bool SparseArray::
is_all_on() const {
  if (_inverse) {
    return _subranges.empty();
  } else {
    return false;
  }
}

/**
 * Returns true if any bit in the indicated range is set, false otherwise.
 */
INLINE bool SparseArray::
has_any_of(int low_bit, int size) const {
  if (_inverse) {
    return !do_has_all(low_bit, low_bit + size);
  } else {
    return do_has_any(low_bit, low_bit + size);
  }
}

/**
 * Returns true if all bits in the indicated range are set, false otherwise.
 */
INLINE bool SparseArray::
has_all_of(int low_bit, int size) const {
  if (_inverse) {
    return !do_has_any(low_bit, low_bit + size);
  } else {
    return do_has_all(low_bit, low_bit + size);
  }
}

/**
 * Sets the indicated range of bits on.
 */
INLINE void SparseArray::
set_range(int low_bit, int size) {
  if (_inverse) {
    return do_remove_range(low_bit, low_bit + size);
  } else {
    return do_add_range(low_bit, low_bit + size);
  }
}

/**
 * Sets the indicated range of bits off.
 */
INLINE void SparseArray::
clear_range(int low_bit, int size) {
  if (_inverse) {
    return do_add_range(low_bit, low_bit + size);
  } else {
    return do_remove_range(low_bit, low_bit + size);
  }
}

/**
 * Sets the indicated range of bits to either on or off.
 */
INLINE void SparseArray::
set_range_to(bool value, int low_bit, int size) {
  if (value) {
    set_range(low_bit, size);
  } else {
    clear_range(low_bit, size);
  }
}

/**
 * Inverts all the bits in the SparseArray.  This is equivalent to array =
 * ~array.
 */
void SparseArray::
invert_in_place() {
  _inverse = !_inverse;
}

/**
 * Sets all the bits in the SparseArray off.
 */
void SparseArray::
clear() {
  _subranges.clear();
  _inverse = false;
}

/**
 *
 */
INLINE bool SparseArray::
operator == (const SparseArray &other) const {
  return compare_to(other) == 0;
}

/**
 *
 */
INLINE bool SparseArray::
operator != (const SparseArray &other) const {
  return compare_to(other) != 0;
}

/**
 * Returns true if the unsigned integer which is represented by this
 * SparseArray is less than that of the other one, false otherwise.
 */
INLINE bool SparseArray::
operator < (const SparseArray &other) const {
  return compare_to(other) < 0;
}

/**
 *
 */
INLINE SparseArray SparseArray::
operator & (const SparseArray &other) const {
  SparseArray result(*this);
  result &= other;
  return result;
}

/**
 *
 */
INLINE SparseArray SparseArray::
operator | (const SparseArray &other) const {
  SparseArray result(*this);
  result |= other;
  return result;
}

/**
 *
 */
INLINE SparseArray SparseArray::
operator ^ (const SparseArray &other) const {
  SparseArray result(*this);
  result ^= other;
  return result;
}

/**
 *
 */
INLINE SparseArray SparseArray::
operator ~ () const {
  SparseArray result(*this);
  result.invert_in_place();
  return result;
}

/**
 *
 */
INLINE SparseArray SparseArray::
operator << (int shift) const {
  SparseArray result(*this);
  result <<= shift;
  return result;
}

/**
 *
 */
INLINE SparseArray SparseArray::
operator >> (int shift) const {
  SparseArray result(*this);
  result >>= shift;
  return result;
}


/**
 * Logical left shift.  Since negative bit positions have meaning in a
 * SparseArray, real bit values are rotated in on the left (not necessarily
 * zero).
 */
void SparseArray::
operator <<= (int shift) {
  do_shift(shift);
}

/**
 * Logical right shift.  The rightmost bits become negative, but are not lost;
 * they will reappear into the zero position if the array is later left-
 * shifted.
 */
void SparseArray::
operator >>= (int shift) {
  do_shift(-shift);
}

/**
 * If this is true, the SparseArray is actually defined as a list of subranges
 * of integers that are *not* in the set.  If this is false (the default),
 * then the subranges define the integers that *are* in the set.  This affects
 * the interpretation of the values returned by iterating through
 * get_num_subranges().
 */
INLINE bool SparseArray::
is_inverse() const {
  return _inverse;
}

/**
 * Returns the number of separate subranges stored in the SparseArray.  You
 * can use this limit to iterate through the subranges, calling
 * get_subrange_begin() and get_subrange_end() for each one.
 *
 * Also see is_inverse().
 */
INLINE size_t SparseArray::
get_num_subranges() const {
  return _subranges.size();
}

/**
 * Returns the first numeric element in the nth subrange.
 *
 * Also see is_inverse().
 */
INLINE int SparseArray::
get_subrange_begin(size_t n) const {
  nassertr(n < _subranges.size(), 0);
  return _subranges[n]._begin;
}

/**
 * Returns the last numeric element, plus one, in the nth subrange.
 *
 * Also see is_inverse().
 */
INLINE int SparseArray::
get_subrange_end(size_t n) const {
  nassertr(n < _subranges.size(), 0);
  return _subranges[n]._end;
}

/**
 *
 */
INLINE SparseArray::Subrange::
Subrange(int begin, int end) :
  _begin(begin),
  _end(end)
{
}

/**
 *
 */
INLINE bool SparseArray::Subrange::
operator < (const SparseArray::Subrange &other) const {
  // We compare the end values, rather than the begin values, to make
  // lower_bound() sensibly return a possible intersection with the indicated
  // Subrange.
  return _end < other._end;
}
