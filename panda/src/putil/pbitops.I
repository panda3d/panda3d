// Filename: pbitops.I
// Created by:  drose (10May08)
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


////////////////////////////////////////////////////////////////////
//     Function: count_bits_in_word
//  Description: Returns the number of 1 bits in the indicated word.
////////////////////////////////////////////////////////////////////
INLINE int
count_bits_in_word(PN_uint32 x) {
  return (int)num_bits_on[x & 0xffff] + (int)num_bits_on[(x >> 16) & 0xffff];
}

////////////////////////////////////////////////////////////////////
//     Function: count_bits_in_word
//  Description: Returns the number of 1 bits in the indicated word.
////////////////////////////////////////////////////////////////////
INLINE int
count_bits_in_word(PN_uint64 x) {
  return count_bits_in_word((PN_uint32)x) + count_bits_in_word((PN_uint32)(x >> 32));
}

////////////////////////////////////////////////////////////////////
//     Function: flood_bits_down
//  Description: Returns a value such that every bit at or below the
//               highest bit in x is 1.
////////////////////////////////////////////////////////////////////
INLINE PN_uint32
flood_bits_down(PN_uint32 x) {
  x |= (x >> 1);
  x |= (x >> 2);
  x |= (x >> 4);
  x |= (x >> 8);
  x |= (x >> 16);
  return x;
}

////////////////////////////////////////////////////////////////////
//     Function: flood_bits_down
//  Description: Returns a value such that every bit at or below the
//               highest bit in x is 1.
////////////////////////////////////////////////////////////////////
INLINE PN_uint64
flood_bits_down(PN_uint64 x) {
  x |= (x >> 1);
  x |= (x >> 2);
  x |= (x >> 4);
  x |= (x >> 8);
  x |= (x >> 16);
  x |= (x >> 32);
  return x;
}

////////////////////////////////////////////////////////////////////
//     Function: flood_bits_up
//  Description: Returns a value such that every bit at or above the
//               highest bit in x is 1.
////////////////////////////////////////////////////////////////////
INLINE PN_uint32
flood_bits_up(PN_uint32 x) {
  x |= (x << 1);
  x |= (x << 2);
  x |= (x << 4);
  x |= (x << 8);
  x |= (x << 16);
  return x;
}

////////////////////////////////////////////////////////////////////
//     Function: flood_bits_up
//  Description: Returns a value such that every bit at or above the
//               highest bit in x is 1.
////////////////////////////////////////////////////////////////////
INLINE PN_uint64
flood_bits_up(PN_uint64 x) {
  x |= (x << 1);
  x |= (x << 2);
  x |= (x << 4);
  x |= (x << 8);
  x |= (x << 16);
  x |= (x << 32);
  return x;
}

////////////////////////////////////////////////////////////////////
//     Function: get_highest_on_bit
//  Description: Returns the index of the highest 1 bit in the word.
//               Returns -1 if there are no 1 bits.
////////////////////////////////////////////////////////////////////
INLINE int
get_highest_on_bit(PN_uint32 x) {
  PN_uint32 w = flood_bits_down(x);
  return count_bits_in_word(w) - 1;
}

////////////////////////////////////////////////////////////////////
//     Function: get_highest_on_bit
//  Description: Returns the index of the highest 1 bit in the word.
//               Returns -1 if there are no 1 bits.
////////////////////////////////////////////////////////////////////
INLINE int
get_highest_on_bit(PN_uint64 x) {
  PN_uint64 w = flood_bits_down(x);
  return count_bits_in_word(w) - 1;
}

////////////////////////////////////////////////////////////////////
//     Function: get_next_higher_bit
//  Description: Returns the smallest power of 2 greater than x.
//
//               Returns the smallest number n such that (1 << n) is
//               larger than x.
////////////////////////////////////////////////////////////////////
INLINE int
get_next_higher_bit(PN_uint32 x) {
  PN_uint32 w = flood_bits_down(x);
  return count_bits_in_word(w);
}

////////////////////////////////////////////////////////////////////
//     Function: get_next_higher_bit
//  Description: Returns the smallest power of 2 greater than x.
//
//               Returns the smallest number n such that (1 << n) is
//               larger than x.
////////////////////////////////////////////////////////////////////
INLINE int
get_next_higher_bit(PN_uint64 x) {
  PN_uint64 w = flood_bits_down(x);
  return count_bits_in_word(w);
}
