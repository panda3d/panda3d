// Filename: nurbsCurveEvaluator.cxx
// Created by:  drose (03Dec02)
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

#include "nurbsCurveEvaluator.h"

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveEvaluator::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
NurbsCurveEvaluator::
NurbsCurveEvaluator() {
  _order = 4;
  _knots_dirty = true;
  _basis_dirty = true;
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveEvaluator::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
NurbsCurveEvaluator::
~NurbsCurveEvaluator() {
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveEvaluator::reset
//       Access: Published
//  Description: Resets all the vertices and knots to their default
//               values, and sets the curve up with the indicated
//               number of vertices.  You must then call set_vertex()
//               repeatedly to fill in all of the vertex values
//               appropriately.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveEvaluator::get_vertex_space
//       Access: Published
//  Description: Returns the coordinate space of the nth control
//               vertex of the curve, expressed as a NodePath.
////////////////////////////////////////////////////////////////////
NodePath NurbsCurveEvaluator::
get_vertex_space(int i, const NodePath &rel_to) const {
#ifndef NDEBUG
  static NodePath empty_node_path;
  nassertr(i >= 0 && i < (int)_vertices.size(), empty_node_path);
#endif
  return _vertices[i].get_space(rel_to);
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveEvaluator::set_knot
//       Access: Published
//  Description: Sets the value of the nth knot.  Each knot value
//               should be greater than or equal to the preceding
//               value.  If no knot values are set, a default knot
//               vector is supplied.
////////////////////////////////////////////////////////////////////
void NurbsCurveEvaluator::
set_knot(int i, float knot) {
  if (_knots_dirty) {
    recompute_knots();
  }
  nassertv(i >= 0 && i < (int)_knots.size());
  _knots[i] = knot;
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveEvaluator::get_knot
//       Access: Published
//  Description: Returns the value of the nth knot.
////////////////////////////////////////////////////////////////////
float NurbsCurveEvaluator::
get_knot(int i) const {
  if (_knots_dirty) {
    ((NurbsCurveEvaluator *)this)->recompute_knots();
  }
  nassertr(i >= 0 && i < (int)_knots.size(), 0.0f);
  return _knots[i];
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveEvaluator::evaluate
//       Access: Published
//  Description: Returns a NurbsCurveResult object that represents the
//               result of applying the knots to all of the current
//               values of the vertices, transformed into the
//               indicated coordinate space.
////////////////////////////////////////////////////////////////////
PT(NurbsCurveResult) NurbsCurveEvaluator::
evaluate(const NodePath &rel_to) const {
  if (_basis_dirty) {
    ((NurbsCurveEvaluator *)this)->recompute_basis();
  }

  // First, transform the vertices as appropriate.
  pvector<LVecBase4f> vecs;
  get_vertices(vecs, rel_to);

  // And apply those transformed vertices to the basis matrices to
  // derive the result.
  return new NurbsCurveResult(_basis, &vecs[0], &_vertices[0],
                              (int)_vertices.size());
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveEvaluator::get_vertices
//       Access: Public
//  Description: Fills the indicated vector with the set of vertices
//               in the curve, transformed to the given space.  This
//               flavor returns the vertices in 4-dimensional
//               homogenous space.
////////////////////////////////////////////////////////////////////
void NurbsCurveEvaluator::
get_vertices(pvector<LVecBase4f> &verts, const NodePath &rel_to) const {
  int num_vertices = (int)_vertices.size();
  verts.reserve(verts.size() + num_vertices);
  int vi;
  for (vi = 0; vi < num_vertices; vi++) {
    NodePath space = _vertices[vi].get_space(rel_to);
    const LVecBase4f &vertex = _vertices[vi].get_vertex();
    if (space.is_empty()) {
      verts.push_back(vertex);
    } else {
      const LMatrix4f &mat = space.get_mat(rel_to);
      verts.push_back(vertex * mat);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveEvaluator::get_vertices
//       Access: Public
//  Description: Fills the indicated vector with the set of vertices
//               in the curve, transformed to the given space.  This
//               flavor returns the vertices in 4-dimensional
//               homogenous space.
////////////////////////////////////////////////////////////////////
void NurbsCurveEvaluator::
get_vertices(pvector<LPoint3f> &verts, const NodePath &rel_to) const {
  int num_vertices = (int)_vertices.size();
  verts.reserve(verts.size() + num_vertices);
  int vi;
  for (vi = 0; vi < num_vertices; vi++) {
    const NodePath &space = _vertices[vi].get_space(rel_to);
    LVecBase4f vertex = _vertices[vi].get_vertex();
    if (!space.is_empty()) {
      const LMatrix4f &mat = space.get_mat(rel_to);
      vertex = vertex * mat;
    }
    LPoint3f v3(vertex[0] / vertex[3], vertex[1] / vertex[3], vertex[2] / vertex[3]);
    verts.push_back(v3);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveEvaluator::recompute_knots
//       Access: Private
//  Description: Creates a default knot vector.
////////////////////////////////////////////////////////////////////
void NurbsCurveEvaluator::
recompute_knots() {
  _knots.clear();
  int num_knots = get_num_knots();
  _knots.reserve(num_knots);

  float value = 0.0f;

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

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveEvaluator::recompute_basis
//       Access: Private
//  Description: Recomputes the basis matrices according to the knot
//               vector.
////////////////////////////////////////////////////////////////////
void NurbsCurveEvaluator::
recompute_basis() {
  if (_knots_dirty) {
    ((NurbsCurveEvaluator *)this)->recompute_knots();
  }

  _basis.clear(_order);
  if ((int)_vertices.size() > _order - 1) {
    int min_knot = _order;
    int max_knot = (int)_vertices.size() + 1;
    
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
