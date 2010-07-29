// Filename: pbitops.I
// Created by:  drose (10May08)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: count_bits_in_word
//  Description: Returns the number of 1 bits in the indicated word.
////////////////////////////////////////////////////////////////////
INLINE int
count_bits_in_word(PN_uint16 x) {
  return (int)num_bits_on[x];
}

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
INLINE PN_uint16
flood_bits_down(PN_uint16 x) {
  x |= (x >> 1);
  x |= (x >> 2);
  x |= (x >> 4);
  x |= (x >> 8);
  return x;
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
INLINE PN_uint16
flood_bits_up(PN_uint16 x) {
  x |= (x << 1);
  x |= (x << 2);
  x |= (x << 4);
  x |= (x << 8);
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
//     Function: get_lowest_on_bit
//  Description: Returns the index of the lowest 1 bit in the word.
//               Returns -1 if there are no 1 bits.
////////////////////////////////////////////////////////////////////
INLINE int
get_lowest_on_bit(PN_uint16 x) {
  if (x == 0) {
    return -1;
  }

  PN_uint16 w = (x & (~x + 1));
  return (int)num_bits_on[w - 1];
}

////////////////////////////////////////////////////////////////////
//     Function: get_lowest_on_bit
//  Description: Returns the index of the lowest 1 bit in the word.
//               Returns -1 if there are no 1 bits.
////////////////////////////////////////////////////////////////////
INLINE int
get_lowest_on_bit(PN_uint32 x) {
  if (x == 0) {
    return -1;
  }

  PN_uint32 w = (x & (~x + 1));
  return count_bits_in_word(w - 1);
}

////////////////////////////////////////////////////////////////////
//     Function: get_lowest_on_bit
//  Description: Returns the index of the lowest 1 bit in the word.
//               Returns -1 if there are no 1 bits.
////////////////////////////////////////////////////////////////////
INLINE int
get_lowest_on_bit(PN_uint64 x) {
  if (x == 0) {
    return -1;
  }

  PN_uint64 w = (x & (~x + 1));
  return count_bits_in_word(w - 1);
}

////////////////////////////////////////////////////////////////////
//     Function: get_highest_on_bit
//  Description: Returns the index of the highest 1 bit in the word.
//               Returns -1 if there are no 1 bits.
////////////////////////////////////////////////////////////////////
INLINE int
get_highest_on_bit(PN_uint16 x) {
  PN_uint16 w = flood_bits_down(x);
  return count_bits_in_word(w) - 1;
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
get_next_higher_bit(PN_uint16 x) {
  PN_uint16 w = flood_bits_down(x);
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
