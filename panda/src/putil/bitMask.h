// Filename: bitMask.h
// Created by:  drose (08Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef BITMASK_H
#define BITMASK_H

#include "pandabase.h"

#include "numeric_types.h"
#include "typedObject.h"
#include "indent.h"

#include <checksumHashGenerator.h>


////////////////////////////////////////////////////////////////////
//       Class : BitMask
// Description : A general bitmask class.  This stores an array of
//               bits of some length that must fit within a given word
//               of the indicated type.  See also BitArray.
////////////////////////////////////////////////////////////////////
template<class WordType, int num_bits>
class BitMask {
PUBLISHED:
  INLINE BitMask();
  INLINE BitMask(WordType init_value);
  INLINE BitMask(const BitMask<WordType, num_bits> &copy);
  INLINE void operator = (const BitMask<WordType, num_bits> &copy);

  INLINE static BitMask<WordType, num_bits> all_on();
  INLINE static BitMask<WordType, num_bits> all_off();
  INLINE static BitMask<WordType, num_bits> lower_on(int on_bits);
  INLINE static BitMask<WordType, num_bits> bit(int index);

  INLINE ~BitMask();

  INLINE int get_num_bits() const;
  INLINE bool get_bit(int index) const;
  INLINE void set_bit(int index);
  INLINE void clear_bit(int index);
  INLINE void set_bit_to(int index, bool value);
  INLINE bool is_zero() const;

  INLINE WordType extract(int low_bit, int size) const;
  INLINE void store(WordType value, int low_bit, int size);
  INLINE WordType get_word() const;
  INLINE void set_word(WordType value);

  INLINE void invert_in_place();
  INLINE void clear();

  void output(ostream &out) const;
  void output_binary(ostream &out, int spaces_every = 4) const;
  void output_hex(ostream &out, int spaces_every = 4) const;
  void write(ostream &out, int indent_level = 0) const;

  INLINE bool operator == (const BitMask<WordType, num_bits> &other) const;
  INLINE bool operator != (const BitMask<WordType, num_bits> &other) const;
  INLINE bool operator < (const BitMask<WordType, num_bits> &other) const;
  INLINE int compare_to(const BitMask<WordType, num_bits> &other) const;

  INLINE BitMask<WordType, num_bits>
  operator & (const BitMask<WordType, num_bits> &other) const;

  INLINE BitMask<WordType, num_bits>
  operator | (const BitMask<WordType, num_bits> &other) const;

  INLINE BitMask<WordType, num_bits>
  operator ^ (const BitMask<WordType, num_bits> &other) const;

  INLINE BitMask<WordType, num_bits>
  operator ~ () const;

  INLINE BitMask<WordType, num_bits>
  operator << (int shift) const;

  INLINE BitMask<WordType, num_bits>
  operator >> (int shift) const;

  INLINE void operator &= (const BitMask<WordType, num_bits> &other);
  INLINE void operator |= (const BitMask<WordType, num_bits> &other);
  INLINE void operator ^= (const BitMask<WordType, num_bits> &other);
  INLINE void operator <<= (int shift);
  INLINE void operator >>= (int shift);

public:
  INLINE void generate_hash(ChecksumHashGenerator &hashgen) const;

private:
  WordType _word;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();

private:
  static TypeHandle _type_handle;
};

#include "bitMask.I"

template<class WordType, int num_bits>
INLINE ostream &operator << (ostream &out, const BitMask<WordType, num_bits> &bitmask) {
  bitmask.output(out);
  return out;
}

// We need to define this temporary macro so we can pass a parameter
// containing a comma through the macro.
#define BITMASK32_DEF BitMask<PN_uint32, 32>
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, BITMASK32_DEF);

typedef BitMask<PN_uint32, 32> BitMask32;

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
