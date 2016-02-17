/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file nurbsCurveResult.h
 * @author drose
 * @date 2002-12-03
 */

#ifndef NURBSCURVERESULT_H
#define NURBSCURVERESULT_H

#include "pandabase.h"
#include "referenceCount.h"
#include "nurbsBasisVector.h"
#include "vector_stdfloat.h"
#include "epvector.h"

class NurbsVertex;

/**
 * The result of a NurbsCurveEvaluator.  This object represents a curve in a
 * particular coordinate space.  It can return the point and/or tangent to the
 * curve at any point.
 *
 * This is not related to NurbsCurve, CubicCurveseg or any of the
 * ParametricCurve-derived objects in this module.  It is a completely
 * parallel implementation of NURBS curves, and will probably eventually
 * replace the whole ParametricCurve class hierarchy.
 */
class EXPCL_PANDA_PARAMETRICS NurbsCurveResult : public ReferenceCount {
public:
  NurbsCurveResult(const NurbsBasisVector &basis,
                   const LVecBase4 vecs[], const NurbsVertex *verts,
                   int num_vertices);

PUBLISHED:
  INLINE ~NurbsCurveResult();

  INLINE PN_stdfloat get_start_t() const;
  INLINE PN_stdfloat get_end_t() const;

  INLINE bool eval_point(PN_stdfloat t, LVecBase3 &point);
  INLINE bool eval_tangent(PN_stdfloat t, LVecBase3 &tangent);
  INLINE PN_stdfloat eval_extended_point(PN_stdfloat t, int d);
  INLINE bool eval_extended_points(PN_stdfloat t, int d,
                                   PN_stdfloat result[], int num_values);

  INLINE int get_num_segments() const;
  void eval_segment_point(int segment, PN_stdfloat t, LVecBase3 &point) const;
  void eval_segment_tangent(int segment, PN_stdfloat t, LVecBase3 &tangent) const;
  PN_stdfloat eval_segment_extended_point(int segment, PN_stdfloat t, int d) const;
  void eval_segment_extended_points(int segment, PN_stdfloat t, int d,
                                    PN_stdfloat result[], int num_values) const;
  INLINE PN_stdfloat get_segment_t(int segment, PN_stdfloat t) const;

  void adaptive_sample(PN_stdfloat tolerance);
  INLINE int get_num_samples() const;
  INLINE PN_stdfloat get_sample_t(int n) const;
  INLINE const LPoint3 &get_sample_point(int n) const;
  MAKE_SEQ(get_sample_ts, get_num_samples, get_sample_t);
  MAKE_SEQ(get_sample_points, get_num_samples, get_sample_point);

private:
  int find_segment(PN_stdfloat t);
  int r_find_segment(PN_stdfloat t, int top, int bot) const;

  void r_adaptive_sample(int segment, PN_stdfloat t0, const LPoint3 &p0,
                         PN_stdfloat t1, const LPoint3 &p1, PN_stdfloat tolerance_2);
  static PN_stdfloat sqr_dist_to_line(const LPoint3 &point, const LPoint3 &origin,
                                const LVector3 &vec);

  NurbsBasisVector _basis;
  const NurbsVertex *_verts;

  // We pre-compose the basis matrix and the geometry vectors, so we have
  // these handy for evaluation.  There is one entry in the _composed for each
  // entry in basis._segments.
  typedef epvector<LMatrix4> ComposedGeom;
  ComposedGeom _composed;

  int _last_segment;
  PN_stdfloat _last_from;
  PN_stdfloat _last_to;

  class AdaptiveSample {
  public:
    INLINE AdaptiveSample(PN_stdfloat t, const LPoint3 &point);
    PN_stdfloat _t;
    LPoint3 _point;
  };
  typedef pvector<AdaptiveSample> AdaptiveResult;
  AdaptiveResult _adaptive_result;
};

#include "nurbsCurveResult.I"

#endif
