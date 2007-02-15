// Filename: sparseArray.h
// Created by:  drose (14Feb07)
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

#ifndef SPARSEARRAY_H
#define SPARSEARRAY_H

#include "pandabase.h"
#include "ordered_vector.h"

class BitArray;

////////////////////////////////////////////////////////////////////
//       Class : SparseArray
// Description : This class records a set of integers, where each
//               integer is either present or not present in the set.
//
//               It is similar in principle and in interface to a
//               BitArray (which can be thought of as a set of
//               integers, one integer corresponding to each different
//               bit position), but the SparseArray is implemented as
//               a list of min/max subrange lists, rather than as a
//               bitmask.  
//
//               This makes it particularly efficient for storing sets
//               which consist of large sections of consecutively
//               included or consecutively excluded elements, with
//               arbitrarily large integers, but particularly
//               inefficient for doing boolean operations such as & or
//               |.
//
//               Also, unlike BitArray, the SparseArray can store
//               negative integers.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA SparseArray {
PUBLISHED:
  INLINE SparseArray();
  INLINE SparseArray(const SparseArray &copy);
  INLINE SparseArray &operator = (const SparseArray &copy);
  SparseArray(const BitArray &from);

  INLINE static SparseArray all_on();
  INLINE static SparseArray all_off();
  INLINE static SparseArray lower_on(int on_bits);
  INLINE static SparseArray bit(int index);
  INLINE static SparseArray range(int low_bit, int size);

  INLINE ~SparseArray();

  INLINE static bool has_max_num_bits();
  INLINE static int get_max_num_bits();

  INLINE int get_num_bits() const;
  INLINE bool get_bit(int index) const;
  INLINE void set_bit(int index);
  INLINE void clear_bit(int index);
  INLINE void set_bit_to(int index, bool value);
  INLINE bool get_highest_bits() const;
  INLINE bool is_zero() const;
  INLINE bool is_all_on() const;

  INLINE bool has_any_of(int low_bit, int size) const;
  INLINE bool has_all_of(int low_bit, int size) const;
  INLINE void set_range(int low_bit, int size);
  INLINE void clear_range(int low_bit, int size);
  INLINE void set_range_to(bool value, int low_bit, int size);

  INLINE void invert_in_place();
  bool has_bits_in_common(const SparseArray &other) const;
  INLINE void clear();

  void output(ostream &out) const;

  INLINE bool operator == (const SparseArray &other) const;
  INLINE bool operator != (const SparseArray &other) const;
  INLINE bool operator < (const SparseArray &other) const;
  int compare_to(const SparseArray &other) const;

  INLINE SparseArray
  operator & (const SparseArray &other) const;

  INLINE SparseArray
  operator | (const SparseArray &other) const;

  INLINE SparseArray
  operator ^ (const SparseArray &other) const;

  INLINE SparseArray
  operator ~ () const;

  INLINE SparseArray
  operator << (int shift) const;

  INLINE SparseArray
  operator >> (int shift) const;

  void operator &= (const SparseArray &other);
  void operator |= (const SparseArray &other);
  void operator ^= (const SparseArray &other);
  INLINE void operator <<= (int shift);
  INLINE void operator >>= (int shift);

  INLINE bool is_inverse() const;
  INLINE int get_num_subranges() const;
  INLINE int get_subrange_begin(int n) const;
  INLINE int get_subrange_end(int n) const;

private:
  void do_add_range(int begin, int end);
  void do_remove_range(int begin, int end);
  bool do_has_any(int begin, int end) const;
  bool do_has_all(int begin, int end) const;

  void do_intersection(const SparseArray &other);
  void do_union(const SparseArray &other);
  void do_intersection_neg(const SparseArray &other);
  void do_shift(int offset);

  // The SparseArray is implemented as a set of non-overlapping
  // Subranges.
  class Subrange {
  public:
    INLINE Subrange(int begin, int end);
    INLINE bool operator < (const Subrange &other) const;

    int _begin, _end;
  };

  typedef ov_set<Subrange> Subranges;
  Subranges _subranges;
  bool _inverse;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    register_type(_type_handle, "SparseArray");
  }

private:
  static TypeHandle _type_handle;
};

#include "sparseArray.I"

INLINE ostream &
operator << (ostream &out, const SparseArray &array) {
  array.output(out);
  return out;
}

#endif

