// Filename: nurbsCurveDrawer.h
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
#ifndef NURBSCURVEDRAWER_H
#define NURBSCURVEDRAWER_H
//
////////////////////////////////////////////////////////////////////
// Includes 
////////////////////////////////////////////////////////////////////

#include "curveDrawer.h"
#include "nurbsCurve.h"
#include "lineSegs.h"


////////////////////////////////////////////////////////////////////
// Defines 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
// 	 Class : NurbsCurveDrawer
// Description : Draws a Nurbs curve, also drawing in the control
//               vertices and tangent vectors.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA NurbsCurveDrawer : public ParametricCurveDrawer {

////////////////////////////////////////////////////////////////////
// Member functions visible to Scheme
////////////////////////////////////////////////////////////////////

PUBLISHED:
  NurbsCurveDrawer(NurbsCurve *curve);
  virtual ~NurbsCurveDrawer();

  void set_cv_color(float r, float g, float b);
  void set_hull_color(float r, float g, float b);
  void set_knot_color(float r, float g, float b);

  virtual bool draw();
  virtual bool recompute(double t1, double t2, ParametricCurve *curve=NULL);

  void set_show_cvs(bool flag);
  bool get_show_cvs() const;
  void set_show_hull(bool flag);
  bool get_show_hull() const;
  void set_show_knots(bool flag);
  bool get_show_knots() const;

  bool hilight(int n, float hr=1.0, float hg=1.0, float hb=0.0);
  bool unhilight(int n);

////////////////////////////////////////////////////////////////////
// Member functions not visible to Scheme
////////////////////////////////////////////////////////////////////
protected:
  LVector3f _cv_color, _hull_color, _knot_color;
  int _num_cvs, _num_hull, _num_knots;
  LineSegs _hull, _knots, _cvs;
  vector<int> _knotnums;

  bool _show_cvs, _show_hull, _show_knots;


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ParametricCurveDrawer::init_type();
    register_type(_type_handle, "NurbsCurveDrawer",
                  ParametricCurveDrawer::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
 
private:
  static TypeHandle _type_handle;
};
  
#endif


