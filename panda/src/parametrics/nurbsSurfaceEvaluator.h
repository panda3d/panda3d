// Filename: nurbsSurfaceEvaluator.h
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

#ifndef NURBSSURFACEEVALUATOR_H
#define NURBSSURFACEEVALUATOR_H

#include "pandabase.h"
#include "nurbsBasisVector.h"
#include "nurbsSurfaceResult.h"
#include "nurbsVertex.h"
#include "pointerTo.h"
#include "vector_float.h"
#include "pvector.h"
#include "nodePath.h"
#include "referenceCount.h"
#include "luse.h"

////////////////////////////////////////////////////////////////////
//       Class : NurbsSurfaceEvaluator
// Description : This class is an abstraction for evaluating NURBS
//               surfaces.  It accepts an array of vertices, each of
//               which may be in a different coordinate space (as
//               defined by a NodePath), as well as an optional knot
//               vector.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA NurbsSurfaceEvaluator : public ReferenceCount {
PUBLISHED:
  NurbsSurfaceEvaluator();
  ~NurbsSurfaceEvaluator();

  INLINE void set_u_order(int u_order);
  INLINE int get_u_order() const;

  INLINE void set_v_order(int v_order);
  INLINE int get_v_order() const;

  void reset(int num_u_vertices, int num_v_vertices);

  INLINE int get_num_u_vertices() const;
  INLINE int get_num_v_vertices() const;
  INLINE void set_vertex(int ui, int vi, const LVecBase4f &vertex);
  INLINE void set_vertex(int ui, int vi, const LVecBase3f &vertex, float weight = 1.0);
  INLINE const LVecBase4f &get_vertex(int ui, int vi) const;
  INLINE LVecBase4f get_vertex(int ui, int vi, const NodePath &rel_to) const;

  INLINE void set_vertex_space(int ui, int vi, const NodePath &space);
  INLINE void set_vertex_space(int ui, int vi, const string &space);
  NodePath get_vertex_space(int ui, int vi, const NodePath &rel_to) const;

  INLINE void set_extended_vertex(int ui, int vi, int d, float value);
  INLINE float get_extended_vertex(int ui, int vi, int d) const;
  void set_extended_vertices(int ui, int vi, int d, 
                             const float values[], int num_values);

  INLINE int get_num_u_knots() const;
  void set_u_knot(int i, float knot);
  float get_u_knot(int i) const;
  void normalize_u_knots();

  INLINE int get_num_v_knots() const;
  void set_v_knot(int i, float knot);
  float get_v_knot(int i) const;
  void normalize_v_knots();

  INLINE int get_num_u_segments() const;
  INLINE int get_num_v_segments() const;

  PT(NurbsSurfaceResult) evaluate(const NodePath &rel_to = NodePath()) const;

  void output(ostream &out) const;

public:
  void get_vertices(pvector<LVecBase4f> &verts, const NodePath &rel_to) const;
  void get_vertices(pvector<LPoint3f> &verts, const NodePath &rel_to) const;

private:
  INLINE NurbsVertex &vert(int ui, int vi);
  INLINE const NurbsVertex &vert(int ui, int vi) const;

  void recompute_u_knots();
  void recompute_v_knots();
  void recompute_u_basis();
  void recompute_v_basis();

  int _u_order;
  int _v_order;

  typedef pvector<NurbsVertex> Vertices;
  Vertices _vertices;
  int _num_u_vertices;
  int _num_v_vertices;

  bool _u_knots_dirty;
  bool _v_knots_dirty;
  typedef vector_float Knots;
  Knots _u_knots;
  Knots _v_knots;

  bool _u_basis_dirty;
  bool _v_basis_dirty;
  NurbsBasisVector _u_basis;
  NurbsBasisVector _v_basis;
};

INLINE ostream &operator << (ostream &out, const NurbsSurfaceEvaluator &n);

#include "nurbsSurfaceEvaluator.I"

#endif

