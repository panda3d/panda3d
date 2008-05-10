// Filename: pbitops.h
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

#ifndef PBITOPS_H
#define PBITOPS_H

#include "pandabase.h"
#include "numeric_types.h"

////////////////////////////////////////////////////////////////////
// This file defines a few low-level bit-operation routines, optimized
// all to heck.
////////////////////////////////////////////////////////////////////

INLINE int count_bits_in_word(PN_uint32 x);
INLINE int count_bits_in_word(PN_uint64 x);

INLINE PN_uint32 flood_bits_down(PN_uint32 x);
INLINE PN_uint64 flood_bits_down(PN_uint64 x);
INLINE PN_uint32 flood_bits_up(PN_uint32 x);
INLINE PN_uint64 flood_bits_up(PN_uint64 x);

INLINE int get_highest_on_bit(PN_uint32 x);
INLINE int get_highest_on_bit(PN_uint64 x);

INLINE int get_next_higher_bit(PN_uint32 x);
INLINE int get_next_higher_bit(PN_uint64 x);

// This table precomputes the number of on bits in each 16-bit word.
extern EXPCL_PANDA_PUTIL unsigned char num_bits_on[65536];

#include "pbitops.I"

#endif
