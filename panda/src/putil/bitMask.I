// Filename: bitMask.I
// Created by:  drose (08Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

template<class WordType, int num_bits>
TypeHandle BitMask<WordType, num_bits>::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: BitMask::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
template<class WordType, int num_bits>
INLINE BitMask<WordType, num_bits>::
BitMask() :
  _word(0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: BitMask::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
template<class WordType, int num_bits>
INLINE BitMask<WordType, num_bits>::
BitMask(WordType init_value) :
  _word(init_value)
{
}

////////////////////////////////////////////////////////////////////
//     Function: BitMask::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
template<class WordType, int num_bits>
INLINE BitMask<WordType, num_bits>::
BitMask(const BitMask<WordType, num_bits> &copy) :
  _word(copy._word)
{
}

////////////////////////////////////////////////////////////////////
//     Function: BitMask::Copy Assignment Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
template<class WordType, int num_bits>
INLINE void BitMask<WordType, num_bits>::
operator = (const BitMask<WordType, num_bits> &copy) {
  _word = copy._word;
}

////////////////////////////////////////////////////////////////////
//     Function: BitMask::Named all_on constructor
//       Access: Published, Static
//  Description: Returns a BitMask whose bits are all on.
////////////////////////////////////////////////////////////////////
template<class WordType, int num_bits>
INLINE BitMask<WordType, num_bits> BitMask<WordType, num_bits>::
all_on() {
  BitMask result;
  result._word = ~0;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: BitMask::Named all_on constructor
//       Access: Published, Static
//  Description: Returns a BitMask whose bits are all off.
////////////////////////////////////////////////////////////////////
template<class WordType, int num_bits>
INLINE BitMask<WordType, num_bits> BitMask<WordType, num_bits>::
all_off() {
  BitMask result;
  result._word = 0;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: BitMask::Named lower_on constructor
//       Access: Published, Static
//  Description: Returns a BitMask whose lower num_bits bits are on.
////////////////////////////////////////////////////////////////////
template<class WordType, int num_bits>
INLINE BitMask<WordType, num_bits> BitMask<WordType, num_bits>::
lower_on(int on_bits) {
  if (on_bits <= 0) {
    return all_off();
  } else if (on_bits >= num_bits) {
    return all_on();
  }
  BitMask result;
  result._word = ((WordType)1 << on_bits) - 1;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: BitMask::Named bit constructor
//       Access: Published, Static
//  Description: Returns a BitMask with only the indicated bit on.
////////////////////////////////////////////////////////////////////
template<class WordType, int num_bits>
INLINE BitMask<WordType, num_bits> BitMask<WordType, num_bits>::
bit(int index) {
  BitMask result;
  result.set_bit(index);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: BitMask::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
template<class WordType, int num_bits>
INLINE BitMask<WordType, num_bits>::
~BitMask() {
}

////////////////////////////////////////////////////////////////////
//     Function: BitMask::get_num_bits
//       Access: Published
//  Description: Returns the number of bits available to set in the
//               bitmask.
////////////////////////////////////////////////////////////////////
template<class WordType, int num_bits>
INLINE int BitMask<WordType, num_bits>::
get_num_bits() const {
  return num_bits;
}

////////////////////////////////////////////////////////////////////
//     Function: BitMask::get_bit
//       Access: Published
//  Description: Returns true if the nth bit is set, false if it is
//               cleared.  index must be in the range [0,
//               get_num_bits).
////////////////////////////////////////////////////////////////////
template<class WordType, int num_bits>
INLINE bool BitMask<WordType, num_bits>::
get_bit(int index) const {
  nassertr(index >= 0 && index < num_bits, false);
  return (_word & ((WordType)1 << index)) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: BitMask::set_bit
//       Access: Published
//  Description: Sets the nth bit on.  index must be in the range
//               [0, get_num_bits).
////////////////////////////////////////////////////////////////////
template<class WordType, int num_bits>
INLINE void BitMask<WordType, num_bits>::
set_bit(int index) {
  nassertv(index >= 0 && index < num_bits);
  _word |= ((WordType)1 << index);
}

////////////////////////////////////////////////////////////////////
//     Function: BitMask::clear_bit
//       Access: Published
//  Description: Sets the nth bit off.  index must be in the range
//               [0, get_num_bits).
////////////////////////////////////////////////////////////////////
template<class WordType, int num_bits>
INLINE void BitMask<WordType, num_bits>::
clear_bit(int index) {
  nassertv(index >= 0 && index < num_bits);
  _word &= ~((WordType)1 << index);
}

////////////////////////////////////////////////////////////////////
//     Function: BitMask::set_bit_to
//       Access: Published
//  Description: Sets the nth bit either on or off, according to the
//               indicated bool value.  index must be in the range [0,
//               get_num_bits).
////////////////////////////////////////////////////////////////////
template<class WordType, int num_bits>
INLINE void BitMask<WordType, num_bits>::
set_bit_to(int index, bool value) {
  if (value) {
    set_bit(index);
  } else {
    clear_bit(index);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BitMask::is_zero
//       Access: Published
//  Description: Returns true if the entire bitmask is zero, false
//               otherwise.
////////////////////////////////////////////////////////////////////
template<class WordType, int num_bits>
INLINE bool BitMask<WordType, num_bits>::
is_zero() const {
  return (_word == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: BitMask::extract
//       Access: Published
//  Description: Returns a word that represents only the indicated
//               range of bits within this BitMask, shifted to the
//               least-significant position.
////////////////////////////////////////////////////////////////////
template<class WordType, int num_bits>
INLINE WordType BitMask<WordType, num_bits>::
extract(int low_bit, int size) const {
  return (_word >> low_bit) &
    BitMask<WordType, num_bits>::lower_on(size)._word;
}

////////////////////////////////////////////////////////////////////
//     Function: BitMask::store
//       Access: Published
//  Description: Stores the indicated word into the indicated range of
//               bits with this BitMask.
////////////////////////////////////////////////////////////////////
template<class WordType, int num_bits>
INLINE void BitMask<WordType, num_bits>::
store(WordType value, int low_bit, int size) {
  WordType mask =
    BitMask<WordType, num_bits>::lower_on(size)._word << low_bit;
  _word = (_word & ~mask) | ((value << low_bit) & mask);
}

////////////////////////////////////////////////////////////////////
//     Function: BitMask::get_word
//       Access: Published
//  Description: Returns the entire BitMask as a single word.
////////////////////////////////////////////////////////////////////
template<class WordType, int num_bits>
INLINE WordType BitMask<WordType, num_bits>::
get_word() const {
  return _word;
}

////////////////////////////////////////////////////////////////////
//     Function: BitMask::set_word
//       Access: Published
//  Description: Sets the entire BitMask to the value indicated by the
//               given word.
////////////////////////////////////////////////////////////////////
template<class WordType, int num_bits>
INLINE void BitMask<WordType, num_bits>::
set_word(WordType value) {
  _word = value;
}

////////////////////////////////////////////////////////////////////
//     Function: BitMask::invert_in_place
//       Access: Published
//  Description: Inverts all the bits in the BitMask.
////////////////////////////////////////////////////////////////////
template<class WordType, int num_bits>
INLINE void BitMask<WordType, num_bits>::
invert_in_place() {
  _word = ~_word;
}

////////////////////////////////////////////////////////////////////
//     Function: BitMask::clear
//       Access: Published
//  Description: Sets all the bits in the BitMask off.
////////////////////////////////////////////////////////////////////
template<class WordType, int num_bits>
INLINE void BitMask<WordType, num_bits>::
clear() {
  _word = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: BitMask::output
//       Access: Published
//  Description: Writes the BitMask out as a binary or a hex number,
//               according to the number of bits.
////////////////////////////////////////////////////////////////////
template<class WordType, int num_bits>
void BitMask<WordType, num_bits>::
output(ostream &out) const {
  if (num_bits >= 40) {
    output_hex(out);
  } else {
    output_binary(out);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BitMask::output_binary
//       Access: Published
//  Description: Writes the BitMask out as a binary number, with
//               spaces every four bits.
////////////////////////////////////////////////////////////////////
template<class WordType, int num_bits>
void BitMask<WordType, num_bits>::
output_binary(ostream &out, int spaces_every) const {
  for (int i = num_bits - 1; i >= 0; i--) {
    if (spaces_every != 0 && ((i % spaces_every) == spaces_every - 1)) {
      out << ' ';
    }
    out << (get_bit(i) ? '1' : '0');
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BitMask::output_hex
//       Access: Published
//  Description: Writes the BitMask out as a hexadecimal number, with
//               spaces every four digits.
////////////////////////////////////////////////////////////////////
template<class WordType, int num_bits>
void BitMask<WordType, num_bits>::
output_hex(ostream &out, int spaces_every) const {
  int num_digits = (num_bits + 3) / 4;

  for (int i = num_digits - 1; i >= 0; i--) {
    WordType digit = extract(i * 4, 4);
    if (spaces_every != 0 && ((i % spaces_every) == spaces_every - 1)) {
      out << ' ';
    }
    if (digit > 9) {
      out << (char)(digit - 10 + 'a');
    } else {
      out << (char)(digit + '0');
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BitMask::write
//       Access: Published
//  Description: Writes the BitMask out as a binary or a hex number,
//               according to the number of bits.
////////////////////////////////////////////////////////////////////
template<class WordType, int num_bits>
void BitMask<WordType, num_bits>::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << *this << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: BitMask::operator ==
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
template<class WordType, int num_bits>
INLINE bool BitMask<WordType, num_bits>::
operator == (const BitMask<WordType, num_bits> &other) const {
  return _word == other._word;
}

////////////////////////////////////////////////////////////////////
//     Function: BitMask::operator !=
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
template<class WordType, int num_bits>
INLINE bool BitMask<WordType, num_bits>::
operator != (const BitMask<WordType, num_bits> &other) const {
  return _word != other._word;
}

////////////////////////////////////////////////////////////////////
//     Function: BitMask::operator <
//       Access: Published
//  Description: The ordering operator is of limited usefulness with a
//               BitMask, however, it has a definition which places
//               all unique BitMasks into a unique ordering.  It may
//               be useful when defining ordered STL containers of
//               BitMasks, for instance; and it's required in order to
//               export any STL container (ordered or unordered) of
//               BitMask under Windows.
////////////////////////////////////////////////////////////////////
template<class WordType, int num_bits>
INLINE bool BitMask<WordType, num_bits>::
operator < (const BitMask<WordType, num_bits> &other) const {
  return _word < other._word;
}

////////////////////////////////////////////////////////////////////
//     Function: BitMask::compare_to
//       Access: Published
//  Description: Returns a number less than zero if this BitMask sorts
//               before the indicated other BitMask, greater than zero
//               if it sorts after, or 0 if they are equivalent.  This
//               is based on the same ordering defined by operator <.
////////////////////////////////////////////////////////////////////
template<class WordType, int num_bits>
INLINE int BitMask<WordType, num_bits>::
compare_to(const BitMask<WordType, num_bits> &other) const {
  if ((*this) < other) {
    return -1;
  } else if (other < (*this)) {
    return 1;
  } else {
    return 0;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BitMask::operator &
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
template<class WordType, int num_bits>
INLINE BitMask<WordType, num_bits> BitMask<WordType, num_bits>::
operator & (const BitMask<WordType, num_bits> &other) const {
  BitMask<WordType, num_bits> result(*this);
  result &= other;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: BitMask::operator |
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
template<class WordType, int num_bits>
INLINE BitMask<WordType, num_bits> BitMask<WordType, num_bits>::
operator | (const BitMask<WordType, num_bits> &other) const {
  BitMask<WordType, num_bits> result(*this);
  result |= other;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: BitMask::operator ^
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
template<class WordType, int num_bits>
INLINE BitMask<WordType, num_bits> BitMask<WordType, num_bits>::
operator ^ (const BitMask<WordType, num_bits> &other) const {
  BitMask<WordType, num_bits> result(*this);
  result ^= other;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: BitMask::operator ~
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
template<class WordType, int num_bits>
INLINE BitMask<WordType, num_bits> BitMask<WordType, num_bits>::
operator ~ () const {
  BitMask<WordType, num_bits> result(*this);
  result._word = ~result._word;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: BitMask::operator <<
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
template<class WordType, int num_bits>
INLINE BitMask<WordType, num_bits> BitMask<WordType, num_bits>::
operator << (int shift) const {
  BitMask<WordType, num_bits> result(*this);
  result <<= shift;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: BitMask::operator >>
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
template<class WordType, int num_bits>
INLINE BitMask<WordType, num_bits> BitMask<WordType, num_bits>::
operator >> (int shift) const {
  BitMask<WordType, num_bits> result(*this);
  result >>= shift;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: BitMask::operator &=
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
template<class WordType, int num_bits>
INLINE void BitMask<WordType, num_bits>::
operator &= (const BitMask<WordType, num_bits> &other) {
  _word &= other._word;
}

////////////////////////////////////////////////////////////////////
//     Function: BitMask::operator |=
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
template<class WordType, int num_bits>
INLINE void BitMask<WordType, num_bits>::
operator |= (const BitMask<WordType, num_bits> &other) {
  _word |= other._word;
}

////////////////////////////////////////////////////////////////////
//     Function: BitMask::operator ^=
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
template<class WordType, int num_bits>
INLINE void BitMask<WordType, num_bits>::
operator ^= (const BitMask<WordType, num_bits> &other) {
  _word ^= other._word;
}

////////////////////////////////////////////////////////////////////
//     Function: BitMask::operator <<=
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
template<class WordType, int num_bits>
INLINE void BitMask<WordType, num_bits>::
operator <<= (int shift) {
  _word <<= shift;
}

////////////////////////////////////////////////////////////////////
//     Function: BitMask::operator >>=
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
template<class WordType, int num_bits>
INLINE void BitMask<WordType, num_bits>::
operator >>= (int shift) {
  _word >>= shift;
}

////////////////////////////////////////////////////////////////////
//     Function: BitMask::generate_hash
//       Access: Public
//  Description: Adds the bitmask to the indicated hash generator.
////////////////////////////////////////////////////////////////////
template<class WordType, int num_bits>
INLINE void BitMask<WordType, num_bits>::
generate_hash(ChecksumHashGenerator &hashgen) const {
  hashgen.add_int(_word);
}

////////////////////////////////////////////////////////////////////
//     Function: BitMask::init_type
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class WordType, int num_bits>
void BitMask<WordType, num_bits>::
init_type() {
  ostringstream str;
  str << "BitMask" << num_bits;
  register_type(_type_handle, str.str());
}
