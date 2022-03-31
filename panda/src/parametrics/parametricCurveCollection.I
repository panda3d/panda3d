/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file parametricCurveCollection.I
 * @author drose
 * @date 2001-03-04
 */

/**
 *
 */
INLINE ParametricCurveCollection::
~ParametricCurveCollection() {
}

/**
 * Returns the number of ParametricCurves in the collection.
 */
INLINE int ParametricCurveCollection::
get_num_curves() const {
  return _curves.size();
}

/**
 * Returns the nth ParametricCurve in the collection.
 */
INLINE ParametricCurve *ParametricCurveCollection::
get_curve(int index) const {
  nassertr(index >= 0 && index < (int)_curves.size(), nullptr);

  return _curves[index];
}

/**
 * Adds a new ParametricCurve to the collection at the indicated index.
 * @deprecated Use insert_curve(index, curve) instead.
 */
INLINE void ParametricCurveCollection::
add_curve(ParametricCurve *curve, int index) {
  insert_curve(std::max(index, 0), curve);
}

/**
 * Returns the maximum T value associated with the *last* curve in the
 * collection.  Normally, this will be either the XYZ or HPR curve, or a
 * timewarp curve.
 */
INLINE PN_stdfloat ParametricCurveCollection::
get_max_t() const {
  if (_curves.empty()) {
    return 0.0f;
  }
  return _curves.back()->get_max_t();
}

/**
 * Computes only the XYZ part of the curves.  See evaluate().
 */
INLINE bool ParametricCurveCollection::
evaluate_xyz(PN_stdfloat t, LVecBase3 &xyz) const {
  LVecBase3 hpr;
  return evaluate(t, xyz, hpr);
}

/**
 * Computes only the HPR part of the curves.  See evaluate().
 */
INLINE bool ParametricCurveCollection::
evaluate_hpr(PN_stdfloat t, LVecBase3 &hpr) const {
  LVecBase3 xyz;
  return evaluate(t, xyz, hpr);
}

/**
 * Adjust the XYZ curve at the indicated time to the new value.  The curve
 * shape will change correspondingly.  Returns true if successful, false if
 * unable to make the adjustment for some reason.
 */
INLINE bool ParametricCurveCollection::
adjust_xyz(PN_stdfloat t, PN_stdfloat x, PN_stdfloat y, PN_stdfloat z) {
  return adjust_xyz(t, LVecBase3(x, y, z));
}

/**
 * Adjust the HPR curve at the indicated time to the new value.  The curve
 * shape will change correspondingly.  Returns true if successful, false if
 * unable to make the adjustment for some reason.
 */
INLINE bool ParametricCurveCollection::
adjust_hpr(PN_stdfloat t, PN_stdfloat h, PN_stdfloat p, PN_stdfloat r) {
  return adjust_hpr(t, LVecBase3(h, p, r));
}
