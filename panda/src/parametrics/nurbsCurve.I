/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file nurbsCurve.I
 * @author drose
 * @date 2001-03-02
 */

/**
 *
 */
INLINE CubicCurveseg *NurbsCurve::
get_curveseg(int ti) {
  return (CubicCurveseg *)PiecewiseCurve::get_curveseg(ti);
}
