// Filename: nurbsCurve.h
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
#ifndef NURBSCURVE_H
#define NURBSCURVE_H

////////////////////////////////////////////////////////////////////
// Includes 
////////////////////////////////////////////////////////////////////
#include "curve.h"

////////////////////////////////////////////////////////////////////
// Defines 
////////////////////////////////////////////////////////////////////

////#define LVecBase3f pfVec3
//typedef pfVec3 LVecBase3f;


class HermiteCurve;

////////////////////////////////////////////////////////////////////
// 	 Class : NurbsCurve
// Description : A Nonuniform Rational B-Spline.
//
//               This class is actually implemented as a
//               PiecewiseCurve made up of several CubicCurvesegs,
//               each of which is created using the nurbs_basis()
//               method.  The list of CV's and knots is kept here,
//               within the NurbsCurve class.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA NurbsCurve : public PiecewiseCurve {

////////////////////////////////////////////////////////////////////
// Member functions visible to Scheme
////////////////////////////////////////////////////////////////////

PUBLISHED:
  NurbsCurve();
  NurbsCurve(const ParametricCurve &hc);
  NurbsCurve(int order, int num_cvs,
	     const double knots[], const LVecBase4f cvs[]);
  virtual ~NurbsCurve();

  void set_order(int order);
  int get_order() const;

  int get_num_cvs() const;
  int get_num_knots() const {
    return _cvs.size() + _order;
  }

  int insert_cv(double t);
  int append_cv(float x, float y, float z);
  inline int append_cv(const LVecBase3f &v) {
    return append_cv(LVecBase4f(v[0], v[1], v[2], 1.0));
  }
  inline int append_cv(const LVecBase4f &v) {
    _cvs.push_back(CV(v, GetKnot(_cvs.size())+1.0));
    return _cvs.size()-1;
  }

  bool remove_cv(int n);
  void remove_all_cvs();

  bool set_cv_point(int n, float x, float y, float z);
  inline bool set_cv_point(int n, const LVecBase3f &v) {
    return set_cv_point(n, v[0], v[1], v[2]);
  }
  void get_cv_point(int n, LVecBase3f &v) const;
  const LVecBase3f &get_cv_point(int n) const;

  bool set_cv_weight(int n, float w);
  float get_cv_weight(int n) const;

  bool set_knot(int n, double t);
  double get_knot(int n) const;

  virtual void write(ostream &out, int indent_level = 0) const;
  void write_cv(ostream &out, int n) const;

  bool recompute();

  void normalize_tlength();

  bool write_egg(const char *filename, CoordinateSystem cs = CS_default);
  bool write_egg(ostream &out, const char *basename, CoordinateSystem cs);

  void splice(double t, const NurbsCurve &other);
  
////////////////////////////////////////////////////////////////////
// Member functions not visible to Scheme
////////////////////////////////////////////////////////////////////
public:
  virtual bool
  rebuild_curveseg(int rtype0, double t0, const LVecBase4f &v0,
		   int rtype1, double t1, const LVecBase4f &v1,
		   int rtype2, double t2, const LVecBase4f &v2,
		   int rtype3, double t3, const LVecBase4f &v3);

  CubicCurveseg *get_curveseg(int ti) {
    return (CubicCurveseg *)PiecewiseCurve::get_curveseg(ti);
  }

  double
  GetKnot(int n) const {
    if (n < _order || _cvs.empty()) {
      return 0.0;
    } else if (n-1 >= (int)_cvs.size()) {
      return _cvs.back()._t;
    } else {
      return _cvs[n-1]._t;
    }
  }

  void format_egg(ostream &out, CoordinateSystem cs, int indent_level) const;

protected:
  int FindCV(double t);

  int _order;

  class CV {
  public:
    CV() {}
    CV(const LVecBase4f &p, double t) : _p(p), _t(t) {}
    LVecBase4f _p;
    double _t;
  };

  vector<CV> _cvs;


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PiecewiseCurve::init_type();
    register_type(_type_handle, "NurbsCurve",
                  PiecewiseCurve::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
 
private:
  static TypeHandle _type_handle;
};

#endif
