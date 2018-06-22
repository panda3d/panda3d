/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bitMask.I
 * @author drose
 * @date 2000-06-08
 */

template<class WType, int nbits>
TypeHandle BitMask<WType, nbits>::_type_handle;

/**
 *
 */
template<class WType, int nbits>
constexpr BitMask<WType, nbits>::
BitMask(WordType init_value) :
  _word(init_value)
{
}

/**
 * Returns a BitMask whose bits are all on.
 */
template<class WType, int nbits>
INLINE BitMask<WType, nbits> BitMask<WType, nbits>::
all_on() {
  BitMask result;
  result._word = (WordType)~0;
  return result;
}

/**
 * Returns a BitMask whose bits are all off.
 */
template<class WType, int nbits>
INLINE BitMask<WType, nbits> BitMask<WType, nbits>::
all_off() {
  BitMask result;
  result._word = 0;
  return result;
}

/**
 * Returns a BitMask whose lower on_bits bits are on.
 */
template<class WType, int nbits>
INLINE BitMask<WType, nbits> BitMask<WType, nbits>::
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

/**
 * Returns a BitMask with only the indicated bit on.
 */
template<class WType, int nbits>
INLINE BitMask<WType, nbits> BitMask<WType, nbits>::
bit(int index) {
  BitMask result;
  result.set_bit(index);
  return result;
}

/**
 * Returns a BitMask whose size bits, beginning at low_bit, are on.
 */
template<class WType, int nbits>
INLINE BitMask<WType, nbits> BitMask<WType, nbits>::
range(int low_bit, int size) {
  BitMask result;
  if (size <= 0) {
    result._word = 0;
  } else if (size >= num_bits) {
    result._word = (WordType)~0;
  } else {
    result._word = ((WordType)1 << size) - 1;
  }
  result._word <<= low_bit;
  return result;
}

/**
 * Returns the number of bits available to set in the bitmask.
 */
template<class WType, int nbits>
constexpr int BitMask<WType, nbits>::
get_num_bits() const {
  return num_bits;
}

/**
 * Returns true if the nth bit is set, false if it is cleared.  index must be
 * in the range [0, num_bits).
 */
template<class WType, int nbits>
INLINE bool BitMask<WType, nbits>::
get_bit(int index) const {
  nassertr(index >= 0 && index < num_bits, false);
  return (_word & ((WordType)1 << index)) != 0;
}

/**
 * Sets the nth bit on.  index must be in the range [0, num_bits).
 */
template<class WType, int nbits>
INLINE void BitMask<WType, nbits>::
set_bit(int index) {
  nassertv(index >= 0 && index < num_bits);
  _word |= ((WordType)1 << index);
}

/**
 * Sets the nth bit off.  index must be in the range [0, num_bits).
 */
template<class WType, int nbits>
INLINE void BitMask<WType, nbits>::
clear_bit(int index) {
  nassertv(index >= 0 && index < num_bits);
  _word &= ~((WordType)1 << index);
}

/**
 * Sets the nth bit either on or off, according to the indicated bool value.
 * index must be in the range [0, num_bits).
 */
template<class WType, int nbits>
INLINE void BitMask<WType, nbits>::
set_bit_to(int index, bool value) {
  if (value) {
    set_bit(index);
  } else {
    clear_bit(index);
  }
}

/**
 * Returns true if the entire bitmask is zero, false otherwise.
 */
template<class WType, int nbits>
INLINE bool BitMask<WType, nbits>::
is_zero() const {
  return (_word == 0);
}

/**
 * Returns true if the entire bitmask is one, false otherwise.
 */
template<class WType, int nbits>
INLINE bool BitMask<WType, nbits>::
is_all_on() const {
  return _word == (WordType)~0;
}

/**
 * Returns a word that represents only the indicated range of bits within this
 * BitMask, shifted to the least-significant position.
 */
template<class WType, int nbits>
INLINE typename BitMask<WType, nbits>::WordType BitMask<WType, nbits>::
extract(int low_bit, int size) const {
  return (_word >> low_bit) &
    BitMask<WType, nbits>::lower_on(size)._word;
}

/**
 * Stores the indicated word into the indicated range of bits with this
 * BitMask.
 */
template<class WType, int nbits>
INLINE void BitMask<WType, nbits>::
store(WordType value, int low_bit, int size) {
  WordType mask = BitMask<WType, nbits>::range(low_bit, size)._word;
  _word = (_word & ~mask) | ((value << low_bit) & mask);
}

/**
 * Returns true if any bit in the indicated range is set, false otherwise.
 */
template<class WType, int nbits>
INLINE bool BitMask<WType, nbits>::
has_any_of(int low_bit, int size) const {
  WordType mask = BitMask<WType, nbits>::range(low_bit, size)._word;
  return (_word & mask) != 0;
}

/**
 * Returns true if all bits in the indicated range are set, false otherwise.
 */
template<class WType, int nbits>
INLINE bool BitMask<WType, nbits>::
has_all_of(int low_bit, int size) const {
  WordType mask = BitMask<WType, nbits>::range(low_bit, size)._word;
  return (_word & mask) == mask;
}

/**
 * Sets the indicated range of bits on.
 */
template<class WType, int nbits>
INLINE void BitMask<WType, nbits>::
set_range(int low_bit, int size) {
  WordType mask = BitMask<WType, nbits>::range(low_bit, size)._word;
  _word |= mask;
}

/**
 * Sets the indicated range of bits off.
 */
template<class WType, int nbits>
INLINE void BitMask<WType, nbits>::
clear_range(int low_bit, int size) {
  WordType mask = BitMask<WType, nbits>::range(low_bit, size)._word;
  _word &= ~mask;
}

/**
 * Sets the indicated range of bits to either on or off.
 */
template<class WType, int nbits>
INLINE void BitMask<WType, nbits>::
set_range_to(bool value, int low_bit, int size) {
  if (value) {
    set_range(low_bit, size);
  } else {
    clear_range(low_bit, size);
  }
}

/**
 * Returns the entire BitMask as a single word.
 */
template<class WType, int nbits>
INLINE typename BitMask<WType, nbits>::WordType BitMask<WType, nbits>::
get_word() const {
  return _word;
}

/**
 * Sets the entire BitMask to the value indicated by the given word.
 */
template<class WType, int nbits>
INLINE void BitMask<WType, nbits>::
set_word(WordType value) {
  _word = value;
}

/**
 * Returns the number of bits that are set to 1 in the mask.
 */
template<class WType, int nbits>
INLINE int BitMask<WType, nbits>::
get_num_on_bits() const {
  return count_bits_in_word((WType)_word);
}

/**
 * Returns the number of bits that are set to 0 in the mask.
 */
template<class WType, int nbits>
INLINE int BitMask<WType, nbits>::
get_num_off_bits() const {
  return count_bits_in_word((WType)(~_word));
}

/**
 * Returns the index of the lowest 1 bit in the mask.  Returns -1 if there are
 * no 1 bits.
 */
template<class WType, int nbits>
INLINE int BitMask<WType, nbits>::
get_lowest_on_bit() const {
  return ::get_lowest_on_bit(_word);
}

/**
 * Returns the index of the lowest 0 bit in the mask.  Returns -1 if there are
 * no 0 bits.
 */
template<class WType, int nbits>
INLINE int BitMask<WType, nbits>::
get_lowest_off_bit() const {
  return (~(*this)).get_lowest_on_bit();
}

/**
 * Returns the index of the highest 1 bit in the mask.  Returns -1 if there
 * are no 1 bits.
 */
template<class WType, int nbits>
INLINE int BitMask<WType, nbits>::
get_highest_on_bit() const {
  return ::get_highest_on_bit(_word);
}

/**
 * Returns the index of the highest 0 bit in the mask.  Returns -1 if there
 * are no 0 bits.
 */
template<class WType, int nbits>
INLINE int BitMask<WType, nbits>::
get_highest_off_bit() const {
  return (~(*this)).get_highest_on_bit();
}

/**
 * Returns the index of the next bit in the mask, above low_bit, whose value
 * is different that the value of low_bit.  Returns low_bit again if all bits
 * higher than low_bit have the same value.
 *
 * This can be used to quickly iterate through all of the bits in the mask.
 */
template<class WType, int nbits>
INLINE int BitMask<WType, nbits>::
get_next_higher_different_bit(int low_bit) const {
  // We are allowed to call this method with low_bit == num_bits, which is the
  // highest value this method will return.
  nassertr(low_bit >= 0, low_bit);
  if (low_bit >= num_bits) {
    return low_bit;
  }

  WordType is_on = (_word & ((WordType)1 << low_bit));
  WordType w;
  if (is_on) {
    // low_bit is 1.  Get the next higher 0 bit.  To do this, invert the word
    // and the get the next higher 1 bit.
    w = ~_word;
  } else {
    // low_bit is 0.  Get the next higher 1 bit.
    w = _word;
  }

  // Mask out all of the bits below low_bit.  Since we already know that
  // low_bit is 0, we can use (1 << low_bit) instead of (1 << (low_bit + 1)),
  // which becomes undefined when (low_bit + 1) == 32.
  w &= ~(((WordType)1 << low_bit) - 1);

  if (w == 0) {
    // All higher bits in the word have the same value.  Since every bit after
    // the topmost bit is 0, we either return the topmost bit + 1 to indicate
    // the next 0 bit, or low_bit to indicate we have reached the end of the
    // number of bits.
    return is_on ? num_bits : low_bit;

  } else {
    // Now determine the lowest 1 bit in the remaining word.  This operation
    // will clear out all bits except for the lowest 1 bit.
    w = (w & (~w + 1));

    // And the answer is the number of bits in (w - 1).
    return count_bits_in_word((WType)(w - 1));
  }
}

/**
 * Inverts all the bits in the BitMask.  This is equivalent to mask = ~mask.
 */
template<class WType, int nbits>
INLINE void BitMask<WType, nbits>::
invert_in_place() {
  _word = ~_word;
}

/**
 * Returns true if this BitMask has any "one" bits in common with the other
 * one, false otherwise.
 *
 * This is equivalent to (mask & other) != 0, but may be faster.  (Actually,
 * it should only be faster in the BitArray case, but this method is provided
 * for the benefit of generic programming algorithms).
 */
template<class WType, int nbits>
INLINE bool BitMask<WType, nbits>::
has_bits_in_common(const BitMask<WType, nbits> &other) const {
  return (_word & other._word) != 0;
}

/**
 * Sets all the bits in the BitMask off.
 */
template<class WType, int nbits>
INLINE void BitMask<WType, nbits>::
clear() {
  _word = 0;
}

/**
 * Writes the BitMask out as a binary or a hex number, according to the number
 * of bits.
 */
template<class WType, int nbits>
void BitMask<WType, nbits>::
output(std::ostream &out) const {
  if (num_bits >= 40) {
    output_hex(out);
  } else {
    output_binary(out);
  }
}

/**
 * Writes the BitMask out as a binary number, with spaces every four bits.
 */
template<class WType, int nbits>
void BitMask<WType, nbits>::
output_binary(std::ostream &out, int spaces_every) const {
  for (int i = num_bits - 1; i >= 0; i--) {
    if (spaces_every != 0 && ((i % spaces_every) == spaces_every - 1)) {
      out << ' ';
    }
    out << (get_bit(i) ? '1' : '0');
  }
}

/**
 * Writes the BitMask out as a hexadecimal number, with spaces every four
 * digits.
 */
template<class WType, int nbits>
void BitMask<WType, nbits>::
output_hex(std::ostream &out, int spaces_every) const {
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

/**
 * Writes the BitMask out as a binary or a hex number, according to the number
 * of bits.
 */
template<class WType, int nbits>
void BitMask<WType, nbits>::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level) << *this << "\n";
}

/**
 *
 */
template<class WType, int nbits>
INLINE bool BitMask<WType, nbits>::
operator == (const BitMask<WType, nbits> &other) const {
  return _word == other._word;
}

/**
 *
 */
template<class WType, int nbits>
INLINE bool BitMask<WType, nbits>::
operator != (const BitMask<WType, nbits> &other) const {
  return _word != other._word;
}

/**
 * The ordering operator is of limited usefulness with a BitMask, however, it
 * has a definition which places all unique BitMasks into a unique ordering.
 * It may be useful when defining ordered STL containers of BitMasks, for
 * instance; and it's required in order to export any STL container (ordered
 * or unordered) of BitMask under Windows.
 */
template<class WType, int nbits>
INLINE bool BitMask<WType, nbits>::
operator < (const BitMask<WType, nbits> &other) const {
  return _word < other._word;
}

/**
 * Returns a number less than zero if this BitMask sorts before the indicated
 * other BitMask, greater than zero if it sorts after, or 0 if they are
 * equivalent.  This is based on the same ordering defined by operator <.
 */
template<class WType, int nbits>
INLINE int BitMask<WType, nbits>::
compare_to(const BitMask<WType, nbits> &other) const {
  if ((*this) < other) {
    return -1;
  } else if (other < (*this)) {
    return 1;
  } else {
    return 0;
  }
}

/**
 *
 */
template<class WType, int nbits>
INLINE BitMask<WType, nbits> BitMask<WType, nbits>::
operator & (const BitMask<WType, nbits> &other) const {
  BitMask<WType, nbits> result(*this);
  result &= other;
  return result;
}

/**
 *
 */
template<class WType, int nbits>
INLINE BitMask<WType, nbits> BitMask<WType, nbits>::
operator | (const BitMask<WType, nbits> &other) const {
  BitMask<WType, nbits> result(*this);
  result |= other;
  return result;
}

/**
 *
 */
template<class WType, int nbits>
INLINE BitMask<WType, nbits> BitMask<WType, nbits>::
operator ^ (const BitMask<WType, nbits> &other) const {
  BitMask<WType, nbits> result(*this);
  result ^= other;
  return result;
}

/**
 *
 */
template<class WType, int nbits>
INLINE BitMask<WType, nbits> BitMask<WType, nbits>::
operator ~ () const {
  return BitMask<WType, nbits>(~_word);
}

/**
 *
 */
template<class WType, int nbits>
INLINE BitMask<WType, nbits> BitMask<WType, nbits>::
operator << (int shift) const {
  BitMask<WType, nbits> result(*this);
  result <<= shift;
  return result;
}

/**
 *
 */
template<class WType, int nbits>
INLINE BitMask<WType, nbits> BitMask<WType, nbits>::
operator >> (int shift) const {
  BitMask<WType, nbits> result(*this);
  result >>= shift;
  return result;
}

/**
 *
 */
template<class WType, int nbits>
INLINE void BitMask<WType, nbits>::
operator &= (const BitMask<WType, nbits> &other) {
  _word &= other._word;
}

/**
 *
 */
template<class WType, int nbits>
INLINE void BitMask<WType, nbits>::
operator |= (const BitMask<WType, nbits> &other) {
  _word |= other._word;
}

/**
 *
 */
template<class WType, int nbits>
INLINE void BitMask<WType, nbits>::
operator ^= (const BitMask<WType, nbits> &other) {
  _word ^= other._word;
}

/**
 *
 */
template<class WType, int nbits>
INLINE void BitMask<WType, nbits>::
operator <<= (int shift) {
  _word <<= shift;
}

/**
 *
 */
template<class WType, int nbits>
INLINE void BitMask<WType, nbits>::
operator >>= (int shift) {
  _word >>= shift;
}

/**
 * Returns a mostly unique integer key per unique bitmask, suitable for using
 * in a hash table.
 */
template<class WType, int nbits>
INLINE int BitMask<WType, nbits>::
get_key() const {
  return (int)_word;
}

/**
 * Returns true if the bitmask is not zero.
 */
template<class WType, int nbits>
INLINE bool BitMask<WType, nbits>::
__nonzero__() const {
  return _word != 0;
}

/**
 * Adds the bitmask to the indicated hash generator.
 */
template<class WType, int nbits>
INLINE void BitMask<WType, nbits>::
generate_hash(ChecksumHashGenerator &hashgen) const {
  hashgen.add_int(_word);
}

/**
 *
 */
template<class WType, int nbits>
void BitMask<WType, nbits>::
init_type(const std::string &name) {
  register_type(_type_handle, name);
}

/**
 * Floods this bitmask's bits upwards.
 */
template<class WType, int nbits>
INLINE void BitMask<WType, nbits>::
flood_up_in_place() {
  _word = ::flood_bits_up(_word);
}

/**
 * Floods this bitmask's bits downwards.
 */
template<class WType, int nbits>
INLINE void BitMask<WType, nbits>::
flood_down_in_place() {
  _word = ::flood_bits_down(_word);
}

/**
 * Returns a BitMask with the bits flooded upwards.
 */
template<class WType, int nbits>
INLINE BitMask<WType, nbits> BitMask<WType, nbits>::
flood_bits_up() const {
  BitMask<WType, nbits> result(::flood_bits_up(_word));
  return result;
}

/**
 * Returns a BitMask with the bits flooded down.
 */
template<class WType, int nbits>
INLINE BitMask<WType, nbits> BitMask<WType, nbits>::
flood_bits_down() const {
  BitMask<WType, nbits> result(::flood_bits_down(_word));
  return result;
}


/**
 * Returns a BitMask with only the next highest bit above the indicated bit
 * on, or all_off.
 */
template<class WType, int nbits>
INLINE BitMask<WType, nbits> BitMask<WType, nbits>::
keep_next_highest_bit() const {
  int low_bit = get_lowest_on_bit();
  if (low_bit >= 0) {
    return BitMask<WType, nbits>::bit(low_bit);
  } else {
    return BitMask<WType, nbits>::all_off();
  }
}

/**
 * Returns a BitMask with only the next lower bit below the indicated bit on,
 * or all_off.
 */
template<class WType, int nbits>
INLINE BitMask<WType, nbits> BitMask<WType, nbits>::
keep_next_lowest_bit() const {
  int high_bit = get_highest_on_bit();
  if (high_bit >= 0) {
    return BitMask<WType, nbits>::bit(high_bit);
  } else {
    return BitMask<WType, nbits>::all_off();
  }
}

/**
 * Returns a BitMask with only the next highest bit above the indicated bit
 * on, or all.
 */
template<class WType, int nbits>
INLINE BitMask<WType, nbits> BitMask<WType, nbits>::
keep_next_highest_bit(int index) const {
  BitMask<WType, nbits> mask,temp;
  nassertr(index >= 0 && index < num_bits, mask);

  mask.set_bit(index);
  mask.flood_down_in_place();
  mask.invert_in_place();
  mask &= *this;
  temp = mask;

  mask <<= 1;
  mask.flood_up_in_place();
  mask.invert_in_place();
  mask &= temp;

  return mask;
}

/**
 * Returns a BitMask with only the next lower bit below the indicated bit on,
 * or all_off.
 */
template<class WType, int nbits>
INLINE BitMask<WType, nbits> BitMask<WType, nbits>::
keep_next_lowest_bit(int index) const {
  BitMask<WType, nbits> mask, temp;
  nassertr(index >= 0 && index < num_bits, mask);

  mask.set_bit(index);
  mask.flood_up_in_place();
  mask.invert_in_place();
  mask &= *this;
  temp = mask;

  mask >>= 1;
  mask.flood_down_in_place();
  mask.invert_in_place();
  mask &= temp;

  return mask;
}

/**
 * Returns a BitMask with only the next highest "on" bit above all "on" bits
 * in the passed in bitmask, or all_off.  If there are no "on" bits in the
 * passed in bitmask, it will return keep_next_highest_bit().
 */
template<class WType, int nbits>
INLINE BitMask<WType, nbits> BitMask<WType, nbits>::
keep_next_highest_bit(const BitMask<WType, nbits> &other) const {
  int high_bit = other.get_highest_on_bit();
  if (high_bit >= 0) {
    return keep_next_highest_bit(high_bit);
  } else {
    return keep_next_highest_bit();
  }
}

/**
 * Returns a BitMask with only the next lowest "on" bit below all "on" bits in
 * the passed in bitmask, or all_off.  If there are no "on" bits in the passed
 * in bitmask, it will return keep_next_lowest_bit().
 */
template<class WType, int nbits>
INLINE BitMask<WType, nbits> BitMask<WType, nbits>::
keep_next_lowest_bit(const BitMask<WType, nbits> &other) const {
  int low_bit = other.get_lowest_on_bit();
  if (low_bit >= 0) {
    return keep_next_lowest_bit(low_bit);
  } else {
    return keep_next_lowest_bit();
  }
}
