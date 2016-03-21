/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file parabola_src.I
 * @author drose
 * @date 2007-10-10
 */

/**
 * Constructs a meaningless degenerate parabola.
 */
INLINE_MATHUTIL FLOATNAME(LParabola)::
FLOATNAME(LParabola)() :
  _a(FLOATNAME(LVecBase3)::zero()),
  _b(FLOATNAME(LVecBase3)::zero()),
  _c(FLOATNAME(LVecBase3)::zero())
{
}

/**
 * Constructs a parabola given the three points of the parametric equation:
 * the acceleration, initial velocity, and start point.
 */
INLINE_MATHUTIL FLOATNAME(LParabola)::
FLOATNAME(LParabola)(const FLOATNAME(LVecBase3) &a,
                    const FLOATNAME(LVecBase3) &b,
                    const FLOATNAME(LVecBase3) &c) :
  _a(a), _b(b), _c(c)
{
}

/**
 *
 */
INLINE_MATHUTIL FLOATNAME(LParabola)::
FLOATNAME(LParabola)(const FLOATNAME(LParabola) &copy) :
  _a(copy._a),
  _b(copy._b),
  _c(copy._c)
{
}

/**
 *
 */
INLINE_MATHUTIL void FLOATNAME(LParabola)::
operator = (const FLOATNAME(LParabola) &copy) {
  _a = copy._a;
  _b = copy._b;
  _c = copy._c;
}

/**
 *
 */
INLINE_MATHUTIL FLOATNAME(LParabola)::
~FLOATNAME(LParabola)() {
}

/**
 * Returns the first point of the parabola's parametric equation: the
 * acceleration.
 */
INLINE_MATHUTIL const FLOATNAME(LVecBase3) &FLOATNAME(LParabola)::
get_a() const {
  return _a;
}

/**
 * Returns the second point of the parabola's parametric equation: the initial
 * velocity.
 */
INLINE_MATHUTIL const FLOATNAME(LVecBase3) &FLOATNAME(LParabola)::
get_b() const {
  return _b;
}

/**
 * Returns the third point of the parabola's parametric equation: the start
 * point.
 */
INLINE_MATHUTIL const FLOATNAME(LVecBase3) &FLOATNAME(LParabola)::
get_c() const {
  return _c;
}

/**
 * Computes the point on the parabola at time t.
 */
INLINE_MATHUTIL FLOATNAME(LPoint3) FLOATNAME(LParabola)::
calc_point(FLOATTYPE t) const {
  return _a * t * t + _b * t + _c;
}
