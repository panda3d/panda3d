/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file curveFitter.h
 * @author drose
 * @date 1998-09-17
 */

#ifndef CURVEFITTER_H
#define CURVEFITTER_H

#include "pandabase.h"

#include "luse.h"

#include "parametricCurveCollection.h"

#include "typedef.h"
#include "pvector.h"

class HermiteCurve;
class ParametricCurve;
class NurbsCurve;

/**
 *
 */
class EXPCL_PANDA_PARAMETRICS CurveFitter {
PUBLISHED:
  CurveFitter();
  ~CurveFitter();

  void reset();
  void add_xyz(PN_stdfloat t, const LVecBase3 &xyz);
  void add_hpr(PN_stdfloat t, const LVecBase3 &hpr);
  void add_xyz_hpr(PN_stdfloat t, const LVecBase3 &xyz, const LVecBase3 &hpr);

  int get_num_samples() const;
  PN_stdfloat get_sample_t(int n) const;
  LVecBase3 get_sample_xyz(int n) const;
  LVecBase3 get_sample_hpr(int n) const;
  LVecBase3 get_sample_tangent(int n) const;
  void remove_samples(int begin, int end);

  void sample(ParametricCurveCollection *curves, int count);
  void wrap_hpr();
  void sort_points();
  void desample(PN_stdfloat factor);

  void compute_tangents(PN_stdfloat scale);
  PT(ParametricCurveCollection) make_hermite() const;
  PT(ParametricCurveCollection) make_nurbs() const;

  void output(std::ostream &out) const;
  void write(std::ostream &out) const;

public:
  class DataPoint {
  public:
    INLINE DataPoint();
    INLINE void output(std::ostream &out) const;
    INLINE bool operator < (const DataPoint &other) const;

    PN_stdfloat _t;
    LVecBase3 _xyz;
    LVecBase3 _hpr;
    LVecBase3 _tangent;
    LVecBase3 _hpr_tangent;
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

INLINE std::ostream &operator << (std::ostream &out, const CurveFitter::DataPoint &dp) {
  dp.output(out);
  return out;
}

INLINE std::ostream &operator << (std::ostream &out, const CurveFitter &cf) {
  cf.output(out);
  return out;
}

#include "curveFitter.I"

#endif
