// Filename: nurbsSurfaceResult.h
// Created by:  drose (10Oct03)
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

#ifndef NURBSSURFACERESULT_H
#define NURBSSURFACERESULT_H

#include "pandabase.h"
#include "referenceCount.h"
#include "nurbsBasisVector.h"

class NurbsVertex;

////////////////////////////////////////////////////////////////////
//       Class : NurbsSurfaceResult
// Description : The result of a NurbsSurfaceEvaluator.  This object
//               represents a surface in a particular coordinate space.
//               It can return the point and/or normal to the surface
//               at any point.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA NurbsSurfaceResult : public ReferenceCount {
public:
  NurbsSurfaceResult(const NurbsBasisVector &u_basis, 
                     const NurbsBasisVector &v_basis, 
                     const LVecBase4f vecs[], const NurbsVertex *verts,
                     int num_u_vertices, int num_v_vertices);

PUBLISHED:
  INLINE ~NurbsSurfaceResult();

  INLINE float get_start_u() const;
  INLINE float get_end_u() const;

  INLINE float get_start_v() const;
  INLINE float get_end_v() const;

  INLINE bool eval_point(float u, float v, LVecBase3f &point);
  INLINE bool eval_normal(float u, float v, LVecBase3f &normal);
  INLINE float eval_extended_point(float u, float v, int d);
  INLINE bool eval_extended_points(float u, float v, int d, 
                                   float result[], int num_values);
  
  INLINE int get_num_u_segments() const;
  INLINE int get_num_v_segments() const;
  void eval_segment_point(int ui, int vi, float u, float v, LVecBase3f &point) const;
  void eval_segment_normal(int ui, int vi, float u, float v, LVecBase3f &normal) const;
  float eval_segment_extended_point(int ui, int vi, float u, float v, int d) const;
  void eval_segment_extended_points(int ui, int vi, float u, float v, int d,
                                    float result[], int num_values) const;
  INLINE float get_segment_u(int ui, float u) const;
  INLINE float get_segment_v(int vi, float v) const;
  
private:
  INLINE int verti(int ui, int vi) const;
  INLINE int segi(int ui, int vi) const;

  int find_u_segment(float u);
  int r_find_u_segment(float u, int top, int bot) const;
  int find_v_segment(float v);
  int r_find_v_segment(float v, int top, int bot) const;

  NurbsBasisVector _u_basis;
  NurbsBasisVector _v_basis;
  const NurbsVertex *_verts;
  int _num_u_vertices;
  int _num_v_vertices;

  // We pre-compose the basis matrix and the geometry vectors, so we
  // have these handy for evaluation.  There is one entry in the
  // _composed for each entry in u_basis._segments *
  // v_basis._segments.
  class ComposedMats {
  public:
    LMatrix4f _x, _y, _z, _w;
  };
  typedef pvector<ComposedMats> ComposedGeom;
  ComposedGeom _composed;


  int _last_u_segment;
  float _last_u_from;
  float _last_u_to;
  int _last_v_segment;
  float _last_v_from;
  float _last_v_to;
};

#include "nurbsSurfaceResult.I"

#endif

