// Filename: hermiteCurve.h
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
#ifndef HERMITECURVE_H
#define HERMITECURVE_H

////////////////////////////////////////////////////////////////////
// Includes 
////////////////////////////////////////////////////////////////////

#include "curve.h"

////////////////////////////////////////////////////////////////////
// Defines 
////////////////////////////////////////////////////////////////////


BEGIN_PUBLISH //[
// Hermite curve continuity types.
#define HC_CUT         1 
// The curve is disconnected at this point.  All points between
// this and the following CV are not part of the curve.

#define HC_FREE        2  
// Tangents are unconstrained.  The curve is continuous, but its first
// derivative is not.  This is G0 geometric continuity.

#define HC_G1          3  
// Tangents are constrained to be collinear.  The curve's derivative
// is not continuous in parametric space, but its geometric slope is.
// The distinction is mainly relevant in the context of animation
// along the curve--when crossing the join point, direction of motion
// will change continuously, but the speed of motion may change
// suddenly.  This is G1 geometric continuity.

#define HC_SMOOTH     4
// Tangents are constrained to be identical.  The curve and its first
// derivative are continuous in parametric space.  When animating
// motion across the join point, speed and direction of motion will
// change continuously.  This is C1 parametric continuity.
END_PUBLISH //]

class NurbsCurve;

////////////////////////////////////////////////////////////////////
// 	 Class : HermiteCurveCV
// Description : A single CV of a Hermite curve.  Hermite curve CV's
//               include an in and out tangent, as well as a position.
////////////////////////////////////////////////////////////////////
class HermiteCurveCV {
public:
  HermiteCurveCV();
  HermiteCurveCV(const HermiteCurveCV &c);
  ~HermiteCurveCV();
  
  void set_point(const LVecBase3f &point) { _p = point; }
  void set_in(const LVecBase3f &in);
  void set_out(const LVecBase3f &out);
  void set_type(int type);
  void set_name(const char *name);

  void Output(ostream &out, int indent, int num_dimensions,
	      bool show_in, bool show_out,
	      double scale_in, double scale_out) const;
  
  LVecBase3f _p, _in, _out;
  int _type;
  char *_name;
};

////////////////////////////////////////////////////////////////////
// 	 Class : HermiteCurve
// Description : A parametric curve defined by a sequence of control
//               vertices, each with an in and out tangent.
//
//               This class is actually implemented as a
//               PiecewiseCurve made up of several CubicCurvesegs,
//               each of which is created using the hermite_basis()
//               method.  The HermiteCurve class itself keeps its own
//               list of the CV's that are used to define the curve
//               (since the CubicCurveseg class doesn't retain these).
////////////////////////////////////////////////////////////////////
class HermiteCurve : public PiecewiseCurve {
PUBLISHED:
  HermiteCurve();
  HermiteCurve(const ParametricCurve &pc);
  virtual ~HermiteCurve();

  int get_num_cvs() const;

  int insert_cv(double t);
  int append_cv(int type, float x, float y, float z);
  inline int append_cv(int type, const LVecBase3f &v) {
    return append_cv(type, v[0], v[1], v[2]);
  }

  bool remove_cv(int n);
  void remove_all_cvs();

  bool set_cv_type(int n, int type);
  bool set_cv_point(int n, float x, float y, float z);
  inline bool set_cv_point(int n, const LVecBase3f &v) {
    return set_cv_point(n, v[0], v[1], v[2]);
  }
  bool set_cv_in(int n, float x, float y, float z);
  inline bool set_cv_in(int n, const LVecBase3f &v) {
    return set_cv_in(n, v[0], v[1], v[2]);
  }
  bool set_cv_out(int n, float x, float y, float z);
  inline bool set_cv_out(int n, const LVecBase3f &v) {
    return set_cv_out(n, v[0], v[1], v[2]);
  }
  bool set_cv_tstart(int n, double tstart);
  bool set_cv_name(int n, const char *name);


  int get_cv_type(int n) const;
  const LVecBase3f &get_cv_point(int n) const;
  void get_cv_point(int n, LVecBase3f &v) const;
  const LVecBase3f &get_cv_in(int n) const;
  void get_cv_in(int n, LVecBase3f &v) const;
  const LVecBase3f &get_cv_out(int n) const;
  void get_cv_out(int n, LVecBase3f &v) const;
  double get_cv_tstart(int n) const;
  const char *get_cv_name(int n) const;

  void Print() const;
  void print_cv(int n) const;

  bool write_egg(const char *filename);
  bool write_egg(ostream &out, const char *basename);
  
public:

  CubicCurveseg *get_curveseg(int ti) {
    return (CubicCurveseg *)PiecewiseCurve::get_curveseg(ti);
  }

  virtual bool
  rebuild_curveseg(int rtype0, double t0, const LVecBase4f &v0,
		   int rtype1, double t1, const LVecBase4f &v1,
		   int rtype2, double t2, const LVecBase4f &v2,
		   int rtype3, double t3, const LVecBase4f &v3);

  void Output(ostream &out, int indent=0) const;

protected:

  void invalidate_cv(int n, bool redo_all);
  int find_cv(double t);
  void recompute_basis();

  vector<HermiteCurveCV> _points;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    register_type(_type_handle, "HermiteCurve");
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
 
private:
  static TypeHandle _type_handle;
};

#endif
