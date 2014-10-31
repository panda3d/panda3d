// Filename: pbitops.h
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

#ifndef PBITOPS_H
#define PBITOPS_H

#include "pandabase.h"
#include "numeric_types.h"

#ifdef _MSC_VER
#include <intrin.h>
#endif

////////////////////////////////////////////////////////////////////
// This file defines a few low-level bit-operation routines, optimized
// all to heck.
////////////////////////////////////////////////////////////////////

INLINE int count_bits_in_word(PN_uint16 x);
INLINE int count_bits_in_word(PN_uint32 x);
INLINE int count_bits_in_word(PN_uint64 x);

INLINE PN_uint16 flood_bits_down(PN_uint16 x);
INLINE PN_uint32 flood_bits_down(PN_uint32 x);
INLINE PN_uint64 flood_bits_down(PN_uint64 x);
INLINE PN_uint16 flood_bits_up(PN_uint16 x);
INLINE PN_uint32 flood_bits_up(PN_uint32 x);
INLINE PN_uint64 flood_bits_up(PN_uint64 x);

INLINE int get_lowest_on_bit(PN_uint16 x);
INLINE int get_lowest_on_bit(PN_uint32 x);
INLINE int get_lowest_on_bit(PN_uint64 x);
INLINE int get_highest_on_bit(PN_uint16 x);
INLINE int get_highest_on_bit(PN_uint32 x);
INLINE int get_highest_on_bit(PN_uint64 x);

INLINE int get_next_higher_bit(PN_uint16 x);
INLINE int get_next_higher_bit(PN_uint32 x);
INLINE int get_next_higher_bit(PN_uint64 x);

// This table precomputes the number of on bits in each 16-bit word.
extern EXPCL_PANDA_PUTIL const unsigned char num_bits_on[65536];

#include "pbitops.I"

#endif
