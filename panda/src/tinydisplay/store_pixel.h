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

#if HAVE_R
  unsigned int r_ops[] = {0, (unsigned int)r, fr, (unsigned int)a, fa};
  unsigned int r_opa = r_ops[zb->blenda.op_rgb] ^ zb->blenda.xor_r;
  unsigned int r_opb = r_ops[zb->blendb.op_rgb] ^ zb->blendb.xor_r;
  (void)r_opa;
  (void)r_opb;
  r = MODE_RGB(r, r_opa, fr, r_opb);
#else
  r = fr;
#endif

#if HAVE_G
  unsigned int g_ops[] = {0, (unsigned int)g, fg, (unsigned int)a, fa};
  unsigned int g_opa = g_ops[zb->blenda.op_rgb] ^ zb->blenda.xor_g;
  unsigned int g_opb = g_ops[zb->blendb.op_rgb] ^ zb->blendb.xor_g;
  (void)g_opa;
  (void)g_opb;
  g = MODE_RGB(g, g_opa, fg, g_opb);
#else
  g = fg;
#endif

#if HAVE_B
  unsigned int b_ops[] = {0, (unsigned int)b, fb, (unsigned int)a, fa};
  unsigned int b_opa = b_ops[zb->blenda.op_rgb] ^ zb->blenda.xor_b;
  unsigned int b_opb = b_ops[zb->blendb.op_rgb] ^ zb->blendb.xor_b;
  (void)b_opa;
  (void)b_opb;
  b = MODE_RGB(b, b_opa, fb, b_opb);
#else
  b = fb;
#endif

#if HAVE_A
  unsigned int a_ops[] = {0, (unsigned int)a, fa, (unsigned int)a, fa};
  unsigned int a_opa = a_ops[zb->blenda.op_alpha] ^ zb->blenda.xor_a;
  unsigned int a_opb = a_ops[zb->blendb.op_alpha] ^ zb->blendb.xor_a;
  (void)a_opa;
  (void)a_opb;
  a = MODE_ALPHA(a, a_opa, fa, a_opb);
#else
  a = fa;
#endif

  result = RGBA_TO_PIXEL(r, g, b, a);
}

#ifdef FNAME_S
/* sRGB variant. */

static void
FNAME_S(store_pixel) (ZBuffer *zb, PIXEL &result, int r, int g, int b, int a) {
#if HAVE_R || HAVE_G || HAVE_B || HAVE_A
  unsigned int fr = PIXEL_SR(result);
  unsigned int fg = PIXEL_SG(result);
  unsigned int fb = PIXEL_SB(result);
  unsigned int fa = PIXEL_A(result);

#if HAVE_R
  unsigned int r_ops[] = {0, (unsigned int)r, fr, (unsigned int)a, fa};
  unsigned int r_opa = r_ops[zb->blenda.op_rgb] ^ zb->blenda.xor_r;
  unsigned int r_opb = r_ops[zb->blendb.op_rgb] ^ zb->blendb.xor_r;
  (void)r_opa;
  (void)r_opb;
  r = MODE_RGB(r, r_opa, fr, r_opb);
#else
  r = fr;
#endif

#if HAVE_G
  unsigned int g_ops[] = {0, (unsigned int)g, fg, (unsigned int)a, fa};
  unsigned int g_opa = g_ops[zb->blenda.op_rgb] ^ zb->blenda.xor_g;
  unsigned int g_opb = g_ops[zb->blendb.op_rgb] ^ zb->blendb.xor_g;
  (void)g_opa;
  (void)g_opb;
  g = MODE_RGB(g, g_opa, fg, g_opb);
#else
  g = fg;
#endif

#if HAVE_B
  unsigned int b_ops[] = {0, (unsigned int)b, fb, (unsigned int)a, fa};
  unsigned int b_opa = b_ops[zb->blenda.op_rgb] ^ zb->blenda.xor_b;
  unsigned int b_opb = b_ops[zb->blendb.op_rgb] ^ zb->blendb.xor_b;
  (void)b_opa;
  (void)b_opb;
  b = MODE_RGB(b, b_opa, fb, b_opb);
#else
  b = fb;
#endif

#if HAVE_A
  unsigned int a_ops[] = {0, (unsigned int)a, fa, (unsigned int)a, fa};
  unsigned int a_opa = a_ops[zb->blenda.op_alpha] ^ zb->blenda.xor_a;
  unsigned int a_opb = a_ops[zb->blendb.op_alpha] ^ zb->blendb.xor_a;
  (void)a_opa;
  (void)a_opb;
  a = MODE_ALPHA(a, a_opa, fa, a_opb);
#else
  a = fa;
#endif

  result = SRGBA_TO_PIXEL(r, g, b, a);
#endif
}

#undef FNAME_S
#endif

#undef FNAME
#undef MODE_RGB
#undef MODE_ALPHA
#undef HAVE_R
#undef HAVE_G
#undef HAVE_B
#undef HAVE_A
