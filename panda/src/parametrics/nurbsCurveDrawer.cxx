// Filename: nurbsCurveDrawer.C
// Created by:  drose (27Feb98)
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

#include "nurbsCurveDrawer.h"
#include "luse.h"
#include "parametrics.h"
#include "typedWriteableReferenceCount.h"
#include "namable.h"

#include <math.h>


TypeHandle NurbsCurveDrawer::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveDrawer::Constructor
//       Access: Public, Scheme
//  Description:
////////////////////////////////////////////////////////////////////
NurbsCurveDrawer::
NurbsCurveDrawer(NurbsCurve *curve) : ParametricCurveDrawer(curve) {
  set_cv_color(1.0, 0.0, 0.0);
  set_hull_color(1.0, 0.5, 0.5);
  set_knot_color(0.0, 0.0, 1.0);
  
  _cvs.set_thickness(4.0);
  _hull.set_thickness(1.0);
  _knots.set_thickness(4.0);
  
  _show_cvs = true;
  _show_hull = true;
  _show_knots = true;
}


////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveDrawer::Destructor
//       Access: Public, Scheme, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
NurbsCurveDrawer::
~NurbsCurveDrawer() {
}



////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveDrawer::set_cv_color
//       Access: Public, Scheme
//  Description: Specifies the color of the CV's.
////////////////////////////////////////////////////////////////////
void NurbsCurveDrawer::
set_cv_color(float r, float g, float b) {
  _cv_color.set(r, g, b);
  _cvs.set_color(r, g, b);

  if (_drawn) {
    draw();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveDrawer::set_knot_color
//       Access: Public, Scheme
//  Description: Specifies the color of the knots.
////////////////////////////////////////////////////////////////////
void NurbsCurveDrawer::
set_knot_color(float r, float g, float b) {
  _knot_color.set(r, g, b);
  _knots.set_color(r, g, b);

  if (_drawn) {
    draw();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveDrawer::set_hull_color
//       Access: Public, Scheme
//  Description: Specifies the color of the convex hull.
////////////////////////////////////////////////////////////////////
void NurbsCurveDrawer::
set_hull_color(float r, float g, float b) {
  _hull_color.set(r, g, b);
  _hull.set_color(r, g, b);

  if (_drawn) {
    draw();
  }
}



////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveDrawer::draw
//       Access: Public, Scheme, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
bool NurbsCurveDrawer::
draw() {
  NurbsCurve *nurbs = (NurbsCurve *)_curve;
  // Make sure the curve is fresh.
  nurbs->recompute();

  // First, draw the curve itself.
  if (!ParametricCurveDrawer::draw()) {
    return false;
  }

  int i;
  if (_show_knots) {
    _num_cvs = nurbs->get_num_cvs();
    _knotnums.erase(_knotnums.begin(), _knotnums.end());

    double lt = -1.0;
    int ki = -1;
    for (i = 0; i < _num_cvs; i++) {
      double t = nurbs->GetKnot(i);
      if (t != lt) {
	lt = t;
	LVector3f knot_pos, knot_tan;
	nurbs->get_pt(nurbs->GetKnot(i), knot_pos, knot_tan);
	_knots.move_to(_mapper(knot_pos, knot_tan, t));
	ki++;
      }
      _knotnums.push_back(ki);
    }

    _knots.create(_geom_node, _frame_accurate);
  }

  if (_show_cvs) {
    _num_cvs = nurbs->get_num_cvs();
    for (i = 0; i < _num_cvs; i++) {
      _cvs.move_to(_mapper(nurbs->get_cv_point(i), LVector3f(0.0, 0.0, 0.0),
			   nurbs->GetKnot(i+1)));
    }

    _cvs.create(_geom_node, _frame_accurate);
  }

  if (_show_hull) {
    _num_cvs = nurbs->get_num_cvs();
    for (i = 0; i < _num_cvs; i++) {
      _hull.draw_to(_mapper(nurbs->get_cv_point(i), LVector3f(0.0, 0.0, 0.0),
			    nurbs->GetKnot(i+1)));
    }

    _hull.create(_geom_node, _frame_accurate);
  }

  return true;
}

    

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveDrawer::recompute
//       Access: Public, Scheme, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
bool NurbsCurveDrawer::
recompute(double t1, double t2, ParametricCurve *curve) {
  return draw();
}


////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveDrawer::set_show_cvs
//       Access: Public, Scheme
//  Description: Sets the flag that hides or shows the CV's.
////////////////////////////////////////////////////////////////////
void NurbsCurveDrawer::
set_show_cvs(bool flag) {
  _show_cvs = flag;
  if (_drawn) {
    draw();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveDrawer::get_show_cvs
//       Access: Public, Scheme
//  Description: Returns the current state of the show-CV's flag.
////////////////////////////////////////////////////////////////////
bool NurbsCurveDrawer::
get_show_cvs() const {
  return _show_cvs;
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveDrawer::set_show_hull
//       Access: Public, Scheme
//  Description: Sets the flag that hides or shows the convex hull.
////////////////////////////////////////////////////////////////////
void NurbsCurveDrawer::
set_show_hull(bool flag) {
  _show_hull = flag;
  if (_drawn) {
    draw();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveDrawer::get_show_hull
//       Access: Public, Scheme
//  Description: Returns the current state of the show-hull flag.
////////////////////////////////////////////////////////////////////
bool NurbsCurveDrawer::
get_show_hull() const {
  return _show_hull;
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveDrawer::set_show_knots
//       Access: Public, Scheme
//  Description: Sets the flag that hides or shows the knots.
////////////////////////////////////////////////////////////////////
void NurbsCurveDrawer::
set_show_knots(bool flag) {
  _show_knots = flag;
  if (_drawn) {
    draw();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveDrawer::get_show_knots
//       Access: Public, Scheme
//  Description: Returns the current state of the show-knots flag.
////////////////////////////////////////////////////////////////////
bool NurbsCurveDrawer::
get_show_knots() const {
  return _show_knots;
}


////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveDrawer::hilight
//       Access: Public, Scheme
//  Description: Hilights a particular CV by showing it and its knot
//               in a different color.  Returns true if the CV exists
//               and has been drawn, false otherwise.
////////////////////////////////////////////////////////////////////
bool NurbsCurveDrawer::
hilight(int n, float hr, float hg, float hb) {
  // If there's no curve, do nothing and return false.
  if (_curve==NULL || !_curve->is_valid()) {
    return false;
  }

  if (n < 0 || n >= _cvs.get_num_vertices()) {
    // Also return false if we're out of range.
    return false;
  }

  NurbsCurve *nurbs = (NurbsCurve *)_curve;
  if (_show_cvs) {
    _cvs.set_vertex_color(n, hr, hg, hb);
  }
  if (_show_knots) {
    ///dnassert(_knotnums[n] >= 0 && _knotnums[n] < _knots.get_num_vertices());
    _knots.set_vertex_color(_knotnums[n], hr, hg, hb);
  }

  return true;
}
  

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveDrawer::unhilight
//       Access: Public, Scheme
//  Description: Removes the hilight previously set on a CV.
////////////////////////////////////////////////////////////////////
bool NurbsCurveDrawer::
unhilight(int n) {
  if (_curve==NULL || !_curve->is_valid()) {
    return false;
  }

  if (n < 0 || n >= _cvs.get_num_vertices()) {
    return false;
  }

  NurbsCurve *nurbs = (NurbsCurve *)_curve;
  if (_show_cvs) {
    _cvs.set_vertex_color(n, _cv_color[0], _cv_color[1], _cv_color[2]);
  }
  if (_show_knots) {
    ///dnassert(_knotnums[n] >= 0 && _knotnums[n] < _knots.get_num_vertices());
    _knots.set_vertex_color(_knotnums[n],
			    _knot_color[0], _knot_color[1], _knot_color[2]);
  }

  return true;
}

