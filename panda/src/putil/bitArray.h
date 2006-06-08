// Filename: bitArray.h
// Created by:  drose (20Jan06)
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

#ifndef BITARRAY_H
#define BITARRAY_H

#include "pandabase.h"

#include "bitMask.h"
#include "numeric_types.h"
#include "typedObject.h"
#include "indent.h"
#include "pointerToArray.h"

#include "checksumHashGenerator.h"

////////////////////////////////////////////////////////////////////
//       Class : BitArray
// Description : A dynamic array with an unlimited number of bits.
//
//               This is similar to a BitMask, except it appears to
//               contain an infinite number of bits.  You can use it
//               very much as you would use a BitMask.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA BitArray {
public:
  typedef BitMask32 MaskType;
  typedef MaskType::WordType WordType;
  enum { num_bits_per_word = MaskType::num_bits };

PUBLISHED:

  INLINE BitArray();
  INLINE BitArray(WordType init_value);
  INLINE BitArray(const BitArray &copy);
  INLINE BitArray &operator = (const BitArray &copy);

  INLINE static BitArray all_on();
  INLINE static BitArray all_off();
  INLINE static BitArray lower_on(int on_bits);
  INLINE static BitArray bit(int index);
  INLINE static BitArray range(int low_bit, int size);

  INLINE ~BitArray();

  INLINE static bool has_max_num_bits();
  INLINE static int get_max_num_bits();

  INLINE static int get_num_bits_per_word();
  INLINE int get_num_bits() const;
  INLINE bool get_bit(int index) const;
  INLINE void set_bit(int index);
  INLINE void clear_bit(int index);
  INLINE void set_bit_to(int index, bool value);
  INLINE bool get_highest_bits() const;
  bool is_zero() const;

  INLINE WordType extract(int low_bit, int size) const;
  INLINE void store(WordType value, int low_bit, int size);
  void set_range(int low_bit, int size);
  void clear_range(int low_bit, int size);
  INLINE void set_range_to(bool value, int low_bit, int size);

  INLINE int get_num_words() const;
  INLINE MaskType get_word(int n) const;
  INLINE void set_word(int n, MaskType value);

  void invert_in_place();
  bool has_bits_in_common(const BitArray &other) const;
  INLINE void clear();

  void output(ostream &out) const;
  void output_binary(ostream &out, int spaces_every = 4) const;
  void output_hex(ostream &out, int spaces_every = 4) const;
  void write(ostream &out, int indent_level = 0) const;

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

INLINE ostream &
operator << (ostream &out, const BitArray &array) {
  array.output(out);
  return out;
}

#endif
