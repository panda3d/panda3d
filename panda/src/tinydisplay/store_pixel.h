// Filename: store_pixel.h
// Created by:  drose (12May08)
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

  r = STORE_PIX_CLAMP(((unsigned int)r * OP_A(fr, r) >> 16) + ((unsigned int)fr * OP_B(fr, r) >> 16));
  g = STORE_PIX_CLAMP(((unsigned int)g * OP_A(fg, g) >> 16) + ((unsigned int)fg * OP_B(fg, g) >> 16));
  b = STORE_PIX_CLAMP(((unsigned int)b * OP_A(fb, b) >> 16) + ((unsigned int)fb * OP_B(fb, b) >> 16));
  a = STORE_PIX_CLAMP(((unsigned int)a * OP_A(fa, a) >> 16) + ((unsigned int)fa * OP_B(fa, a) >> 16));
  result = RGBA_TO_PIXEL(r, g, b, a);
}


#undef FNAME  
#undef OP_A
#undef OP_B
