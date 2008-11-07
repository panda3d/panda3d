// Filename: nurbsCurveResult.h
// Created by:  drose (03Dec02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef NURBSCURVERESULT_H
#define NURBSCURVERESULT_H

#include "pandabase.h"
#include "referenceCount.h"
#include "nurbsBasisVector.h"
#include "vector_float.h"

class NurbsVertex;

////////////////////////////////////////////////////////////////////
//       Class : NurbsCurveResult
// Description : The result of a NurbsCurveEvaluator.  This object
//               represents a curve in a particular coordinate space.
//               It can return the point and/or tangent to the curve
//               at any point.
//
//               This is not related to NurbsCurve, CubicCurveseg or
//               any of the ParametricCurve-derived objects in this
//               module.  It is a completely parallel implementation
//               of NURBS curves, and will probably eventually replace
//               the whole ParametricCurve class hierarchy.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PARAMETRICS NurbsCurveResult : public ReferenceCount {
public:
  NurbsCurveResult(const NurbsBasisVector &basis, 
                   const LVecBase4f vecs[], const NurbsVertex *verts,
                   int num_vertices);

PUBLISHED:
  INLINE ~NurbsCurveResult();

  INLINE float get_start_t() const;
  INLINE float get_end_t() const;

  INLINE bool eval_point(float t, LVecBase3f &point);
  INLINE bool eval_tangent(float t, LVecBase3f &tangent);
  INLINE float eval_extended_point(float t, int d);
  INLINE bool eval_extended_points(float t, int d, 
                                   float result[], int num_values);
  
  INLINE int get_num_segments() const;
  void eval_segment_point(int segment, float t, LVecBase3f &point) const;
  void eval_segment_tangent(int segment, float t, LVecBase3f &tangent) const;
  float eval_segment_extended_point(int segment, float t, int d) const;
  void eval_segment_extended_points(int segment, float t, int d,
                                    float result[], int num_values) const;
  INLINE float get_segment_t(int segment, float t) const;

  void adaptive_sample(float tolerance);
  INLINE int get_num_samples() const;
  INLINE float get_sample_t(int n) const;
  INLINE const LPoint3f &get_sample_point(int n) const;
  MAKE_SEQ(get_sample_ts, get_num_samples, get_sample_t);
  MAKE_SEQ(get_sample_points, get_num_samples, get_sample_points);
  
private:
  int find_segment(float t);
  int r_find_segment(float t, int top, int bot) const;

  void r_adaptive_sample(int segment, float t0, const LPoint3f &p0, 
                         float t1, const LPoint3f &p1, float tolerance_2);
  static float sqr_dist_to_line(const LPoint3f &point, const LPoint3f &origin, 
                                const LVector3f &vec);

  NurbsBasisVector _basis;
  const NurbsVertex *_verts;

  // We pre-compose the basis matrix and the geometry vectors, so we
  // have these handy for evaluation.  There is one entry in the
  // _composed for each entry in basis._segments.
  typedef pvector<LMatrix4f> ComposedGeom;
  ComposedGeom _composed;

  int _last_segment;
  float _last_from;
  float _last_to;

  class AdaptiveSample {
  public:
    INLINE AdaptiveSample(float t, const LPoint3f &point);
    float _t;
    LPoint3f _point;
  };
  typedef pvector<AdaptiveSample> AdaptiveResult;
  AdaptiveResult _adaptive_result;
};

#include "nurbsCurveResult.I"

#endif

