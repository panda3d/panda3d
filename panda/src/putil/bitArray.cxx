/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bitArray.cxx
 * @author drose
 * @date 2006-01-20
 */

#include "bitArray.h"
#include "sparseArray.h"
#include "datagram.h"
#include "datagramIterator.h"

using std::max;
using std::min;
using std::ostream;

TypeHandle BitArray::_type_handle;

/**
 *
 */
BitArray::
BitArray(const SparseArray &from) {
  _highest_bits = 0;

  int num_subranges = from.get_num_subranges();
  for (int i = 0; i < num_subranges; ++i) {
    int begin = from.get_subrange_begin(i);
    int end = from.get_subrange_end(i);
    set_range(begin, end - begin);
  }

  if (from.is_inverse()) {
    invert_in_place();
  }
}

/**
 * Returns true if the entire bitmask is zero, false otherwise.
 */
bool BitArray::
is_zero() const {
  if (_highest_bits) {
    // If all the infinite highest bits are set, certainly the bitmask is
    // nonzero.
    return false;
  }

  // Start from the high end, since that's more likely to be nonzero.
  Array::reverse_iterator ai;
  for (ai = _array.rbegin(); ai != _array.rend(); ++ai) {
    if (!(*ai).is_zero()) {
      return false;
    }
  }
  return true;
}

/**
 * Returns true if the entire bitmask is one, false otherwise.
 */
bool BitArray::
is_all_on() const {
  if (!_highest_bits) {
    // If all the infinite highest bits are not set, certainly the bitmask is
    // not all on.
    return false;
  }

  Array::reverse_iterator ai;
  for (ai = _array.rbegin(); ai != _array.rend(); ++ai) {
    if (!(*ai).is_all_on()) {
      return false;
    }
  }
  return true;
}

/**
 * Returns true if any bit in the indicated range is set, false otherwise.
 */
bool BitArray::
has_any_of(int low_bit, int size) const {
  if ((size_t)(low_bit + size) > get_num_bits()) {
    // This range touches the highest bits.
    if (_highest_bits) {
      return true;
    }
  }

  int w = low_bit / num_bits_per_word;
  int b = low_bit % num_bits_per_word;

  if (w >= (int)get_num_words()) {
    // This range is entirely among the highest bits.
    return (_highest_bits != 0);
  }
  if (b + size <= num_bits_per_word) {
    // The whole thing fits within one word of the array.
    return get_word(w).has_any_of(b, size);
  }

  int num_high_bits = num_bits_per_word - b;
  if (_array[w].has_any_of(b, num_high_bits)) {
    return true;
  }
  size -= num_high_bits;
  ++w;

  while (size > 0) {
    if (size <= num_bits_per_word) {
      // The remainder fits within one word of the array.
      return _array[w].has_any_of(0, size);
    }

    // Keep going.
    if (!_array[w].is_zero()) {
      return true;
    }
    size -= num_bits_per_word;
    ++w;

    if (w >= (int)get_num_words()) {
      // Now we're up to the highest bits.
      return (_highest_bits != 0);
    }
  }

  return false;
}

/**
 * Returns true if all bits in the indicated range are set, false otherwise.
 */
bool BitArray::
has_all_of(int low_bit, int size) const {
  if ((size_t)(low_bit + size) > get_num_bits()) {
    // This range touches the highest bits.
    if (!_highest_bits) {
      return false;
    }
  }

  int w = low_bit / num_bits_per_word;
  int b = low_bit % num_bits_per_word;

  if (w >= (int)get_num_words()) {
    // This range is entirely among the highest bits.
    return (_highest_bits != 0);
  }
  if (b + size <= num_bits_per_word) {
    // The whole thing fits within one word of the array.
    return get_word(w).has_all_of(b, size);
  }

  int num_high_bits = num_bits_per_word - b;
  if (!_array[w].has_all_of(b, num_high_bits)) {
    return false;
  }
  size -= num_high_bits;
  ++w;

  while (size > 0) {
    if (size <= num_bits_per_word) {
      // The remainder fits within one word of the array.
      return _array[w].has_all_of(0, size);
    }

    // Keep going.
    if (!_array[w].is_all_on()) {
      return false;
    }
    size -= num_bits_per_word;
    ++w;

    if (w >= (int)get_num_words()) {
      // Now we're up to the highest bits.
      return (_highest_bits != 0);
    }
  }

  return true;
}

/**
 * Sets the indicated range of bits on.
 */
void BitArray::
set_range(int low_bit, int size) {
  int w = low_bit / num_bits_per_word;
  int b = low_bit % num_bits_per_word;

  if (w >= (int)get_num_words() && _highest_bits) {
    // All the highest bits are already on.
    return;
  }
  if (b + size <= num_bits_per_word) {
    // The whole thing fits within one word of the array.
    ensure_has_word(w);
    _array[w].set_range(b, size);
    normalize();
    return;
  }

  ensure_has_word(w);
  int num_high_bits = num_bits_per_word - b;
  _array[w].set_range(b, num_high_bits);
  size -= num_high_bits;
  ++w;

  while (size > 0) {
    if (size <= num_bits_per_word) {
      // The remainder fits within one word of the array.
      ensure_has_word(w);
      _array[w].set_range(0, size);
      normalize();
      return;
    }

    // Keep going.
    ensure_has_word(w);
    _array[w] = MaskType::all_on();
    size -= num_bits_per_word;
    ++w;

    if (w >= (int)get_num_words() && _highest_bits) {
      // All the highest bits are already on.
      normalize();
      return;
    }
  }
  normalize();
}

/**
 * Sets the indicated range of bits off.
 */
void BitArray::
clear_range(int low_bit, int size) {
  int w = low_bit / num_bits_per_word;
  int b = low_bit % num_bits_per_word;

  if (w >= (int)get_num_words() && !_highest_bits) {
    // All the highest bits are already off.
    return;
  }
  if (b + size <= num_bits_per_word) {
    // The whole thing fits within one word of the array.
    ensure_has_word(w);
    _array[w].clear_range(b, size);
    normalize();
    return;
  }

  ensure_has_word(w);
  int num_high_bits = num_bits_per_word - b;
  _array[w].clear_range(b, num_high_bits);
  size -= num_high_bits;
  ++w;

  while (size > 0) {
    if (size <= num_bits_per_word) {
      // The remainder fits within one word of the array.
      ensure_has_word(w);
      _array[w].clear_range(0, size);
      normalize();
      return;
    }

    // Keep going.
    ensure_has_word(w);
    _array[w] = MaskType::all_off();
    size -= num_bits_per_word;
    ++w;

    if (w >= (int)get_num_words() && !_highest_bits) {
      // All the highest bits are already off.
      normalize();
      return;
    }
  }
  normalize();
}

/**
 * Returns the number of bits that are set to 1 in the array.  Returns -1 if
 * there are an infinite number of 1 bits.
 */
int BitArray::
get_num_on_bits() const {
  if (_highest_bits) {
    return -1;
  }

  int result = 0;
  Array::const_iterator ai;
  for (ai = _array.begin(); ai != _array.end(); ++ai) {
    result += (*ai).get_num_on_bits();
  }
  return result;
}

/**
 * Returns the number of bits that are set to 0 in the array.  Returns -1 if
 * there are an infinite number of 0 bits.
 */
int BitArray::
get_num_off_bits() const {
  if (!_highest_bits) {
    return -1;
  }

  int result = 0;
  Array::const_iterator ai;
  for (ai = _array.begin(); ai != _array.end(); ++ai) {
    result += (*ai).get_num_off_bits();
  }
  return result;
}

/**
 * Returns the index of the lowest 1 bit in the array.  Returns -1 if there
 * are no 1 bits.
 */
int BitArray::
get_lowest_on_bit() const {
  int num_words = get_num_words();
  for (int w = 0; w < num_words; ++w) {
    int b = _array[w].get_lowest_on_bit();
    if (b != -1) {
      return w * num_bits_per_word + b;
    }
  }
  if (_highest_bits) {
    return num_words * num_bits_per_word;
  } else {
    return -1;
  }
}

/**
 * Returns the index of the lowest 0 bit in the array.  Returns -1 if there
 * are no 0 bits.
 */
int BitArray::
get_lowest_off_bit() const {
  int num_words = get_num_words();
  for (int w = 0; w < num_words; ++w) {
    int b = _array[w].get_lowest_off_bit();
    if (b != -1) {
      return w * num_bits_per_word + b;
    }
  }
  if (!_highest_bits) {
    return num_words * num_bits_per_word;
  } else {
    return -1;
  }
}

/**
 * Returns the index of the highest 1 bit in the array.  Returns -1 if there
 * are no 1 bits or if there an infinite number of 1 bits.
 */
int BitArray::
get_highest_on_bit() const {
  if (_highest_bits) {
    return -1;
  }
  int num_words = get_num_words();
  for (int w = num_words - 1; w >= 0; --w) {
    int b = _array[w].get_highest_on_bit();
    if (b != -1) {
      return w * num_bits_per_word + b;
    }
  }
  return -1;
}

/**
 * Returns the index of the highest 0 bit in the array.  Returns -1 if there
 * are no 0 bits or if there an infinite number of 1 bits.
 */
int BitArray::
get_highest_off_bit() const {
  if (!_highest_bits) {
    return -1;
  }
  int num_words = get_num_words();
  for (int w = num_words - 1; w >= 0; --w) {
    int b = _array[w].get_highest_off_bit();
    if (b != -1) {
      return w * num_bits_per_word + b;
    }
  }
  return -1;
}

/**
 * Returns the index of the next bit in the array, above low_bit, whose value
 * is different that the value of low_bit.  Returns low_bit again if all bits
 * higher than low_bit have the same value.
 *
 * This can be used to quickly iterate through all of the bits in the array.
 */
int BitArray::
get_next_higher_different_bit(int low_bit) const {
  int w = low_bit / num_bits_per_word;
  int b = low_bit % num_bits_per_word;
  int num_words = get_num_words();
  if (w >= num_words) {
    return low_bit;
  }
  int b2 = _array[w].get_next_higher_different_bit(b);
  if (b2 != b && b2 < num_bits_per_word) {
    // The next higher bit is within the same word.
    return w * num_bits_per_word + b2;
  }
  // Look for the next word with anything interesting.
  MaskType skip_next = (_array[w].get_bit(b)) ? MaskType::all_on() : MaskType::all_off();
  int w2 = w;
  ++w2;
  while (w2 < num_words && _array[w2] == skip_next) {
    ++w2;
  }
  if (w2 >= num_words) {
    // All bits higher are the same value.
    int is_on = _array[w].get_bit(b);
    return is_on ? (num_words * num_bits_per_word) : low_bit;
  }
  if (_array[w2].get_bit(0) != _array[w].get_bit(b)) {
    // The first bit of word w2 is different.
    return w2 * num_bits_per_word;
  }

  b2 = _array[w2].get_next_higher_different_bit(0);
  return w2 * num_bits_per_word + b2;
}

/**
 * Inverts all the bits in the BitArray.  This is equivalent to array =
 * ~array.
 */
void BitArray::
invert_in_place() {
  _highest_bits = !_highest_bits;
  copy_on_write();
  Array::iterator ai;
  for (ai = _array.begin(); ai != _array.end(); ++ai) {
    (*ai) = ~(*ai);
  }
}

/**
 * Returns true if this BitArray has any "one" bits in common with the other
 * one, false otherwise.
 *
 * This is equivalent to (array & other) != 0, but may be faster.
 */
bool BitArray::
has_bits_in_common(const BitArray &other) const {
  if (_highest_bits && other._highest_bits) {
    // Yup, in fact we have an infinite number of bits in common.
    return true;
  }

  size_t num_common_words = min(_array.size(), other._array.size());

  // Consider the words that are on top of either array.
  if (other._array.size() < _array.size() && other._highest_bits) {
    // The other array has fewer actual words, and the top n words of the
    // other array are all ones.  We have bits in common if any of our top n
    // words are nonzero.
    Array::const_iterator ai;
    for (ai = _array.begin() + other._array.size();
         ai != _array.end();
         ++ai) {
      if (!(*ai).is_zero()) {
        return true;
      }
    }

  } else if (_array.size() < other._array.size() && _highest_bits) {
    // This array has fewer actual words, and the top n words of this array
    // are all ones.  We have bits in common if any of the the other's top n
    // words are nonzero.
    Array::const_iterator ai;
    for (ai = other._array.begin() + _array.size();
         ai != other._array.end();
         ++ai) {
      if (!(*ai).is_zero()) {
        return true;
      }
    }
  }

  // Consider the words that both arrays have in common.
  for (size_t i = 0; i < num_common_words; ++i) {
    if (!(_array[i] & other._array[i]).is_zero()) {
      return true;
    }
  }

  // Nope, nothing.
  return false;
}

/**
 * Writes the BitArray out as a hex number.  For a BitArray, this is always
 * the same as output_hex(); it's too confusing for the output format to
 * change back and forth at runtime.
 */
void BitArray::
output(ostream &out) const {
  output_hex(out);
}

/**
 * Writes the BitArray out as a binary number, with spaces every four bits.
 */
void BitArray::
output_binary(ostream &out, int spaces_every) const {
  if (_highest_bits) {
    out << "...1 ";
  }
  int num_bits = max((int)get_num_bits(), spaces_every);
  for (int i = num_bits - 1; i >= 0; i--) {
    if (spaces_every != 0 && ((i % spaces_every) == spaces_every - 1)) {
      out << ' ';
    }
    out << (get_bit(i) ? '1' : '0');
  }
}

/**
 * Writes the BitArray out as a hexadecimal number, with spaces every four
 * digits.
 */
void BitArray::
output_hex(ostream &out, int spaces_every) const {
  int num_bits = get_num_bits();
  int num_digits = max((num_bits + 3) / 4, spaces_every);

  if (_highest_bits) {
    out << "...f ";
  }

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
 * Writes the BitArray out as a binary or a hex number, according to the
 * number of bits.
 */
void BitArray::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << *this << "\n";
}

/**
 * Returns a number less than zero if this BitArray sorts before the indicated
 * other BitArray, greater than zero if it sorts after, or 0 if they are
 * equivalent.  This is based on the same ordering defined by operator <.
 */
int BitArray::
compare_to(const BitArray &other) const {
  if (_highest_bits != other._highest_bits) {
    return _highest_bits ? 1 : -1;
  }

  int num_words = max(get_num_words(), other.get_num_words());

  // Compare from highest-order to lowest-order word.
  for (int i = num_words - 1; i >= 0; --i) {
    int compare = get_word(i).compare_to(other.get_word(i));
    if (compare != 0) {
      return compare;
    }
  }

  return 0;
}

/**
 *
 */
void BitArray::
operator &= (const BitArray &other) {
  size_t num_common_words = min(_array.size(), other._array.size());

  copy_on_write();

  // Consider the words that are on top of either array.
  if (other._array.size() < _array.size() && !other._highest_bits) {
    // The other array has fewer actual words, and the top n words of the
    // other array are all zeroes.  "mask off" the top n words of this array.
    _array.erase(_array.begin() + other._array.size(), _array.end());

  } else if (_array.size() < other._array.size() && _highest_bits) {
    // This array has fewer actual words, and the top n words of this array
    // are all ones.  "mask on" the top n words of the other array.
    Array::const_iterator ai;
    for (ai = other._array.begin() + _array.size();
         ai != other._array.end();
         ++ai) {
      _array.push_back(*ai);
    }
  }

  // Consider the words that both arrays have in common.
  for (size_t i = 0; i < num_common_words; ++i) {
    _array[i] &= other._array[i];
  }

  _highest_bits &= other._highest_bits;
  normalize();
}

/**
 *
 */
void BitArray::
operator |= (const BitArray &other) {
  size_t num_common_words = min(_array.size(), other._array.size());

  copy_on_write();

  // Consider the words that are on top of either array.
  if (other._array.size() < _array.size() && other._highest_bits) {
    // The other array has fewer actual words, and the top n words of the
    // other array are all ones.  The top n words of this array become ones
    // too (which means we can drop them out).
    _array.erase(_array.begin() + other._array.size(), _array.end());

  } else if (_array.size() < other._array.size() && !_highest_bits) {
    // This array has fewer actual words, and the top n words of this array
    // are all zeros.  Copy in the top n words of the other array.
    Array::const_iterator ai;
    for (ai = other._array.begin() + _array.size();
         ai != other._array.end();
         ++ai) {
      _array.push_back(*ai);
    }
  }

  // Consider the words that both arrays have in common.
  for (size_t i = 0; i < num_common_words; ++i) {
    _array[i] |= other._array[i];
  }

  _highest_bits |= other._highest_bits;
  normalize();
}

/**
 *
 */
void BitArray::
operator ^= (const BitArray &other) {
  size_t num_common_words = min(_array.size(), other._array.size());

  copy_on_write();

  // Consider the words that are on top of either array.
  if (other._array.size() < _array.size() && other._highest_bits) {
    // The other array has fewer actual words, and the top n words of the
    // other array are all ones.  The top n words of this array get inverted.
    Array::iterator ai;
    for (ai = _array.begin() + other._array.size();
         ai != _array.end();
         ++ai) {
      (*ai).invert_in_place();
    }

  } else if (_array.size() < other._array.size()) {
    if (!_highest_bits) {
      // This array has fewer actual words, and the top n words of this array
      // are all zeros.  Copy in the top n words of the other array.
      Array::const_iterator ai;
      for (ai = other._array.begin() + _array.size();
           ai != other._array.end();
           ++ai) {
        _array.push_back(*ai);
      }
    } else {
      // This array has fewer actual words, and the top n words of this array
      // are all ones.  Copy in the top n words of the other array, inverted.
      Array::const_iterator ai;
      for (ai = other._array.begin() + _array.size();
           ai != other._array.end();
           ++ai) {
        _array.push_back(~(*ai));
      }
    }
  }

  // Consider the words that both arrays have in common.
  for (size_t i = 0; i < num_common_words; ++i) {
    _array[i] ^= other._array[i];
  }

  _highest_bits ^= other._highest_bits;
  normalize();
}

/**
 * Logical left shift.  The rightmost bits are filled in with zeroes.  Since
 * this is an infinite bit array, none of the bits on the left are lost.
 */
void BitArray::
operator <<= (int shift) {
  if (shift == 0 || _array.empty()) {
    return;
  }
  if (shift < 0) {
    operator >>= (-shift);
    return;
  }

  int w = shift / num_bits_per_word;
  int b = shift % num_bits_per_word;

  if (b == 0) {
    // Easy case--word-at-a-time.
    Array new_array;
    new_array.reserve(_array.size() + w);
    for (int i = 0; i < w; ++i) {
      new_array.push_back(MaskType::all_off());
    }
    Array::const_iterator ai;
    for (ai = _array.begin(); ai != _array.end(); ++ai) {
      new_array.push_back(*ai);
    }
    _array = new_array;

  } else {
    // Harder case--we have to shuffle bits between words.
    Array new_array;
    new_array.reserve(_array.size() + w + 1);
    for (int i = 0; i < w; ++i) {
      new_array.push_back(MaskType::all_off());
    }

    int downshift_count = num_bits_per_word - b;
    MaskType lower_mask = MaskType::lower_on(downshift_count);
    MaskType upper_mask = ~lower_mask;

    Array::const_iterator ai = _array.begin();
    nassertv(ai != _array.end());
    MaskType next_bits = ((*ai) & upper_mask) >> downshift_count;
    new_array.push_back(((*ai) & lower_mask) << b);
    ++ai;
    while (ai != _array.end()) {
      new_array.push_back((((*ai) & lower_mask) << b) | next_bits);
      next_bits = ((*ai) & upper_mask) >> downshift_count;
      ++ai;
    }

    // Finally, the top n bits.
    if (_highest_bits) {
      next_bits |= ~MaskType::lower_on(b);
    }
    new_array.push_back(next_bits);
    _array = new_array;
  }

  normalize();
}

/**
 * Logical right shift.  The rightmost bits are lost.  Since this is an
 * infinite bit array, there is no question of sign extension; there is no
 * need to synthesize bits on the left.
 */
void BitArray::
operator >>= (int shift) {
  if (shift == 0 || _array.empty()) {
    return;
  }
  if (shift < 0) {
    operator <<= (-shift);
    return;
  }

  int w = shift / num_bits_per_word;
  int b = shift % num_bits_per_word;

  if (w >= (int)_array.size()) {
    // Trivial case--shift to nothing.
    _array.clear();
    return;
  }

  if (b == 0) {
    // Easy case--word-at-a-time.
    Array new_array;
    new_array.reserve(_array.size() - w);
    Array::const_iterator ai;
    for (ai = _array.begin() + w; ai != _array.end(); ++ai) {
      new_array.push_back(*ai);
    }
    _array = new_array;

  } else {
    // Harder case--we have to shuffle bits between words.
    Array new_array;
    new_array.reserve(_array.size() - w);

    int upshift_count = num_bits_per_word - b;
    MaskType lower_mask = MaskType::lower_on(b);
    MaskType upper_mask = ~lower_mask;

    Array::const_iterator ai = _array.begin() + w;
    nassertv(ai < _array.end());
    MaskType next_bits = ((*ai) & upper_mask) >> b;

    ++ai;
    while (ai != _array.end()) {
      new_array.push_back((((*ai) & lower_mask) << upshift_count) | next_bits);
      next_bits = ((*ai) & upper_mask) >> b;
      ++ai;
    }

    // Finally, the top n bits.
    if (_highest_bits) {
      next_bits |= ~MaskType::lower_on(upshift_count);
    }
    new_array.push_back(next_bits);
    _array = new_array;
  }

  normalize();
}

/**
 * Adds the bitmask to the indicated hash generator.
 */
void BitArray::
generate_hash(ChecksumHashGenerator &hashgen) const {
  hashgen.add_int(_highest_bits);
  Array::const_iterator ai;
  for (ai = _array.begin(); ai != _array.end(); ++ai) {
    hashgen.add_int((*ai).get_word());
  }
}

/**
 * Ensures that at least word n has been allocated into the array.
 */
void BitArray::
ensure_has_word(int n) {
  copy_on_write();

  if (_highest_bits) {
    while ((size_t)n >= _array.size()) {
      _array.push_back(MaskType::all_on());
    }
  } else {
    while ((size_t)n >= _array.size()) {
      _array.push_back(MaskType::all_off());
    }
  }
}

/**
 * Ensures that the array is the smallest array that represents this same
 * value, by removing the topmost words that are all bits off (or on).
 */
void BitArray::
normalize() {
  if (_highest_bits) {
    if (!_array.empty() && _array.back() == MaskType::all_on()) {
      copy_on_write();
      _array.pop_back();
      while (!_array.empty() && _array.back() == MaskType::all_on()) {
        _array.pop_back();
      }
    }
  } else {
    if (!_array.empty() && _array.back().is_zero()) {
      copy_on_write();
      _array.pop_back();
      while (!_array.empty() && _array.back().is_zero()) {
        _array.pop_back();
      }
    }
  }
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void BitArray::
write_datagram(BamWriter *manager, Datagram &dg) const {
  dg.add_uint32(_array.size());
  Array::const_iterator ai;
  for (ai = _array.begin(); ai != _array.end(); ++ai) {
    dg.add_uint32((*ai).get_word());
  }
  dg.add_uint8(_highest_bits);
}

/**
 * Reads the object that was previously written to a Bam file.
 */
void BitArray::
read_datagram(DatagramIterator &scan, BamReader *manager) {
  size_t num_words = scan.get_uint32();
  _array = Array::empty_array(num_words);
  for (size_t i = 0; i < num_words; ++i) {
    _array[i] = WordType(scan.get_uint32());
  }
  _highest_bits = scan.get_uint8();
}
