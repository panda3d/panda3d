// Filename: parametricCurveDrawer.cxx
// Created by:  drose (14Mar97)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////


#include "parametricCurveDrawer.h"
#include "parametricCurve.h"
#include "config_parametrics.h"

TypeHandle ParametricCurveDrawer::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ParametricCurveDrawer::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
ParametricCurveDrawer::
ParametricCurveDrawer() {
  _lines.set_color(1.0f, 1.0f, 1.0f);
  _ticks.set_color(1.0f, 0.0f, 0.0f);
  _tick_scale = 0.1;
  _num_segs = 100.0;
  _num_ticks = 0.0f;
  _frame_accurate = false;
  _geom_node = new GeomNode("pcd");
  _drawn = false;
}

////////////////////////////////////////////////////////////////////
//     Function: ParametricCurveDrawer::Destructor
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
ParametricCurveDrawer::
~ParametricCurveDrawer() {
  hide();

  // Unregister all the curves.
  clear_curves();
}


////////////////////////////////////////////////////////////////////
//     Function: ParametricCurveDrawer::set_curve
//       Access: Published
//  Description: Sets the drawer up to draw just the one curve.
////////////////////////////////////////////////////////////////////
void ParametricCurveDrawer::
set_curve(ParametricCurve *curve) {
  PT(ParametricCurveCollection) curves = new ParametricCurveCollection();
  curves->add_curve(curve);
  set_curves(curves);
}

////////////////////////////////////////////////////////////////////
//     Function: ParametricCurveDrawer::set_curves
//       Access: Published
//  Description: Sets the drawer up to draw the curves in the
//               indicated collection.  The drawer will actually draw
//               just the first XYZ curve in the collection, but if
//               one or more timewarps are present, this will affect
//               the placement of tick marks.
////////////////////////////////////////////////////////////////////
void ParametricCurveDrawer::
set_curves(ParametricCurveCollection *curves) {
  if (curves != _curves) {
    // First, unregister the old curves.
    if (_curves != (ParametricCurveCollection *)NULL) {
      _curves->unregister_drawer(this);
    }

    _curves = curves;

    // Now, register the new ones.
    if (_curves != (ParametricCurveCollection *)NULL) {
      _curves->register_drawer(this);
    }

    redraw();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ParametricCurveDrawer::clear_curves
//       Access: Published
//  Description: Empties the list of curves the drawer will update.
//               It will draw nothing.
////////////////////////////////////////////////////////////////////
void ParametricCurveDrawer::
clear_curves() {
  set_curves((ParametricCurveCollection *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: ParametricCurveDrawer::get_curves
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
ParametricCurveCollection *ParametricCurveDrawer::
get_curves() {
  return _curves;
}


////////////////////////////////////////////////////////////////////
//     Function: ParametricCurveDrawer::get_geom_node
//       Access: Published
//  Description: Returns a pointer to the drawer's GeomNode.  This is
//               where the drawer will build the visible
//               representation of the curve.  This GeomNode must be
//               inserted into the scene graph to make the curve
//               visible.  The GeomNode remains connected to the drawer,
//               so that future updates to the drawer will reflect in
//               the GeomNode, and the GeomNode will be emptied when the
//               drawer destructs.  Also see detach_geom_node().
////////////////////////////////////////////////////////////////////
GeomNode *ParametricCurveDrawer::
get_geom_node() {
  return _geom_node;
}


////////////////////////////////////////////////////////////////////
//     Function: ParametricCurveDrawer::detach_geom_node
//       Access: Published
//  Description: Detaches the GeomNode from the drawer so that the
//               drawing will remain after the death of the drawer.
//               Returns the now-static GeomNode.  A new, dynamic GeomNode
//               is created for the drawer's future use; get_geom_node()
//               will return this new GeomNode which will be empty until
//               the next call to draw().
////////////////////////////////////////////////////////////////////
PT(GeomNode) ParametricCurveDrawer::
detach_geom_node() {
  if (!_drawn) {
    draw();
  }
  PT(GeomNode) g = _geom_node;
  _geom_node = new GeomNode("pcd");
  _drawn = false;
  return g;
}


////////////////////////////////////////////////////////////////////
//     Function: ParametricCurveDrawer::set_num_segs
//       Access: Published
//  Description: Specifies the number of line segments used to
//               approximate the curve for each parametric unit.  This
//               just affects the visual appearance of the curve as it
//               is drawn.  The total number of segments drawn for the
//               curve will be get_max_t() * get_num_segs().
////////////////////////////////////////////////////////////////////
void ParametricCurveDrawer::
set_num_segs(float num_segs) {
  _num_segs = num_segs;
  redraw();
}

////////////////////////////////////////////////////////////////////
//     Function: ParametricCurveDrawer::get_num_segs
//       Access: Published
//  Description: Returns the number of line segments used to
//               approximate the curve for each parametric unit.  This
//               just affects the visual appearance of the curve as it
//               is drawn.  The total number of segments drawn for the
//               curve will be get_max_t() * get_num_segs().
////////////////////////////////////////////////////////////////////
float ParametricCurveDrawer::
get_num_segs() const {
  return _num_segs;
}


////////////////////////////////////////////////////////////////////
//     Function: ParametricCurveDrawer::set_num_ticks
//       Access: Published
//  Description: Specifies the number of time tick marks drawn
//               for each unit of time.  These tick marks are drawn at
//               equal increments in time to give a visual
//               approximation of speed.  Specify 0 to disable drawing
//               of tick marks.
////////////////////////////////////////////////////////////////////
void ParametricCurveDrawer::
set_num_ticks(float num_ticks) {
  _num_ticks = num_ticks;
  redraw();
}

////////////////////////////////////////////////////////////////////
//     Function: ParametricCurveDrawer::get_num_ticks
//       Access: Published
//  Description: Returns the number of time tick marks per unit of
//               time drawn.
////////////////////////////////////////////////////////////////////
float ParametricCurveDrawer::
get_num_ticks() const {
  return _num_ticks;
}



////////////////////////////////////////////////////////////////////
//     Function: ParametricCurveDrawer::set_color
//       Access: Published
//  Description: Specifies the color of the curve when it is drawn.
//               The default is white.
////////////////////////////////////////////////////////////////////
void ParametricCurveDrawer::
set_color(float r, float g, float b) {
  _lines.set_color(r, g, b);
  redraw();
}


////////////////////////////////////////////////////////////////////
//     Function: ParametricCurveDrawer::set_color
//       Access: Published
//  Description: Specifies the color of the time tick marks drawn on
//               the curve.  The default is red.
////////////////////////////////////////////////////////////////////
void ParametricCurveDrawer::
set_tick_color(float r, float g, float b) {
  _ticks.set_color(r, g, b);
  redraw();
}

////////////////////////////////////////////////////////////////////
//     Function: ParametricCurveDrawer::set_thickness
//       Access: Public
//  Description: Specifies the thickness of the line in pixels drawn
//               to represent the curve.  Note that pixel thickness of
//               a line segment is not supported by DirectX.
////////////////////////////////////////////////////////////////////
void ParametricCurveDrawer::
set_thickness(float thick) {
  _lines.set_thickness(thick);
  redraw();
}

////////////////////////////////////////////////////////////////////
//     Function: ParametricCurveDrawer::set_frame_accurate
//       Access: Published
//  Description: Specifies whether the curve drawn is to be
//               frame-accurate.  If true, then changes made to the
//               curve dynamically after it has been drawn will be
//               reflected correctly in the render window.  If false,
//               dynamic updates may be drawn before the rest of the
//               scene has updated.
////////////////////////////////////////////////////////////////////
void ParametricCurveDrawer::
set_frame_accurate(bool frame_accurate) {
  _frame_accurate = frame_accurate;
  redraw();
}

////////////////////////////////////////////////////////////////////
//     Function: ParametricCurveDrawer::get_frame_accurate
//       Access: Published
//  Description: Returns whether the curve is drawn in frame-accurate
//               mode.
////////////////////////////////////////////////////////////////////
bool ParametricCurveDrawer::
get_frame_accurate() const {
  return _frame_accurate;
}



////////////////////////////////////////////////////////////////////
//     Function: ParametricCurveDrawer::draw
//       Access: Published, Virtual
//  Description: Creates a series of line segments that approximates
//               the curve.  These line segments may be made visible
//               by parenting the node returned by get_geom_node()
//               into the scene graph.
////////////////////////////////////////////////////////////////////
bool ParametricCurveDrawer::
draw() {
  // First, remove the old drawing, if any.
  hide();

  _drawn = true;

  // If there's no curve, draw nothing and return false.
  if (_curves == (ParametricCurveCollection *)NULL) {
    return false;
  }

  ParametricCurve *xyz_curve = _curves->get_default_curve();
  if (xyz_curve == (ParametricCurve *)NULL) {
    return false;
  }

  // Otherwise, let's go to town!

  // Make sure the curve(s) are fresh.
  _curves->recompute();

  int total_segs = (int)cfloor(_curves->get_max_t() * _num_segs + 0.5);

  float max_t = xyz_curve->get_max_t();
  float scale = max_t / (float)(total_segs-1);
  float t;
  LVecBase3f point;
  bool last_in, next_in;

  last_in = false;
  int i;

  for (i = 0; i < total_segs; i++) {
    t = (float)i * scale;

    next_in = xyz_curve->get_point(t, point);

    if (!next_in || !last_in) {
      _lines.move_to(point);
    } else {
      _lines.draw_to(point);
    }
    last_in = next_in;
  }

  _lines.create(_geom_node, _frame_accurate);

  max_t = get_max_t();
  scale = max_t / (float)(total_segs-1);

  // Now draw the time tick marks.
  if (_num_ticks > 0.0f) {
    int total_ticks = (int)cfloor(max_t * _num_ticks + 0.5);
    ParametricCurve *xyz_curve = _curves->get_default_curve();
    //    ParametricCurve *hpr_curve = _curves->get_hpr_curve();

    scale = max_t / (float)total_ticks;

    for (i = 0; i <= total_ticks; i++) {
      t = (float)i * scale;
      float t0 = _curves->evaluate_t(t);
      LVecBase3f tangent;

      if (xyz_curve->get_pt(t0, point, tangent)) {
        // Draw crosses.
        LVecBase3f t1, t2;
        get_tick_marks(tangent, t1, t2);

        _ticks.move_to(point - t1 * _tick_scale);
        _ticks.draw_to(point + t1 * _tick_scale);
        _ticks.move_to(point - t2 * _tick_scale);
        _ticks.draw_to(point + t2 * _tick_scale);
      }
    }
    _ticks.create(_geom_node, _frame_accurate);
  }

  return true;
}




////////////////////////////////////////////////////////////////////
//     Function: ParametricCurveDrawer::hide
//       Access: Published
//  Description: Removes the lines that were created by a previous
//               call to draw().
////////////////////////////////////////////////////////////////////
void ParametricCurveDrawer::
hide() {
  _geom_node->remove_all_geoms();
  _drawn = false;
}


////////////////////////////////////////////////////////////////////
//     Function: ParametricCurveDrawer::set_tick_scale
//       Access: Published
//  Description: Sets the visible size of the time tick marks or
//               geometry.
////////////////////////////////////////////////////////////////////
void ParametricCurveDrawer::
set_tick_scale(float scale) {
  _tick_scale = scale;
  redraw();
}


////////////////////////////////////////////////////////////////////
//     Function: ParametricCurveDrawer::get_tick_scale
//       Access: Published
//  Description: Returns the size of the time tick marks or geometry.
////////////////////////////////////////////////////////////////////
float ParametricCurveDrawer::
get_tick_scale() const {
  return _tick_scale;
}

////////////////////////////////////////////////////////////////////
//     Function: ParametricCurveDrawer::get_tick_marks
//       Access: Private, Static
//  Description: Given a tangent vector, computes two vectors at right
//               angles to the tangent and to each other, suitable for
//               drawing as tick marks.
////////////////////////////////////////////////////////////////////
void ParametricCurveDrawer::
get_tick_marks(const LVecBase3f &tangent, LVecBase3f &t1, LVecBase3f &t2) {
  LVector3f tn = tangent;
  tn.normalize();

  // Decide the smallest axis of tn and cross with the corresponding
  // unit vector.
  if (fabs(tn[0]) <= fabs(tn[1]) && fabs(tn[0]) <= fabs(tn[2])) {
    // X is smallest.
    t1 = tn.cross(LVector3f(1.0f, 0.0f, 0.0f));

  } else if (fabs(tn[1]) <= fabs(tn[2])) {
    // Y is smallest.
    t1 = tn.cross(LVector3f(0.0f, 1.0f, 0.0f));

  } else {
    // Z is smallest.
    t1 = tn.cross(LVector3f(0.0f, 0.0f, 1.0f));
  }

  t2 = tn.cross(t1);
}
