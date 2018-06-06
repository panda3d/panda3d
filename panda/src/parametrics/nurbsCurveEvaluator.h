/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file nurbsCurveEvaluator.h
 * @author drose
 * @date 2002-12-03
 */

#ifndef NURBSCURVEEVALUATOR_H
#define NURBSCURVEEVALUATOR_H

#include "pandabase.h"
#include "nurbsBasisVector.h"
#include "nurbsCurveResult.h"
#include "nurbsVertex.h"
#include "pointerTo.h"
#include "vector_stdfloat.h"
#include "pvector.h"
#include "epvector.h"
#include "nodePath.h"
#include "referenceCount.h"
#include "luse.h"

/**
 * This class is an abstraction for evaluating NURBS curves.  It accepts an
 * array of vertices, each of which may be in a different coordinate space (as
 * defined by a NodePath), as well as an optional knot vector.
 *
 * This is not related to NurbsCurve, CubicCurveseg or any of the
 * ParametricCurve-derived objects in this module.  It is a completely
 * parallel implementation of NURBS curves, and will probably eventually
 * replace the whole ParametricCurve class hierarchy.
 */
class EXPCL_PANDA_PARAMETRICS NurbsCurveEvaluator : public ReferenceCount {
PUBLISHED:
  NurbsCurveEvaluator();
  ~NurbsCurveEvaluator();

  INLINE void set_order(int order);
  INLINE int get_order() const;

  void reset(int num_vertices);

  INLINE int get_num_vertices() const;
  INLINE void set_vertex(int i, const LVecBase4 &vertex);
  INLINE void set_vertex(int i, const LVecBase3 &vertex, PN_stdfloat weight = 1.0);
  INLINE const LVecBase4 &get_vertex(int i) const;
  INLINE LVecBase4 get_vertex(int i, const NodePath &rel_to) const;
  MAKE_SEQ(get_vertices, get_num_vertices, get_vertex);

  INLINE void set_vertex_space(int i, const NodePath &space);
  INLINE void set_vertex_space(int i, const std::string &space);
  NodePath get_vertex_space(int i, const NodePath &rel_to) const;

  INLINE void set_extended_vertex(int i, int d, PN_stdfloat value);
  INLINE PN_stdfloat get_extended_vertex(int i, int d) const;
  void set_extended_vertices(int i, int d,
                             const PN_stdfloat values[], int num_values);

  INLINE int get_num_knots() const;
  void set_knot(int i, PN_stdfloat knot);
  PN_stdfloat get_knot(int i) const;
  MAKE_SEQ(get_knots, get_num_knots, get_knot);
  void normalize_knots();

  INLINE int get_num_segments() const;

  PT(NurbsCurveResult) evaluate(const NodePath &rel_to = NodePath()) const;
  PT(NurbsCurveResult) evaluate(const NodePath &rel_to,
                                const LMatrix4 &mat) const;

  void output(std::ostream &out) const;

public:
  typedef epvector<LVecBase4> Vert4Array;
  typedef pvector<LPoint3> Vert3Array;
  void get_vertices(Vert4Array &verts, const NodePath &rel_to) const;
  void get_vertices(Vert3Array &verts, const NodePath &rel_to) const;

private:
  void recompute_knots();
  void recompute_basis();

  int _order;

  typedef epvector<NurbsVertex> Vertices;
  Vertices _vertices;

  bool _knots_dirty;
  typedef vector_stdfloat Knots;
  Knots _knots;

  bool _basis_dirty;
  NurbsBasisVector _basis;
};

INLINE std::ostream &operator << (std::ostream &out, const NurbsCurveEvaluator &n);

#include "nurbsCurveEvaluator.I"

#endif
