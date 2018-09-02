/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bitMask.h
 * @author drose
 * @date 2000-06-08
 */

#ifndef BITMASK_H
#define BITMASK_H

#include "pandabase.h"
#include "pbitops.h"
#include "numeric_types.h"
#include "typedObject.h"
#include "indent.h"
#include "pnotify.h"

#include "checksumHashGenerator.h"


/**
 * A general bitmask class.  This stores an array of bits of some length that
 * must fit within a given word of the indicated type.  See also BitArray.
 */
template<class WType, int nbits>
class BitMask {
public:
  typedef WType WordType;

PUBLISHED:
  enum { num_bits = nbits };

  constexpr BitMask() = default;
  ALWAYS_INLINE constexpr BitMask(WordType init_value);

  INLINE static BitMask<WType, nbits> all_on();
  INLINE static BitMask<WType, nbits> all_off();
  INLINE static BitMask<WType, nbits> lower_on(int on_bits);
  INLINE static BitMask<WType, nbits> bit(int index);
  INLINE static BitMask<WType, nbits> range(int low_bit, int size);

  constexpr static bool has_max_num_bits() { return true; }
  constexpr static int get_max_num_bits() { return num_bits; }

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
  INLINE WordType get_word() const;
  INLINE void set_word(WordType value);

  INLINE int get_num_on_bits() const;
  INLINE int get_num_off_bits() const;
  INLINE int get_lowest_on_bit() const;
  INLINE int get_lowest_off_bit() const;
  INLINE int get_highest_on_bit() const;
  INLINE int get_highest_off_bit() const;
  INLINE int get_next_higher_different_bit(int low_bit) const;

  INLINE void invert_in_place();
  INLINE bool has_bits_in_common(const BitMask<WType, nbits> &other) const;
  INLINE void clear();

  void output(std::ostream &out) const;
  void output_binary(std::ostream &out, int spaces_every = 4) const;
  void output_hex(std::ostream &out, int spaces_every = 4) const;
  void write(std::ostream &out, int indent_level = 0) const;

  INLINE bool operator == (const BitMask<WType, nbits> &other) const;
  INLINE bool operator != (const BitMask<WType, nbits> &other) const;
  INLINE bool operator < (const BitMask<WType, nbits> &other) const;
  INLINE int compare_to(const BitMask<WType, nbits> &other) const;

  INLINE BitMask<WType, nbits>
  operator & (const BitMask<WType, nbits> &other) const;

  INLINE BitMask<WType, nbits>
  operator | (const BitMask<WType, nbits> &other) const;

  INLINE BitMask<WType, nbits>
  operator ^ (const BitMask<WType, nbits> &other) const;

  INLINE BitMask<WType, nbits>
  operator ~ () const;

  INLINE BitMask<WType, nbits>
  operator << (int shift) const;

  INLINE BitMask<WType, nbits>
  operator >> (int shift) const;

  INLINE void operator &= (const BitMask<WType, nbits> &other);
  INLINE void operator |= (const BitMask<WType, nbits> &other);
  INLINE void operator ^= (const BitMask<WType, nbits> &other);
  INLINE void operator <<= (int shift);
  INLINE void operator >>= (int shift);

  INLINE void flood_down_in_place();
  INLINE void flood_up_in_place();
  INLINE BitMask<WType, nbits> flood_bits_down() const;
  INLINE BitMask<WType, nbits> flood_bits_up() const;
  INLINE BitMask<WType, nbits> keep_next_highest_bit() const;
  INLINE BitMask<WType, nbits> keep_next_lowest_bit() const;
  INLINE BitMask<WType, nbits> keep_next_highest_bit(int index) const;
  INLINE BitMask<WType, nbits> keep_next_lowest_bit(int index) const;
  INLINE BitMask<WType, nbits> keep_next_highest_bit(const BitMask<WType, nbits> &other) const;
  INLINE BitMask<WType, nbits> keep_next_lowest_bit(const BitMask<WType, nbits> &other) const;

  INLINE int get_key() const;

  INLINE bool __nonzero__() const;

public:
  INLINE void generate_hash(ChecksumHashGenerator &hashgen) const;

private:
  WordType _word = 0u;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type(const std::string &name);

private:
  static TypeHandle _type_handle;
};

#include "bitMask.I"

template<class WType, int nbits>
INLINE std::ostream &operator << (std::ostream &out, const BitMask<WType, nbits> &bitmask) {
  bitmask.output(out);
  return out;
}

// We need to define this temporary macro so we can pass a parameter
// containing a comma through the macro.
#define BITMASK16_DEF BitMask<uint16_t, 16>
#define BITMASK32_DEF BitMask<uint32_t, 32>
#define BITMASK64_DEF BitMask<uint64_t, 64>
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_PUTIL, EXPTP_PANDA_PUTIL, BITMASK16_DEF);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_PUTIL, EXPTP_PANDA_PUTIL, BITMASK32_DEF);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_PUTIL, EXPTP_PANDA_PUTIL, BITMASK64_DEF);

typedef BitMask<uint16_t, 16> BitMask16;
typedef BitMask<uint32_t, 32> BitMask32;
typedef BitMask<uint64_t, 64> BitMask64;

#if NATIVE_WORDSIZE == 32
typedef BitMask32 BitMaskNative;
#elif NATIVE_WORDSIZE == 64
typedef BitMask64 BitMaskNative;
#else
#error No definition for NATIVE_WORDSIZE--should be defined in dtoolbase.h.
#endif  // NATIVE_WORDSIZE

#endif
