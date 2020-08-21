/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file doubleBitMask.I
 * @author drose
 * @date 2000-06-08
 */

template<class BMType>
TypeHandle DoubleBitMask<BMType>::_type_handle;

/**
 * Returns a DoubleBitMask whose bits are all on.
 */
template<class BMType>
INLINE DoubleBitMask<BMType> DoubleBitMask<BMType>::
all_on() {
  DoubleBitMask<BMType> result;
  result._lo = BitMaskType::all_on();
  result._hi = BitMaskType::all_on();
  return result;
}

/**
 * Returns a DoubleBitMask whose bits are all off.
 */
template<class BMType>
INLINE DoubleBitMask<BMType> DoubleBitMask<BMType>::
all_off() {
  DoubleBitMask<BMType> result;
  result._lo.clear();
  result._hi.clear();
  return result;
}

/**
 * Returns a DoubleBitMask whose lower on_bits bits are on.
 */
template<class BMType>
INLINE DoubleBitMask<BMType> DoubleBitMask<BMType>::
lower_on(int on_bits) {
  if (on_bits <= 0) {
    return all_off();
  } else if (on_bits >= num_bits) {
    return all_on();
  }
  DoubleBitMask<BMType> result;
  if (on_bits <= half_bits) {
    result._lo = BitMaskType::lower_on(on_bits);
  } else {
    result._lo = BitMaskType::all_on();
    result._hi = BitMaskType::lower_on(on_bits - half_bits);
  }
  return result;
}

/**
 * Returns a DoubleBitMask with only the indicated bit on.
 */
template<class BMType>
INLINE DoubleBitMask<BMType> DoubleBitMask<BMType>::
bit(int index) {
  DoubleBitMask<BMType> result;
  result.set_bit(index);
  return result;
}

/**
 * Returns a DoubleBitMask whose size bits, beginning at low_bit, are on.
 */
template<class BMType>
INLINE DoubleBitMask<BMType> DoubleBitMask<BMType>::
range(int low_bit, int size) {
  DoubleBitMask<BMType> result;
  result.set_range(low_bit, size);
  return result;
}

/**
 * Returns the number of bits available to set in the doubleBitMask.
 */
template<class BMType>
constexpr int DoubleBitMask<BMType>::
get_num_bits() const {
  return num_bits;
}

/**
 * Returns true if the nth bit is set, false if it is cleared.  index must be
 * in the range [0, num_bits).
 */
template<class BMType>
INLINE bool DoubleBitMask<BMType>::
get_bit(int index) const {
  if (index < half_bits) {
    return _lo.get_bit(index);
  } else {
    return _hi.get_bit(index - half_bits);
  }
}

/**
 * Sets the nth bit on.  index must be in the range [0, num_bits).
 */
template<class BMType>
INLINE void DoubleBitMask<BMType>::
set_bit(int index) {
  if (index < half_bits) {
    _lo.set_bit(index);
  } else {
    _hi.set_bit(index - half_bits);
  }
}

/**
 * Sets the nth bit off.  index must be in the range [0, num_bits).
 */
template<class BMType>
INLINE void DoubleBitMask<BMType>::
clear_bit(int index) {
  if (index < half_bits) {
    _lo.clear_bit(index);
  } else {
    _hi.clear_bit(index - half_bits);
  }
}

/**
 * Sets the nth bit either on or off, according to the indicated bool value.
 * index must be in the range [0, num_bits).
 */
template<class BMType>
INLINE void DoubleBitMask<BMType>::
set_bit_to(int index, bool value) {
  if (index < half_bits) {
    _lo.set_bit_to(index, value);
  } else {
    _hi.set_bit_to(index - half_bits, value);
  }
}

/**
 * Returns true if the entire doubleBitMask is zero, false otherwise.
 */
template<class BMType>
INLINE bool DoubleBitMask<BMType>::
is_zero() const {
  return (_lo.is_zero() && _hi.is_zero());
}

/**
 * Returns true if the entire doubleBitMask is one, false otherwise.
 */
template<class BMType>
INLINE bool DoubleBitMask<BMType>::
is_all_on() const {
  return (_lo.is_all_on() && _hi.is_all_on());
}

/**
 * Returns a word that represents only the indicated range of bits within this
 * DoubleBitMask, shifted to the least-significant position.
 */
template<class BMType>
INLINE typename DoubleBitMask<BMType>::WordType DoubleBitMask<BMType>::
extract(int low_bit, int size) const {
  if (low_bit >= half_bits) {
    return _hi.extract(low_bit - half_bits, size);
  } else if (low_bit + size < half_bits) {
    return _lo.extract(low_bit, size);
  } else {
    int hi_portion = low_bit + size - half_bits;
    int lo_portion = size - hi_portion;
    return (_hi.extract(0, hi_portion) << lo_portion) |
      _lo.extract(low_bit, lo_portion);
  }
}

/**
 * Stores the indicated word into the indicated range of bits with this
 * DoubleBitMask.
 */
template<class BMType>
INLINE void DoubleBitMask<BMType>::
store(WordType value, int low_bit, int size) {
  if (low_bit >= half_bits) {
    _hi.store(value, low_bit - half_bits, size);
  } else if (low_bit + size < half_bits) {
    _lo.store(value, low_bit, size);
  } else {
    int hi_portion = low_bit + size - half_bits;
    int lo_portion = size - hi_portion;

    _hi.store(value >> lo_portion, 0, hi_portion);
    _lo.store(value, low_bit, lo_portion);
  }
}

/**
 * Returns true if any bit in the indicated range is set, false otherwise.
 */
template<class BMType>
INLINE bool DoubleBitMask<BMType>::
has_any_of(int low_bit, int size) const {
  if (low_bit >= half_bits) {
    return _hi.has_any_of(low_bit - half_bits, size);
  } else if (low_bit + size < half_bits) {
    return _lo.has_any_of(low_bit, size);
  } else {
    int hi_portion = low_bit + size - half_bits;
    int lo_portion = size - hi_portion;
    return _hi.has_any_of(0, hi_portion)
        || _lo.has_any_of(low_bit, lo_portion);
  }
}

/**
 * Returns true if all bits in the indicated range are set, false otherwise.
 */
template<class BMType>
INLINE bool DoubleBitMask<BMType>::
has_all_of(int low_bit, int size) const {
  if (low_bit >= half_bits) {
    return _hi.has_all_of(low_bit - half_bits, size);
  } else if (low_bit + size < half_bits) {
    return _lo.has_all_of(low_bit, size);
  } else {
    int hi_portion = low_bit + size - half_bits;
    int lo_portion = size - hi_portion;
    return _hi.has_all_of(0, hi_portion)
        && _lo.has_all_of(low_bit, lo_portion);
  }
}

/**
 * Sets the indicated range of bits on.
 */
template<class BMType>
INLINE void DoubleBitMask<BMType>::
set_range(int low_bit, int size) {
  if (low_bit >= half_bits) {
    _hi.set_range(low_bit - half_bits, size);
  } else if (low_bit + size < half_bits) {
    _lo.set_range(low_bit, size);
  } else {
    int hi_portion = low_bit + size - half_bits;
    int lo_portion = size - hi_portion;

    _hi.set_range(0, hi_portion);
    _lo.set_range(low_bit, lo_portion);
  }
}

/**
 * Sets the indicated range of bits off.
 */
template<class BMType>
INLINE void DoubleBitMask<BMType>::
clear_range(int low_bit, int size) {
  if (low_bit >= half_bits) {
    _hi.clear_range(low_bit - half_bits, size);
  } else if (low_bit + size < half_bits) {
    _lo.clear_range(low_bit, size);
  } else {
    int hi_portion = low_bit + size - half_bits;
    int lo_portion = size - hi_portion;

    _hi.clear_range(0, hi_portion);
    _lo.clear_range(low_bit, lo_portion);
  }
}

/**
 * Sets the indicated range of bits to either on or off.
 */
template<class BMType>
INLINE void DoubleBitMask<BMType>::
set_range_to(bool value, int low_bit, int size) {
  if (value) {
    set_range(low_bit, size);
  } else {
    clear_range(low_bit, size);
  }
}

/**
 * Returns the number of bits that are set to 1 in the mask.
 */
template<class BMType>
INLINE int DoubleBitMask<BMType>::
get_num_on_bits() const {
  return _lo.get_num_on_bits() + _hi.get_num_on_bits();
}

/**
 * Returns the number of bits that are set to 0 in the mask.
 */
template<class BMType>
INLINE int DoubleBitMask<BMType>::
get_num_off_bits() const {
  return _lo.get_num_off_bits() + _hi.get_num_off_bits();
}

/**
 * Returns the index of the lowest 1 bit in the mask.  Returns -1 if there are
 * no 1 bits.
 */
template<class BMType>
INLINE int DoubleBitMask<BMType>::
get_lowest_on_bit() const {
  int result = _lo.get_lowest_on_bit();
  if (result == -1) {
    result = _hi.get_lowest_on_bit();
    if (result != -1) {
      result += half_bits;
    }
  }
  return result;
}

/**
 * Returns the index of the lowest 0 bit in the mask.  Returns -1 if there are
 * no 0 bits.
 */
template<class BMType>
INLINE int DoubleBitMask<BMType>::
get_lowest_off_bit() const {
  int result = _lo.get_lowest_off_bit();
  if (result == -1) {
    result = _hi.get_lowest_off_bit();
    if (result != -1) {
      result += half_bits;
    }
  }
  return result;
}

/**
 * Returns the index of the highest 1 bit in the mask.  Returns -1 if there
 * are no 1 bits.
 */
template<class BMType>
INLINE int DoubleBitMask<BMType>::
get_highest_on_bit() const {
  int result = _hi.get_highest_on_bit();
  if (result == -1) {
    result = _lo.get_highest_on_bit();
  } else {
    result += half_bits;
  }
  return result;
}

/**
 * Returns the index of the highest 0 bit in the mask.  Returns -1 if there
 * are no 0 bits.
 */
template<class BMType>
INLINE int DoubleBitMask<BMType>::
get_highest_off_bit() const {
  int result = _hi.get_highest_off_bit();
  if (result == -1) {
    result = _lo.get_highest_off_bit();
  } else {
    result += half_bits;
  }
  return result;
}

/**
 * Returns the index of the next bit in the mask, above low_bit, whose value
 * is different that the value of low_bit.  Returns low_bit again if all bits
 * higher than low_bit have the same value.
 *
 * This can be used to quickly iterate through all of the bits in the mask.
 */
template<class BMType>
INLINE int DoubleBitMask<BMType>::
get_next_higher_different_bit(int low_bit) const {
  if (low_bit > half_bits) {
    return _hi.get_next_higher_different_bit(low_bit - half_bits) + half_bits;
  }
  int result = _lo.get_next_higher_different_bit(low_bit);
  if (result != low_bit) {
    return result;
  }
  if (_lo.get_bit(low_bit)) {
    result = _hi.get_lowest_off_bit();
  } else {
    result = _hi.get_lowest_on_bit();
  }
  if (result == -1) {
    return low_bit;
  }
  return result + half_bits;
}

/**
 * Inverts all the bits in the DoubleBitMask.  This is equivalent to mask =
 * ~mask.
 */
template<class BMType>
INLINE void DoubleBitMask<BMType>::
invert_in_place() {
  _lo.invert_in_place();
  _hi.invert_in_place();
}

/**
 * Returns true if this DoubleBitMask has any "one" bits in common with the
 * other one, false otherwise.
 *
 * This is equivalent to (mask & other) != 0, but may be faster.  (Actually,
 * it should only be faster in the BitArray case, but this method is provided
 * for the benefit of generic programming algorithms).
 */
template<class BMType>
INLINE bool DoubleBitMask<BMType>::
has_bits_in_common(const DoubleBitMask<BMType> &other) const {
  return _lo.has_bits_in_common(other._lo) ||
    _hi.has_bits_in_common(other._hi);
}

/**
 * Sets all the bits in the DoubleBitMask off.
 */
template<class BMType>
INLINE void DoubleBitMask<BMType>::
clear() {
  _lo.clear();
  _hi.clear();
}

/**
 * Writes the DoubleBitMask out as a binary or a hex number, according to the
 * number of bits.
 */
template<class BMType>
void DoubleBitMask<BMType>::
output(std::ostream &out) const {
  output_hex(out);
}

/**
 * Writes the DoubleBitMask out as a binary number, with spaces every four
 * bits.
 */
template<class BMType>
void DoubleBitMask<BMType>::
output_binary(std::ostream &out, int spaces_every) const {
  _hi.output_binary(out);
  out << ' ';
  _lo.output_binary(out);
}

/**
 * Writes the DoubleBitMask out as a hexadecimal number, with spaces every
 * four digits.
 */
template<class BMType>
void DoubleBitMask<BMType>::
output_hex(std::ostream &out, int spaces_every) const {
  _hi.output_hex(out);
  out << ' ';
  _lo.output_hex(out);
}

/**
 * Writes the DoubleBitMask out as a binary or a hex number, according to the
 * number of bits.
 */
template<class BMType>
void DoubleBitMask<BMType>::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level) << *this << "\n";
}

/**
 *
 */
template<class BMType>
INLINE bool DoubleBitMask<BMType>::
operator == (const DoubleBitMask<BMType> &other) const {
  return _lo == other._lo && _hi == other._hi;
}

/**
 *
 */
template<class BMType>
INLINE bool DoubleBitMask<BMType>::
operator != (const DoubleBitMask<BMType> &other) const {
  return _lo != other._lo && _hi != other._hi;
}

/**
 * The ordering operator is of limited usefulness with a DoubleBitMask,
 * however, it has a definition which places all unique DoubleBitMasks into a
 * unique ordering.  It may be useful when defining ordered STL containers of
 * DoubleBitMasks, for instance; and it's required in order to export any STL
 * container (ordered or unordered) of DoubleBitMask under Windows.
 */
template<class BMType>
INLINE bool DoubleBitMask<BMType>::
operator < (const DoubleBitMask<BMType> &other) const {
  int cmp = _hi.compare_to(other._hi);
  if (cmp != 0) {
    return cmp < 0;
  }
  return _lo < other._lo;
}

/**
 * Returns a number less than zero if this DoubleBitMask sorts before the
 * indicated other DoubleBitMask, greater than zero if it sorts after, or 0 if
 * they are equivalent.  This is based on the same ordering defined by
 * operator <.
 */
template<class BMType>
INLINE int DoubleBitMask<BMType>::
compare_to(const DoubleBitMask<BMType> &other) const {
  int cmp = _hi.compare_to(other._hi);
  if (cmp != 0) {
    return cmp;
  }
  return _lo.compare_to(other._lo);
}

/**
 *
 */
template<class BMType>
INLINE DoubleBitMask<BMType> DoubleBitMask<BMType>::
operator & (const DoubleBitMask<BMType> &other) const {
  DoubleBitMask<BMType> result(*this);
  result &= other;
  return result;
}

/**
 *
 */
template<class BMType>
INLINE DoubleBitMask<BMType> DoubleBitMask<BMType>::
operator | (const DoubleBitMask<BMType> &other) const {
  DoubleBitMask<BMType> result(*this);
  result |= other;
  return result;
}

/**
 *
 */
template<class BMType>
INLINE DoubleBitMask<BMType> DoubleBitMask<BMType>::
operator ^ (const DoubleBitMask<BMType> &other) const {
  DoubleBitMask<BMType> result(*this);
  result ^= other;
  return result;
}

/**
 *
 */
template<class BMType>
INLINE DoubleBitMask<BMType> DoubleBitMask<BMType>::
operator ~ () const {
  DoubleBitMask<BMType> result(*this);
  result.invert_in_place();
  return result;
}

/**
 *
 */
template<class BMType>
INLINE DoubleBitMask<BMType> DoubleBitMask<BMType>::
operator << (int shift) const {
  DoubleBitMask<BMType> result(*this);
  result <<= shift;
  return result;
}

/**
 *
 */
template<class BMType>
INLINE DoubleBitMask<BMType> DoubleBitMask<BMType>::
operator >> (int shift) const {
  DoubleBitMask<BMType> result(*this);
  result >>= shift;
  return result;
}

/**
 *
 */
template<class BMType>
INLINE void DoubleBitMask<BMType>::
operator &= (const DoubleBitMask<BMType> &other) {
  _lo &= other._lo;
  _hi &= other._hi;
}

/**
 *
 */
template<class BMType>
INLINE void DoubleBitMask<BMType>::
operator |= (const DoubleBitMask<BMType> &other) {
  _lo |= other._lo;
  _hi |= other._hi;
}

/**
 *
 */
template<class BMType>
INLINE void DoubleBitMask<BMType>::
operator ^= (const DoubleBitMask<BMType> &other) {
  _lo ^= other._lo;
  _hi ^= other._hi;
}

/**
 *
 */
template<class BMType>
INLINE void DoubleBitMask<BMType>::
operator <<= (int shift) {
  _hi = (_hi << shift) | ((_lo >> (half_bits - shift)) & BitMaskType::lower_on(shift));
  _lo <<= shift;
}

/**
 *
 */
template<class BMType>
INLINE void DoubleBitMask<BMType>::
operator >>= (int shift) {
  _lo = (_lo >> shift) | ((_hi & BitMaskType::lower_on(shift)) << (half_bits - shift));
  _hi >>= shift;
}

/**
 * Adds the doubleBitMask to the indicated hash generator.
 */
template<class BMType>
INLINE void DoubleBitMask<BMType>::
generate_hash(ChecksumHashGenerator &hashgen) const {
  _hi.generate_hash(hashgen);
  _lo.generate_hash(hashgen);
}

/**
 *
 */
template<class BMType>
void DoubleBitMask<BMType>::
init_type() {
  std::ostringstream str;
  str << "DoubleBitMask" << num_bits;
  register_type(_type_handle, str.str());
}
