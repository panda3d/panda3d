/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file piecewiseCurve.cxx
 * @author drose
 * @date 2001-03-04
 */

#include "piecewiseCurve.h"
#include "cubicCurveseg.h"
#include "config_parametrics.h"

#include "datagram.h"
#include "datagramIterator.h"
#include "bamWriter.h"
#include "bamReader.h"

using std::cerr;

TypeHandle PiecewiseCurve::_type_handle;

/**
 *
 */
PiecewiseCurve::
PiecewiseCurve() {
  _last_ti = 0;
}

/**
 *
 */
PiecewiseCurve::
~PiecewiseCurve() {
  remove_all_curvesegs();
}


/**
 * Returns true if the curve is defined.  In the case of a PiecewiseCurve,
 * this means we have at least one segment.
 */
bool PiecewiseCurve::
is_valid() const {
  return !_segs.empty();
}

/**
 * Returns the upper bound of t for the entire curve.  The curve is defined in
 * the range 0.0f <= t <= get_max_t().
 */
PN_stdfloat PiecewiseCurve::
get_max_t() const {
  return _segs.empty() ? 0.0f : _segs.back()._tend;
}


/**
 * Returns the point of the curve at a given parametric point t.  Returns true
 * if t is in the valid range 0.0f <= t <= get_max_t(); if t is outside this
 * range, sets point to the value of the curve at the beginning or end
 * (whichever is nearer) and returns false.
 */
bool PiecewiseCurve::
get_point(PN_stdfloat t, LVecBase3 &point) const {
  const ParametricCurve *curve;
  bool result = find_curve(curve, t);
  if (curve == nullptr){
    return false;
  }
  // We use | instead of || so we won't short-circuit this calculation.
  return result | curve->get_point(t, point);
}


/**
 * Returns the tangent of the curve at a given parametric point t.
 */
bool PiecewiseCurve::
get_tangent(PN_stdfloat t, LVecBase3 &tangent) const {
  const ParametricCurve *curve;
  bool result = find_curve(curve, t);

  // We use | instead of || so we won't short-circuit this calculation.
  return result | curve->get_tangent(t, tangent);
}


/**
 * Returns the tangent of the first derivative of the curve at the point t.
 */
bool PiecewiseCurve::
get_2ndtangent(PN_stdfloat t, LVecBase3 &tangent2) const {
  const ParametricCurve *curve;
  bool result = find_curve(curve, t);

  // We use | instead of || so we won't short-circuit this calculation.
  return result | curve->get_2ndtangent(t, tangent2);
}

/**
 * Recomputes the curve such that it passes through the point (px, py, pz) at
 * time t, but keeps the same tangent value at that point.
 */
bool PiecewiseCurve::
adjust_point(PN_stdfloat t,
             PN_stdfloat px, PN_stdfloat py, PN_stdfloat pz) {
  if (parametrics_cat.is_debug()) {
    parametrics_cat.debug()
      << "Adjusting point at " << t << " to " << px << " " << py << " "
      << pz << "\n";
  }

  const ParametricCurve *curve;
  bool result = find_curve(curve, t);

  if (!result) {
    cerr << "No curve segment at t = " << t << "\n";
    return false;
  }

  rebuild_curveseg(RT_CV | RT_KEEP_ORIG, 0.0f, LVecBase4(),
                   RT_POINT, t, LVecBase4(px, py, pz, 1.0f),
                   RT_TANGENT | RT_KEEP_ORIG, t, LVecBase4(),
                   RT_CV | RT_KEEP_ORIG, 0.0f, LVecBase4());
  return true;
}

/**
 * Recomputes the curve such that it has the tangent (tx, ty, tz) at time t,
 * but keeps the same position at the point.
 */
bool PiecewiseCurve::
adjust_tangent(PN_stdfloat t,
               PN_stdfloat tx, PN_stdfloat ty, PN_stdfloat tz) {
  const ParametricCurve *curve;
  bool result = find_curve(curve, t);

  if (!result) {
    cerr << "No curve segment at t = " << t << "\n";
    return false;
  }

  rebuild_curveseg(RT_CV | RT_KEEP_ORIG, 0.0f, LVecBase4(),
                   RT_POINT | RT_KEEP_ORIG, t, LVecBase4(),
                   RT_TANGENT, t, LVecBase4(tx, ty, tz, 0.0f),
                   RT_CV | RT_KEEP_ORIG, 0.0f, LVecBase4());
  return true;
}

/**
 * Recomputes the curve such that it passes through the point (px, py, pz)
 * with the tangent (tx, ty, tz).
 */
bool PiecewiseCurve::
adjust_pt(PN_stdfloat t,
          PN_stdfloat px, PN_stdfloat py, PN_stdfloat pz,
          PN_stdfloat tx, PN_stdfloat ty, PN_stdfloat tz) {
  const ParametricCurve *curve;
  bool result = find_curve(curve, t);

  if (!result) {
    cerr << "No curve segment at t = " << t << "\n";
    return false;
  }

  rebuild_curveseg(RT_CV | RT_KEEP_ORIG, 0.0f, LVecBase4(),
                   RT_POINT, t, LVecBase4(px, py, pz, 1.0f),
                   RT_TANGENT, t, LVecBase4(tx, ty, tz, 0.0f),
                   RT_CV | RT_KEEP_ORIG, 0.0f, LVecBase4());
  return true;
}


/**
 * Simultaneously returns the point and tangent of the curve at a given
 * parametric point t.
 */
bool PiecewiseCurve::
get_pt(PN_stdfloat t, LVecBase3 &point, LVecBase3 &tangent) const {
  const ParametricCurve *curve;
  bool result = find_curve(curve, t);

  // We use | instead of || so we won't short-circuit this calculation.
  return result | curve->get_pt(t, point, tangent);
}


/**
 * Returns the number of curve segments that make up the Piecewise curve.
 */
int PiecewiseCurve::
get_num_segs() const {
  return _segs.size();
}

/**
 * Returns the curve segment corresponding to the given index.
 */
ParametricCurve *PiecewiseCurve::
get_curveseg(int ti) {
  assert(ti >= 0 && ti < (int)_segs.size());
  return _segs[ti]._curve;
}


/**
 * Inserts a new curve segment at the indicated index.  The curve segment must
 * have been allocated via new; it will be freed using delete when it is
 * removed or the PiecewiseCurve destructs.
 *
 * If the curve segment is not inserted at the end, its tlength is subtracted
 * from that of the following segment, so that the overall length of the curve
 * is not changed.
 */
bool PiecewiseCurve::
insert_curveseg(int ti, ParametricCurve *seg, PN_stdfloat tlength) {
  if (ti < 0 || ti > (int)_segs.size()) {
    return false;
  }

  if (ti == (int)_segs.size()) {
    _segs.push_back(Curveseg(seg, get_max_t() + tlength));

  } else if (ti==0) {
    _segs.insert(_segs.begin(),
                 Curveseg(seg, tlength));

  } else {
    _segs.insert(_segs.begin() + ti,
                 Curveseg(seg, _segs[ti-1]._tend + tlength));
  }

  return true;
}


/**
 * Removes the given curve segment from the curve and frees it.  Returns true
 * if the segment was defined, false otherwise.
 */
bool PiecewiseCurve::
remove_curveseg(int ti) {
  if (ti < 0 || ti >= (int)_segs.size()) {
    return false;
  }

  PN_stdfloat tlength = get_tlength(ti);
  _segs.erase(_segs.begin() + ti);

  // Now update the _tend figures for everything after the one we removed.
  while (ti < (int)_segs.size()) {
    _segs[ti]._tend -= tlength;
    ti++;
  }

  _last_ti = 0;
  return true;
}

/**
 * Removes all curve segments from the curve.
 */
void PiecewiseCurve::
remove_all_curvesegs() {
  _segs.erase(_segs.begin(), _segs.end());
  _last_ti = 0;
}

/**
 * Returns the parametric length of the given segment of the curve.
 */
PN_stdfloat PiecewiseCurve::
get_tlength(int ti) const {
  assert(ti >= 0 && ti < (int)_segs.size());
  return (ti==0) ? _segs[ti]._tend : _segs[ti]._tend - _segs[ti-1]._tend;
}

/**
 * Returns the parametric start of the given segment of the curve.
 */
PN_stdfloat PiecewiseCurve::
get_tstart(int ti) const {
  assert(ti >= 0 && ti <= (int)_segs.size());
  return (ti==0) ? 0.0f : _segs[ti-1]._tend;
}

/**
 * Returns the parametric end of the given segment of the curve.
 */
PN_stdfloat PiecewiseCurve::
get_tend(int ti) const {
  assert(ti >= 0 && ti < (int)_segs.size());
  return _segs[ti]._tend;
}


/**
 * Sets the parametric length of the given segment of the curve.  The length
 * of the following segment is lengthened by the corresponding amount to keep
 * the overall length of the curve the same.
 */
bool PiecewiseCurve::
set_tlength(int ti, PN_stdfloat tlength) {
  if (ti < 0 || ti >= (int)_segs.size()) {
    return false;
  }

  _segs[ti]._tend += tlength - get_tlength(ti);
  return true;
}



/**
 * Defines the curve as a general NURBS curve.  The order is the degree plus
 * one and must be 1, 2, 3, or 4; cvs is an array of num_cvs points each with
 * a homogeneous coordinate; knots is an array of num_cvs+order knot values.
 *
 * This creates the individual curve segments and sets up the basis matrices,
 * but does not store the CV's or knot values so the curve shape is not later
 * modifiable.
 */
void PiecewiseCurve::
make_nurbs(int order, int num_cvs,
           const PN_stdfloat knots[], const LVecBase4 cvs[]) {
  remove_all_curvesegs();

  for (int i=0; i<num_cvs - order + 1; i++) {
    if (knots[i+order] > knots[i+order-1]) {
      int ti = get_num_segs();
      bool result =
        insert_curveseg(ti, new CubicCurveseg(order, knots+i, cvs+i),
                        knots[i+order] - knots[i+order-1]);
      assert(result);
    }
  }
}


/**
 * Fills up the indicated vector with a list of BezierSeg structs that
 * describe the curve.  This assumes the curve is a PiecewiseCurve of
 * CubicCurvesegs.  Returns true if successful, false otherwise.
 */
bool PiecewiseCurve::
get_bezier_segs(BezierSegs &bz_segs) const {
  bz_segs.erase(bz_segs.begin(), bz_segs.end());
  int i;
  BezierSeg seg;
  for (i = 0; i < (int)_segs.size(); i++) {
    if (!_segs[i]._curve->get_bezier_seg(seg)) {
      return false;
    }
    seg._t = _segs[i]._tend;
    bz_segs.push_back(seg);
  }

  return true;
}

/**
 * Rebuilds the current curve segment (as selected by the most recent call to
 * find_curve()) according to the specified properties (see
 * CubicCurveseg::compute_seg).  Returns true if possible, false if something
 * goes horribly wrong.
 */
bool PiecewiseCurve::
rebuild_curveseg(int, PN_stdfloat, const LVecBase4 &,
                 int, PN_stdfloat, const LVecBase4 &,
                 int, PN_stdfloat, const LVecBase4 &,
                 int, PN_stdfloat, const LVecBase4 &) {
  cerr << "rebuild_curveseg not implemented for this curve type.\n";
  return false;
}

/**
 * Finds the curve corresponding to the given value of t.  If t is inside the
 * curve's defined range, sets curve to the appropriate segment, translates t
 * to [0,1] to index into the segment's coordinate system, and returns true.
 * If t is outside the curve's defined range, sets curve to the nearest
 * segment and t to the nearest point on this segment, and returns false.
 */
bool PiecewiseCurve::
find_curve(const ParametricCurve *&curve, PN_stdfloat &t) const {
  // Check the index computed by the last call to find_curve().  If it's still
  // a reasonable starting value, start searching from there.  This way, we
  // take advantage of locality of reference: the search is trivial it is the
  // same segment as last time, or the next segment after the last one.
  if (_last_ti>0 && _segs[_last_ti-1]._tend>=t) {
    // However, if the new t value precedes that of last time, we'll have to
    // start over.

    // We do some messy casting so we can get away with assigning a value to a
    // member within a const function.  This assignment doesn't really count
    // as a const violation since we're just updating a cached value, not
    // changing any real data of the class.
    ((PiecewiseCurve *)this)->_last_ti = 0;
  }

  int ti;
  for (ti = _last_ti; ti < (int)_segs.size(); ti++) {
    if (_segs[ti]._tend+0.00001f > t) {
      break;
    }
  }

  if (ti < (int)_segs.size()) {
    // Adjust t to the range [0,1).
    if (ti > 0) {
      t = (t - _segs[ti-1]._tend) / (_segs[ti]._tend - _segs[ti-1]._tend);
    } else {
      t /= _segs[0]._tend;
    }
  }

  if (t < 0) {
    // Oops.
    curve = _segs[0]._curve;
    t = 0.0f;
    return false;
  }

  if (ti >= (int)_segs.size() || !_segs[ti]._curve->is_valid()) {
    assert(ti <= (int)_segs.size());

    // If we're out of bounds, or the curve is undefined, we're probably
    // screwed.  There's one exception: if we were right on a border between
    // curves, try the curve before.

    if (ti > 0 && t < _segs[ti-1]._tend+0.0001f) {
      ti--;
      t = 1.0f;
    }

    if (ti >= (int)_segs.size()) {
      if (_segs.empty()) {
        curve = nullptr;
        t = 0.0f;
        return false;
      } else {
        curve = _segs.back()._curve;
        t = 1.0f;
        return false;
      }
    } else if (!_segs[ti]._curve->is_valid()) {
      curve = _segs[ti]._curve;
      return false;
    }
  }

  // Again, some messy casting so we can get away with updating the cached
  // index value for next time.
  ((PiecewiseCurve *)this)->_last_ti = ti;

  // Now scale t back into the curve's own valid range.
  t *= _segs[ti]._curve->get_max_t();
  curve = _segs[ti]._curve;
  return true;
}


/**
 * Returns a number in the range [0,1], representing the conversion of t into
 * the current segment's coordinate system (the segment last returned by
 * find_curve). This operation is already performed automatically on the t
 * passed into find_seg; this function is useful only to adjust a different
 * value into the same range.
 *
 * It is an error to call this function if find_curve() has not yet been
 * called, or if find_curve() returned false from its previous call.
 */
PN_stdfloat PiecewiseCurve::
current_seg_range(PN_stdfloat t) const {
  int ti = _last_ti;

  assert(ti < (int)_segs.size());

  // Adjust t to the range [0,1).
  if (ti > 0) {
    t = (t - _segs[ti-1]._tend) / (_segs[ti]._tend - _segs[ti-1]._tend);
  } else {
    t /= _segs[0]._tend;
  }

  return t;
}

/**
 * Function to write the important information in the particular object to a
 * Datagram
 */
void PiecewiseCurve::
write_datagram(BamWriter *manager, Datagram &me) {
  ParametricCurve::write_datagram(manager, me);

  me.add_uint32(_segs.size());
  size_t i;
  for (i = 0; i < _segs.size(); i++) {
    const Curveseg &seg = _segs[i];
    manager->write_pointer(me, seg._curve);
    me.add_float64(seg._tend);
  }

  _last_ti = 0;
}

/**
 * Function that reads out of the datagram (or asks manager to read) all of
 * the data that is needed to re-create this object and stores it in the
 * appropiate place
 */
void PiecewiseCurve::
fillin(DatagramIterator &scan, BamReader *manager) {
  ParametricCurve::fillin(scan, manager);

  size_t num_segs = scan.get_uint32();
  _segs.reserve(num_segs);
  size_t i;
  for (i = 0; i < num_segs; i++) {
    Curveseg seg;
    manager->read_pointer(scan);
    seg._curve = nullptr;
    seg._tend = scan.get_float64();
    _segs.push_back(seg);
  }
}

/**
 * Takes in a vector of pointes to TypedWritable objects that correspond to
 * all the requests for pointers that this object made to BamReader.
 */
int PiecewiseCurve::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int used = ParametricCurve::complete_pointers(p_list, manager);

  size_t i;
  for (i = 0; i < _segs.size(); i++) {
    _segs[i]._curve = DCAST(ParametricCurve, p_list[used + i]);
  }

  return used + _segs.size();
}
