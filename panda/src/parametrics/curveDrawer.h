// Filename: curveDrawer.h
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
#ifndef CURVEDRAWER_H
#define CURVEDRAWER_H
//
////////////////////////////////////////////////////////////////////
// Includes 
////////////////////////////////////////////////////////////////////

#include "curve.h"
#include "lineSegs.h"
////#include <Performer/pr/pfLinMath.h>

////////////////////////////////////////////////////////////////////
// Defines 
////////////////////////////////////////////////////////////////////

typedef LVector3f LVector3fMapper(const LVector3f &point, 
			     const LVector3f &tangent, 
			     double t);

BEGIN_PUBLISH //[
// The different kinds of ParametricCurveDrawer graph types
#define PCD_DEFAULT 1
#define PCD_XVST    2
#define PCD_YVST    3
#define PCD_ZVST    4
#define PCD_DXVST   6
#define PCD_DYVST   7
#define PCD_DZVST   8
#define PCD_IXVST   9
#define PCD_IYVST   10
END_PUBLISH //]

class ParametricSurface;

////////////////////////////////////////////////////////////////////
// 	 Class : ParametricCurveDrawer
// Description : Draws a 3-d parametric curve in the scene by creating
//               a series of line segments to approximate the curve.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ParametricCurveDrawer {

////////////////////////////////////////////////////////////////////
// Member functions visible to Scheme
////////////////////////////////////////////////////////////////////

PUBLISHED:
  ParametricCurveDrawer(ParametricCurve *curve);
  virtual ~ParametricCurveDrawer();

  void set_curve(ParametricCurve *curve);
  ParametricCurve *get_curve();

  void set_time_curve(ParametricCurve *curve);
  ParametricCurve *get_time_curve();

  GeomNode *get_geom_node();
  GeomNode *detach_geom_node();

  void set_num_segs(int num_segs);
  int get_num_segs() const;

  void set_num_ticks(int num_ticks);
  int get_num_ticks() const;

  void set_color(float r, float g, float b);
  void set_tick_color(float r, float g, float b);

  void set_frame_accurate(bool frame_accurate);
  bool get_frame_accurate() const;

  virtual bool draw();
  virtual bool recompute(double t1, double t2, ParametricCurve *curve=NULL);
  void hide();

  void set_tick_scale(double scale);
  double get_tick_scale() const;

  void set_graph_type(int graph_type);

////////////////////////////////////////////////////////////////////
// Member functions not visible to Scheme
////////////////////////////////////////////////////////////////////

public:
  double get_max_t() const {
    return _time_curve==NULL ? _curve->get_max_t() : _time_curve->get_max_t();
  }

  void disable(ParametricCurve *curve);

  void set_mapper(LVector3fMapper *mapper);

  static LVector3f DefaultMap(const LVector3f &point, const LVector3f &, double);
  static LVector3f XvsT(const LVector3f &point, const LVector3f &, double t);
  static LVector3f iXvsT(const LVector3f &point, const LVector3f &, double t);
  static LVector3f YvsT(const LVector3f &point, const LVector3f &, double t);
  static LVector3f iYvsT(const LVector3f &point, const LVector3f &, double t);
  static LVector3f ZvsT(const LVector3f &point, const LVector3f &, double t);
  static LVector3f dXvsT(const LVector3f &, const LVector3f &tangent, double t);
  static LVector3f dYvsT(const LVector3f &, const LVector3f &tangent, double t);
  static LVector3f dZvsT(const LVector3f &, const LVector3f &tangent, double t);

protected:
  static void get_tick_marks(const LVector3f &tangent, LVector3f &t1, LVector3f &t2);

  PT(GeomNode) _geom_node;
  int _num_segs;
  ParametricCurve *_curve, *_time_curve;
  LineSegs _lines, _ticks;
  bool _drawn;
  int _num_ticks;
  double _tick_scale;
  bool _frame_accurate;
  LVector3fMapper *_mapper;


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    register_type(_type_handle, "ParametricCurveDrawer");
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
 
private:
  static TypeHandle _type_handle;
};
  
#endif
