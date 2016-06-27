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

INLINE int count_bits_in_word(uint16_t x);
INLINE int count_bits_in_word(uint32_t x);
INLINE int count_bits_in_word(uint64_t x);

INLINE uint16_t flood_bits_down(uint16_t x);
INLINE uint32_t flood_bits_down(uint32_t x);
INLINE uint64_t flood_bits_down(uint64_t x);
INLINE uint16_t flood_bits_up(uint16_t x);
INLINE uint32_t flood_bits_up(uint32_t x);
INLINE uint64_t flood_bits_up(uint64_t x);

INLINE int get_lowest_on_bit(uint16_t x);
INLINE int get_lowest_on_bit(uint32_t x);
INLINE int get_lowest_on_bit(uint64_t x);
INLINE int get_highest_on_bit(uint16_t x);
INLINE int get_highest_on_bit(uint32_t x);
INLINE int get_highest_on_bit(uint64_t x);

INLINE int get_next_higher_bit(uint16_t x);
INLINE int get_next_higher_bit(uint32_t x);
INLINE int get_next_higher_bit(uint64_t x);

// This table precomputes the number of on bits in each 16-bit word.
extern EXPCL_PANDA_PUTIL const unsigned char num_bits_on[65536];

#include "pbitops.I"

#endif
