// Filename: parametricCurveDrawer.h
// Created by:  drose (14Mar97)
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

#ifndef PARAMETRICCURVEDRAWER_H
#define PARAMETRICCURVEDRAWER_H

#include "parametricCurveCollection.h"

#include "lineSegs.h"
#include "pandaNode.h"

#include "typedObject.h"

////////////////////////////////////////////////////////////////////
//       Class : ParametricCurveDrawer
// Description : Draws a 3-d parametric curve in the scene by creating
//               a series of line segments to approximate the curve.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ParametricCurveDrawer : public TypedObject {
PUBLISHED:
  ParametricCurveDrawer();
  virtual ~ParametricCurveDrawer();

  void set_curve(ParametricCurve *curve);
  void set_curves(ParametricCurveCollection *curves);
  void clear_curves();
  ParametricCurveCollection *get_curves();

  GeomNode *get_geom_node();
  PT(GeomNode) detach_geom_node();

  void set_num_segs(float num_segs);
  float get_num_segs() const;

  void set_num_ticks(float num_ticks);
  float get_num_ticks() const;

  void set_color(float r, float g, float b);
  void set_tick_color(float r, float g, float b);

  void set_thickness(float thick);

  void set_frame_accurate(bool frame_accurate);
  bool get_frame_accurate() const;

  virtual bool draw();
  void hide();

  void set_tick_scale(float scale);
  float get_tick_scale() const;

public:
  INLINE float get_max_t() const;
  INLINE void redraw();

private:
  static void get_tick_marks(const LVecBase3f &tangent, LVecBase3f &t1, LVecBase3f &t2);

protected:
  PT(GeomNode) _geom_node;
  PT(ParametricCurveCollection) _curves;
  bool _frame_accurate;

private:
  float _num_segs;
  LineSegs _lines, _ticks;
  bool _drawn;
  float _num_ticks;
  float _tick_scale;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    register_type(_type_handle, "ParametricCurveDrawer",
                  TypedObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "parametricCurveDrawer.I"

#endif
