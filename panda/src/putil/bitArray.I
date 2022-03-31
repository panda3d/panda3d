/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bitArray.I
 * @author drose
 * @date 2006-01-20
 */

/**
 *
 */
INLINE BitArray::
BitArray() {
  _highest_bits = 0;
}

/**
 *
 */
INLINE BitArray::
BitArray(WordType init_value) {
  if (init_value != 0) {
    _array.push_back(MaskType(init_value));
  }
  _highest_bits = 0;
}

/**
 * Returns a BitArray with an infinite array of bits, all on.
 */
INLINE BitArray BitArray::
all_on() {
  BitArray result;
  result._highest_bits = 1;
  return result;
}

/**
 * Returns a BitArray whose bits are all off.
 */
INLINE BitArray BitArray::
all_off() {
  return BitArray();
}

/**
 * Returns a BitArray whose lower on_bits bits are on.
 */
INLINE BitArray BitArray::
lower_on(int on_bits) {
  BitArray result;
  result.set_range(0, on_bits);
  return result;
}

/**
 * Returns a BitArray with only the indicated bit on.
 */
INLINE BitArray BitArray::
bit(int index) {
  BitArray result;
  result.set_bit(index);
  return result;
}

/**
 * Returns a BitArray whose size bits, beginning at low_bit, are on.
 */
INLINE BitArray BitArray::
range(int low_bit, int size) {
  BitArray result;
  result.set_range(low_bit, size);
  return result;
}

/**
 * Returns the current number of possibly different bits in this array.  There
 * are actually an infinite number of bits, but every bit higher than this bit
 * will have the same value, either 0 or 1 (see get_highest_bits()).
 *
 * This number may grow and/or shrink automatically as needed.
 */
INLINE size_t BitArray::
get_num_bits() const {
  return get_num_words() * (size_t)num_bits_per_word;
}

/**
 * Returns true if the nth bit is set, false if it is cleared.  It is valid
 * for n to increase beyond get_num_bits(), but the return value
 * get_num_bits() will always be the same.
 */
INLINE bool BitArray::
get_bit(int index) const {
  nassertr(index >= 0, false);
  int w = index / num_bits_per_word;
  int b = index % num_bits_per_word;
  if ((size_t)w >= get_num_words()) {
    return get_highest_bits();
  } else {
    return (_array[w].get_bit(b));
  }
}

/**
 * Sets the nth bit on.  If n >= get_num_bits(), this automatically extends
 * the array.
 */
INLINE void BitArray::
set_bit(int index) {
  nassertv(index >= 0);
  int w = index / num_bits_per_word;
  int b = index % num_bits_per_word;
  if ((size_t)w >= get_num_words() && _highest_bits) {
    // All the highest bits are already on.
    return;
  }
  ensure_has_word(w);
  _array[w].set_bit(b);
  normalize();
}

/**
 * Sets the nth bit off.  If n >= get_num_bits(), this automatically extends
 * the array.
 */
INLINE void BitArray::
clear_bit(int index) {
  nassertv(index >= 0);
  int w = index / num_bits_per_word;
  int b = index % num_bits_per_word;
  if ((size_t)w >= get_num_words() && !_highest_bits) {
    // All the highest bits are already off.
    return;
  }
  ensure_has_word(w);
  _array[w].clear_bit(b);
  normalize();
}

/**
 * Sets the nth bit either on or off, according to the indicated bool value.
 */
INLINE void BitArray::
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
INLINE bool BitArray::
get_highest_bits() const {
  return (_highest_bits != 0);
}

/**
 * Returns a word that represents only the indicated range of bits within this
 * BitArray, shifted to the least-significant position.  size must be <=
 * get_num_bits_per_word().
 */
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

/**
 * Stores the indicated word into the indicated range of bits with this
 * BitArray.
 */
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

/**
 * Sets the indicated range of bits to either on or off.
 */
INLINE void BitArray::
set_range_to(bool value, int low_bit, int size) {
  if (value) {
    set_range(low_bit, size);
  } else {
    clear_range(low_bit, size);
  }
}

/**
 * Returns the number of possibly-unique words stored in the array.
 */
INLINE size_t BitArray::
get_num_words() const {
  return _array.size();
}

/**
 * Returns the nth word in the array.  It is valid for n to be greater than
 * get_num_words(), but the return value beyond get_num_words() will always be
 * the same.
 */
INLINE BitArray::MaskType BitArray::
get_word(size_t n) const {
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

/**
 * Replaces the nth word in the array.  If n >= get_num_words(), this
 * automatically extends the array.
 */
INLINE void BitArray::
set_word(size_t n, WordType value) {
  ensure_has_word(n);
  _array[n] = value;
  normalize();
}

/**
 * Sets all the bits in the BitArray off.
 */
void BitArray::
clear() {
  _array.clear();
  _highest_bits = 0;
}

/**
 *
 */
INLINE bool BitArray::
operator == (const BitArray &other) const {
  return compare_to(other) == 0;
}

/**
 *
 */
INLINE bool BitArray::
operator != (const BitArray &other) const {
  return compare_to(other) != 0;
}

/**
 * Returns true if the unsigned integer which is represented by this BitArray
 * is less than that of the other one, false otherwise.
 */
INLINE bool BitArray::
operator < (const BitArray &other) const {
  return compare_to(other) < 0;
}

/**
 *
 */
INLINE BitArray BitArray::
operator & (const BitArray &other) const {
  BitArray result(*this);
  result &= other;
  return result;
}

/**
 *
 */
INLINE BitArray BitArray::
operator | (const BitArray &other) const {
  BitArray result(*this);
  result |= other;
  return result;
}

/**
 *
 */
INLINE BitArray BitArray::
operator ^ (const BitArray &other) const {
  BitArray result(*this);
  result ^= other;
  return result;
}

/**
 *
 */
INLINE BitArray BitArray::
operator ~ () const {
  BitArray result(*this);
  result.invert_in_place();
  return result;
}

/**
 *
 */
INLINE BitArray BitArray::
operator << (int shift) const {
  BitArray result(*this);
  result <<= shift;
  return result;
}

/**
 *
 */
INLINE BitArray BitArray::
operator >> (int shift) const {
  BitArray result(*this);
  result >>= shift;
  return result;
}

/**
 * Called internally just before writing to the _array member, this makes a
 * new copy of _array if it appears to be shared with any other objects--thus
 * achieving copy-on-write.
 */
INLINE void BitArray::
copy_on_write() {
  if (_array.get_ref_count() > 1) {
    PTA(MaskType) new_array;
    new_array.v() = _array.v();
    _array = new_array;
  }
}
