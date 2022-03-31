/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file nurbsSurfaceEvaluator.I
 * @author drose
 * @date 2003-10-10
 */

/**
 * Sets the order of the surface in the U direction.  This resets the knot
 * vector to the default knot vector for the number of vertices.
 *
 * The order must be 1, 2, 3, or 4, and the value is one more than the degree
 * of the surface.
 */
INLINE void NurbsSurfaceEvaluator::
set_u_order(int u_order) {
  _u_order = u_order;
  _u_knots_dirty = true;
  _u_basis_dirty = true;
}

/**
 * Returns the order of the surface in the U direction as set by a previous
 * call to set_u_order().
 */
INLINE int NurbsSurfaceEvaluator::
get_u_order() const {
  return _u_order;
}

/**
 * Sets the order of the surface in the V direction.  This resets the knot
 * vector to the default knot vector for the number of vertices.
 *
 * The order must be 1, 2, 3, or 4, and the value is one more than the degree
 * of the surface.
 */
INLINE void NurbsSurfaceEvaluator::
set_v_order(int v_order) {
  _v_order = v_order;
  _v_knots_dirty = true;
  _v_basis_dirty = true;
}

/**
 * Returns the order of the surface in the V direction as set by a previous
 * call to set_v_order().
 */
INLINE int NurbsSurfaceEvaluator::
get_v_order() const {
  return _v_order;
}

/**
 * Returns the number of control vertices in the U direction on the surface.
 * This is the number passed to the last call to reset().
 */
INLINE int NurbsSurfaceEvaluator::
get_num_u_vertices() const {
  return _num_u_vertices;
}

/**
 * Returns the number of control vertices in the V direction on the surface.
 * This is the number passed to the last call to reset().
 */
INLINE int NurbsSurfaceEvaluator::
get_num_v_vertices() const {
  return _num_v_vertices;
}

/**
 * Sets the nth control vertex of the surface, as a vertex in 4-d homogeneous
 * space.  In this form, the first three components of the vertex should
 * already have been scaled by the fourth component, which is the homogeneous
 * weight.
 */
INLINE void NurbsSurfaceEvaluator::
set_vertex(int ui, int vi, const LVecBase4 &vertex) {
  nassertv(ui >= 0 && ui < _num_u_vertices &&
           vi >= 0 && vi < _num_v_vertices);
  vert(ui, vi).set_vertex(vertex);
}

/**
 * Sets the nth control vertex of the surface.  This flavor sets the vertex as
 * a 3-d coordinate and a weight; the 3-d coordinate values are implicitly
 * scaled up by the weight factor.
 */
INLINE void NurbsSurfaceEvaluator::
set_vertex(int ui, int vi, const LVecBase3 &vertex, PN_stdfloat weight) {
  nassertv(ui >= 0 && ui < _num_u_vertices &&
           vi >= 0 && vi < _num_v_vertices);
  vert(ui, vi).set_vertex(LVecBase4(vertex[0] * weight, vertex[1] * weight, vertex[2] * weight, weight));
}

/**
 * Returns the nth control vertex of the surface, relative to its indicated
 * coordinate space.
 */
INLINE const LVecBase4 &NurbsSurfaceEvaluator::
get_vertex(int ui, int vi) const {
  nassertr(ui >= 0 && ui < _num_u_vertices &&
           vi >= 0 && vi < _num_v_vertices, LVecBase4::zero());
  return vert(ui, vi).get_vertex();
}

/**
 * Returns the nth control vertex of the surface, relative to the given
 * coordinate space.
 */
INLINE LVecBase4 NurbsSurfaceEvaluator::
get_vertex(int ui, int vi, const NodePath &rel_to) const {
  nassertr(ui >= 0 && ui < _num_u_vertices &&
           vi >= 0 && vi < _num_v_vertices, LVecBase4::zero());

  NodePath space = vert(ui, vi).get_space(rel_to);
  const LVecBase4 &vertex = vert(ui, vi).get_vertex();
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
 * different NodePath for different instances of the surface.
 */
INLINE void NurbsSurfaceEvaluator::
set_vertex_space(int ui, int vi, const NodePath &space) {
  nassertv(ui >= 0 && ui < _num_u_vertices &&
           vi >= 0 && vi < _num_v_vertices);
  vert(ui, vi).set_space(space);
}

/**
 * Sets the coordinate space of the nth control vertex.  If this is not
 * specified, or is set to an empty string, the nth control vertex is deemed
 * to be in the coordinate space passed to evaluate().
 *
 * This specifies the space as a string, which describes the path to find the
 * node relative to the rel_to NodePath when the surface is evaluated.
 */
INLINE void NurbsSurfaceEvaluator::
set_vertex_space(int ui, int vi, const std::string &space) {
  nassertv(ui >= 0 && ui < _num_u_vertices &&
           vi >= 0 && vi < _num_v_vertices);
  vert(ui, vi).set_space(space);
}

/**
 * Sets an n-dimensional vertex value.  This allows definition of a NURBS
 * surface or surface in a sparse n-dimensional space, typically used for
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
INLINE void NurbsSurfaceEvaluator::
set_extended_vertex(int ui, int vi, int d, PN_stdfloat value) {
  nassertv(ui >= 0 && ui < _num_u_vertices &&
           vi >= 0 && vi < _num_v_vertices);
  vert(ui, vi).set_extended_vertex(d, value);
}

/**
 * Returns an n-dimensional vertex value.  See set_extended_vertex().  This
 * returns the value set for the indicated dimension, or 0.0 if nothing has
 * been set.
 */
INLINE PN_stdfloat NurbsSurfaceEvaluator::
get_extended_vertex(int ui, int vi, int d) const {
  nassertr(ui >= 0 && ui < _num_u_vertices &&
           vi >= 0 && vi < _num_v_vertices, 0.0f);
  return vert(ui, vi).get_extended_vertex(d);
}

/**
 * Returns the number of knot values in the surface in the U direction.  This
 * is based on the number of vertices and the order.
 */
INLINE int NurbsSurfaceEvaluator::
get_num_u_knots() const {
  return _num_u_vertices + _u_order;
}

/**
 * Returns the number of knot values in the surface in the V direction.  This
 * is based on the number of vertices and the order.
 */
INLINE int NurbsSurfaceEvaluator::
get_num_v_knots() const {
  return _num_v_vertices + _v_order;
}

/**
 * Returns the number of piecewise continuous segments in the surface in the U
 * direction.  This is based on the knot vector.
 */
INLINE int NurbsSurfaceEvaluator::
get_num_u_segments() const {
  if (_u_basis_dirty) {
    ((NurbsSurfaceEvaluator *)this)->recompute_u_basis();
  }
  return _u_basis.get_num_segments();
}

/**
 * Returns the number of piecewise continuous segments in the surface in the V
 * direction.  This is based on the knot vector.
 */
INLINE int NurbsSurfaceEvaluator::
get_num_v_segments() const {
  if (_v_basis_dirty) {
    ((NurbsSurfaceEvaluator *)this)->recompute_v_basis();
  }
  return _v_basis.get_num_segments();
}

/**
 * Internal accessor to dereference the 2-d vertex coordinate pair into a
 * linear list of vertices.
 */
INLINE NurbsVertex &NurbsSurfaceEvaluator::
vert(int ui, int vi) {
  return _vertices[ui * _num_v_vertices + vi];
}

/**
 * Internal accessor to dereference the 2-d vertex coordinate pair into a
 * linear list of vertices.
 */
INLINE const NurbsVertex &NurbsSurfaceEvaluator::
vert(int ui, int vi) const {
  return _vertices[ui * _num_v_vertices + vi];
}

INLINE std::ostream &
operator << (std::ostream &out, const NurbsSurfaceEvaluator &n) {
  n.output(out);
  return out;
}
