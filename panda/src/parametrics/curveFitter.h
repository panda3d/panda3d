// Filename: curveFitter.h
// Created by:  drose (17Sep98)
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

#ifndef CURVEFITTER_H
#define CURVEFITTER_H

#include "pandabase.h"

#include "luse.h"

#include "parametricCurveCollection.h"

#include "typedef.h"
#include "pvector.h"

class HermiteCurve;
class ParametricCurve;
class ClassicNurbsCurve;

////////////////////////////////////////////////////////////////////
//       Class : CurveFitter
// Description :
////////////////////////////////////////////////////////////////////
class CurveFitter {
PUBLISHED:
  CurveFitter();
  ~CurveFitter();

  void reset();
  void add_xyz(float t, const LVecBase3f &xyz);
  void add_hpr(float t, const LVecBase3f &hpr);
  void add_xyz_hpr(float t, const LVecBase3f &xyz, const LVecBase3f &hpr);

  int get_num_samples() const;
  float get_sample_t(int n) const;
  LVecBase3f get_sample_xyz(int n) const;
  LVecBase3f get_sample_hpr(int n) const;
  LVecBase3f get_sample_tangent(int n) const;
  void remove_samples(int begin, int end);

  void sample(ParametricCurveCollection *curves, int count);
  void wrap_hpr();
  void sort_points();
  void desample(float factor);

  void compute_tangents(float scale);
  PT(ParametricCurveCollection) make_hermite() const;
  PT(ParametricCurveCollection) make_nurbs() const;

  void output(ostream &out) const;
  void write(ostream &out) const;

public:
  class DataPoint {
  public:
    INLINE DataPoint();
    INLINE void output(ostream &out) const;
    INLINE bool operator < (const DataPoint &other) const;

    float _t;
    LVecBase3f _xyz;
    LVecBase3f _hpr;
    LVecBase3f _tangent;
    LVecBase3f _hpr_tangent;
  };

  typedef pvector<DataPoint> Data;
  Data _data;

  bool _got_xyz;
  bool _got_hpr;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    register_type(_type_handle, "CurveFitter");
  }

private:
  static TypeHandle _type_handle;
};

INLINE ostream &operator << (ostream &out, const CurveFitter::DataPoint &dp) {
  dp.output(out);
  return out;
}

INLINE ostream &operator << (ostream &out, const CurveFitter &cf) {
  cf.output(out);
  return out;
}

#include "curveFitter.I"

#endif
