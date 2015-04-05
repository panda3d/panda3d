// Filename: bitArray.I
// Created by:  drose (20Jan06)
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
//     Function: BitArray::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BitArray::
BitArray() {
  _highest_bits = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: BitArray::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BitArray::
BitArray(WordType init_value) {
  if (init_value != 0) {
    _array.push_back(MaskType(init_value));
  }
  _highest_bits = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: BitArray::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BitArray::
BitArray(const BitArray &copy) :
  _array(copy._array),
  _highest_bits(copy._highest_bits)
{
}

////////////////////////////////////////////////////////////////////
//     Function: BitArray::Copy Assignment Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BitArray &BitArray::
operator = (const BitArray &copy) {
  _array = copy._array;
  _highest_bits = copy._highest_bits;
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: BitArray::Named all_on constructor
//       Access: Published, Static
//  Description: Returns a BitArray with an infinite array of bits,
//               all on.
////////////////////////////////////////////////////////////////////
INLINE BitArray BitArray::
all_on() {
  BitArray result;
  result._highest_bits = 1;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: BitArray::Named all_on constructor
//       Access: Published, Static
//  Description: Returns a BitArray whose bits are all off.
////////////////////////////////////////////////////////////////////
INLINE BitArray BitArray::
all_off() {
  return BitArray();
}

////////////////////////////////////////////////////////////////////
//     Function: BitArray::Named lower_on constructor
//       Access: Published, Static
//  Description: Returns a BitArray whose lower on_bits bits are on.
////////////////////////////////////////////////////////////////////
INLINE BitArray BitArray::
lower_on(int on_bits) {
  BitArray result;
  result.set_range(0, on_bits);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: BitArray::Named bit constructor
//       Access: Published, Static
//  Description: Returns a BitArray with only the indicated bit on.
////////////////////////////////////////////////////////////////////
INLINE BitArray BitArray::
bit(int index) {
  BitArray result;
  result.set_bit(index);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: BitArray::Named range constructor
//       Access: Published, Static
//  Description: Returns a BitArray whose size bits, beginning at
//               low_bit, are on.
////////////////////////////////////////////////////////////////////
INLINE BitArray BitArray::
range(int low_bit, int size) {
  BitArray result;
  result.set_range(low_bit, size);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: BitArray::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BitArray::
~BitArray() {
}

////////////////////////////////////////////////////////////////////
//     Function: BitArray::has_max_num_bits
//       Access: Published, Static
//  Description: Returns true if there is a maximum number of bits
//               that may be stored in this structure, false
//               otherwise.  If this returns true, the number may be
//               queried in get_max_num_bits().
//
//               This method always returns false.  The BitArray has
//               no maximum number of bits.  This method is defined so
//               generic programming algorithms can use BitMask or
//               BitArray interchangeably.
////////////////////////////////////////////////////////////////////
CONSTEXPR bool BitArray::
has_max_num_bits() {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: BitArray::get_max_num_bits
//       Access: Published, Static
//  Description: If get_max_num_bits() returned true, this method may
//               be called to return the maximum number of bits that
//               may be stored in this structure.  It is an error to
//               call this if get_max_num_bits() return false.
//
//               It is always an error to call this method.  The
//               BitArray has no maximum number of bits.  This method
//               is defined so generic programming algorithms can use
//               BitMask or BitArray interchangeably.
////////////////////////////////////////////////////////////////////
CONSTEXPR int BitArray::
get_max_num_bits() {
  return INT_MAX;
}

////////////////////////////////////////////////////////////////////
//     Function: BitArray::get_num_bits_per_word
//       Access: Published, Static
//  Description: Returns the number of bits stored per word
//               internally.  This is of interest only in that it
//               limits the maximum number of bits that may be queried
//               or set at once by extract() and store().
////////////////////////////////////////////////////////////////////
CONSTEXPR int BitArray::
get_num_bits_per_word() {
  return num_bits_per_word;
}

////////////////////////////////////////////////////////////////////
//     Function: BitArray::get_num_bits
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
INLINE int BitArray::
get_num_bits() const {
  return get_num_words() * num_bits_per_word;
}

////////////////////////////////////////////////////////////////////
//     Function: BitArray::get_bit
//       Access: Published
//  Description: Returns true if the nth bit is set, false if it is
//               cleared.  It is valid for n to increase beyond
//               get_num_bits(), but the return value get_num_bits()
//               will always be the same.
////////////////////////////////////////////////////////////////////
INLINE bool BitArray::
get_bit(int index) const {
  nassertr(index >= 0, false);
  int w = index / num_bits_per_word;
  int b = index % num_bits_per_word;
  if (w >= get_num_words()) {
    return get_highest_bits();
  } else {
    return (_array[w].get_bit(b));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BitArray::set_bit
//       Access: Published
//  Description: Sets the nth bit on.  If n >= get_num_bits(), this
//               automatically extends the array.
////////////////////////////////////////////////////////////////////
INLINE void BitArray::
set_bit(int index) {
  nassertv(index >= 0);
  int w = index / num_bits_per_word;
  int b = index % num_bits_per_word;
  if (w >= get_num_words() && _highest_bits) {
    // All the highest bits are already on.
    return;
  }
  ensure_has_word(w);
  _array[w].set_bit(b);
  normalize();
}

////////////////////////////////////////////////////////////////////
//     Function: BitArray::clear_bit
//       Access: Published
//  Description: Sets the nth bit off.  If n >= get_num_bits(), this
//               automatically extends the array.
////////////////////////////////////////////////////////////////////
INLINE void BitArray::
clear_bit(int index) {
  nassertv(index >= 0);
  int w = index / num_bits_per_word;
  int b = index % num_bits_per_word;
  if (w >= get_num_words() && !_highest_bits) {
    // All the highest bits are already off.
    return;
  }
  ensure_has_word(w);
  _array[w].clear_bit(b);
  normalize();
}

////////////////////////////////////////////////////////////////////
//     Function: BitArray::set_bit_to
//       Access: Published
//  Description: Sets the nth bit either on or off, according to the
//               indicated bool value.
////////////////////////////////////////////////////////////////////
INLINE void BitArray::
set_bit_to(int index, bool value) {
  if (value) {
    set_bit(index);
  } else {
    clear_bit(index);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BitArray::get_highest_bits
//       Access: Published
//  Description: Returns true if the infinite set of bits beyond
//               get_num_bits() are all on, or false of they are all
//               off.
////////////////////////////////////////////////////////////////////
INLINE bool BitArray::
get_highest_bits() const {
  return (_highest_bits != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: BitArray::extract
//       Access: Published
//  Description: Returns a word that represents only the indicated
//               range of bits within this BitArray, shifted to the
//               least-significant position.  size must be <=
//               get_num_bits_per_word().
////////////////////////////////////////////////////////////////////
INLINE BitArray::WordType BitArray::
extract(int low_bit, int size) const {
  nassertr(size >= 0 && size <= num_bits_per_word, 0);
  int w = low_bit / num_bits_per_word;
  int b = low_bit % num_bits_per_word;

  if (b + size < num_bits_per_word) {
    // The whole thing fits within one word of the array.
    return get_word(w).extract(b, size);

  } else {
    // We have to split it across two words.
    int num_lower_bits = num_bits_per_word - b;
    int num_higher_bits = size - num_lower_bits;

    return get_word(w).extract(b, num_lower_bits) |
      (get_word(w + 1).extract(0, num_higher_bits) << num_lower_bits);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BitArray::store
//       Access: Published
//  Description: Stores the indicated word into the indicated range of
//               bits with this BitArray.
////////////////////////////////////////////////////////////////////
INLINE void BitArray::
store(WordType value, int low_bit, int size) {
  nassertv(size >= 0);
  int w = low_bit / num_bits_per_word;
  int b = low_bit % num_bits_per_word;

  if (b + size < num_bits_per_word) {
    // The whole thing fits within one word of the array.
    ensure_has_word(w);
    _array[w].store(value, b, size);

  } else {
    // We have to split it across two words.
    int num_lower_bits = num_bits_per_word - b;
    int num_higher_bits = size - num_lower_bits;

    ensure_has_word(w + 1);
    _array[w].store(value, b, num_lower_bits);
    _array[w + 1].store(value >> num_lower_bits, 0, num_higher_bits);
  }
  normalize();
}

////////////////////////////////////////////////////////////////////
//     Function: BitArray::set_range_to
//       Access: Published
//  Description: Sets the indicated range of bits to either on or off.
////////////////////////////////////////////////////////////////////
INLINE void BitArray::
set_range_to(bool value, int low_bit, int size) {
  if (value) {
    set_range(low_bit, size);
  } else {
    clear_range(low_bit, size);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BitArray::get_num_words
//       Access: Published
//  Description: Returns the number of possibly-unique words stored in
//               the array.
////////////////////////////////////////////////////////////////////
INLINE int BitArray::
get_num_words() const {
  return _array.size();
}

////////////////////////////////////////////////////////////////////
//     Function: BitArray::get_word
//       Access: Published
//  Description: Returns the nth word in the array.  It is valid for n
//               to be greater than get_num_words(), but the return
//               value beyond get_num_words() will always be the same.
////////////////////////////////////////////////////////////////////
INLINE BitArray::MaskType BitArray::
get_word(int n) const {
  nassertr(n >= 0, MaskType::all_off());
  if (n < get_num_words()) {
    return _array[n];
  }
  if (_highest_bits) {
    return MaskType::all_on();
  } else {
    return MaskType::all_off();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BitArray::set_word
//       Access: Published
//  Description: Replaces the nth word in the array.  If n >=
//               get_num_words(), this automatically extends the
//               array.
////////////////////////////////////////////////////////////////////
INLINE void BitArray::
set_word(int n, WordType value) {
  nassertv(n >= 0);
  ensure_has_word(n);
  _array[n] = value;
  normalize();
}

////////////////////////////////////////////////////////////////////
//     Function: BitArray::clear
//       Access: Published
//  Description: Sets all the bits in the BitArray off.
////////////////////////////////////////////////////////////////////
void BitArray::
clear() {
  _array.clear();
  _highest_bits = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: BitArray::operator ==
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool BitArray::
operator == (const BitArray &other) const {
  return compare_to(other) == 0;
}

////////////////////////////////////////////////////////////////////
//     Function: BitArray::operator !=
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool BitArray::
operator != (const BitArray &other) const {
  return compare_to(other) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: BitArray::operator <
//       Access: Published
//  Description: Returns true if the unsigned integer which is
//               represented by this BitArray is less than that of the
//               other one, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool BitArray::
operator < (const BitArray &other) const {
  return compare_to(other) < 0;
}

////////////////////////////////////////////////////////////////////
//     Function: BitArray::operator &
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BitArray BitArray::
operator & (const BitArray &other) const {
  BitArray result(*this);
  result &= other;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: BitArray::operator |
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BitArray BitArray::
operator | (const BitArray &other) const {
  BitArray result(*this);
  result |= other;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: BitArray::operator ^
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BitArray BitArray::
operator ^ (const BitArray &other) const {
  BitArray result(*this);
  result ^= other;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: BitArray::operator ~
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BitArray BitArray::
operator ~ () const {
  BitArray result(*this);
  result.invert_in_place();
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: BitArray::operator <<
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BitArray BitArray::
operator << (int shift) const {
  BitArray result(*this);
  result <<= shift;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: BitArray::operator >>
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BitArray BitArray::
operator >> (int shift) const {
  BitArray result(*this);
  result >>= shift;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: BitArray::copy_on_write
//       Access: Private
//  Description: Called internally just before writing to the _array
//               member, this makes a new copy of _array if it appears
//               to be shared with any other objects--thus achieving
//               copy-on-write.
////////////////////////////////////////////////////////////////////
INLINE void BitArray::
copy_on_write() {
  if (_array.get_ref_count() > 1) {
    PTA(MaskType) new_array;
    new_array.v() = _array.v();
    _array = new_array;
  }
}
