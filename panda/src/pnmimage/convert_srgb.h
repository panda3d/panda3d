/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file convert_srgb.h
 * @author rdb
 * @date 2014-11-13
 */

#ifndef CONVERT_SRGB_H
#define CONVERT_SRGB_H

#include "pandabase.h"
#include "luse.h"
#include "pnmimage_base.h"

// The below functions can encode and decode sRGB colors in various
// representations.  Some of them are implemented using look-up tables, some
// others using SSE2 intrinsics.
extern EXPCL_PANDA_PNMIMAGE const unsigned char to_srgb8_table[256];
extern EXPCL_PANDA_PNMIMAGE const unsigned char to_linear_uchar_table[256];
extern EXPCL_PANDA_PNMIMAGE const float to_linear_float_table[256];

BEGIN_PUBLISH

INLINE float decode_sRGB_float(unsigned char val);
INLINE float decode_sRGB_float(float val);
INLINE unsigned char decode_sRGB_uchar(unsigned char val);
INLINE unsigned char decode_sRGB_uchar(float val);

INLINE float encode_sRGB_float(unsigned char val);
INLINE float encode_sRGB_float(float val);
INLINE unsigned char encode_sRGB_uchar(unsigned char val);
INLINE unsigned char encode_sRGB_uchar(float val);

END_PUBLISH

// These functions convert more than one component in one go, which can be
// faster due to vectorization.
INLINE void encode_sRGB_uchar(const LColorf &from, xel &into);
INLINE void encode_sRGB_uchar(const LColorf &from, xel &into, xelval &into_alpha);

INLINE void encode_sRGB_uchar(const LColord &from, xel &into);
INLINE void encode_sRGB_uchar(const LColord &from, xel &into, xelval &into_alpha);

// Use these functions if you know that SSE2 support is available.  Otherwise,
// they will crash!
#if defined(__SSE2__) || defined(__i386__) || defined(_M_IX86) || defined(_M_X64) || defined(_M_AMD64)
EXPCL_PANDA_PNMIMAGE unsigned char encode_sRGB_uchar_sse2(float val);
EXPCL_PANDA_PNMIMAGE void encode_sRGB_uchar_sse2(const LColorf &from,
                                                 xel &into);
EXPCL_PANDA_PNMIMAGE void encode_sRGB_uchar_sse2(const LColorf &from,
                                                 xel &into, xelval &into_alpha);

// Use the following to find out if you can call either of the above.
EXPCL_PANDA_PNMIMAGE bool has_sse2_sRGB_encode();
#else
// The target architecture can't support the SSE2 extension at all.
#define encode_sRGB_uchar_sse2 encode_sRGB_uchar
#define has_sse2_sRGB_encode() (false)
#endif

#include "convert_srgb.I"

#endif
