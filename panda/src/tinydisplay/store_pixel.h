/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file store_pixel.h
 * @author drose
 * @date 2008-05-12
 */

/* Definition of a function to store a pixel in the framebuffer, using
   user-specified color blending. */

/* This file generates lots of "template" variations, using #define
   and #include, similar to the way ztriangle.h works. */

static void
FNAME(store_pixel) (ZBuffer *zb, PIXEL &result, int r, int g, int b, int a) {
  unsigned int fr = PIXEL_R(result);
  unsigned int fg = PIXEL_G(result);
  unsigned int fb = PIXEL_B(result);
  unsigned int fa = PIXEL_A(result);

  r = STORE_PIXEL_0(fr, ((unsigned int)r * OP_A(fr, r) >> 16) + ((unsigned int)fr * OP_B(fr, r) >> 16));
  g = STORE_PIXEL_1(fg, ((unsigned int)g * OP_A(fg, g) >> 16) + ((unsigned int)fg * OP_B(fg, g) >> 16));
  b = STORE_PIXEL_2(fb, ((unsigned int)b * OP_A(fb, b) >> 16) + ((unsigned int)fb * OP_B(fb, b) >> 16));
  a = STORE_PIXEL_3(fa, ((unsigned int)a * OP_A(fa, a) >> 16) + ((unsigned int)fa * OP_B(fa, a) >> 16));
  result = RGBA_TO_PIXEL(r, g, b, a);
}

#ifdef FNAME_S
/* sRGB variant. */

static void
FNAME_S(store_pixel) (ZBuffer *zb, PIXEL &result, int r, int g, int b, int a) {
  unsigned int fr = PIXEL_SR(result);
  unsigned int fg = PIXEL_SG(result);
  unsigned int fb = PIXEL_SB(result);
  unsigned int fa = PIXEL_A(result);

  r = STORE_PIXEL_0(fr, ((unsigned int)r * OP_A(fr, r) >> 16) + ((unsigned int)fr * OP_B(fr, r) >> 16));
  g = STORE_PIXEL_1(fg, ((unsigned int)g * OP_A(fg, g) >> 16) + ((unsigned int)fg * OP_B(fg, g) >> 16));
  b = STORE_PIXEL_2(fb, ((unsigned int)b * OP_A(fb, b) >> 16) + ((unsigned int)fb * OP_B(fb, b) >> 16));
  a = STORE_PIXEL_3(fa, ((unsigned int)a * OP_A(fa, a) >> 16) + ((unsigned int)fa * OP_B(fa, a) >> 16));
  result = SRGBA_TO_PIXEL(r, g, b, a);
}

#undef FNAME_S
#endif

#undef FNAME
#undef OP_A
#undef OP_B
#undef STORE_PIXEL_0
#undef STORE_PIXEL_1
#undef STORE_PIXEL_2
#undef STORE_PIXEL_3
