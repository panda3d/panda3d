// Filename: nurbsPPCurve.cxx
// Created by:  drose (01Mar01)
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

#include "nurbsPPCurve.h"
#include "config_parametrics.h"

#include "indent.h"


TypeHandle NurbsPPCurve::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: NurbsPPCurve::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
NurbsPPCurve::
NurbsPPCurve() {
  _nurbs_valid = false;
  _order = 4;
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsPPCurve::Copy Constructor
//       Access: Published
//  Description: Constructs a NURBS curve equivalent to the indicated
//               (possibly non-NURBS) curve.
////////////////////////////////////////////////////////////////////
NurbsPPCurve::
NurbsPPCurve(const ParametricCurve &pc) {
  _nurbs_valid = false;
  _order = 4;

  if (!pc.convert_to_nurbs(this)) {
    parametrics_cat->warning()
      << "Cannot make a NURBS from the indicated curve.\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsPPCurve::Constructor
//       Access: Published
//  Description: Constructs a NURBS curve according to the indicated
//               NURBS parameters.
////////////////////////////////////////////////////////////////////
NurbsPPCurve::
NurbsPPCurve(int order, int num_cvs,
             const float knots[], const LVecBase4f cvs[]) {
  _nurbs_valid = false;
  _order = order;

  _points = Points(cvs, cvs + num_cvs);
  _knots = Knots(knots, knots + num_cvs + _order);
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsPPCurve::Destructor
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
NurbsPPCurve::
~NurbsPPCurve() {
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsPPCurve::get_max_t
//       Access: Published, Virtual
//  Description: Returns the upper bound of t for the entire curve.
//               The curve is defined in the range 0.0f <= t <=
//               get_max_t().
////////////////////////////////////////////////////////////////////
float NurbsPPCurve::
get_max_t() const {
  if (_nurbs_valid) {
    return _nurbs.maxKnot();

  } else {
    if (_knots.empty()) {
      return 0.0f;
    } else {
      return _knots.back();
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsPPCurve::set_order
//       Access: Published, Virtual
//  Description: Changes the order of the curve.  Must be a value from
//               1 to 4.  Can only be done when there are no cv's.
////////////////////////////////////////////////////////////////////
void NurbsPPCurve::
set_order(int order) {
  nassertv(order >= 1 && order <= 4);
  nassertv(!_nurbs_valid && _points.empty());

  _order = order;
}


////////////////////////////////////////////////////////////////////
//     Function: NurbsPPCurve::get_order
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
int NurbsPPCurve::
get_order() const {
  if (_nurbs_valid) {
    return _nurbs.degree() + 1;
  } else {
    return _order;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: NurbsPPCurve::get_num_cvs
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
int NurbsPPCurve::
get_num_cvs() const {
  if (_nurbs_valid) {
    return _nurbs.ctrlPnts().size();
  } else {
    return _points.size();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsPPCurve::get_num_knots
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
int NurbsPPCurve::
get_num_knots() const {
  if (_nurbs_valid) {
    return _nurbs.knot().size();
  } else {
    return _knots.size();
  }
}


////////////////////////////////////////////////////////////////////
//     Function: NurbsPPCurve::insert_cv
//       Access: Published, Virtual
//  Description: Inserts a new CV into the middle of the curve at the
//               indicated parametric value.  This doesn't change the
//               shape or timing of the curve; however, it is
//               irreversible: if the new CV is immediately removed,
//               the curve will be changed.  Returns true on success,
//               false on failure.
////////////////////////////////////////////////////////////////////
bool NurbsPPCurve::
insert_cv(float t) {
  if (!make_nurbs_valid()) {
    return false;
  }

  PLib::NurbsCurvef result;
  _nurbs.knotInsertion(t, 1, result);
  _nurbs = result;
  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: NurbsPPCurve::remove_cv
//       Access: Published, Virtual
//  Description: Removes the indicated CV from the curve.  Returns
//               true if the CV index was valid, false otherwise.
////////////////////////////////////////////////////////////////////
bool NurbsPPCurve::
remove_cv(int n) {
  if (_nurbs_valid) {
    _nurbs.removeKnot(n + _order, 1, 1);

  } else {
    if (n < 0 || n >= (int)_points.size()) {
      return false;
    }

    _points.erase(_points.begin() + n);
    if (_points.empty()) {
      _knots.clear();
    } else {
      _knots.erase(_knots.begin() + _order + n);
    }
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsPPCurve::remove_all_cvs
//       Access: Published, Virtual
//  Description: Removes all CV's from the curve.
////////////////////////////////////////////////////////////////////
void NurbsPPCurve::
remove_all_cvs() {
  _nurbs_valid = false;
  _points.clear();
  _knots.clear();
}


////////////////////////////////////////////////////////////////////
//     Function: NurbsPPCurve::set_cv
//       Access: Published, Virtual
//  Description: Repositions the indicated CV.  Returns true if
//               successful, false otherwise.
////////////////////////////////////////////////////////////////////
bool NurbsPPCurve::
set_cv(int n, const LVecBase4f &v) {
  nassertr(n >= 0 && n < get_num_cvs(), false);

  if (_nurbs_valid) {
    _nurbs.modCP(n, PLib::HPoint3Df(v[0], v[1], v[2], v[3]));
  } else {
    _points[n] = v;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsPPCurve::get_cv
//       Access: Published, Virtual
//  Description: Returns the position in homogeneous space of the
//               indicated CV.
////////////////////////////////////////////////////////////////////
LVecBase4f NurbsPPCurve::
get_cv(int n) const {
  nassertr(n >= 0 && n < get_num_cvs(), LVecBase4f::zero());

  if (_nurbs_valid) {
    PLib::HPoint3Df p = _nurbs.ctrlPnts(n);
    return LVecBase4f(p.x(), p.y(), p.z(), p.w());
  } else {
    return _points[n];
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsPPCurve::set_knot
//       Access: Published, Virtual
//  Description: Sets the value of the indicated knot directly.
////////////////////////////////////////////////////////////////////
bool NurbsPPCurve::
set_knot(int n, float t) {
  nassertr(n >= 0 && n < get_num_knots(), false);

  make_arrays_valid();
  nassertr(n >= 0 && n < (int)_knots.size(), false);
  _knots[n] = t;

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsPPCurve::get_knot
//       Access: Published, Virtual
//  Description: Returns the nth knot value on the curve.
////////////////////////////////////////////////////////////////////
float NurbsPPCurve::
get_knot(int n) const {
  nassertr(n >= 0 && n < get_num_knots(), 0.0f);

  if (_nurbs_valid) {
    return _nurbs.knot(n);
  } else {
    nassertr(n >= 0 && n < (int)_knots.size(), 0.0f);
    return _knots[n];
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsPPCurve::recompute
//       Access: Published, Virtual
//  Description: Recalculates the curve basis according to the latest
//               position of the CV's, knots, etc.  Until this
//               function is called, adjusting the NURBS parameters
//               will have no visible effect on the curve.  Returns
//               true if the resulting curve is valid, false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool NurbsPPCurve::
recompute() {
  return make_nurbs_valid();
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsPPCurve::get_point
//       Access: Public, Virtual
//  Description: Returns the point of the curve at a given parametric
//               point t.  Returns true if t is in the valid range 0.0f
//               <= t <= get_max_t(); if t is outside this range, sets
//               point to the value of the curve at the beginning or
//               end (whichever is nearer) and returns false.
////////////////////////////////////////////////////////////////////
bool NurbsPPCurve::
get_point(float t, LVecBase3f &point) const {
  nassertr(_nurbs_valid, false);

  bool in_range = true;
  float max_t = get_max_t();

  if (t < 0.0f) {
    t = 0.0f;
    in_range = false;

  } else if (t > max_t) {
    t = max_t;
    in_range = false;
  }

  PLib::Point3Df p = Cp(t, _nurbs);
  point.set(p.x(), p.y(), p.z());
  return in_range;
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsPPCurve::get_tangent
//       Access: Public, Virtual
//  Description: Returns the tangent of the curve at a given parametric
//               point t.  Returns true if t is in the valid range 0.0f
//               <= t <= get_max_t(); if t is outside this range, sets
//               tangent to the value of the curve at the beginning or
//               end (whichever is nearer) and returns false.
////////////////////////////////////////////////////////////////////
bool NurbsPPCurve::
get_tangent(float t, LVecBase3f &tangent) const {
  nassertr(_nurbs_valid, false);

  bool in_range = true;
  float max_t = get_max_t();

  if (t < 0.0f) {
    t = 0.0f;
    in_range = false;

  } else if (t > max_t) {
    t = max_t;
    in_range = false;
  }

  PLib::Point3Df p = _nurbs.derive3D(t, 1);
  tangent.set(p.x(), p.y(), p.z());
  return in_range;
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsPPCurve::get_pt
//       Access: Public, Virtual
//  Description: Returns the both the point and the tangent
//               simultaneously.
////////////////////////////////////////////////////////////////////
bool NurbsPPCurve::
get_pt(float t, LVecBase3f &point, LVecBase3f &tangent) const {
  nassertr(_nurbs_valid, false);

  bool in_range = true;
  float max_t = get_max_t();

  if (t < 0.0f) {
    t = 0.0f;
    in_range = false;

  } else if (t > max_t) {
    t = max_t;
    in_range = false;
  }

  PLib::Point3Df p = Cp(t, _nurbs);
  point.set(p.x(), p.y(), p.z());
  p = _nurbs.derive3D(t, 1);
  tangent.set(p.x(), p.y(), p.z());
  return in_range;
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsPPCurve::get_2ndtangent
//       Access: Public, Virtual
//  Description: Returns the second derivative of the curve at a given
//               parametric point t.  Returns true if t is in the
//               valid range 0.0f <= t <= get_max_t(); if t is outside
//               this range, sets tangent to the value of the curve at
//               the beginning or end (whichever is nearer) and
//               returns false.
////////////////////////////////////////////////////////////////////
bool NurbsPPCurve::
get_2ndtangent(float t, LVecBase3f &tangent2) const {
  nassertr(_nurbs_valid, false);

  bool in_range = true;
  float max_t = get_max_t();

  if (t < 0.0f) {
    t = 0.0f;
    in_range = false;

  } else if (t > max_t) {
    t = max_t;
    in_range = false;
  }

  PLib::Point3Df p = _nurbs.derive3D(t, 2);
  tangent2.set(p.x(), p.y(), p.z());
  return in_range;
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsPPCurve::stitch
//       Access: Published, Virtual
//  Description: Regenerates this curve as one long curve: the first
//               curve connected end-to-end with the second one.
//               Either a or b may be the same as 'this'.
//
//               Returns true if successful, false on failure or if
//               the curve type does not support stitching.
////////////////////////////////////////////////////////////////////
bool NurbsPPCurve::
stitch(const ParametricCurve *a, const ParametricCurve *b) {
  // First, make a copy of both of our curves.  This ensures they are
  // of the correct type, and also protects us in case one of them is
  // the same as 'this'.
  PT(NurbsPPCurve) na = new NurbsPPCurve(*a);
  PT(NurbsPPCurve) nb = new NurbsPPCurve(*b);

  if (na->get_num_cvs() == 0 || nb->get_num_cvs() == 0) {
    return false;
  }

  if (!na->make_nurbs_valid()) {
    return false;
  }

  // First, translate curve B to move its first CV to curve A's last
  // CV.
  LVecBase3f point_offset =
    na->get_cv_point(na->get_num_cvs() - 1) - nb->get_cv_point(0);
  int num_b_cvs = nb->get_num_cvs();
  for (int i = 0; i < num_b_cvs; i++) {
    nb->set_cv_point(i, nb->get_cv_point(i) + point_offset);
  }

  // Now get the arrays from B and reparameterize them so that the
  // first knot value of B is the last knot value of A.
  Points b_points;
  Knots b_knots;
  int b_order;
  nb->copy_arrays(b_points, b_knots, b_order);
  nassertr(!b_knots.empty(), false)

  float knot_offset = na->get_max_t() - b_knots.front();
  Knots::iterator ki;
  for (ki = b_knots.begin(); ki != b_knots.end(); ++ki) {
    (*ki) += knot_offset;
  }

  // Now we can regenerate the other curve.
  PLib::NurbsCurvef b_nurbs;
  if (!make_nurbs_from(b_nurbs, b_points, b_knots, b_order)) {
    return false;
  }

  PLib::NurbsCurvef result;
  if (!result.mergeOf(na->_nurbs, b_nurbs)) {
    return false;
  }

  _nurbs = result;
  _nurbs_valid = true;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsPPCurve::get_nurbs_interface
//       Access: Public, Virtual
//  Description: Returns a pointer to the object as a
//               NurbsCurveInterface object if it happens to be a
//               NURBS-style curve; otherwise, returns NULL.
////////////////////////////////////////////////////////////////////
NurbsCurveInterface *NurbsPPCurve::
get_nurbs_interface() {
  return this;
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsPPCurve::convert_to_nurbs
//       Access: Protected, Virtual
//  Description: Stores in the indicated NurbsCurve a NURBS
//               representation of an equivalent curve.  Returns true
//               if successful, false otherwise.
////////////////////////////////////////////////////////////////////
bool NurbsPPCurve::
convert_to_nurbs(ParametricCurve *nc) const {
  nc->set_curve_type(_curve_type);
  return NurbsCurveInterface::convert_to_nurbs(nc);
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsPPCurve::write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void NurbsPPCurve::
write(ostream &out, int indent_level) const {
  NurbsCurveInterface::write(out, indent_level);
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsPPCurve::append_cv_impl
//       Access: Protected, Virtual
//  Description: Adds a new CV to the end of the curve.  Creates a new
//               knot value by adding 1 to the last knot value.
//               Returns the index of the new CV.
////////////////////////////////////////////////////////////////////
int NurbsPPCurve::
append_cv_impl(const LVecBase4f &cv) {
  make_arrays_valid();

  _points.push_back(cv);

  if (_knots.empty()) {
    for (int i = 0; i < _order; i++) {
      _knots.push_back(0.0f);
    }
    _knots.push_back(1.0f);
  } else {
    _knots.push_back(_knots.back() + 1.0f);
  }

  return _points.size() - 1;
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsPPCurve::format_egg
//       Access: Protected, Virtual
//  Description: Formats the curve as an egg structure to write to the
//               indicated stream.  Returns true on success, false on
//               failure.
////////////////////////////////////////////////////////////////////
bool NurbsPPCurve::
format_egg(ostream &out, const string &name, const string &curve_type,
           int indent_level) const {
  return NurbsCurveInterface::format_egg(out, name, curve_type, indent_level);
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsPPCurve::make_nurbs_valid
//       Access: Private
//  Description: Converts from the _knots and _points array
//               representation into the actual _nurbs representation.
//               Returns true if successful, false otherwise.
////////////////////////////////////////////////////////////////////
bool NurbsPPCurve::
make_nurbs_valid() {
  if (_nurbs_valid) {
    return true;
  }

  if (!make_nurbs_from(_nurbs, _points, _knots, _order)) {
    return false;
  }

  _nurbs_valid = true;
  _points.clear();
  _knots.clear();
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsPPCurve::make_arrays_valid
//       Access: Private
//  Description: Converts from the _nurbs representation to separate
//               _knots and _points arrays.
////////////////////////////////////////////////////////////////////
void NurbsPPCurve::
make_arrays_valid() {
  if (!_nurbs_valid) {
    return;
  }

  make_arrays_from(_nurbs, _points, _knots, _order);

  _nurbs_valid = false;
  _nurbs = PLib::NurbsCurvef();
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsPPCurve::copy_nurbs
//       Access: Private
//  Description: Fills nurbs up with a copy of the nurbs curve.
//               Returns true if valid, false otherwise.
////////////////////////////////////////////////////////////////////
bool NurbsPPCurve::
copy_nurbs(PLib::NurbsCurvef &nurbs) const {
  if (_nurbs_valid) {
    nurbs = _nurbs;
    return true;
  } else {
    return make_nurbs_from(nurbs, _points, _knots, _order);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsPPCurve::copy_arrays
//       Access: Private
//  Description: Fills the arrays up with a copy of the nurbs data.
////////////////////////////////////////////////////////////////////
void NurbsPPCurve::
copy_arrays(NurbsPPCurve::Points &points, NurbsPPCurve::Knots &knots,
            int &order) const {
  if (_nurbs_valid) {
    make_arrays_from(_nurbs, points, knots, order);
  } else {
    points = _points;
    knots = _knots;
    order = _order;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsPPCurve::make_nurbs_from
//       Access: Private, Static
//  Description: Makes a NURBS curve from the indicated array values.
//               Returns true if successful, false otherwise.
////////////////////////////////////////////////////////////////////
bool NurbsPPCurve::
make_nurbs_from(PLib::NurbsCurvef &nurbs,
                const NurbsPPCurve::Points &points,
                const NurbsPPCurve::Knots &knots, int order) {
  if (order < 1 || knots.size() != points.size() + order) {
    parametrics_cat.error()
      << "Invalid NURBS curve: order " << order << " with "
      << points.size() << " CV's and " << knots.size() << " knots.\n";
    nassertr(false, false);
    return false;
  }

  Vector_HPoint3Df v_points(points.size());
  Vector_FLOAT v_knots(knots.size());

  size_t i;
  for (i = 0; i < points.size(); i++) {
    const LVecBase4f &p = points[i];
    v_points[i] = PLib::HPoint3Df(p[0], p[1], p[2], p[3]);
  }
  for (i = 0; i < knots.size(); i++) {
    v_knots[i] = knots[i];
  }

  nassertr(v_knots.size() == v_points.size() + order, false);

  nurbs.reset(v_points, v_knots, order - 1);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsPPCurve::make_arrays_from
//       Access: Private, Static
//  Description: Fills up the array values from the given NURBS curve.
////////////////////////////////////////////////////////////////////
void NurbsPPCurve::
make_arrays_from(const PLib::NurbsCurvef &nurbs,
                 NurbsPPCurve::Points &points,
                 NurbsPPCurve::Knots &knots, int &order) {
  const Vector_HPoint3Df &v_points = nurbs.ctrlPnts();
  const Vector_FLOAT &v_knots = nurbs.knot();

  points.clear();
  knots.clear();

  points.reserve(v_points.size());
  knots.reserve(v_knots.size());

  Vector_HPoint3Df::const_iterator pi;
  for (pi = v_points.begin(); pi != v_points.end(); ++pi) {
    const PLib::HPoint3Df &p = (*pi);
    points.push_back(LVecBase4f(p.x(), p.y(), p.z(), p.w()));
  }
  Vector_FLOAT::const_iterator ki;
  for (ki = v_knots.begin(); ki != v_knots.end(); ++ki) {
    knots.push_back(*ki);
  }

  order = nurbs.degree() + 1;
}
