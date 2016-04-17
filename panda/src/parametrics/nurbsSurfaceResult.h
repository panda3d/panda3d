/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file nurbsSurfaceResult.h
 * @author drose
 * @date 2003-10-10
 */

#ifndef NURBSSURFACERESULT_H
#define NURBSSURFACERESULT_H

#include "pandabase.h"
#include "referenceCount.h"
#include "nurbsBasisVector.h"
#include "epvector.h"

class NurbsVertex;

/**
 * The result of a NurbsSurfaceEvaluator.  This object represents a surface in
 * a particular coordinate space.  It can return the point and/or normal to
 * the surface at any point.
 */
class EXPCL_PANDA_PARAMETRICS NurbsSurfaceResult : public ReferenceCount {
public:
  NurbsSurfaceResult(const NurbsBasisVector &u_basis,
                     const NurbsBasisVector &v_basis,
                     const LVecBase4 vecs[], const NurbsVertex *verts,
                     int num_u_vertices, int num_v_vertices);

PUBLISHED:
  INLINE ~NurbsSurfaceResult();

  INLINE PN_stdfloat get_start_u() const;
  INLINE PN_stdfloat get_end_u() const;

  INLINE PN_stdfloat get_start_v() const;
  INLINE PN_stdfloat get_end_v() const;

  INLINE bool eval_point(PN_stdfloat u, PN_stdfloat v, LVecBase3 &point);
  INLINE bool eval_normal(PN_stdfloat u, PN_stdfloat v, LVecBase3 &normal);
  INLINE PN_stdfloat eval_extended_point(PN_stdfloat u, PN_stdfloat v, int d);
  INLINE bool eval_extended_points(PN_stdfloat u, PN_stdfloat v, int d,
                                   PN_stdfloat result[], int num_values);

  INLINE int get_num_u_segments() const;
  INLINE int get_num_v_segments() const;
  void eval_segment_point(int ui, int vi, PN_stdfloat u, PN_stdfloat v, LVecBase3 &point) const;
  void eval_segment_normal(int ui, int vi, PN_stdfloat u, PN_stdfloat v, LVecBase3 &normal) const;
  PN_stdfloat eval_segment_extended_point(int ui, int vi, PN_stdfloat u, PN_stdfloat v, int d) const;
  void eval_segment_extended_points(int ui, int vi, PN_stdfloat u, PN_stdfloat v, int d,
                                    PN_stdfloat result[], int num_values) const;
  INLINE PN_stdfloat get_segment_u(int ui, PN_stdfloat u) const;
  INLINE PN_stdfloat get_segment_v(int vi, PN_stdfloat v) const;

private:
  INLINE int verti(int ui, int vi) const;
  INLINE int segi(int ui, int vi) const;

  int find_u_segment(PN_stdfloat u);
  int r_find_u_segment(PN_stdfloat u, int top, int bot) const;
  int find_v_segment(PN_stdfloat v);
  int r_find_v_segment(PN_stdfloat v, int top, int bot) const;

  NurbsBasisVector _u_basis;
  NurbsBasisVector _v_basis;
  const NurbsVertex *_verts;
  int _num_u_vertices;
  int _num_v_vertices;

  // We pre-compose the basis matrix and the geometry vectors, so we have
  // these handy for evaluation.  There is one entry in the _composed for each
  // entry in u_basis._segments * v_basis._segments.
  class ComposedMats {
  public:
    LMatrix4 _x, _y, _z, _w;
  };
  typedef epvector<ComposedMats> ComposedGeom;
  ComposedGeom _composed;


  int _last_u_segment;
  PN_stdfloat _last_u_from;
  PN_stdfloat _last_u_to;
  int _last_v_segment;
  PN_stdfloat _last_v_from;
  PN_stdfloat _last_v_to;
};

#include "nurbsSurfaceResult.I"

#endif
