/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pbitops.h
 * @author drose
 * @date 2008-05-10
 */

#ifndef PBITOPS_H
#define PBITOPS_H

#include "pandabase.h"
#include "numeric_types.h"

#if defined(_MSC_VER) && !defined(CPPPARSER)
#include <intrin.h>
#endif

// This file defines a few low-level bit-operation routines, optimized all to
// heck.

INLINE int count_bits_in_word(unsigned short x);
INLINE int count_bits_in_word(unsigned int x);
INLINE int count_bits_in_word(unsigned long x);
INLINE int count_bits_in_word(unsigned long long x);

INLINE unsigned short flood_bits_down(unsigned short x);
INLINE unsigned int flood_bits_down(unsigned int x);
INLINE unsigned long flood_bits_down(unsigned long x);
INLINE unsigned long long flood_bits_down(unsigned long long x);
INLINE unsigned short flood_bits_up(unsigned short x);
INLINE unsigned int flood_bits_up(unsigned int x);
INLINE unsigned long flood_bits_up(unsigned long x);
INLINE unsigned long long flood_bits_up(unsigned long long x);

INLINE int get_lowest_on_bit(unsigned short x);
INLINE int get_lowest_on_bit(unsigned int x);
INLINE int get_lowest_on_bit(unsigned long x);
INLINE int get_lowest_on_bit(unsigned long long x);
INLINE int get_highest_on_bit(unsigned short x);
INLINE int get_highest_on_bit(unsigned int x);
INLINE int get_highest_on_bit(unsigned long x);
INLINE int get_highest_on_bit(unsigned long long x);

INLINE int get_next_higher_bit(unsigned short x);
INLINE int get_next_higher_bit(unsigned int x);
INLINE int get_next_higher_bit(unsigned long x);
INLINE int get_next_higher_bit(unsigned long long x);

// This table precomputes the number of on bits in each 16-bit word.
extern EXPCL_PANDA_PUTIL const unsigned char num_bits_on[65536];

#include "pbitops.I"

#endif
