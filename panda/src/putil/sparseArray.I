// Filename: sparseArray.I
// Created by:  drose (14Feb07)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: SparseArray::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE SparseArray::
SparseArray() : _inverse(false) {
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE SparseArray::
SparseArray(const SparseArray &copy) :
  _subranges(copy._subranges),
  _inverse(copy._inverse)
{
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::Copy Assignment Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE SparseArray &SparseArray::
operator = (const SparseArray &copy) {
  _subranges = copy._subranges;
  _inverse = copy._inverse;
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::Named all_on constructor
//       Access: Published, Static
//  Description: Returns a SparseArray with an infinite array of bits,
//               all on.
////////////////////////////////////////////////////////////////////
INLINE SparseArray SparseArray::
all_on() {
  SparseArray result;
  result._inverse = true;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::Named all_on constructor
//       Access: Published, Static
//  Description: Returns a SparseArray whose bits are all off.
////////////////////////////////////////////////////////////////////
INLINE SparseArray SparseArray::
all_off() {
  return SparseArray();
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::Named lower_on constructor
//       Access: Published, Static
//  Description: Returns a SparseArray whose lower on_bits bits are on.
////////////////////////////////////////////////////////////////////
INLINE SparseArray SparseArray::
lower_on(int on_bits) {
  SparseArray result;
  result.set_range(0, on_bits);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::Named bit constructor
//       Access: Published, Static
//  Description: Returns a SparseArray with only the indicated bit on.
////////////////////////////////////////////////////////////////////
INLINE SparseArray SparseArray::
bit(int index) {
  SparseArray result;
  result.set_bit(index);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::Named range constructor
//       Access: Published, Static
//  Description: Returns a SparseArray whose size bits, beginning at
//               low_bit, are on.
////////////////////////////////////////////////////////////////////
INLINE SparseArray SparseArray::
range(int low_bit, int size) {
  SparseArray result;
  result.set_range(low_bit, size);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE SparseArray::
~SparseArray() {
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::has_max_num_bits
//       Access: Published, Static
//  Description: Returns true if there is a maximum number of bits
//               that may be stored in this structure, false
//               otherwise.  If this returns true, the number may be
//               queried in get_max_num_bits().
//
//               This method always returns false.  The SparseArray has
//               no maximum number of bits.  This method is defined so
//               generic programming algorithms can use BitMask or
//               SparseArray interchangeably.
////////////////////////////////////////////////////////////////////
INLINE bool SparseArray::
has_max_num_bits() {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::get_max_num_bits
//       Access: Published, Static
//  Description: If get_max_num_bits() returned true, this method may
//               be called to return the maximum number of bits that
//               may be stored in this structure.  It is an error to
//               call this if get_max_num_bits() return false.
//
//               It is always an error to call this method.  The
//               SparseArray has no maximum number of bits.  This method
//               is defined so generic programming algorithms can use
//               BitMask or SparseArray interchangeably.
////////////////////////////////////////////////////////////////////
INLINE int SparseArray::
get_max_num_bits() {
  nassertr(false, 0);
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::get_num_bits
//       Access: Published
//  Description: Returns the current number of possibly different bits
//               in this array.  There are actually an infinite number
//               of bits, but every bit higher than this bit will have
//               the same value, either 0 or 1 (see
//               get_highest_bits()).
//
//               This number may grow and/or shrink automatically as
//               needed.
////////////////////////////////////////////////////////////////////
INLINE int SparseArray::
get_num_bits() const {
  if (_subranges.empty()) {
    return 0;
  } else {
    Subranges::const_iterator si = _subranges.begin() + _subranges.size() - 1;
    return (*si)._end;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::get_bit
//       Access: Published
//  Description: Returns true if the nth bit is set, false if it is
//               cleared.  It is valid for n to increase beyond
//               get_num_bits(), but the return value get_num_bits()
//               will always be the same.
////////////////////////////////////////////////////////////////////
INLINE bool SparseArray::
get_bit(int index) const {
  return has_any_of(index, 1);
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::set_bit
//       Access: Published
//  Description: Sets the nth bit on.  If n >= get_num_bits(), this
//               automatically extends the array.
////////////////////////////////////////////////////////////////////
INLINE void SparseArray::
set_bit(int index) {
  set_range(index, 1);
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::clear_bit
//       Access: Published
//  Description: Sets the nth bit off.  If n >= get_num_bits(), this
//               automatically extends the array.
////////////////////////////////////////////////////////////////////
INLINE void SparseArray::
clear_bit(int index) {
  clear_range(index, 1);
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::set_bit_to
//       Access: Published
//  Description: Sets the nth bit either on or off, according to the
//               indicated bool value.
////////////////////////////////////////////////////////////////////
INLINE void SparseArray::
set_bit_to(int index, bool value) {
  if (value) {
    set_bit(index);
  } else {
    clear_bit(index);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::get_highest_bits
//       Access: Published
//  Description: Returns true if the infinite set of bits beyond
//               get_num_bits() are all on, or false of they are all
//               off.
////////////////////////////////////////////////////////////////////
INLINE bool SparseArray::
get_highest_bits() const {
  return _inverse;
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::is_zero
//       Access: Published
//  Description: Returns true if the entire bitmask is zero, false
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool SparseArray::
is_zero() const {
  if (_inverse) {
    return false;
  } else {
    return _subranges.empty();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::is_all_on
//       Access: Published
//  Description: Returns true if the entire bitmask is one, false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool SparseArray::
is_all_on() const {
  if (_inverse) {
    return _subranges.empty();
  } else {
    return false;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::has_any_of
//       Access: Published
//  Description: Returns true if any bit in the indicated range is
//               set, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool SparseArray::
has_any_of(int low_bit, int size) const {
  if (_inverse) {
    return !do_has_all(low_bit, low_bit + size);
  } else {
    return do_has_any(low_bit, low_bit + size);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::has_all_of
//       Access: Published
//  Description: Returns true if all bits in the indicated range are
//               set, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool SparseArray::
has_all_of(int low_bit, int size) const {
  if (_inverse) {
    return !do_has_any(low_bit, low_bit + size);
  } else {
    return do_has_all(low_bit, low_bit + size);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::set_range
//       Access: Published
//  Description: Sets the indicated range of bits on.
////////////////////////////////////////////////////////////////////
INLINE void SparseArray::
set_range(int low_bit, int size) {
  if (_inverse) {
    return do_remove_range(low_bit, low_bit + size);
  } else {
    return do_add_range(low_bit, low_bit + size);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::clear_range
//       Access: Published
//  Description: Sets the indicated range of bits off.
////////////////////////////////////////////////////////////////////
INLINE void SparseArray::
clear_range(int low_bit, int size) {
  if (_inverse) {
    return do_add_range(low_bit, low_bit + size);
  } else {
    return do_remove_range(low_bit, low_bit + size);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::set_range_to
//       Access: Published
//  Description: Sets the indicated range of bits to either on or off.
////////////////////////////////////////////////////////////////////
INLINE void SparseArray::
set_range_to(bool value, int low_bit, int size) {
  if (value) {
    set_range(low_bit, size);
  } else {
    clear_range(low_bit, size);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::invert_in_place
//       Access: Published
//  Description: Inverts all the bits in the SparseArray.  This is
//               equivalent to array = ~array.
////////////////////////////////////////////////////////////////////
void SparseArray::
invert_in_place() {
  _inverse = !_inverse;
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::clear
//       Access: Published
//  Description: Sets all the bits in the SparseArray off.
////////////////////////////////////////////////////////////////////
void SparseArray::
clear() {
  _subranges.clear();
  _inverse = false;
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::operator ==
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool SparseArray::
operator == (const SparseArray &other) const {
  return compare_to(other) == 0;
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::operator !=
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool SparseArray::
operator != (const SparseArray &other) const {
  return compare_to(other) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::operator <
//       Access: Published
//  Description: Returns true if the unsigned integer which is
//               represented by this SparseArray is less than that of the
//               other one, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool SparseArray::
operator < (const SparseArray &other) const {
  return compare_to(other) < 0;
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::operator &
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE SparseArray SparseArray::
operator & (const SparseArray &other) const {
  SparseArray result(*this);
  result &= other;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::operator |
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE SparseArray SparseArray::
operator | (const SparseArray &other) const {
  SparseArray result(*this);
  result |= other;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::operator ^
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE SparseArray SparseArray::
operator ^ (const SparseArray &other) const {
  SparseArray result(*this);
  result ^= other;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::operator ~
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE SparseArray SparseArray::
operator ~ () const {
  SparseArray result(*this);
  result.invert_in_place();
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::operator <<
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE SparseArray SparseArray::
operator << (int shift) const {
  SparseArray result(*this);
  result <<= shift;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::operator >>
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE SparseArray SparseArray::
operator >> (int shift) const {
  SparseArray result(*this);
  result >>= shift;
  return result;
}


////////////////////////////////////////////////////////////////////
//     Function: SparseArray::operator <<=
//       Access: Published
//  Description: Logical left shift.  Since negative bit positions
//               have meaning in a SparseArray, real bit values are
//               rotated in on the left (not necessarily zero).
////////////////////////////////////////////////////////////////////
void SparseArray::
operator <<= (int shift) {
  do_shift(shift);
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::operator >>=
//       Access: Published
//  Description: Logical right shift.  The rightmost bits become
//               negative, but are not lost; they will reappear into
//               the zero position if the array is later left-shifted.
////////////////////////////////////////////////////////////////////
void SparseArray::
operator >>= (int shift) {
  do_shift(-shift);
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::is_inverse
//       Access: Published
//  Description: If this is true, the SparseArray is actually defined
//               as a list of subranges of integers that are *not* in
//               the set.  If this is false (the default), then the
//               subranges define the integers that *are* in the set.
//               This affects the interpretation of the values
//               returned by iterating through get_num_subranges().
////////////////////////////////////////////////////////////////////
INLINE bool SparseArray::
is_inverse() const {
  return _inverse;
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::get_num_subranges
//       Access: Published
//  Description: Returns the number of separate subranges stored in
//               the SparseArray.  You can use this limit to iterate
//               through the subranges, calling get_subrange_begin()
//               and get_subrange_end() for each one.
//
//               Also see is_inverse().
////////////////////////////////////////////////////////////////////
INLINE int SparseArray::
get_num_subranges() const {
  return _subranges.size();
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::get_subrange_begin
//       Access: Published
//  Description: Returns the first numeric element in the nth
//               subrange.
//
//               Also see is_inverse().
////////////////////////////////////////////////////////////////////
INLINE int SparseArray::
get_subrange_begin(int n) const {
  nassertr(n >= 0 && n < (int)_subranges.size(), 0);
  return _subranges[n]._begin;
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::get_subrange_end
//       Access: Published
//  Description: Returns the last numeric element, plus one, in the
//               nth subrange.
//
//               Also see is_inverse().
////////////////////////////////////////////////////////////////////
INLINE int SparseArray::
get_subrange_end(int n) const {
  nassertr(n >= 0 && n < (int)_subranges.size(), 0);
  return _subranges[n]._end;
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::Subrange::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE SparseArray::Subrange::
Subrange(int begin, int end) :
  _begin(begin),
  _end(end)
{
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::Subrange::operator <
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool SparseArray::Subrange::
operator < (const SparseArray::Subrange &other) const {
  // We compare the end values, rather than the begin values, to make
  // lower_bound() sensibly return a possible intersection with the
  // indicated Subrange.
  return _end < other._end;
}

