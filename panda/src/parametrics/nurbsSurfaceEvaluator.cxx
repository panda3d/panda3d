/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file nurbsSurfaceEvaluator.cxx
 * @author drose
 * @date 2003-10-10
 */

#include "nurbsSurfaceEvaluator.h"

/**
 *
 */
NurbsSurfaceEvaluator::
NurbsSurfaceEvaluator() {
  _u_order = 4;
  _v_order = 4;
  _u_knots_dirty = true;
  _v_knots_dirty = true;
  _u_basis_dirty = true;
  _v_basis_dirty = true;
}

/**
 *
 */
NurbsSurfaceEvaluator::
~NurbsSurfaceEvaluator() {
}

/**
 * Resets all the vertices and knots to their default values, and sets the
 * surface up with the indicated number of vertices.  You must then call
 * set_vertex() repeatedly to fill in all of the vertex values appropriately.
 */
void NurbsSurfaceEvaluator::
reset(int num_u_vertices, int num_v_vertices) {
  int num_vertices = num_u_vertices * num_v_vertices;
  _vertices.clear();
  _vertices.reserve(num_vertices);
  _num_u_vertices = num_u_vertices;
  _num_v_vertices = num_v_vertices;

  for (int i = 0; i < num_vertices; i++) {
    _vertices.push_back(NurbsVertex());
  }
  _u_knots_dirty = true;
  _v_knots_dirty = true;
  _u_basis_dirty = true;
  _v_basis_dirty = true;
}

/**
 * Returns the coordinate space of the nth control vertex of the surface,
 * expressed as a NodePath.
 */
NodePath NurbsSurfaceEvaluator::
get_vertex_space(int ui, int vi, const NodePath &rel_to) const {
#ifndef NDEBUG
  static NodePath empty_node_path;
  nassertr(ui >= 0 && ui < _num_u_vertices &&
           vi >= 0 && vi < _num_v_vertices, empty_node_path);
#endif
  return vert(ui, vi).get_space(rel_to);
}

/**
 * Simultaneously sets several extended values in the slots d through (d +
 * num_values - 1) from the num_values elements of the indicated array.  This
 * is equivalent to calling set_extended_vertex() num_values times.  See
 * set_extended_vertex().
 */
void NurbsSurfaceEvaluator::
set_extended_vertices(int ui, int vi, int d,
                      const PN_stdfloat values[], int num_values) {
  nassertv(ui >= 0 && ui < _num_u_vertices &&
           vi >= 0 && vi < _num_v_vertices);

  NurbsVertex &vertex = vert(ui, vi);
  for (int n = 0; n < num_values; n++) {
    vertex.set_extended_vertex(d + n, values[n]);
  }
}

/**
 * Sets the value of the nth knot.  Each knot value should be greater than or
 * equal to the preceding value.  If no knot values are set, a default knot
 * vector is supplied.
 */
void NurbsSurfaceEvaluator::
set_u_knot(int i, PN_stdfloat knot) {
  if (_u_knots_dirty) {
    recompute_u_knots();
  }
  nassertv(i >= 0 && i < (int)_u_knots.size());
  _u_knots[i] = knot;
}

/**
 * Returns the value of the nth knot.
 */
PN_stdfloat NurbsSurfaceEvaluator::
get_u_knot(int i) const {
  if (_u_knots_dirty) {
    ((NurbsSurfaceEvaluator *)this)->recompute_u_knots();
  }
  nassertr(i >= 0 && i < (int)_u_knots.size(), 0.0f);
  return _u_knots[i];
}

/**
 * Normalizes the knot sequence so that the parametric range of the surface in
 * the U direction is 0 .. 1.
 */
void NurbsSurfaceEvaluator::
normalize_u_knots() {
  if (_u_knots_dirty) {
    recompute_u_knots();
  }

  if (_num_u_vertices > _u_order - 1) {
    double min_value = _u_knots[_u_order - 1];
    double max_value = _u_knots[_num_u_vertices];
    double range = (max_value - min_value);

    for (Knots::iterator ki = _u_knots.begin(); ki != _u_knots.end(); ++ki) {
      (*ki) = ((*ki) - min_value) / range;
    }
    _u_basis_dirty = true;
  }
}

/**
 * Sets the value of the nth knot.  Each knot value should be greater than or
 * equal to the preceding value.  If no knot values are set, a default knot
 * vector is supplied.
 */
void NurbsSurfaceEvaluator::
set_v_knot(int i, PN_stdfloat knot) {
  if (_v_knots_dirty) {
    recompute_v_knots();
  }
  nassertv(i >= 0 && i < (int)_v_knots.size());
  _v_knots[i] = knot;
}

/**
 * Returns the value of the nth knot.
 */
PN_stdfloat NurbsSurfaceEvaluator::
get_v_knot(int i) const {
  if (_v_knots_dirty) {
    ((NurbsSurfaceEvaluator *)this)->recompute_v_knots();
  }
  nassertr(i >= 0 && i < (int)_v_knots.size(), 0.0f);
  return _v_knots[i];
}

/**
 * Normalizes the knot sequence so that the parametric range of the surface in
 * the U direction is 0 .. 1.
 */
void NurbsSurfaceEvaluator::
normalize_v_knots() {
  if (_v_knots_dirty) {
    recompute_v_knots();
  }

  if (_num_v_vertices > _v_order - 1) {
    double min_value = _v_knots[_v_order - 1];
    double max_value = _v_knots[_num_v_vertices];
    double range = (max_value - min_value);

    for (Knots::iterator ki = _v_knots.begin(); ki != _v_knots.end(); ++ki) {
      (*ki) = ((*ki) - min_value) / range;
    }
    _v_basis_dirty = true;
  }
}

/**
 * Returns a NurbsSurfaceResult object that represents the result of applying
 * the knots to all of the current values of the vertices, transformed into
 * the indicated coordinate space.
 */
PT(NurbsSurfaceResult) NurbsSurfaceEvaluator::
evaluate(const NodePath &rel_to) const {
  if (_u_basis_dirty) {
    ((NurbsSurfaceEvaluator *)this)->recompute_u_basis();
  }
  if (_v_basis_dirty) {
    ((NurbsSurfaceEvaluator *)this)->recompute_v_basis();
  }

  // First, transform the vertices as appropriate.
  Vert4Array vecs;
  get_vertices(vecs, rel_to);

  // And apply those transformed vertices to the basis matrices to derive the
  // result.
  return new NurbsSurfaceResult(_u_basis, _v_basis,
                                &vecs[0], &_vertices[0],
                                _num_u_vertices, _num_v_vertices);
}

/**
 *
 */
void NurbsSurfaceEvaluator::
output(std::ostream &out) const {
  out << "NurbsSurface, (" << get_num_u_knots() << ", " << get_num_v_knots()
      << ") knots.";
}

/**
 * Fills the indicated vector with the set of vertices in the surface,
 * transformed to the given space.  This flavor returns the vertices in
 * 4-dimensional homogenous space.
 *
 * Vertices are arranged in linear sequence, with the v coordinate changing
 * more rapidly.
 */
void NurbsSurfaceEvaluator::
get_vertices(NurbsSurfaceEvaluator::Vert4Array &verts, const NodePath &rel_to) const {
  int num_vertices = (int)_vertices.size();
  verts.reserve(verts.size() + num_vertices);
  int vi;
  for (vi = 0; vi < num_vertices; vi++) {
    NodePath space = _vertices[vi].get_space(rel_to);
    const LVecBase4 &vertex = _vertices[vi].get_vertex();
    if (space.is_empty()) {
      verts.push_back(vertex);
    } else {
      CPT(TransformState) transform = space.get_transform(rel_to);
      const LMatrix4 &mat = transform->get_mat();
      verts.push_back(vertex * mat);
    }
  }
}

/**
 * Fills the indicated vector with the set of vertices in the surface,
 * transformed to the given space.  This flavor returns the vertices in
 * 3-dimensional space.
 *
 * Vertices are arranged in linear sequence, with the v coordinate changing
 * more rapidly.
 */
void NurbsSurfaceEvaluator::
get_vertices(NurbsSurfaceEvaluator::Vert3Array &verts, const NodePath &rel_to) const {
  int num_vertices = (int)_vertices.size();
  verts.reserve(verts.size() + num_vertices);
  int vi;
  for (vi = 0; vi < num_vertices; vi++) {
    const NodePath &space = _vertices[vi].get_space(rel_to);
    LVecBase4 vertex = _vertices[vi].get_vertex();
    if (!space.is_empty()) {
      CPT(TransformState) transform = space.get_transform(rel_to);
      const LMatrix4 &mat = transform->get_mat();
      vertex = vertex * mat;
    }
    LPoint3 v3(vertex[0] / vertex[3], vertex[1] / vertex[3], vertex[2] / vertex[3]);
    verts.push_back(v3);
  }
}

/**
 * Creates a default knot vector.
 */
void NurbsSurfaceEvaluator::
recompute_u_knots() {
  _u_knots.clear();
  int num_knots = get_num_u_knots();
  _u_knots.reserve(num_knots);

  PN_stdfloat value = 0.0f;

  int i = 0;
  while (i < _u_order) {
    _u_knots.push_back(value);
    i++;
  }
  while (i < num_knots - _u_order) {
    value += 1.0f;
    _u_knots.push_back(value);
    i++;
  }
  value += 1.0f;
  while (i < num_knots) {
    _u_knots.push_back(value);
    i++;
  }

  _u_knots_dirty = false;
}

/**
 * Creates a default knot vector.
 */
void NurbsSurfaceEvaluator::
recompute_v_knots() {
  _v_knots.clear();
  int num_knots = get_num_v_knots();
  _v_knots.reserve(num_knots);

  PN_stdfloat value = 0.0f;

  int i = 0;
  while (i < _v_order) {
    _v_knots.push_back(value);
    i++;
  }
  while (i < num_knots - _v_order) {
    value += 1.0f;
    _v_knots.push_back(value);
    i++;
  }
  value += 1.0f;
  while (i < num_knots) {
    _v_knots.push_back(value);
    i++;
  }

  _v_knots_dirty = false;
}

/**
 * Recomputes the basis matrices according to the knot vector.
 */
void NurbsSurfaceEvaluator::
recompute_u_basis() {
  if (_u_knots_dirty) {
    ((NurbsSurfaceEvaluator *)this)->recompute_u_knots();
  }

  _u_basis.clear(_u_order);
  if (_num_u_vertices > _u_order - 1) {
    int min_knot = _u_order;
    int max_knot = _num_u_vertices;

    for (int i = min_knot; i <= max_knot; i++) {
      nassertv(i - 1 >= 0 && i < (int)_u_knots.size());
      if (_u_knots[i - 1] < _u_knots[i]) {
        // Here's a non-empty segment.
        _u_basis.append_segment(i - _u_order, &_u_knots[i - _u_order]);
      }
    }
  }

  _u_basis_dirty = false;
}

/**
 * Recomputes the basis matrices according to the knot vector.
 */
void NurbsSurfaceEvaluator::
recompute_v_basis() {
  if (_v_knots_dirty) {
    ((NurbsSurfaceEvaluator *)this)->recompute_v_knots();
  }

  _v_basis.clear(_v_order);
  if (_num_v_vertices > _v_order - 1) {
    int min_knot = _v_order;
    int max_knot = _num_v_vertices;

    for (int i = min_knot; i <= max_knot; i++) {
      nassertv(i - 1 >= 0 && i < (int)_v_knots.size());
      if (_v_knots[i - 1] < _v_knots[i]) {
        // Here's a non-empty segment.
        _v_basis.append_segment(i - _v_order, &_v_knots[i - _v_order]);
      }
    }
  }

  _v_basis_dirty = false;
}
