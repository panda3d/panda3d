/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file nurbsSurfaceEvaluator.h
 * @author drose
 * @date 2003-10-10
 */

#ifndef NURBSSURFACEEVALUATOR_H
#define NURBSSURFACEEVALUATOR_H

#include "pandabase.h"
#include "nurbsBasisVector.h"
#include "nurbsSurfaceResult.h"
#include "nurbsVertex.h"
#include "pointerTo.h"
#include "vector_stdfloat.h"
#include "pvector.h"
#include "epvector.h"
#include "nodePath.h"
#include "referenceCount.h"
#include "luse.h"

/**
 * This class is an abstraction for evaluating NURBS surfaces.  It accepts an
 * array of vertices, each of which may be in a different coordinate space (as
 * defined by a NodePath), as well as an optional knot vector.
 */
class EXPCL_PANDA_PARAMETRICS NurbsSurfaceEvaluator : public ReferenceCount {
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
  INLINE void set_vertex(int ui, int vi, const LVecBase4 &vertex);
  INLINE void set_vertex(int ui, int vi, const LVecBase3 &vertex, PN_stdfloat weight = 1.0);
  INLINE const LVecBase4 &get_vertex(int ui, int vi) const;
  INLINE LVecBase4 get_vertex(int ui, int vi, const NodePath &rel_to) const;

  INLINE void set_vertex_space(int ui, int vi, const NodePath &space);
  INLINE void set_vertex_space(int ui, int vi, const std::string &space);
  NodePath get_vertex_space(int ui, int vi, const NodePath &rel_to) const;

  INLINE void set_extended_vertex(int ui, int vi, int d, PN_stdfloat value);
  INLINE PN_stdfloat get_extended_vertex(int ui, int vi, int d) const;
  void set_extended_vertices(int ui, int vi, int d,
                             const PN_stdfloat values[], int num_values);

  INLINE int get_num_u_knots() const;
  void set_u_knot(int i, PN_stdfloat knot);
  PN_stdfloat get_u_knot(int i) const;
  MAKE_SEQ(get_u_knots, get_num_u_knots, get_u_knot);
  void normalize_u_knots();

  INLINE int get_num_v_knots() const;
  void set_v_knot(int i, PN_stdfloat knot);
  PN_stdfloat get_v_knot(int i) const;
  MAKE_SEQ(get_v_knots, get_num_v_knots, get_v_knot);
  void normalize_v_knots();

  INLINE int get_num_u_segments() const;
  INLINE int get_num_v_segments() const;

  PT(NurbsSurfaceResult) evaluate(const NodePath &rel_to = NodePath()) const;

  void output(std::ostream &out) const;

  MAKE_PROPERTY(u_order, get_u_order, set_u_order);
  MAKE_PROPERTY(v_order, get_v_order, set_v_order);
  MAKE_SEQ_PROPERTY(u_knots, get_num_u_knots, get_u_knot, set_u_knot);
  MAKE_SEQ_PROPERTY(v_knots, get_num_v_knots, get_v_knot, set_v_knot);

public:
  typedef epvector<LVecBase4> Vert4Array;
  typedef pvector<LPoint3> Vert3Array;
  void get_vertices(Vert4Array &verts, const NodePath &rel_to) const;
  void get_vertices(Vert3Array &verts, const NodePath &rel_to) const;

private:
  INLINE NurbsVertex &vert(int ui, int vi);
  INLINE const NurbsVertex &vert(int ui, int vi) const;

  void recompute_u_knots();
  void recompute_v_knots();
  void recompute_u_basis();
  void recompute_v_basis();

  int _u_order;
  int _v_order;

  typedef epvector<NurbsVertex> Vertices;
  Vertices _vertices;
  int _num_u_vertices;
  int _num_v_vertices;

  bool _u_knots_dirty;
  bool _v_knots_dirty;
  typedef vector_stdfloat Knots;
  Knots _u_knots;
  Knots _v_knots;

  bool _u_basis_dirty;
  bool _v_basis_dirty;
  NurbsBasisVector _u_basis;
  NurbsBasisVector _v_basis;
};

INLINE std::ostream &operator << (std::ostream &out, const NurbsSurfaceEvaluator &n);

#include "nurbsSurfaceEvaluator.I"

#endif
