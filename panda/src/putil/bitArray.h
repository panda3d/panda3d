/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bitArray.h
 * @author drose
 * @date 2006-01-20
 */

#ifndef BITARRAY_H
#define BITARRAY_H

#include "pandabase.h"

#include "bitMask.h"
#include "numeric_types.h"
#include "typedObject.h"
#include "indent.h"
#include "pointerToArray.h"

#include "checksumHashGenerator.h"

class SparseArray;
class BamWriter;
class BamReader;
class Datagram;
class DatagramIterator;

/**
 * A dynamic array with an unlimited number of bits.
 *
 * This is similar to a BitMask, except it appears to contain an infinite
 * number of bits.  You can use it very much as you would use a BitMask.
 */
class EXPCL_PANDA_PUTIL BitArray {
public:
  typedef BitMaskNative MaskType;
  typedef MaskType::WordType WordType;

PUBLISHED:
  enum { num_bits_per_word = MaskType::num_bits };

  INLINE BitArray();
  INLINE BitArray(WordType init_value);
  BitArray(const SparseArray &from);

  INLINE static BitArray all_on();
  INLINE static BitArray all_off();
  INLINE static BitArray lower_on(int on_bits);
  INLINE static BitArray bit(int index);
  INLINE static BitArray range(int low_bit, int size);

  constexpr static bool has_max_num_bits() { return false; }
  constexpr static int get_max_num_bits() { return INT_MAX; }

  constexpr static int get_num_bits_per_word() { return num_bits_per_word; }
  INLINE size_t get_num_bits() const;
  INLINE bool get_bit(int index) const;
  INLINE void set_bit(int index);
  INLINE void clear_bit(int index);
  INLINE void set_bit_to(int index, bool value);
  INLINE bool get_highest_bits() const;
  bool is_zero() const;
  bool is_all_on() const;

  INLINE WordType extract(int low_bit, int size) const;
  INLINE void store(WordType value, int low_bit, int size);
  bool has_any_of(int low_bit, int size) const;
  bool has_all_of(int low_bit, int size) const;
  void set_range(int low_bit, int size);
  void clear_range(int low_bit, int size);
  INLINE void set_range_to(bool value, int low_bit, int size);

  int get_num_on_bits() const;
  int get_num_off_bits() const;
  int get_lowest_on_bit() const;
  int get_lowest_off_bit() const;
  int get_highest_on_bit() const;
  int get_highest_off_bit() const;
  int get_next_higher_different_bit(int low_bit) const;

  INLINE size_t get_num_words() const;
  INLINE MaskType get_word(size_t n) const;
  INLINE void set_word(size_t n, WordType value);

  void invert_in_place();
  bool has_bits_in_common(const BitArray &other) const;
  INLINE void clear();

  void output(std::ostream &out) const;
  void output_binary(std::ostream &out, int spaces_every = 4) const;
  void output_hex(std::ostream &out, int spaces_every = 4) const;
  void write(std::ostream &out, int indent_level = 0) const;

  INLINE bool operator == (const BitArray &other) const;
  INLINE bool operator != (const BitArray &other) const;
  INLINE bool operator < (const BitArray &other) const;
  int compare_to(const BitArray &other) const;

  INLINE BitArray
  operator & (const BitArray &other) const;

  INLINE BitArray
  operator | (const BitArray &other) const;

  INLINE BitArray
  operator ^ (const BitArray &other) const;

  INLINE BitArray
  operator ~ () const;

  INLINE BitArray
  operator << (int shift) const;

  INLINE BitArray
  operator >> (int shift) const;

  void operator &= (const BitArray &other);
  void operator |= (const BitArray &other);
  void operator ^= (const BitArray &other);
  void operator <<= (int shift);
  void operator >>= (int shift);

public:
  void generate_hash(ChecksumHashGenerator &hashgen) const;

private:
  INLINE void copy_on_write();
  void ensure_has_word(int n);
  void normalize();

private:
  typedef PTA(MaskType) Array;
  Array _array;
  int _highest_bits;  // Either 0 or 1.

public:
  void write_datagram(BamWriter *manager, Datagram &dg) const;
  void read_datagram(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    register_type(_type_handle, "BitArray");
  }

private:
  static TypeHandle _type_handle;
};

#include "bitArray.I"

INLINE std::ostream &
operator << (std::ostream &out, const BitArray &array) {
  array.output(out);
  return out;
}

#endif
