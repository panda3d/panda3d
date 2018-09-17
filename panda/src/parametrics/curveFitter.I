/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file curveFitter.I
 * @author drose
 * @date 2001-03-04
 */

/**
 *
 */
INLINE CurveFitter::DataPoint::
DataPoint() :
  _t(0.0f),
  _xyz(0.0f, 0.0f, 0.0f),
  _hpr(0.0f, 0.0f, 0.0f),
  _tangent(0.0f, 0.0f, 0.0f),
  _hpr_tangent(0.0f, 0.0f, 0.0f)
{
}

/**
 *
 */
INLINE void CurveFitter::DataPoint::
output(std::ostream &out) const {
  out << "Time " << _t << " xyz " << _xyz << " hpr " << _hpr
      << " tan " << _tangent;
}

/**
 *
 */
INLINE bool CurveFitter::DataPoint::
operator < (const DataPoint &other) const {
  return _t < other._t;
}
