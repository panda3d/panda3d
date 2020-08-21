/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggNurbsCurve.I
 * @author drose
 * @date 2000-02-15
 */

/**
 *
 */
INLINE EggNurbsCurve::
EggNurbsCurve(const std::string &name) : EggCurve(name) {
  _order = 0;
}

/**
 *
 */
INLINE EggNurbsCurve::
EggNurbsCurve(const EggNurbsCurve &copy) :
  EggCurve(copy),
  _knots(copy._knots),
  _order(copy._order)
{
}

/**
 *
 */
INLINE EggNurbsCurve &EggNurbsCurve::
operator = (const EggNurbsCurve &copy) {
  EggCurve::operator = (copy);
  _knots = copy._knots;
  _order = copy._order;
  return *this;
}

/**
 * Directly changes the order to the indicated value (which must be an integer
 * in the range 1 <= order <= 4).  If possible, it is preferable to use the
 * setup() method instead of this method, since changing the order directly
 * may result in an invalid curve.
 */
INLINE void EggNurbsCurve::
set_order(int order) {
  nassertv(order >= 1 && order <= 4);
  _order = order;
}

/**
 * Resets the value of the indicated knot as indicated.  k must be in the
 * range 0 <= k < get_num_knots(), and the value must be in the range
 * get_knot(k - 1) <= value <= get_knot(k + 1).
 */
INLINE void EggNurbsCurve::
set_knot(int k, double value) {
  nassertv(k >= 0 && k < (int)_knots.size());
  _knots[k] = value;
}

/**
 * Returns the order of the curve.  The order is the degree of the NURBS
 * equation plus 1; for a typical NURBS, the order is 4.  With this
 * implementation of NURBS, the order must be in the range [1, 4].
 */
INLINE int EggNurbsCurve::
get_order() const {
  return _order;
}

/**
 * Returns the degree of the curve.  For a typical NURBS, the degree is 3.
 */
INLINE int EggNurbsCurve::
get_degree() const {
  return _order - 1;
}

/**
 * Returns the number of knots.
 */
INLINE int EggNurbsCurve::
get_num_knots() const {
  return _knots.size();
}

/**
 * Returns the total number of control vertices that *should* be defined for
 * the curve.  This is determined by the number of knots and the order, in
 * each direction; it does not necessarily reflect the number of vertices that
 * have actually been added to the curve.  (However, if the number of vertices
 * in the curve are wrong, the curve is invalid.)
 */
INLINE int EggNurbsCurve::
get_num_cvs() const {
  return get_num_knots() - get_order();
}

/**
 * Returns the nth knot value defined.
 */
INLINE double EggNurbsCurve::
get_knot(int k) const {
  nassertr(k >= 0 && k < (int)_knots.size(), 0.0);
  return _knots[k];
}
