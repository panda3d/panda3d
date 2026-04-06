/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file nurbsCurveEvaluator.I
 * @author drose
 * @date 2002-12-05
 */

/**
 * Sets the order of the curve.  This resets the knot vector to the default
 * knot vector for the number of vertices.
 *
 * The order must be 1, 2, 3, or 4, and the value is one more than the degree
 * of the curve.
 */
INLINE void NurbsCurveEvaluator::
set_order(int order) {
  _order = order;
  _knots_dirty = true;
  _basis_dirty = true;
}

/**
 * Returns the order of the curve as set by a previous call to set_order().
 */
INLINE int NurbsCurveEvaluator::
get_order() const {
  return _order;
}

/**
 * Returns the number of control vertices in the curve.  This is the number
 * passed to the last call to reset().
 */
INLINE int NurbsCurveEvaluator::
get_num_vertices() const {
  return (int)_vertices.size();
}

/**
 * Sets the nth control vertex of the curve, as a vertex in 4-d homogeneous
 * space.  In this form, the first three components of the vertex should
 * already have been scaled by the fourth component, which is the homogeneous
 * weight.
 */
INLINE void NurbsCurveEvaluator::
set_vertex(int i, const LVecBase4 &vertex) {
  nassertv(i >= 0 && i < (int)_vertices.size());
  _vertices[i].set_vertex(vertex);
}

/**
 * Sets the nth control vertex of the curve.  This flavor sets the vertex as a
 * 3-d coordinate and a weight; the 3-d coordinate values are implicitly
 * scaled up by the weight factor.
 */
INLINE void NurbsCurveEvaluator::
set_vertex(int i, const LVecBase3 &vertex, PN_stdfloat weight) {
  nassertv(i >= 0 && i < (int)_vertices.size());
  _vertices[i].set_vertex(LVecBase4(vertex[0] * weight, vertex[1] * weight, vertex[2] * weight, weight));
}

/**
 * Returns the nth control vertex of the curve, relative to its indicated
 * coordinate space.
 */
INLINE const LVecBase4 &NurbsCurveEvaluator::
get_vertex(int i) const {
  nassertr(i >= 0 && i < (int)_vertices.size(), LVecBase4::zero());
  return _vertices[i].get_vertex();
}

/**
 * Returns the nth control vertex of the curve, relative to the given
 * coordinate space.
 */
INLINE LVecBase4 NurbsCurveEvaluator::
get_vertex(int i, const NodePath &rel_to) const {
  nassertr(i >= 0 && i < (int)_vertices.size(), LVecBase4::zero());

  NodePath space = _vertices[i].get_space(rel_to);
  const LVecBase4 &vertex = _vertices[i].get_vertex();
  if (space.is_empty()) {
    return vertex;
  } else {
    const LMatrix4 &mat = space.get_mat(rel_to);
    return vertex * mat;
  }
}

/**
 * Sets the coordinate space of the nth control vertex.  If this is not
 * specified, or is set to an empty NodePath, the nth control vertex is deemed
 * to be in the coordinate space passed to evaluate().
 *
 * This specifies the space as a fixed NodePath, which is always the same
 * NodePath.  Also see setting the space as a path string, which can specify a
 * different NodePath for different instances of the curve.
 */
INLINE void NurbsCurveEvaluator::
set_vertex_space(int i, const NodePath &space) {
  nassertv(i >= 0 && i < (int)_vertices.size());
  _vertices[i].set_space(space);
}

/**
 * Sets the coordinate space of the nth control vertex.  If this is not
 * specified, or is set to an empty string, the nth control vertex is deemed
 * to be in the coordinate space passed to evaluate().
 *
 * This specifies the space as a string, which describes the path to find the
 * node relative to the rel_to NodePath when the curve is evaluated.
 */
INLINE void NurbsCurveEvaluator::
set_vertex_space(int i, const std::string &space) {
  nassertv(i >= 0 && i < (int)_vertices.size());
  _vertices[i].set_space(space);
}

/**
 * Sets an n-dimensional vertex value.  This allows definition of a NURBS
 * surface or curve in a sparse n-dimensional space, typically used for
 * associating additional properties (like color or joint membership) with
 * each vertex of a surface.
 *
 * The value d is an arbitrary integer value and specifies the dimension of
 * question for this particular vertex.  Any number of dimensions may be
 * specified, and they need not be consecutive.  If a value for a given
 * dimension is not specified, is it implicitly 0.0.
 *
 * The value is implicitly scaled by the homogenous weight value--that is, the
 * fourth component of the value passed to set_vertex().  This means the
 * ordinary vertex must be set first, before the extended vertices can be set.
 */
INLINE void NurbsCurveEvaluator::
set_extended_vertex(int i, int d, PN_stdfloat value) {
  nassertv(i >= 0 && i < (int)_vertices.size());
  _vertices[i].set_extended_vertex(d, value);
}

/**
 * Returns an n-dimensional vertex value.  See set_extended_vertex().  This
 * returns the value set for the indicated dimension, or 0.0 if nothing has
 * been set.
 */
INLINE PN_stdfloat NurbsCurveEvaluator::
get_extended_vertex(int i, int d) const {
  nassertr(i >= 0 && i < (int)_vertices.size(), 0.0f);
  return _vertices[i].get_extended_vertex(d);
}

/**
 * Returns the number of knot values in the curve.  This is based on the
 * number of vertices and the order.
 */
INLINE int NurbsCurveEvaluator::
get_num_knots() const {
  return (int)_vertices.size() + _order;
}

/**
 * Returns the number of piecewise continuous segments in the curve.  This is
 * based on the knot vector.
 */
INLINE int NurbsCurveEvaluator::
get_num_segments() const {
  if (_basis_dirty) {
    ((NurbsCurveEvaluator *)this)->recompute_basis();
  }
  return _basis.get_num_segments();
}

INLINE std::ostream &
operator << (std::ostream &out, const NurbsCurveEvaluator &n) {
  n.output(out);
  return out;
}
