// Filename: nurbsPPCurve.h
// Created by:  drose (01Mar01)
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

#ifndef NURBSPPCURVE_H
#define NURBSPPCURVE_H

#include "pandabase.h"

#include "parametricCurve.h"
#include "nurbsCurveInterface.h"

#include <nurbs.hh>

////////////////////////////////////////////////////////////////////
//       Class : NurbsPPCurve
// Description : A Nonuniform Rational B-Spline.
//
//               This class is an extension to Panda NURBS curves,
//               taking advantage of the external NURBS++ library, if
//               it is available.  It stands in for the Panda-native
//               ClassicNurbsCurve, providing more and better
//               functionality.
//
//               This file is only compiled if NURBS++ is available.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA NurbsPPCurve : public ParametricCurve, public NurbsCurveInterface {
PUBLISHED:
  NurbsPPCurve();
  NurbsPPCurve(const ParametricCurve &pc);
public:
  NurbsPPCurve(int order, int num_cvs,
               const float knots[], const LVecBase4f cvs[]);
PUBLISHED:
  virtual ~NurbsPPCurve();

public:
  // We don't need to re-publish these, since they're all published
  // from NurbsCurveInterface.
  virtual float get_max_t() const;

  virtual void set_order(int order);
  virtual int get_order() const;

  virtual int get_num_cvs() const;
  virtual int get_num_knots() const;

  virtual bool insert_cv(float t);

  virtual bool remove_cv(int n);
  virtual void remove_all_cvs();

  virtual bool set_cv(int n, const LVecBase4f &v);
  virtual LVecBase4f get_cv(int n) const;

  virtual bool set_knot(int n, float t);
  virtual float get_knot(int n) const;

  virtual bool recompute();

public:
  virtual bool get_point(float t, LVecBase3f &point) const;
  virtual bool get_tangent(float t, LVecBase3f &tangent) const;
  virtual bool get_pt(float t, LVecBase3f &point, LVecBase3f &tangent) const;
  virtual bool get_2ndtangent(float t, LVecBase3f &tangent2) const;

  virtual bool stitch(const ParametricCurve *a, const ParametricCurve *b);

  virtual NurbsCurveInterface *get_nurbs_interface();
  virtual bool convert_to_nurbs(ParametricCurve *nc) const;
  virtual void write(ostream &out, int indent_level = 0) const;

protected:
  virtual int append_cv_impl(const LVecBase4f &v);
  virtual bool format_egg(ostream &out, const string &name,
                          const string &curve_type, int indent_level) const;

private:
  typedef pvector<LVecBase4f> Points;
  typedef pvector<float> Knots;

  bool make_nurbs_valid();
  void make_arrays_valid();
  bool copy_nurbs(PLib::NurbsCurvef &nurbs) const;
  void copy_arrays(Points &points, Knots &knots, int &order) const;

  static bool make_nurbs_from(PLib::NurbsCurvef &nurbs,
                              const Points &points, const Knots &knots,
                              int order);
  static void make_arrays_from(const PLib::NurbsCurvef &nurbs,
                               Points &points, Knots &knots, int &order);

  PLib::NurbsCurvef _nurbs;

  int _order;
  Points _points;
  Knots _knots;
  bool _nurbs_valid;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ParametricCurve::init_type();
    NurbsCurveInterface::init_type();
    register_type(_type_handle, "NurbsPPCurve",
                  ParametricCurve::get_class_type(),
                  NurbsCurveInterface::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

typedef NurbsPPCurve NurbsCurve;

#endif
