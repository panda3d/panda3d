// Filename: curve.C
// Created by:  drose (14Mar97)
//
////////////////////////////////////////////////////////////////////
// Copyright (C) 1992,93,94,95,96,97  Walt Disney Imagineering, Inc.
//
// These  coded  instructions,  statements,  data   structures   and
// computer  programs contain unpublished proprietary information of
// Walt Disney Imagineering and are protected by  Federal  copyright
// law.  They may  not be  disclosed to third  parties  or copied or
// duplicated in any form, in whole or in part,  without  the  prior
// written consent of Walt Disney Imagineering Inc.
////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////

#include "curve.h"
#include "config_parametrics.h"
#include "nurbsCurve.h"
#include "curveDrawer.h"


////////////////////////////////////////////////////////////////////
// Statics
////////////////////////////////////////////////////////////////////

TypeHandle ParametricCurve::_type_handle;
TypeHandle PiecewiseCurve::_type_handle;
TypeHandle CubicCurveseg::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: ParametricCurve::Constructor
//       Access: Public
//  Description: This is a virtual base class.  Don't try to construct
//               one from Scheme.
////////////////////////////////////////////////////////////////////
ParametricCurve::
ParametricCurve() {
  _curve_type = PCT_NONE;
  _num_dimensions = 3;
}


////////////////////////////////////////////////////////////////////
//     Function: ParametricCurve::is_valid
//       Access: Public, Scheme, Virtual
//  Description: Returns true if the curve is defined.  This base
//               class function always returns true; derived classes
//               might override this to sometimes return false.
////////////////////////////////////////////////////////////////////
bool ParametricCurve::
is_valid() const {
  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: ParametricCurve::get_max_t
//       Access: Public, Scheme, Virtual
//  Description: Returns the upper bound of t for the entire curve.
//               The curve is defined in the range 0.0 <= t <=
//               get_max_t().  This base class function always returns
//               1.0; derived classes might override this to return
//               something else.
////////////////////////////////////////////////////////////////////
double ParametricCurve::
get_max_t() const {
  return 1.0;
}


////////////////////////////////////////////////////////////////////
//     Function: ParametricCurve::set_curve_type
//       Access: Public, Scheme
//  Description: Sets the flag indicating the use to which the curve
//               is intended to be put.  This flag is optional and
//               only serves to provide a hint to the egg reader and
//               writer code; it has no effect on the curve's
//               behavior.
//
//               Setting the curve type also sets the num_dimensions
//               to 3 or 1 according to the type.
//
//               THis flag may have one of the values PCT_XYZ,
//               PCT_HPR, or PCT_T.
////////////////////////////////////////////////////////////////////
void ParametricCurve::
set_curve_type(int type) {
  _curve_type = type;
  switch (_curve_type) {
  case PCT_XYZ:
  case PCT_HPR:
  case PCT_NONE:
    _num_dimensions = 3;
    break;

  case PCT_T:
    _num_dimensions = 1;
    break;

  default:
    assert(0);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ParametricCurve::get_curve_type
//       Access: Public, Scheme
//  Description: Returns the flag indicating the use to which the curve
//               is intended to be put.
////////////////////////////////////////////////////////////////////
int ParametricCurve::
get_curve_type() const {
  return _curve_type;
}

////////////////////////////////////////////////////////////////////
//     Function: ParametricCurve::set_num_dimensions
//       Access: Public, Scheme
//  Description: Specifies the number of significant dimensions in the
//               curve's vertices.  This should be one of 1, 2, or 3.
//               Normally, XYZ and HPR curves have three dimensions;
//               time curves should always have one dimension.  This
//               only serves as a hint to the mopath editor, and also
//               controls how the curve is written out.
////////////////////////////////////////////////////////////////////
void ParametricCurve::
set_num_dimensions(int num) {
  _num_dimensions = num;
}

////////////////////////////////////////////////////////////////////
//     Function: ParametricCurve::get_num_dimensions
//       Access: Public, Scheme
//  Description: Returns the number of significant dimensions in the
//               curve's vertices, as set by a previous call to
//               set_num_dimensions().  This is only a hint as to how
//               the curve is intended to be used; the actual number
//               of dimensions of any curve is always three.
////////////////////////////////////////////////////////////////////
int ParametricCurve::
get_num_dimensions() const {
  return _num_dimensions;
}


////////////////////////////////////////////////////////////////////
//     Function: ParametricCurve::calc_length
//       Access: Public, Scheme
//  Description: Approximates the length of the entire curve to within
//               a few decimal places.
////////////////////////////////////////////////////////////////////
float ParametricCurve::
calc_length() const {
  return calc_length(0.0, get_max_t());
}

////////////////////////////////////////////////////////////////////
//     Function: ParametricCurve::calc_length
//       Access: Public, Scheme
//  Description: Approximates the length of the curve segment from
//               parametric time from to time to.
////////////////////////////////////////////////////////////////////
float ParametricCurve::
calc_length(double from, double to) const {
  double t1, t2;
  LVector3f p1, p2;

  // Normally we expect from < to.  If they came in backwards, reverse
  // them.
  if (to < from) {
    double temp = to;
    to = from;
    from = temp;
  }

  // Start with a segment for each unit of t.
  int num_segs = (int)floor(to - from + 1);
  t2 = from;
  get_point(t2, p2);
  float net = 0.0;

  for (int i = 1; i <= num_segs; i++) {
    t1 = t2;
    p1 = p2;

    t2 = (to - from) * (double)i / (double)num_segs + from;
    get_point(t2, p2);

    net += r_calc_length(t1, t2, p1, p2, (p1 - p2).length());
  }
  return net;
}

////////////////////////////////////////////////////////////////////
//     Function: ParametricCurve::compute_t
//       Access: Public, Scheme
//  Description: The inverse of calc_length: given a distance offset
//               along the curve from an arbitrary point, compute the
//               new value of t.  This function is quite expensive.
////////////////////////////////////////////////////////////////////
double ParametricCurve::
compute_t(double start_t, double length_offset, double guess,
	  double threshold) const {
  if (length_offset > 0.0) {
    // If the length_offset is positive, we are looking forward.
    // Enforce that the guess is greater than the start.
    if (guess < start_t) {
      guess = start_t + (start_t - guess);
    } else if (guess == start_t) {
      guess = start_t + 1.0;
    }

  } else if (length_offset < 0.0) {
    // If the length offset is negative, we are looking backward.
    // Enforce that the guess is less than the start.
    if (guess > start_t) {
      guess = start_t - (guess - start_t);
    } else if (guess == start_t) {
      guess = start_t - 1.0;
    }

  } else {
    // If the length_offset is zero, we're just being silly.
    return start_t;
  }

  // First, compute the length of the curve segment from start_t to
  // guess.
  double actual_length = calc_length(start_t, guess);
  double max_t = get_max_t();
  bool clamped = false;

  // Are we close enough yet?
  cerr << "Got " << actual_length << " wanted " << length_offset << "\n";
  while (fabs(actual_length - length_offset) > threshold) {
    // Not close enough: use the computed length to calculate where
    // the t_offset should be if the curve were evenly distributed
    // across its entire range.
    guess = (guess - start_t) * length_offset / actual_length + start_t;

    // Clamp it to the end of the curve.
    if (guess > max_t) {
      if (clamped) {
	return max_t;
      }
      clamped = true;
      guess = max_t;
    } else {
      clamped = false;
    }

    actual_length = calc_length(start_t, guess);
    cerr << "Got " << actual_length << " wanted " << length_offset << "\n";
  }
  cerr << "The answer is " << guess << "\n\n";

  return guess;
}

////////////////////////////////////////////////////////////////////
//     Function: ParametricCurve::convert_to_nurbs
//       Access: Public
//  Description: Stores in the indicated NurbsCurve a NURBS
//               representation of an equivalent curve.  Returns true
//               if successful, false otherwise.
////////////////////////////////////////////////////////////////////
bool ParametricCurve::
convert_to_nurbs(NurbsCurve &nc) const {
  BezierSegs bz_segs;
  if (!GetBezierSegs(bz_segs)) {
    return false;
  }

  nc.remove_all_cvs();
  nc.set_curve_type(_curve_type);
  nc.set_order(4);
  if (!bz_segs.empty()) {
    int i;
    for (i = 0; i<bz_segs.size(); i++) {
      nc.append_cv(bz_segs[i]._v[0]);
      nc.append_cv(bz_segs[i]._v[1]);
      nc.append_cv(bz_segs[i]._v[2]);
      if (i == bz_segs.size()-1 ||
	  !bz_segs[i]._v[3].almost_equal(bz_segs[i+1]._v[0], 0.0001)) {
	nc.append_cv(bz_segs[i]._v[3]);
      }
    }

    double t;
    int ki = 4;
    nc.set_knot(0, 0.0);
    nc.set_knot(1, 0.0);
    nc.set_knot(2, 0.0);
    nc.set_knot(3, 0.0);

    for (i = 0; i<bz_segs.size(); i++) {
      t = bz_segs[i]._t;

      nc.set_knot(ki, t);
      nc.set_knot(ki+1, t);
      nc.set_knot(ki+2, t);
      ki += 3;
      if (i == bz_segs.size()-1 ||
	  !bz_segs[i]._v[3].almost_equal(bz_segs[i+1]._v[0], 0.0001)) {
	nc.set_knot(ki, t);
	ki++;
      }
    }
  }
  nc.recompute();

  return nc.is_valid();
}


////////////////////////////////////////////////////////////////////
//     Function: ParametricCurve::ascii_draw
//       Access: Public, Scheme
//  Description: Draws a cheesy representation of the curve in the XZ
//               plane using ASCII characters.  This is entirely so I
//               can debug the curve from home.
////////////////////////////////////////////////////////////////////
void ParametricCurve::
ascii_draw() const {
  // First, get an approximate bounds on the curve.

  float minx, minz, maxx, maxz;

  LVector3f p;
  get_point(0.0, p);
  minx = maxx = p[0];
  minz = maxz = p[2];

  double t;

  for (t = 0.0; t <= get_max_t(); t += 0.0625) {
    get_point(t, p);
    minx = min(minx, p[0]);
    maxx = max(maxx, p[0]);
    minz = min(minz, p[2]);
    maxz = max(maxz, p[2]);
  }

  // Set up the 2-d character buffer we will draw into.
  static const int rows = 40;
  static const int cols = 78;

  char text[rows][cols+1];

  int r, c;
  for (r = 0; r<rows; r++) {
    memset(text[r], ' ', cols);
    text[r][cols] = '\0';
  }

  double xscale = cols / max(maxx - minx, (float)1.0);
  double zscale = rows / max(maxz - minz, (float)1.0);


  // Now draw into the buffer.

  for (t = 0.0; t <= get_max_t(); t += 0.0625) {
    if (get_point(t, p)) {
      c = (p[0] - minx) * xscale;
      r = (p[2] - minz) * zscale;

      if (r>=0 && r<rows && c>=0 && c<cols) {
	int digit = ((int)floor(t))%10;
	text[rows-1-r][c] = digit + '0';
      }
    }
  }

  // And output the buffer.

  for (r = 0; r<rows; r++) {
    cout << text[r] << "\n";
  }
  cout << "\n" << flush;
}



////////////////////////////////////////////////////////////////////
//     Function: ParametricCurve::register_drawer
//       Access: Public
//  Description: Registers a Drawer with this curve that will
//               automatically be updated whenever the curve is
//               modified, so that the visible representation of the
//               curve is kept up to date.  This is called
//               automatically by the ParametricCurveDrawer
//               constructor.
//
//               Any number of Drawers may be registered with a
//               particular curve.
////////////////////////////////////////////////////////////////////
void ParametricCurve::
register_drawer(ParametricCurveDrawer *drawer) {
  _drawers.push_back(drawer);
}

////////////////////////////////////////////////////////////////////
//     Function: ParametricCurve::unregister_drawer
//       Access: Public
//  Description: Removes a previously registered drawer from the list
//               of automatically-refreshed drawers.  This is called
//               automatically by the ParametricCurveDrawer
//               destructor.
////////////////////////////////////////////////////////////////////
void ParametricCurve::
unregister_drawer(ParametricCurveDrawer *drawer) {
  _drawers.remove(drawer);
}




////////////////////////////////////////////////////////////////////
//     Function: ParametricCurve::Destructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
ParametricCurve::
~ParametricCurve() {
  // We must disable each of the drawers that were registered to us,
  // so they don't try to access our pointers any more.
  DrawerList::iterator d;

  for (d = _drawers.begin();
       d != _drawers.end();
       ++d) {
    (*d)->disable(this);
  }
}


////////////////////////////////////////////////////////////////////
//     Function: ParametricCurve::invalidate
//       Access: Protected
//  Description: Called from a base class to mark a section of the
//               curve that has been modified and must be redrawn or
//               recomputed in some way.
////////////////////////////////////////////////////////////////////
void ParametricCurve::
invalidate(double t1, double t2) {
  if (t1 <= t2) {
    DrawerList::iterator n;
    for (n = _drawers.begin();
	 n != _drawers.end();
	 ++n) {
      (*n)->recompute(max(t1, 0.0), min(t2, get_max_t()), this);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ParametricCurve::invalidate_all
//       Access: Protected
//  Description: Called from a base class to indicate that the curve
//               has changed in some substantial way and must be
//               entirely redrawn.
////////////////////////////////////////////////////////////////////
void ParametricCurve::
invalidate_all() {
  DrawerList::iterator n;
  for (n = _drawers.begin();
       n != _drawers.end();
       ++n) {
    (*n)->draw();
  }
}


////////////////////////////////////////////////////////////////////
//     Function: ParametricCurve::r_calc_length
//       Access: Public, Scheme
//  Description: The recursive implementation of calc_length.  This
//               function calculates the length of a segment of the
//               curve between points t1 and t2, which presumably
//               evaluate to the endpoints p1 and p2, and the segment
//               has the length seglength.
////////////////////////////////////////////////////////////////////
float ParametricCurve::
r_calc_length(double t1, double t2, const LVector3f &p1, const LVector3f &p2,
	      float seglength) const {
  static const float length_tolerance = 0.0000001;
  static const double t_tolerance = 0.000001;

  if (t2 - t1 < t_tolerance) {
    // Stop recursing--we've just walked off the limit for
    // representing smaller values of t.
    return 0.0;
  } else {
    double tmid;
    LVector3f pmid;
    float left, right;

    // Calculate the point on the curve midway between the two
    // endpoints.
    tmid = (t1+t2)/2.0;
    get_point(tmid, pmid);

    // Did we increase the length of the segment measurably?
    left = (p1 - pmid).length();
    right = (pmid - p2).length();

    if ((left + right) - seglength < length_tolerance) {
      // No.  We're done.
      return seglength;
    } else {
      // Yes.  Keep going.
      return r_calc_length(t1, tmid, p1, pmid, left) +
             r_calc_length(tmid, t2, pmid, p2, right);
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: ParametricCurve::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void ParametricCurve::
write_datagram(BamWriter *, Datagram &) {
  // TODO: write the write_datagram.
}


////////////////////////////////////////////////////////////////////
//     Function: PiecewiseCurve::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PiecewiseCurve::
PiecewiseCurve() {
  _last_ti = 0;
}


////////////////////////////////////////////////////////////////////
//     Function: PiecewiseCurve::is_valid
//       Access: Public, Scheme, Virtual
//  Description: Returns true if the curve is defined.  In the case of
//               a PiecewiseCurve, this means we have at least one
//               segment.
////////////////////////////////////////////////////////////////////
bool PiecewiseCurve::
is_valid() const {
  return !_segs.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: PiecewiseCurve::get_max_t
//       Access: Public, Scheme, Virtual
//  Description: Returns the upper bound of t for the entire curve.
//               The curve is defined in the range 0.0 <= t <=
//               get_max_t().
////////////////////////////////////////////////////////////////////
double PiecewiseCurve::
get_max_t() const {
  return _segs.empty() ? 0.0 : _segs.back()._tend;
}


////////////////////////////////////////////////////////////////////
//     Function: PiecewiseCurve::get_point
//       Access: Public, Scheme, Virtual
//  Description: Returns the point of the curve at a given parametric
//               point t.  Returns true if t is in the valid range 0.0
//               <= t <= get_max_t(); if t is outside this range, sets
//               point to the value of the curve at the beginning or
//               end (whichever is nearer) and returns false.
////////////////////////////////////////////////////////////////////
bool PiecewiseCurve::
get_point(double t, LVector3f &point) const {
  const ParametricCurve *curve;
  bool result = find_curve(curve, t);

  // We use | instead of || so we won't short-circuit this calculation.
  return result | curve->get_point(t, point);
}


////////////////////////////////////////////////////////////////////
//     Function: PiecewiseCurve::get_tangent
//       Access: Public, Scheme, Virtual
//  Description: Returns the tangent of the curve at a given parametric
//               point t.
////////////////////////////////////////////////////////////////////
bool PiecewiseCurve::
get_tangent(double t, LVector3f &tangent) const {
  const ParametricCurve *curve;
  bool result = find_curve(curve, t);

  // We use | instead of || so we won't short-circuit this calculation.
  return result | curve->get_tangent(t, tangent);
}


////////////////////////////////////////////////////////////////////
//     Function: PiecewiseCurve::get_2ndtangent
//       Access: Public, Scheme, Virtual
//  Description: Returns the tangent of the first derivative of the
//               curve at the point t.
////////////////////////////////////////////////////////////////////
bool PiecewiseCurve::
get_2ndtangent(double t, LVector3f &tangent2) const {
  const ParametricCurve *curve;
  bool result = find_curve(curve, t);

  // We use | instead of || so we won't short-circuit this calculation.
  return result | curve->get_2ndtangent(t, tangent2);
}

////////////////////////////////////////////////////////////////////
//     Function: PiecewiseCurve::adjust_point
//       Access: Public, Scheme
//  Description: Recomputes the curve such that it passes through the
//               point (px, py, pz) at time t, but keeps the same
//               tangent value at that point.
////////////////////////////////////////////////////////////////////
bool PiecewiseCurve::
adjust_point(double t,
	     float px, float py, float pz) {
  const ParametricCurve *curve;
  bool result = find_curve(curve, t);

  if (!result) {
    cerr << "No curve segment at t = " << t << "\n";
    return false;
  }

  rebuild_curveseg(RT_CV | RT_KEEP_ORIG, 0.0, LVector4f(),
		   RT_POINT, t, LVector4f(px, py, pz, 1.0),
		   RT_TANGENT | RT_KEEP_ORIG, t, LVector4f(),
		   RT_CV | RT_KEEP_ORIG, 0.0, LVector4f());
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PiecewiseCurve::adjust_tangent
//       Access: Public, Scheme
//  Description: Recomputes the curve such that it has the tangent
//               (tx, ty, tz) at time t, but keeps the same position
//               at the point.
////////////////////////////////////////////////////////////////////
bool PiecewiseCurve::
adjust_tangent(double t,
	       float tx, float ty, float tz) {
  const ParametricCurve *curve;
  bool result = find_curve(curve, t);

  if (!result) {
    cerr << "No curve segment at t = " << t << "\n";
    return false;
  }

  rebuild_curveseg(RT_CV | RT_KEEP_ORIG, 0.0, LVector4f(),
		   RT_POINT | RT_KEEP_ORIG, t, LVector4f(),
		   RT_TANGENT, t, LVector4f(tx, ty, tz, 0.0),
		   RT_CV | RT_KEEP_ORIG, 0.0, LVector4f());
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PiecewiseCurve::adjust_pt
//       Access: Public, Scheme
//  Description: Recomputes the curve such that it passes through the
//               point (px, py, pz) with the tangent (tx, ty, tz).
////////////////////////////////////////////////////////////////////
bool PiecewiseCurve::
adjust_pt(double t,
	  float px, float py, float pz,
	  float tx, float ty, float tz) {
  const ParametricCurve *curve;
  bool result = find_curve(curve, t);

  if (!result) {
    cerr << "No curve segment at t = " << t << "\n";
    return false;
  }

  rebuild_curveseg(RT_CV | RT_KEEP_ORIG, 0.0, LVector4f(),
		   RT_POINT, t, LVector4f(px, py, pz, 1.0),
		   RT_TANGENT, t, LVector4f(tx, ty, tz, 0.0),
		   RT_CV | RT_KEEP_ORIG, 0.0, LVector4f());
  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: PiecewiseCurve::get_pt
//       Access: Public, Scheme, Virtual
//  Description: Simultaneously returns the point and tangent of the
//               curve at a given parametric point t.
////////////////////////////////////////////////////////////////////
bool PiecewiseCurve::
get_pt(double t, LVector3f &point, LVector3f &tangent) const {
  const ParametricCurve *curve;
  bool result = find_curve(curve, t);

  // We use | instead of || so we won't short-circuit this calculation.
  return result | curve->get_pt(t, point, tangent);
}


////////////////////////////////////////////////////////////////////
//     Function: PiecewiseCurve::get_num_segs
//       Access: Public
//  Description: Returns the number of curve segments that make up the
//               Piecewise curve.
////////////////////////////////////////////////////////////////////
int PiecewiseCurve::
get_num_segs() const {
  return _segs.size();
}

////////////////////////////////////////////////////////////////////
//     Function: PiecewiseCurve::get_curveseg
//       Access: Public
//  Description: Returns the curve segment corresponding to the given
//               index.
////////////////////////////////////////////////////////////////////
ParametricCurve *PiecewiseCurve::
get_curveseg(int ti) {
  assert(ti>=0 && ti<_segs.size());
  return _segs[ti]._curve;
}


////////////////////////////////////////////////////////////////////
//     Function: PiecewiseCurve::insert_curveseg
//       Access: Public
//  Description: Inserts a new curve segment at the indicated index.
//               The curve segment must have been allocated via
//               new; it will be freed using delete when it is removed
//               or the PiecewiseCurve destructs.
//
//               If the curve segment is not inserted at the end, its
//               tlength is subtracted from that of the following
//               segment, so that the overall length of the curve is
//               not changed.
////////////////////////////////////////////////////////////////////
bool PiecewiseCurve::
insert_curveseg(int ti, ParametricCurve *seg, double tlength) {
  if (ti<0 || ti>_segs.size()) {
    return false;
  }

  if (ti==_segs.size()) {
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


////////////////////////////////////////////////////////////////////
//     Function: PiecewiseCurve::remove_curveseg
//       Access: Public
//  Description: Removes the given curve segment from the curve and
//               frees it.  Returns true if the segment was defined,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool PiecewiseCurve::
remove_curveseg(int ti) {
  if (ti<0 || ti>=_segs.size()) {
    return false;
  }

  int tlength = get_tlength(ti);
  _segs.erase(_segs.begin() + ti);

  // Now update the _tend figures for everything after the one we
  // removed.
  while (ti < _segs.size()) {
    _segs[ti]._tend -= tlength;
    ti++;
  }

  _last_ti = 0;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PiecewiseCurve::remove_all_curvesegs
//       Access: Public
//  Description: Removes all curve segments from the curve.
////////////////////////////////////////////////////////////////////
void PiecewiseCurve::
remove_all_curvesegs() {
  _segs.erase(_segs.begin(), _segs.end());
  _last_ti = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: PiecewiseCurve::get_tlength
//       Access: Public
//  Description: Returns the parametric length of the given segment of
//               the curve.
////////////////////////////////////////////////////////////////////
double PiecewiseCurve::
get_tlength(int ti) const {
  assert(ti>=0 && ti<_segs.size());
  return (ti==0) ? _segs[ti]._tend : _segs[ti]._tend - _segs[ti-1]._tend;
}

////////////////////////////////////////////////////////////////////
//     Function: PiecewiseCurve::get_tstart
//       Access: Public
//  Description: Returns the parametric start of the given segment of
//               the curve.
////////////////////////////////////////////////////////////////////
double PiecewiseCurve::
get_tstart(int ti) const {
  assert(ti>=0 && ti<=_segs.size());
  return (ti==0) ? 0.0 : _segs[ti-1]._tend;
}

////////////////////////////////////////////////////////////////////
//     Function: PiecewiseCurve::get_tend
//       Access: Public
//  Description: Returns the parametric end of the given segment of
//               the curve.
////////////////////////////////////////////////////////////////////
double PiecewiseCurve::
get_tend(int ti) const {
  assert(ti>=0 && ti<_segs.size());
  return _segs[ti]._tend;
}


////////////////////////////////////////////////////////////////////
//     Function: PiecewiseCurve::set_tlength
//       Access: Public
//  Description: Sets the parametric length of the given segment of
//               the curve.  The length of the following segment is
//               lengthened by the corresponding amount to keep the
//               overall length of the curve the same.
////////////////////////////////////////////////////////////////////
bool PiecewiseCurve::
set_tlength(int ti, double tlength) {
  if (ti<0 || ti>=_segs.size()) {
    return false;
  }

  _segs[ti]._tend += tlength - get_tlength(ti);
  return true;
}



////////////////////////////////////////////////////////////////////
//     Function: PiecewiseCurve::make_nurbs
//       Access: Public
//  Description: Defines the curve as a general NURBS curve.  The
//               order is the degree plus one and must be 1, 2, 3, or
//               4; cvs is an array of num_cvs points each with a
//               homogeneous coordinate; knots is an array of
//               num_cvs+order knot values.
//
//               This creates the individual curve segments and sets
//               up the basis matrices, but does not store the CV's or
//               knot values so the curve shape is not later
//               modifiable.
////////////////////////////////////////////////////////////////////
void PiecewiseCurve::
make_nurbs(int order, int num_cvs,
	   const double knots[], const LVector4f cvs[]) {
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


////////////////////////////////////////////////////////////////////
//     Function: PiecewiseCurve::GetBezierSegs
//       Access: Public, Virtual
//  Description: Fills up the indicated vector with a list of
//               BezierSeg structs that describe the curve.  This
//               assumes the curve is a PiecewiseCurve of
//               CubicCurvesegs.  Returns true if successful, false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool PiecewiseCurve::
GetBezierSegs(BezierSegs &bz_segs) const {
  bz_segs.erase(bz_segs.begin(), bz_segs.end());
  int i;
  BezierSeg seg;
  for (i = 0; i < _segs.size(); i++) {
    if (!_segs[i]._curve->GetBezierSeg(seg)) {
      return false;
    }
    seg._t = _segs[i]._tend;
    bz_segs.push_back(seg);
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PiecewiseCurve::rebuild_curveseg
//       Access: Public, Virtual
//  Description: Rebuilds the current curve segment (as selected by
//               the most recent call to find_curve()) according to
//               the specified properties (see
//               CubicCurveseg::compute_seg).  Returns true if
//               possible, false if something goes horribly wrong.
////////////////////////////////////////////////////////////////////
bool PiecewiseCurve::
rebuild_curveseg(int, double, const LVector4f &,
		 int, double, const LVector4f &,
		 int, double, const LVector4f &,
		 int, double, const LVector4f &) {
  cerr << "rebuild_curveseg not implemented for this curve type.\n";
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PiecewiseCurve::Destructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
PiecewiseCurve::
~PiecewiseCurve() {
  remove_all_curvesegs();
}

////////////////////////////////////////////////////////////////////
//     Function: PiecewiseCurve::find_curve
//       Access: Protected
//  Description: Finds the curve corresponding to the given value of
//               t.  If t is inside the curve's defined range, sets
//               curve to the appropriate segment, translates t to
//               [0,1] to index into the segment's coordinate system,
//               and returns true.  If t is outside the curve's
//               defined range, sets curve to the nearest segment and
//               t to the nearest point on this segment, and returns
//               false.
////////////////////////////////////////////////////////////////////
bool PiecewiseCurve::
find_curve(const ParametricCurve *&curve, double &t) const {
  // Check the index computed by the last call to find_curve().  If
  // it's still a reasonable starting value, start searching from
  // there.  This way, we take advantage of locality of reference: the
  // search is trivial it is the same segment as last time, or the
  // next segment after the last one.
  if (_last_ti>0 && _segs[_last_ti-1]._tend>=t) {
    // However, if the new t value precedes that of last time, we'll
    // have to start over.

    // We do some messy casting so we can get away with assigning a
    // value to a member within a const function.  This assignment
    // doesn't really count as a const violation since we're just
    // updating a cached value, not changing any real data of the
    // class.
    ((PiecewiseCurve *)this)->_last_ti = 0;
  }

  int ti;
  for (ti = _last_ti; ti<_segs.size(); ti++) {
    if (_segs[ti]._tend+0.00001 > t) {
      break;
    }
  }

  if (ti < _segs.size()) {
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
    t = 0.0;
    return false;
  }

  if (ti >= _segs.size() || !_segs[ti]._curve->is_valid()) {
    assert(ti <= _segs.size());

    // If we're out of bounds, or the curve is undefined, we're probably
    // screwed.  There's one exception: if we were right on a border between
    // curves, try the curve before.

    if (ti > 0 && t < _segs[ti-1]._tend+0.0001) {
      ti--;
      t = 1.0;
    }

    if (ti >= _segs.size()) {
      if (_segs.empty()) {
	curve = NULL;
	t = 0.0;
	return false;
      } else {
	curve = _segs.back()._curve;
	t = 1.0;
	return false;
      }
    } else if (!_segs[ti]._curve->is_valid()) {
      curve = _segs[ti]._curve;
      return false;
    }
  }

  // Again, some messy casting so we can get away with updating the
  // cached index value for next time.
  ((PiecewiseCurve *)this)->_last_ti = ti;

  // Now scale t back into the curve's own valid range.
  t *= _segs[ti]._curve->get_max_t();
  curve = _segs[ti]._curve;
  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: PiecewiseCurve::current_seg_range
//       Access: Protected
//  Description: Returns a number in the range [0,1], representing the
//               conversion of t into the current segment's coordinate
//               system (the segment last returned by find_curve).
//               This operation is already performed automatically on
//               the t passed into find_seg; this function is useful
//               only to adjust a different value into the same range.
//
//               It is an error to call this function if find_curve()
//               has not yet been called, or if find_curve() returned
//               false from its previous call.
////////////////////////////////////////////////////////////////////
double PiecewiseCurve::
current_seg_range(double t) const {
  int ti = _last_ti;

  assert(ti < _segs.size());

  // Adjust t to the range [0,1).
  if (ti > 0) {
    t = (t - _segs[ti-1]._tend) / (_segs[ti]._tend - _segs[ti-1]._tend);
  } else {
    t /= _segs[0]._tend;
  }

  return t;
}

////////////////////////////////////////////////////////////////////
//     Function: CubicCurveseg::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CubicCurveseg::
CubicCurveseg() {
}

////////////////////////////////////////////////////////////////////
//     Function: CubicCurveseg::Constructor
//       Access: Public
//  Description: Creates the curveseg given the four basis vectors
//               (the columns of the matrix) explicitly.
////////////////////////////////////////////////////////////////////
CubicCurveseg::
CubicCurveseg(const LMatrix4f &basis) {
  Bx = basis.get_col(0);
  By = basis.get_col(1);
  Bz = basis.get_col(2);
  Bw = basis.get_col(3);
  rational = true;
}


////////////////////////////////////////////////////////////////////
//     Function: CubicCurveseg::Constructor
//       Access: Public
//  Description: Creates the curveseg as a Bezier segment.
////////////////////////////////////////////////////////////////////
CubicCurveseg::
CubicCurveseg(const BezierSeg &seg) {
  bezier_basis(seg);
}


////////////////////////////////////////////////////////////////////
//     Function: CubicCurveseg::Constructor
//       Access: Public
//  Description: Creates the curveseg as a NURBS segment.  See
//               nurbs_basis for a description of the parameters.
////////////////////////////////////////////////////////////////////
CubicCurveseg::
CubicCurveseg(int order, const double knots[], const LVector4f cvs[]) {
  nurbs_basis(order, knots, cvs);
}



////////////////////////////////////////////////////////////////////
//     Function: CubicCurveseg::get_point
//       Access: Public, Scheme, Virtual
//  Description: Computes the surface point at a given parametric
//               point t.
////////////////////////////////////////////////////////////////////
bool CubicCurveseg::
get_point(double t, LVector3f &point) const {
  evaluate_point(LVector4f(t*t*t, t*t, t, 1.0), point);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: CubicCurveseg::get_tangent
//       Access: Public, Scheme, Virtual
//  Description: Computes the surface tangent at a given parametric
//               point t.
////////////////////////////////////////////////////////////////////
bool CubicCurveseg::
get_tangent(double t, LVector3f &tangent) const {
  evaluate_vector(LVector4f(3.0*t*t, 2.0*t, 1.0, 0.0), tangent);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: CubicCurveseg::get_pt
//       Access: Public, Scheme, Virtual
//  Description: Simultaneously computes the point and the tangent at
//               the given parametric point.
////////////////////////////////////////////////////////////////////
bool CubicCurveseg::
get_pt(double t, LVector3f &point, LVector3f &tangent) const {
  evaluate_point(LVector4f(t*t*t, t*t, t, 1.0), point);
  evaluate_vector(LVector4f(3.0*t*t, 2.0*t, 1.0, 0.0), tangent);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: CubicCurveseg::get_2ndtangent
//       Access: Public, Scheme, Virtual
//  Description: Computes the surface 2nd-order tangent at a given
//               parametric point t.
////////////////////////////////////////////////////////////////////
bool CubicCurveseg::
get_2ndtangent(double t, LVector3f &tangent2) const {
  evaluate_vector(LVector4f(6.0*t, 2.0, 0.0, 0.0), tangent2);
  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: CubicCurveseg::bezier_basis
//       Access: Public
//  Description: Defines the curve segment as a Bezier.  This only
//               sets up the basis vectors, so the curve will be
//               computed correctly; it does not retain the CV's.
////////////////////////////////////////////////////////////////////
void CubicCurveseg::
bezier_basis(const BezierSeg &seg) {
  static LMatrix4f
    Mb(-1, 3, -3, 1,
       3, -6, 3, 0,
       -3, 3, 0, 0,
       1, 0, 0, 0);

  LVector4f Gx(seg._v[0][0], seg._v[1][0], seg._v[2][0], seg._v[3][0]);
  LVector4f Gy(seg._v[0][1], seg._v[1][1], seg._v[2][1], seg._v[3][1]);
  LVector4f Gz(seg._v[0][2], seg._v[1][2], seg._v[2][2], seg._v[3][2]);

  Bx = Gx * Mb;
  By = Gy * Mb;
  Bz = Gz * Mb;
  rational = false;
}

static LVector4f
nurbs_blending_function(int order, int i, int j,
			const double knots[]) {
  // This is doubly recursive.  Ick.
  LVector4f r;

  if (j==1) {
    if (i==order-1 && knots[i] < knots[i+1]) {
      r.set(0.0, 0.0, 0.0, 1.0);
    } else {
      r.set(0.0, 0.0, 0.0, 0.0);
    }

  } else {
    LVector4f bi0 = nurbs_blending_function(order, i, j-1, knots);
    LVector4f bi1 = nurbs_blending_function(order, i+1, j-1, knots);

    float d0 = knots[i+j-1] - knots[i];
    float d1 = knots[i+j] - knots[i+1];

    // First term.  Division by zero is defined to equal zero.
    if (d0 != 0.0) {
      if (d1 != 0.0) {
	r = bi0 / d0 - bi1 / d1;
      } else {
	r = bi0 / d0;
      }

    } else if (d1 != 0.0) {
      r = - bi1 / d1;

    } else {
      r.set(0.0, 0.0, 0.0, 0.0);
    }

    // scale by t.
    r[0] = r[1];
    r[1] = r[2];
    r[2] = r[3];
    r[3] = 0.0;

    // Second term.
    if (d0 != 0.0) {
      if (d1 != 0.0) {
	r += bi0 * (- knots[i] / d0) + bi1 * (knots[i+j] / d1);
      } else {
	r += bi0 * (- knots[i] / d0);
      }

    } else if (d1 != 0.0) {
      r += bi1 * (knots[i+j] / d1);
    }
  }

  return r;
}

void
compute_nurbs_basis(int order,
		    const double knots_in[],
		    LMatrix4f &basis) {
  int i;

  // Scale the supplied knots to the range 0..1.
  double knots[8];
  double mink = knots_in[order-1];
  double maxk = knots_in[order];

  if (mink==maxk) {
    // Huh.  What were you thinking?  This is a trivial NURBS.
    parametrics_cat->warning()
      << "Trivial NURBS curve specified." << endl;
    memset((void *)&basis, 0, sizeof(LMatrix4f));
    return;
  }

  for (i = 0; i<2*order; i++) {
    knots[i] = (knots_in[i] - mink) / (maxk-mink);
  }


  LVector4f b[4];
  for (i = 0; i<order; i++) {
    b[i] = nurbs_blending_function(order, i, order, knots);
  }

  for (i = 0; i<order; i++) {
    basis.set_row(i, b[i]);
  }

  for (i=order; i<4; i++) {
    basis.set_row(i, LVector4f::zero());
  }
}



////////////////////////////////////////////////////////////////////
//     Function: CubicCurveseg::nurbs_basis
//       Access: Public
//  Description: Defines the curve segment as a NURBS.  Order is one
//               more than the degree, and must be 1, 2, 3, or 4;
//               knots is an array of order*2 values, and cvs is an
//               array of order values.
////////////////////////////////////////////////////////////////////
void CubicCurveseg::
nurbs_basis(int order, const double knots[], const LVector4f cvs[]) {
  assert(order>=1 && order<=4);

  LMatrix4f B;
  compute_nurbs_basis(order, knots, B);

  // Create a local copy of our CV's, so we can zero out the unused
  // elements.
  LVector4f c[4];
  for (int i = 0; i < 4; i++) {
    c[i] = (i<order) ? cvs[i] : LVector4f(0.0, 0.0, 0.0, 0.0);
  }

  Bx = LVector4f(c[0][0], c[1][0], c[2][0], c[3][0]) * B;
  By = LVector4f(c[0][1], c[1][1], c[2][1], c[3][1]) * B;
  Bz = LVector4f(c[0][2], c[1][2], c[2][2], c[3][2]) * B;
  Bw = LVector4f(c[0][3], c[1][3], c[2][3], c[3][3]) * B;

  rational = true;
}

////////////////////////////////////////////////////////////////////
//     Function: CubicCurveseg::GetBezierSeg
//       Access: Public, Virtual
//  Description: Fills the BezierSeg structure with a description of
//               the curve segment as a Bezier, if possible, but does
//               not change the _t member of the structure.  Returns
//               true if successful, false otherwise.
////////////////////////////////////////////////////////////////////
bool CubicCurveseg::
GetBezierSeg(BezierSeg &seg) const {
  static LMatrix4f
    Mbi(0.0, 0.0, 0.0, 1.0,
	0.0, 0.0, 1.0/3.0, 1.0,
	0.0, 1.0/3.0, 2.0/3.0, 1.0,
	1.0, 1.0, 1.0, 1.0);

  LVector4f Gx = Bx * Mbi;
  LVector4f Gy = By * Mbi;
  LVector4f Gz = Bz * Mbi;

  if (rational) {
    LVector4f Gw = Bw * Mbi;
    seg._v[0].set(Gx[0]/Gw[0], Gy[0]/Gw[0], Gz[0]/Gw[0]);
    seg._v[1].set(Gx[1]/Gw[1], Gy[1]/Gw[1], Gz[1]/Gw[1]);
    seg._v[2].set(Gx[2]/Gw[2], Gy[2]/Gw[2], Gz[2]/Gw[2]);
    seg._v[3].set(Gx[3]/Gw[3], Gy[3]/Gw[3], Gz[3]/Gw[3]);
  } else {
    seg._v[0].set(Gx[0], Gy[0], Gz[0]);
    seg._v[1].set(Gx[1], Gy[1], Gz[1]);
    seg._v[2].set(Gx[2], Gy[2], Gz[2]);
    seg._v[3].set(Gx[3], Gy[3], Gz[3]);
  }

  return true;
}


// We need this operator since Performer didn't supply it.
inline LVector4f
operator * (const LMatrix4f &M, const LVector4f &v) {
  return LVector4f(M(0,0)*v[0] + M(0,1)*v[1] + M(0,2)*v[2] + M(0,3)*v[3],
		M(1,0)*v[0] + M(1,1)*v[1] + M(1,2)*v[2] + M(1,3)*v[3],
		M(2,0)*v[0] + M(2,1)*v[1] + M(2,2)*v[2] + M(2,3)*v[3],
		M(3,0)*v[0] + M(3,1)*v[1] + M(3,2)*v[2] + M(3,3)*v[3]);
}

////////////////////////////////////////////////////////////////////
//     Function: compute_seg_col
//  Description: Interprets the parameters for a particular column of
//               compute_seg.  Builds the indicated column of T
//               and P.
////////////////////////////////////////////////////////////////////
static bool
compute_seg_col(int c,
		int rtype, double t, const LVector4f &v,
		const LMatrix4f &B,
		const LMatrix4f &Bi,
		const LMatrix4f &G,
		const LMatrix4f &GB,
		LMatrix4f &T, LMatrix4f &P) {
  int keep_orig = (rtype & RT_KEEP_ORIG);

  switch (rtype & RT_BASE_TYPE) {
    // RT_point defines the point on the curve at t.  This is the vector
    // [ t^3 t^2 t^1 t^0 ].
  case RT_POINT:
    T.set_col(c, LVector4f(t*t*t, t*t, t, 1.0));
    if (keep_orig) {
      LVector4f ov = GB * LVector4f(t*t*t, t*t, t, 1.0);

      P.set_col(c, ov);
    } else {
      P.set_col(c, v);
    }
    break;

    // RT_tangent defines the tangent to the curve at t.  This is
    // the vector [ 3t^2 2t 1 0 ].
  case RT_TANGENT:
    T.set_col(c, LVector4f(3.0*t*t, 2.0*t, 1.0, 0.0));
    if (keep_orig) {
      LVector4f ov = GB * LVector4f(3.0*t*t, 2.0*t, 1.0, 0.0);
      P.set_col(c, ov);
    } else {
      P.set_col(c, v);
    }
    break;

    // RT_cv defines the cth control point.  This is the cth column
    // vector from Bi.
  case RT_CV:
    T.set_col(c, Bi.get_col(c));
    if (keep_orig) {
      P.set_col(c, G.get_col(c));
    } else {
      P.set_col(c, v);
    }
    break;

  default:
    cerr << "Invalid rebuild type in compute_seg\n";
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: CubicCurveseg::compute_seg
//       Access: Public, Static
//  Description: Given a set of four properties of a curve segment
//               (e.g. four points, four tangent values, four control
//               points, or any combination), and a basis matrix,
//               computes the corresponding geometry matrix that
//               (together with the basis matrix) represents the curve
//               that satisfies the four properties.
//
//               The basis matrix is passed in as B, and its inverse
//               must be precomputed and passed in as Bi.
//
//               The result is returned in the matrix G, each column
//               of which represents the cth control vertex.  If any
//               of the four properties has RT_KEEP_ORIG set (see
//               below), G's input value is used to define the
//               original shape of the curve; otherwise, G's input
//               value is ignored.
//
//               Each property is defined by an rtype, which may be
//               any of RT_POINT, RT_TANGENT, or RT_CV, and may or may
//               not be or'ed with RT_KEEP_ORIG.  The meanings of the
//               types are as follows:
//
//               RT_POINT defines a specific point which the curve
//               segment must pass through.  t is in the range [0,1]
//               and represents the parametric value at which the
//               curve segment will intersect the given point.  If
//               RT_KEEP_ORIG is not set, v defines the point;
//               otherwise, v is ignored and the original curve at
//               point t defines the point.
//
//               RT_TANGENT defines a specific tangent value which the
//               curve segment must have at point t.  As with
//               RT_POINT, if RT_KEEP_ORIG is not set, v defines the
//               tangent; otherwise, v is ignored and the original
//               curve defines the tangent.
//
//               RT_CV defines a specific control vertex which the
//               curve segment must have.  In this case, t is ignored.
//               The position within the argument list determines
//               which control vertex is applicable; e.g. rtype0 =
//               RT_CV defines control vertex 0, and rtype2 = RT_CV
//               defines control vertex 2.  If RT_KEEP_ORIG is not
//               set, v defines the new control vertex; otherwise, the
//               control vertex is taken from G.
//
//               The return value is true if all the parameters are
//               sensible, or false if there is some error.
////////////////////////////////////////////////////////////////////
bool CubicCurveseg::
compute_seg(int rtype0, double t0, const LVector4f &v0,
	    int rtype1, double t1, const LVector4f &v1,
	    int rtype2, double t2, const LVector4f &v2,
	    int rtype3, double t3, const LVector4f &v3,
	    const LMatrix4f &B,
	    const LMatrix4f &Bi,
	    LMatrix4f &G) {

  // We can define a cubic curve segment given four arbitrary
  // properties of the segment: any point along the curve, any tangent
  // along the curve, any control point.  Given any four such
  // properties, a single cubic curve segment is defined.

  // For a given cubic curve segment so defined, and given a basis
  // matrix B, we can define the four control vertices that represent
  // the segment with the basis matrix.  That is, we can define the
  // matrix G such that G * B * tc, where tc is [ t^3 t^2 t^1 t^0 ]
  // for t in [ 0..1 ], represents the point on the curve segment
  // corresponding to t.

  // First, we build a matrix T, such that each of the four columns of
  // T contains the vector that would compute the corresponding
  // property.  We also build a corresponding matrix P, such that each
  // of its columns contains the vector that is the solution of the
  // corresponding column in T.

  LMatrix4f T, P, GB;

  // GB is G * B, but we only need to compute this if any of the
  // columns wants the value from the original G.
  if ((rtype0 | rtype1 | rtype2 | rtype3) & RT_KEEP_ORIG) {
    GB = G * B;
  }

  if (! (compute_seg_col(0, rtype0, t0, v0, B, Bi, G, GB, T, P) &&
	 compute_seg_col(1, rtype1, t1, v1, B, Bi, G, GB, T, P) &&
	 compute_seg_col(2, rtype2, t2, v2, B, Bi, G, GB, T, P) &&
	 compute_seg_col(3, rtype3, t3, v3, B, Bi, G, GB, T, P))) {
    return false;
  }

  LMatrix4f Ti;
  Ti = invert(T);

  // Now we have T and P such that P represents the solution of T,
  // when T is applied to the geometry and basis matrices.  That is,
  // each column of P represents the solution computed by the
  // corresponding column of T.  P = G * B * T.

  // We simply solve for G and get G = P * T^(-1) * B^(-1).

  G = P * Ti * Bi;

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: CubicCurveseg::Destructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
CubicCurveseg::
~CubicCurveseg() {
}
