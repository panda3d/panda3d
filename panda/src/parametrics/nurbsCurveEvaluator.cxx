/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file nurbsCurveEvaluator.cxx
 * @author drose
 * @date 2002-12-03
 */

#include "nurbsCurveEvaluator.h"

/**
 *
 */
NurbsCurveEvaluator::
NurbsCurveEvaluator() {
  _order = 4;
  _knots_dirty = true;
  _basis_dirty = true;
}

/**
 *
 */
NurbsCurveEvaluator::
~NurbsCurveEvaluator() {
}

/**
 * Resets all the vertices and knots to their default values, and sets the
 * curve up with the indicated number of vertices.  You must then call
 * set_vertex() repeatedly to fill in all of the vertex values appropriately.
 */
void NurbsCurveEvaluator::
reset(int num_vertices) {
  _vertices.clear();
  _vertices.reserve(num_vertices);

  for (int i = 0; i < num_vertices; i++) {
    _vertices.push_back(NurbsVertex());
  }
  _knots_dirty = true;
  _basis_dirty = true;
}

/**
 * Returns the coordinate space of the nth control vertex of the curve,
 * expressed as a NodePath.
 */
NodePath NurbsCurveEvaluator::
get_vertex_space(int i, const NodePath &rel_to) const {
#ifndef NDEBUG
  static NodePath empty_node_path;
  nassertr(i >= 0 && i < (int)_vertices.size(), empty_node_path);
#endif
  return _vertices[i].get_space(rel_to);
}

/**
 * Simultaneously sets several extended values in the slots d through (d +
 * num_values - 1) from the num_values elements of the indicated array.  This
 * is equivalent to calling set_extended_vertex() num_values times.  See
 * set_extended_vertex().
 */
void NurbsCurveEvaluator::
set_extended_vertices(int i, int d, const PN_stdfloat values[], int num_values) {
  nassertv(i >= 0 && i < (int)_vertices.size());

  NurbsVertex &vertex = _vertices[i];
  for (int n = 0; n < num_values; n++) {
    vertex.set_extended_vertex(d + n, values[n]);
  }
}

/**
 * Sets the value of the nth knot.  Each knot value should be greater than or
 * equal to the preceding value.  If no knot values are set, a default knot
 * vector is supplied.
 */
void NurbsCurveEvaluator::
set_knot(int i, PN_stdfloat knot) {
  if (_knots_dirty) {
    recompute_knots();
  }
  nassertv(i >= 0 && i < (int)_knots.size());
  _knots[i] = knot;
}

/**
 * Returns the value of the nth knot.
 */
PN_stdfloat NurbsCurveEvaluator::
get_knot(int i) const {
  if (_knots_dirty) {
    ((NurbsCurveEvaluator *)this)->recompute_knots();
  }
  nassertr(i >= 0 && i < (int)_knots.size(), 0.0f);
  return _knots[i];
}

/**
 * Normalizes the knot sequence so that the parametric range of the curve is 0
 * .. 1.
 */
void NurbsCurveEvaluator::
normalize_knots() {
  if (_knots_dirty) {
    recompute_knots();
  }

  if (get_num_vertices() > _order - 1) {
    double min_value = _knots[_order - 1];
    double max_value = _knots[get_num_vertices()];
    double range = (max_value - min_value);

    for (Knots::iterator ki = _knots.begin(); ki != _knots.end(); ++ki) {
      (*ki) = ((*ki) - min_value) / range;
    }
    _basis_dirty = true;
  }
}

/**
 * Returns a NurbsCurveResult object that represents the result of applying
 * the knots to all of the current values of the vertices, transformed into
 * the indicated coordinate space.
 */
PT(NurbsCurveResult) NurbsCurveEvaluator::
evaluate(const NodePath &rel_to) const {
  if (_basis_dirty) {
    ((NurbsCurveEvaluator *)this)->recompute_basis();
  }

  // First, transform the vertices as appropriate.
  Vert4Array vecs;
  get_vertices(vecs, rel_to);

  // And apply those transformed vertices to the basis matrices to derive the
  // result.
  return new NurbsCurveResult(_basis, &vecs[0], &_vertices[0],
                              (int)_vertices.size());
}

/**
 * Returns a NurbsCurveResult object that represents the result of applying
 * the knots to all of the current values of the vertices, transformed into
 * the indicated coordinate space, and then further transformed by the
 * indicated matrix.
 */
PT(NurbsCurveResult) NurbsCurveEvaluator::
evaluate(const NodePath &rel_to, const LMatrix4 &mat) const {
  if (_basis_dirty) {
    ((NurbsCurveEvaluator *)this)->recompute_basis();
  }

  // First, transform the vertices as appropriate.
  Vert4Array vecs;
  get_vertices(vecs, rel_to);

  // And then apply the indicated matrix.
  Vert4Array::iterator vi;
  for (vi = vecs.begin(); vi != vecs.end(); ++vi) {
    (*vi) = (*vi) * mat;
  }

  // And apply those transformed vertices to the basis matrices to derive the
  // result.
  return new NurbsCurveResult(_basis, &vecs[0], &_vertices[0],
                              (int)_vertices.size());
}

/**
 *
 */
void NurbsCurveEvaluator::
output(std::ostream &out) const {
  out << "NurbsCurve, " << get_num_knots() << " knots.";
}


/**
 * Fills the indicated vector with the set of vertices in the curve,
 * transformed to the given space.  This flavor returns the vertices in
 * 4-dimensional homogenous space.
 */
void NurbsCurveEvaluator::
get_vertices(NurbsCurveEvaluator::Vert4Array &verts, const NodePath &rel_to) const {
  int num_vertices = (int)_vertices.size();
  verts.reserve(verts.size() + num_vertices);
  int vi;
  for (vi = 0; vi < num_vertices; vi++) {
    verts.push_back(get_vertex(vi, rel_to));
  }
}

/**
 * Fills the indicated vector with the set of vertices in the curve,
 * transformed to the given space.  This flavor returns the vertices in
 * 3-dimensional space.
 */
void NurbsCurveEvaluator::
get_vertices(NurbsCurveEvaluator::Vert3Array &verts, const NodePath &rel_to) const {
  int num_vertices = (int)_vertices.size();
  verts.reserve(verts.size() + num_vertices);
  int vi;
  for (vi = 0; vi < num_vertices; vi++) {
    LVecBase4 vertex = get_vertex(vi, rel_to);
    // Avoid division by zero
    if (vertex[3] == 0.0) {
      verts.push_back(LPoint3(vertex[0], vertex[1], vertex[2]));
    } else {
      LPoint3 v3(vertex[0] / vertex[3], vertex[1] / vertex[3], vertex[2] / vertex[3]);
      verts.push_back(v3);
    }
  }
}

/**
 * Creates a default knot vector.
 */
void NurbsCurveEvaluator::
recompute_knots() {
  _knots.clear();
  int num_knots = get_num_knots();
  _knots.reserve(num_knots);

  PN_stdfloat value = 0.0f;

  int i = 0;
  while (i < _order) {
    _knots.push_back(value);
    i++;
  }
  while (i < num_knots - _order) {
    value += 1.0f;
    _knots.push_back(value);
    i++;
  }
  value += 1.0f;
  while (i < num_knots) {
    _knots.push_back(value);
    i++;
  }

  _knots_dirty = false;
}

/**
 * Recomputes the basis matrices according to the knot vector.
 */
void NurbsCurveEvaluator::
recompute_basis() {
  if (_knots_dirty) {
    ((NurbsCurveEvaluator *)this)->recompute_knots();
  }

  _basis.clear(_order);
  if ((int)_vertices.size() > _order - 1) {
    int min_knot = _order;
    int max_knot = (int)_vertices.size();

    for (int i = min_knot; i <= max_knot; i++) {
      nassertv(i - 1 >= 0 && i < (int)_knots.size());
      if (_knots[i - 1] < _knots[i]) {
        // Here's a non-empty segment.
        _basis.append_segment(i - _order, &_knots[i - _order]);
      }
    }
  }

  _basis_dirty = false;
}
