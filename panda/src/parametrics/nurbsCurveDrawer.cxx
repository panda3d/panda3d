// Filename: nurbsCurveDrawer.cxx
// Created by:  drose (27Feb98)
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

#include "nurbsCurveDrawer.h"
#include "nurbsCurveInterface.h"
#include "parametricCurve.h"


TypeHandle NurbsCurveDrawer::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveDrawer::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
NurbsCurveDrawer::
NurbsCurveDrawer() {
  set_cv_color(1.0f, 0.0f, 0.0f);
  set_hull_color(1.0f, 0.5, 0.5);
  set_knot_color(0.0f, 0.0f, 1.0f);

  _cvs.set_thickness(4.0f);
  _hull.set_thickness(1.0f);
  _knots.set_thickness(4.0f);

  _show_cvs = true;
  _show_hull = true;
  _show_knots = true;
}


////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveDrawer::Destructor
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
NurbsCurveDrawer::
~NurbsCurveDrawer() {
}



////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveDrawer::set_cv_color
//       Access: Published
//  Description: Specifies the color of the CV's.
////////////////////////////////////////////////////////////////////
void NurbsCurveDrawer::
set_cv_color(float r, float g, float b) {
  _cv_color.set(r, g, b);
  _cvs.set_color(r, g, b);

  redraw();
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveDrawer::set_knot_color
//       Access: Published
//  Description: Specifies the color of the knots.
////////////////////////////////////////////////////////////////////
void NurbsCurveDrawer::
set_knot_color(float r, float g, float b) {
  _knot_color.set(r, g, b);
  _knots.set_color(r, g, b);

  redraw();
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveDrawer::set_hull_color
//       Access: Published
//  Description: Specifies the color of the convex hull.
////////////////////////////////////////////////////////////////////
void NurbsCurveDrawer::
set_hull_color(float r, float g, float b) {
  _hull_color.set(r, g, b);
  _hull.set_color(r, g, b);

  redraw();
}



////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveDrawer::draw
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
bool NurbsCurveDrawer::
draw() {
  // First, draw the curve itself.
  if (!ParametricCurveDrawer::draw()) {
    return false;
  }

  ParametricCurve *curve = (ParametricCurve *)NULL;
  NurbsCurveInterface *nurbs = (NurbsCurveInterface *)NULL;

  if (_curves != (ParametricCurveCollection *)NULL) {
    curve = _curves->get_default_curve();

    if (curve != (ParametricCurve *)NULL) {
      nurbs = curve->get_nurbs_interface();
    }
  }

  if (nurbs == (NurbsCurveInterface *)NULL) {
    // The rest of this depends on having an actual NURBS curve.
    return true;
  }

  int i;
  if (_show_knots) {
    _num_cvs = nurbs->get_num_cvs();
    _knotnums.erase(_knotnums.begin(), _knotnums.end());

    float lt = -1.0f;
    int ki = -1;
    for (i = 0; i < _num_cvs; i++) {
      float t = nurbs->get_knot(i);
      if (t != lt) {
        lt = t;
        LVecBase3f knot_pos;
        curve->get_point(nurbs->get_knot(i), knot_pos);
        _knots.move_to(knot_pos);
        ki++;
      }
      _knotnums.push_back(ki);
    }

    _knots.create(_geom_node, _frame_accurate);
  }

  if (_show_cvs) {
    _num_cvs = nurbs->get_num_cvs();
    for (i = 0; i < _num_cvs; i++) {
      _cvs.move_to(nurbs->get_cv_point(i));
    }

    _cvs.create(_geom_node, _frame_accurate);
  }

  if (_show_hull) {
    _num_cvs = nurbs->get_num_cvs();
    for (i = 0; i < _num_cvs; i++) {
      _hull.draw_to(nurbs->get_cv_point(i));
    }

    _hull.create(_geom_node, _frame_accurate);
  }

  return true;
}



////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveDrawer::recompute
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
bool NurbsCurveDrawer::
recompute(float t1, float t2, ParametricCurve *curve) {
  return draw();
}


////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveDrawer::set_show_cvs
//       Access: Published
//  Description: Sets the flag that hides or shows the CV's.
////////////////////////////////////////////////////////////////////
void NurbsCurveDrawer::
set_show_cvs(bool flag) {
  _show_cvs = flag;
  redraw();
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveDrawer::get_show_cvs
//       Access: Published
//  Description: Returns the current state of the show-CV's flag.
////////////////////////////////////////////////////////////////////
bool NurbsCurveDrawer::
get_show_cvs() const {
  return _show_cvs;
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveDrawer::set_show_hull
//       Access: Published
//  Description: Sets the flag that hides or shows the convex hull.
////////////////////////////////////////////////////////////////////
void NurbsCurveDrawer::
set_show_hull(bool flag) {
  _show_hull = flag;
  redraw();
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveDrawer::get_show_hull
//       Access: Published
//  Description: Returns the current state of the show-hull flag.
////////////////////////////////////////////////////////////////////
bool NurbsCurveDrawer::
get_show_hull() const {
  return _show_hull;
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveDrawer::set_show_knots
//       Access: Published
//  Description: Sets the flag that hides or shows the knots.
////////////////////////////////////////////////////////////////////
void NurbsCurveDrawer::
set_show_knots(bool flag) {
  _show_knots = flag;
  redraw();
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveDrawer::get_show_knots
//       Access: Published
//  Description: Returns the current state of the show-knots flag.
////////////////////////////////////////////////////////////////////
bool NurbsCurveDrawer::
get_show_knots() const {
  return _show_knots;
}


////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveDrawer::hilight
//       Access: Published
//  Description: Hilights a particular CV by showing it and its knot
//               in a different color.  Returns true if the CV exists
//               and has been drawn, false otherwise.
////////////////////////////////////////////////////////////////////
bool NurbsCurveDrawer::
hilight(int n, float hr, float hg, float hb) {
  if (n < 0 || n >= _cvs.get_num_vertices()) {
    // Return false if we're out of range.
    return false;
  }

  if (_show_cvs) {
    _cvs.set_vertex_color(n, hr, hg, hb);
  }
  if (_show_knots) {
    nassertr(_knotnums[n] >= 0 && _knotnums[n] < _knots.get_num_vertices(), false);
    _knots.set_vertex_color(_knotnums[n], hr, hg, hb);
  }

  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveDrawer::unhilight
//       Access: Published
//  Description: Removes the hilight previously set on a CV.
////////////////////////////////////////////////////////////////////
bool NurbsCurveDrawer::
unhilight(int n) {
  if (n < 0 || n >= _cvs.get_num_vertices()) {
    return false;
  }

  if (_show_cvs) {
    _cvs.set_vertex_color(n, _cv_color[0], _cv_color[1], _cv_color[2]);
  }
  if (_show_knots) {
    nassertr(_knotnums[n] >= 0 && _knotnums[n] < _knots.get_num_vertices(), false);
    _knots.set_vertex_color(_knotnums[n],
                            _knot_color[0], _knot_color[1], _knot_color[2]);
  }

  return true;
}

