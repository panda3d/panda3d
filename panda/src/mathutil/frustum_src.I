/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file frustum_src.I
 * @author mike
 * @date 1997-01-09
 */

/**
 *
 */
INLINE_MATHUTIL FLOATNAME(LFrustum)::
FLOATNAME(LFrustum)() {
  _fnear = FLOATCONST(1.4142);
  _ffar = FLOATCONST(10.0);
  _l = -1.0f;
  _r = 1.0f;
  _t = 1.0f;
  _b = -1.0f;
}

/**
 * Sets up a two-dimensional orthographic frustum
 */
INLINE_MATHUTIL void FLOATNAME(LFrustum)::make_ortho_2D() {
  make_ortho(-1.0f, 1.0f);
}

/**
 * Sets up a two-dimensional orthographic frustum
 */
INLINE_MATHUTIL void FLOATNAME(LFrustum)::
make_ortho_2D(FLOATTYPE l, FLOATTYPE r, FLOATTYPE t, FLOATTYPE b) {
  make_ortho(-1.0f, 1.0f, l, r, t, b);
}

/**
 * Behaves like gluOrtho
 */
INLINE_MATHUTIL void FLOATNAME(LFrustum)::make_ortho(FLOATTYPE fnear, FLOATTYPE ffar) {
  _fnear = fnear;
  _ffar = ffar;
  _l = -1.0f;
  _r = 1.0f;
  _t = 1.0f;
  _b = -1.0f;
}

/**
 * Behaves like gluOrtho
 */
INLINE_MATHUTIL void FLOATNAME(LFrustum)::
make_ortho(FLOATTYPE fnear, FLOATTYPE ffar, FLOATTYPE l, FLOATTYPE r,
           FLOATTYPE t, FLOATTYPE b) {
  _fnear = fnear;
  _ffar = ffar;
  _l = l;
  _r = r;
  _t = t;
  _b = b;
}

/**
 * Behaves like gluPerspective (Aspect = width/height, Yfov in degrees) aspect
 * +------------+ |            | 1 |            | yfov |            |
 * +------------+
 *
 * -------+------ \     | \    | \   | \  | \ | \| W yfov
 *
 */
INLINE_MATHUTIL void FLOATNAME(LFrustum)::
make_perspective_hfov(FLOATTYPE hfov, FLOATTYPE aspect, FLOATTYPE fnear,
                      FLOATTYPE ffar) {
  _fnear = fnear;
  _ffar = ffar;
  _r = tan(deg_2_rad(hfov) * FLOATCONST(0.5)) * _fnear;
  _l = -_r;
  _t = _r / aspect;
  _b = -_t;
}


INLINE_MATHUTIL void FLOATNAME(LFrustum)::
make_perspective_vfov(FLOATTYPE yfov, FLOATTYPE aspect, FLOATTYPE fnear,
                      FLOATTYPE ffar) {
  _fnear = fnear;
  _ffar = ffar;
  _t = tan(deg_2_rad(yfov) * 0.5f) * _fnear;
  _b = -_t;
  _r = _t * aspect;
  _l = -_r;
}


INLINE_MATHUTIL void FLOATNAME(LFrustum)::
make_perspective(FLOATTYPE xfov, FLOATTYPE yfov, FLOATTYPE fnear,
                 FLOATTYPE ffar) {
  _fnear = fnear;
  _ffar = ffar;
  _t = tan(deg_2_rad(yfov) * 0.5f) * _fnear;
  _b = -_t;
  _r = tan(deg_2_rad(xfov) * 0.5f) * _fnear;
  _l = -_r;
}

/**
 *
 */
INLINE_MATHUTIL void FLOATNAME(LFrustum)::
get_perspective_params(FLOATTYPE& yfov, FLOATTYPE& aspect,
                       FLOATTYPE& fnear, FLOATTYPE& ffar) const {
  yfov = rad_2_deg(atan(_t / _fnear)) * 2.0f;
  aspect = _r / _t;
  fnear = _fnear;
  ffar = _ffar;
}

/**
 *
 */
INLINE_MATHUTIL void FLOATNAME(LFrustum)::
get_perspective_params(FLOATTYPE& xfov, FLOATTYPE& yfov, FLOATTYPE& aspect,
                       FLOATTYPE& fnear, FLOATTYPE& ffar) const {
  xfov = rad_2_deg(atan(_r / _fnear)) * 2.0f;
  get_perspective_params(yfov, aspect, fnear, ffar);
}

/**
 * This computes a transform matrix that performs the perspective transform
 * defined by the frustum, accordinate to the indicated coordinate system.
 */
INLINE_MATHUTIL FLOATNAME(LMatrix4) FLOATNAME(LFrustum)::
get_perspective_projection_mat(CoordinateSystem cs) const {
  if (cs == CS_default) {
    cs = get_default_coordinate_system();
  }

  FLOATTYPE recip_r_minus_l = 1.0f/(_r - _l);
  FLOATTYPE recip_t_minus_b = 1.0f/(_t - _b);
  FLOATTYPE two_fnear = 2.0f*_fnear;

  FLOATTYPE d = (_r + _l) * recip_r_minus_l;
  FLOATTYPE a = two_fnear * recip_r_minus_l;
  FLOATTYPE e = two_fnear * recip_t_minus_b;
  FLOATTYPE b = (_t + _b) * recip_t_minus_b;
  FLOATTYPE c, f;

  // Take the limits if either near or far is infinite.
  if (cinf(_ffar)) {
    c = 1;
    f = -2 * _fnear;
  } else if (cinf(_fnear)) {
    c = -1;
    f = 2 * _ffar;
  } else {
    FLOATTYPE recip_far_minus_near = 1.0f / (_ffar - _fnear);
    c = (_ffar + _fnear) * recip_far_minus_near;
    f = -_ffar * two_fnear * recip_far_minus_near;
  }

/*
  FLOATTYPE a = (2.0f * _fnear) / (_r - _l);
  FLOATTYPE b = (_t + _b) / (_t - _b);
  FLOATTYPE c = (_ffar + _fnear) / (_ffar - _fnear);
  FLOATTYPE d = (_r + _l) / (_r - _l);
  FLOATTYPE e = (2.0f * _fnear) / (_t - _b);
  FLOATTYPE f = (-2.0f * _ffar * _fnear) / (_ffar - _fnear);
*/

  switch (cs) {
  case CS_zup_right:
    return FLOATNAME(LMatrix4)(  a, 0.0f, 0.0f, 0.0f,
                               0.0f,  -b,   c, 1.0f,
                                 d,   e, 0.0f, 0.0f,
                               0.0f, 0.0f,   f, 0.0f);

  case CS_yup_right:
    return FLOATNAME(LMatrix4)(  a, 0.0f, 0.0f, 0.0f,
                               0.0f,   e, 0.0f, 0.0f,
                                  d,   b,  -c, -1.0f,
                               0.0f, 0.0f,   f, 0.0f);

  case CS_zup_left:
    return FLOATNAME(LMatrix4)(  a, 0.0f, 0.0f, 0.0f,
                               0.0f,  b,   -c, -1.0f,
                                 d,   e,  0.0f, 0.0f,
                               0.0f, 0.0f,   f, 0.0f);

  case CS_yup_left:
    return FLOATNAME(LMatrix4)(  a, 0.0f, 0.0f, 0.0f,
                               0.0f,   e, 0.0f, 0.0f,
                                 -d,  -b,    c, 1.0f,
                               0.0f, 0.0f,   f, 0.0f);

  default:
    mathutil_cat.error()
      << "Invalid coordinate system!\n";
    return FLOATNAME(LMatrix4)::ident_mat();
  }
}

/**
 * This computes a transform matrix that performs the orthographic transform
 * defined by the frustum, accordinate to the indicated coordinate system.
 */
INLINE_MATHUTIL FLOATNAME(LMatrix4) FLOATNAME(LFrustum)::
get_ortho_projection_mat(CoordinateSystem cs) const {
  if (cs == CS_default) {
    cs = get_default_coordinate_system();
  }

  FLOATTYPE a = 2.0f / (_r - _l);
  FLOATTYPE b = 2.0f / (_t - _b);
  FLOATTYPE c = 2.0f / (_ffar - _fnear);
  FLOATTYPE d = (_r + _l) * a * 0.5f;
  FLOATTYPE e = (_t + _b) * b * 0.5f;
  FLOATTYPE f = (_ffar + _fnear) * c * 0.5f;

/*
  FLOATTYPE a = 2.0f / (_r - _l);
  FLOATTYPE b = 2.0f / (_t - _b);
  FLOATTYPE c = 2.0f / (_ffar - _fnear);
  FLOATTYPE d = (_r + _l) / (_r + _l)
  FLOATTYPE e = (_t + _b) / (_t - _b);
  FLOATTYPE f = (_ffar + _fnear) / (_ffar - _fnear);
*/
  switch (cs) {
  case CS_zup_right:
    return FLOATNAME(LMatrix4)(  a, 0.0f, 0.0f, 0.0f,
                               0.0f, 0.0f,  -c, 0.0f,
                               0.0f,  -b, 0.0f, 0.0f,
                                -d,  -e,  -f, 1.0f);

  case CS_yup_right:
    return FLOATNAME(LMatrix4)(  a, 0.0f, 0.0f, 0.0f,
                               0.0f,   b, 0.0f, 0.0f,
                               0.0f, 0.0f,  -c, 0.0f,
                                -d,  -e,  -f, 1.0f);

  case CS_zup_left:
    return FLOATNAME(LMatrix4)(  a, 0.0f, 0.0f, 0.0f,
                               0.0f, 0.0f,  -c, 0.0f,
                               0.0f,   b, 0.0f, 0.0f,
                                -d,  -e,  -f, 1.0f);

  case CS_yup_left:
    return FLOATNAME(LMatrix4)(  a, 0.0f, 0.0f, 0.0f,
                               0.0f,   b, 0.0f, 0.0f,
                               0.0f, 0.0f,   c, 0.0f,
                                -d,  -e,  -f, 1.0f);

  default:
    mathutil_cat.error()
      << "Invalid coordinate system!\n";
    return FLOATNAME(LMatrix4)::ident_mat();
  }
}
