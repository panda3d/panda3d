// Filename: convert_srgb.h
// Created by:  rdb (13Nov14)
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

#ifndef CONVERT_SRGB_H
#define CONVERT_SRGB_H

#include "pandabase.h"
#include "luse.h"
#include "pnmimage_base.h"

// The below functions can encode and decode sRGB colors in various
// representations.  Some of them are implemented using look-up tables,
// some others using SSE2 intrinsics.
extern EXPCL_PANDA_PNMIMAGE ALIGN_64BYTE const unsigned char to_srgb8_table[256];
extern EXPCL_PANDA_PNMIMAGE ALIGN_64BYTE const unsigned char to_linear_uchar_table[256];
extern EXPCL_PANDA_PNMIMAGE ALIGN_64BYTE const float to_linear_float_table[256];

EXPCL_PANDA_PNMIMAGE CONSTEXPR float decode_sRGB_float(unsigned char val);
EXPCL_PANDA_PNMIMAGE INLINE float decode_sRGB_float(float val);
EXPCL_PANDA_PNMIMAGE CONSTEXPR unsigned char decode_sRGB_uchar(unsigned char val);
EXPCL_PANDA_PNMIMAGE INLINE unsigned char decode_sRGB_uchar(float val);

EXPCL_PANDA_PNMIMAGE INLINE float encode_sRGB_float(unsigned char val);
EXPCL_PANDA_PNMIMAGE INLINE float encode_sRGB_float(float val);
EXPCL_PANDA_PNMIMAGE CONSTEXPR unsigned char encode_sRGB_uchar(unsigned char val);
EXPCL_PANDA_PNMIMAGE INLINE unsigned char encode_sRGB_uchar(float val);

// These functions convert more than one component in one go,
// which can be faster due to vectorization.
EXPCL_PANDA_PNMIMAGE INLINE void encode_sRGB_uchar(const LColorf &from,
                                                   xel &into);
EXPCL_PANDA_PNMIMAGE INLINE void encode_sRGB_uchar(const LColorf &from,
                                                   xel &into, xelval &into_alpha);

// Use these functions if you know that SSE2 support is available.
// Otherwise, they will crash!
EXPCL_PANDA_PNMIMAGE unsigned char encode_sRGB_uchar_sse2(float val);
EXPCL_PANDA_PNMIMAGE void encode_sRGB_uchar_sse2(const LColorf &from,
                                                 xel &into);
EXPCL_PANDA_PNMIMAGE void encode_sRGB_uchar_sse2(const LColorf &from,
                                                 xel &into, xelval &into_alpha);

// Use the following to find out if you can call either of the above.
EXPCL_PANDA_PNMIMAGE bool has_sse2_sRGB_encode();

#include "convert_srgb.I"

#endif
