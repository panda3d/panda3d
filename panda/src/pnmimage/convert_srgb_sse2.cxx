// Filename: convert_srgb_sse2.cxx
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

// This file should always be compiled with SSE2 support.  These
// functions will only be called when SSE2 support is detected at
// run-time.

#include "convert_srgb.h"
#include "luse.h"

#if defined(__SSE2__) || (_M_IX86_FP >= 2) || defined(_M_X64) || defined(_M_AMD64)

#include <xmmintrin.h>
#include <emmintrin.h>

static INLINE __m128i _encode_sRGB_sse2_mul255(__m128 val) {
  // This an SSE2-based approximation of the sRGB encode function.
  // It has a maximum error of around 0.001, which is by far small
  // enough for a uchar.  It is also at least 10x as fast as the
  // original; up to 40x when taking advantage of vectorization.
  // Note that the fourth float is only multiplied with 255.

  // Part of the code in this function is derived from:
  // http://stackoverflow.com/a/6486630/2135754

  // Clamp to 0-1 range.
  val = _mm_max_ps(val, _mm_set1_ps(0.0f));
  val = _mm_min_ps(val, _mm_set1_ps(1.0f));

  // Pre-multiply with constant factor to adjust for exp bias.
  __m128 xf = _mm_mul_ps(val, _mm_set1_ps(6.3307e18f));

  // Approximate logarithm by... casting!
  xf = _mm_cvtepi32_ps(_mm_castps_si128(xf));

  // Multiply 'logarithm' by power.
  xf = _mm_mul_ps(xf, _mm_set1_ps(2.0f / 3.0f));

  // Reverse operation of above: cast the other way.
  xf = _mm_castsi128_ps(_mm_cvtps_epi32(xf));

  // Make an overestimate and an underestimate.
  __m128 xover = _mm_mul_ps(val, xf);
  __m128 xunder = _mm_mul_ps(_mm_mul_ps(val, val),
                             _mm_rsqrt_ps(xf));

  // Average the two factors, with a slight bias.
  __m128 xavg = _mm_mul_ps(_mm_add_ps(xover, xunder),
                           _mm_set1_ps(0.5286098f));

  // Take square root twice.  Note that this is faster than
  // the more expensive _mm_sqrt_ps instruction.
  xavg = _mm_mul_ps(xavg, _mm_rsqrt_ps(xavg));
  xavg = _mm_mul_ps(xavg, _mm_rsqrt_ps(xavg));

  // Bring it into the correct range.  These factors are determined
  // not on the basis of accuracy, but are chosen such that the
  // decoder lookup table produces an equivalent result for any value.
  xavg = _mm_mul_ps(xavg, _mm_set1_ps(269.122f));
  xavg = _mm_sub_ps(xavg, _mm_set1_ps(13.55f));

  // Compute the linear section.  This is also the path that
  // the alpha channel takes, so we set the alpha multiplier
  // to 255 (since alpha is not sRGB-converted).
  __m128 lval = _mm_mul_ps(val,
    _mm_set_ps(255.0f, 3294.6f, 3294.6f, 3294.6f));

  lval = _mm_add_ps(lval, _mm_set1_ps(0.5f));

  // Decide which version to return.  Rig the alpha
  // comparator to always fail so that the linear path
  // is always chosen for alpha.
  __m128 mask = _mm_cmpge_ps(val,
    _mm_set_ps(2.0f, 0.0031308f, 0.0031308f, 0.0031308f));

  // This is a non-branching way to return one or the other value.
  return _mm_cvttps_epi32(_mm_or_ps(
    _mm_and_ps(mask, xavg),
    _mm_andnot_ps(mask, lval)));
}

unsigned char
encode_sRGB_uchar_sse2(float val) {
  // Running only a single component through this function is still
  // way faster than the equivalent non-SSE2 version.
  return (unsigned char)
    _mm_extract_epi16(_encode_sRGB_sse2_mul255(_mm_set1_ps(val)), 0);
}

void
encode_sRGB_uchar_sse2(const LColorf &color, xel &into) {
#ifdef LINMATH_ALIGN
  __m128 vec = _mm_load_ps(color.get_data());
#else
  __m128 vec = _mm_loadu_ps(color.get_data());
#endif

  __m128i vals = _encode_sRGB_sse2_mul255(vec);
  into.r = _mm_extract_epi16(vals, 0);
  into.g = _mm_extract_epi16(vals, 2);
  into.b = _mm_extract_epi16(vals, 4);
}

void
encode_sRGB_uchar_sse2(const LColorf &color, xel &into, xelval &into_alpha) {
#ifdef LINMATH_ALIGN
  __m128 vec = _mm_load_ps(color.get_data());
#else
  __m128 vec = _mm_loadu_ps(color.get_data());
#endif

  __m128i vals = _encode_sRGB_sse2_mul255(vec);
  into.r = _mm_extract_epi16(vals, 0);
  into.g = _mm_extract_epi16(vals, 2);
  into.b = _mm_extract_epi16(vals, 4);
  into_alpha = _mm_extract_epi16(vals, 6);
}

#elif defined(__i386__) || defined(_M_IX86)
// Somehow we're still compiling this without SSE2 support, even though the
// target architecture could (in theory) support SSE2.  We still have to
// define these functions, but emit a warning that the build system isn't
// configured properly.
#warning convert_srgb_sse2.cxx is being compiled without SSE2 support!

unsigned char
encode_sRGB_uchar_sse2(float val) {
  return encode_sRGB_uchar(val);
}

void
encode_sRGB_uchar_sse2(const LColorf &color, xel &into) {
  encode_sRGB_uchar(color, into);
}

void
encode_sRGB_uchar_sse2(const LColorf &color, xel &into, xelval &into_alpha) {
  encode_sRGB_uchar(color, into, into_alpha);
}

#endif
