/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file doubleBitMask.h
 * @author drose
 * @date 2000-06-08
 */

#ifndef DOUBLEBITMASK_H
#define DOUBLEBITMASK_H

#include "pandabase.h"

#include "bitMask.h"

/**
 * This is a special BitMask type that is implemented as a pair of lesser
 * BitMask types, to present a double-wide bit mask.  For instance, on a
 * 32-bit system, this can be used to make a single 64-bit bit mask.  More of
 * these can be ganged up together to make a 128-bit mask, and so on.
 */
template<class BMType>
class DoubleBitMask {
public:
  typedef typename BMType::WordType WordType;

PUBLISHED:
  typedef BMType BitMaskType;

  enum {
    half_bits = BMType::num_bits,
    num_bits = BMType::num_bits * 2,
  };

  constexpr DoubleBitMask() = default;

  INLINE static DoubleBitMask<BMType> all_on();
  INLINE static DoubleBitMask<BMType> all_off();
  INLINE static DoubleBitMask<BMType> lower_on(int on_bits);
  INLINE static DoubleBitMask<BMType> bit(int index);
  INLINE static DoubleBitMask<BMType> range(int low_bit, int size);

  constexpr static bool has_max_num_bits() {return true;}
  constexpr static int get_max_num_bits() {return num_bits;}

  constexpr int get_num_bits() const;
  INLINE bool get_bit(int index) const;
  INLINE void set_bit(int index);
  INLINE void clear_bit(int index);
  INLINE void set_bit_to(int index, bool value);
  INLINE bool is_zero() const;
  INLINE bool is_all_on() const;

  INLINE WordType extract(int low_bit, int size) const;
  INLINE void store(WordType value, int low_bit, int size);
  INLINE bool has_any_of(int low_bit, int size) const;
  INLINE bool has_all_of(int low_bit, int size) const;
  INLINE void set_range(int low_bit, int size);
  INLINE void clear_range(int low_bit, int size);
  INLINE void set_range_to(bool value, int low_bit, int size);

  INLINE int get_num_on_bits() const;
  INLINE int get_num_off_bits() const;
  INLINE int get_lowest_on_bit() const;
  INLINE int get_lowest_off_bit() const;
  INLINE int get_highest_on_bit() const;
  INLINE int get_highest_off_bit() const;
  INLINE int get_next_higher_different_bit(int low_bit) const;

  INLINE void invert_in_place();
  INLINE bool has_bits_in_common(const DoubleBitMask<BMType> &other) const;
  INLINE void clear();

  void output(std::ostream &out) const;
  void output_binary(std::ostream &out, int spaces_every = 4) const;
  void output_hex(std::ostream &out, int spaces_every = 4) const;
  void write(std::ostream &out, int indent_level = 0) const;

  INLINE bool operator == (const DoubleBitMask<BMType> &other) const;
  INLINE bool operator != (const DoubleBitMask<BMType> &other) const;
  INLINE bool operator < (const DoubleBitMask<BMType> &other) const;
  INLINE int compare_to(const DoubleBitMask<BMType> &other) const;

  INLINE DoubleBitMask<BMType>
  operator & (const DoubleBitMask<BMType> &other) const;

  INLINE DoubleBitMask<BMType>
  operator | (const DoubleBitMask<BMType> &other) const;

  INLINE DoubleBitMask<BMType>
  operator ^ (const DoubleBitMask<BMType> &other) const;

  INLINE DoubleBitMask<BMType>
  operator ~ () const;

  INLINE DoubleBitMask<BMType>
  operator << (int shift) const;

  INLINE DoubleBitMask<BMType>
  operator >> (int shift) const;

  INLINE void operator &= (const DoubleBitMask<BMType> &other);
  INLINE void operator |= (const DoubleBitMask<BMType> &other);
  INLINE void operator ^= (const DoubleBitMask<BMType> &other);
  INLINE void operator <<= (int shift);
  INLINE void operator >>= (int shift);

public:
  INLINE void generate_hash(ChecksumHashGenerator &hashgen) const;

private:
  BitMaskType _lo, _hi;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();

private:
  static TypeHandle _type_handle;
};

#include "doubleBitMask.I"

template<class BMType>
INLINE std::ostream &operator << (std::ostream &out, const DoubleBitMask<BMType> &doubleBitMask) {
  doubleBitMask.output(out);
  return out;
}

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_PUTIL, EXPTP_PANDA_PUTIL, DoubleBitMask<BitMaskNative>);
typedef DoubleBitMask<BitMaskNative> DoubleBitMaskNative;

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_PUTIL, EXPTP_PANDA_PUTIL, DoubleBitMask<DoubleBitMaskNative>);
typedef DoubleBitMask<DoubleBitMaskNative> QuadBitMaskNative;

#endif
