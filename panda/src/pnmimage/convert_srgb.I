/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file convert_srgb.I
 * @author rdb
 * @date 2014-10-29
 */

/**
 * Decodes the sRGB-encoded unsigned char value to a linearized float in the
 * range 0-1.
 */
INLINE float decode_sRGB_float(unsigned char val) {
  return to_linear_float_table[val];
}

/**
 * Decodes the sRGB-encoded floating-point value in the range 0-1 to a
 * linearized float in the range 0-1.  Inputs outside this range produce
 * invalid results.
 */
INLINE float decode_sRGB_float(float val) {
  return (val <= 0.04045f)
    ? (val * (1.f / 12.92f))
    : cpow((val + 0.055f) * (1.f / 1.055f), 2.4f);
}

/**
 * Decodes the sRGB-encoded unsigned char value to a linearized unsigned char
 * value.
 */
INLINE unsigned char decode_sRGB_uchar(unsigned char val) {
  return to_linear_uchar_table[val];
}

/**
 * Decodes the sRGB-encoded floating-point value in the range 0-1 to a
 * linearized unsigned char value.  Inputs outside this range are clamped.
 */
INLINE unsigned char decode_sRGB_uchar(float val) {
  return (val <= 0.04045f)
    ? (unsigned char)(std::max(0.f, val) * (255.f / 12.92f) + 0.5f)
    : (unsigned char)(cpow((std::min(val, 1.f) + 0.055f) * (1.f / 1.055f), 2.4f) * 255.f + 0.5f);
}

/**
 * Encodes the linearized unsigned char value to an sRGB-encoded floating-
 * point value in ther range 0-1.
 */
INLINE float
encode_sRGB_float(unsigned char val) {
  // This seems like a very unlikely use case, so I didn't bother making a
  // look-up table for this.
  return (val == 0) ? 0
    : (1.055f * cpow((float)val * (1.f / 255.f), 0.41666f) - 0.055f);
}

/**
 * Encodes the linearized floating-point value in the range 0-1 to an sRGB-
 * encoded float in the range 0-1.  Inputs outside this range produce invalid
 * results.
 */
INLINE float
encode_sRGB_float(float val) {
  return (val < 0.0031308f)
    ? (val * 12.92f)
    : (1.055f * cpow(val, 0.41666f) - 0.055f);
}

/**
 * Encodes the linearized unsigned char value to an sRGB-encoded unsigned char
 * value.
 */
INLINE unsigned char
encode_sRGB_uchar(unsigned char val) {
  return to_srgb8_table[val];
}

/**
 * Encodes the linearized floating-point value in the range 0-1 to an sRGB-
 * encoded unsigned char value.  Inputs outside this range are clamped.
 *
 * When SSE2 support is known at compile time, this automatically uses an
 * optimized version.  Otherwise, it does not attempt runtime CPU detection.
 * If you know that SSE2 is supported (ie.  if the function
 * has_sse2_sRGB_encode() returns true) you should call encode_sRGB_uchar_sse2
 * instead.
 */
INLINE unsigned char
encode_sRGB_uchar(float val) {
#if defined(__SSE2__) || (_M_IX86_FP >= 2) || defined(_M_X64) || defined(_M_AMD64)
  // Use a highly optimized approximation that has more than enough accuracy
  // for an unsigned char.
  return encode_sRGB_uchar_sse2(val);
#else
  return (val < 0.0031308f)
    ? (unsigned char) (std::max(0.f, val) * 3294.6f + 0.5f)
    : (unsigned char) (269.025f * cpow(std::min(val, 1.f), 0.41666f) - 13.525f);
#endif
}

/**
 * Encodes the linearized floating-point color value an sRGB-encoded xel in
 * the range 0-255.
 *
 * When SSE2 support is known at compile time, this automatically uses an
 * optimized version.  Otherwise, it does not attempt runtime CPU detection.
 * If you know that SSE2 is supported (ie.  if the function
 * has_sse2_sRGB_encode() returns true) you should call encode_sRGB_uchar_sse2
 * instead.
 */
INLINE void
encode_sRGB_uchar(const LColorf &color, xel &into) {
#if defined(__SSE2__) || (_M_IX86_FP >= 2) || defined(_M_X64) || defined(_M_AMD64)
  // SSE2 support compiled-in; we're guaranteed to have it.
  encode_sRGB_uchar_sse2(color, into);
#else
  // Boring, slow, non-SSE2 version.
  PPM_ASSIGN(into,
    encode_sRGB_uchar(color[0]),
    encode_sRGB_uchar(color[1]),
    encode_sRGB_uchar(color[2]));
#endif
}

/**
 * Encodes the linearized floating-point color value an sRGB-encoded xel and
 * alpha in the range 0-255. The alpha value is not sRGB-encoded.
 *
 * When SSE2 support is known at compile time, this automatically uses an
 * optimized version.  Otherwise, it does not attempt runtime CPU detection.
 * If you know that SSE2 is supported (ie.  if the function
 * has_sse2_sRGB_encode() returns true) you should call encode_sRGB_uchar_sse2
 * instead.
 */
INLINE void
encode_sRGB_uchar(const LColorf &color, xel &into, xelval &into_alpha) {
#if defined(__SSE2__) || (_M_IX86_FP >= 2) || defined(_M_X64) || defined(_M_AMD64)
  // SSE2 support compiled-in; we're guaranteed to have it.
  encode_sRGB_uchar_sse2(color, into, into_alpha);
#else
  // Boring, slow, non-SSE2 version.
  PPM_ASSIGN(into,
    encode_sRGB_uchar(color[0]),
    encode_sRGB_uchar(color[1]),
    encode_sRGB_uchar(color[2]));

  into_alpha = (xelval) (color[3] * 255.f + 0.5f);
#endif
}


/**
 * Double-precision versions of the above.
 */
INLINE void
encode_sRGB_uchar(const LColord &color, xel &into) {
  return encode_sRGB_uchar(LCAST(float, color), into);
}

INLINE void
encode_sRGB_uchar(const LColord &color, xel &into, xelval &into_alpha) {
  return encode_sRGB_uchar(LCAST(float, color), into, into_alpha);
}
