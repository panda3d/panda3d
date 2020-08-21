/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggMesherEdge.I
 * @author drose
 * @date 2005-03-13
 */

/**
 * Defines an edge as a pair of vertices.  The _opposite pointer should be
 * filled in explicitly by the caller.
 */
INLINE EggMesherEdge::
EggMesherEdge(int vi_a, int vi_b) : _vi_a(vi_a), _vi_b(vi_b) {
  _opposite = nullptr;
}

/**
 *
 */
INLINE EggMesherEdge::
EggMesherEdge(const EggMesherEdge &copy) :
  _vi_a(copy._vi_a),
  _vi_b(copy._vi_b),
  _strips(copy._strips),
  _opposite(copy._opposite)
{
}

/**
 * Returns true if the edge contains the indicated vertex index, false
 * otherwise.
 */
INLINE bool EggMesherEdge::
contains_vertex(int vi) const {
  return (_vi_a == vi || _vi_b == vi);
}

/**
 * Returns true if this edge represents the same line segment as the other
 * edge, in either direction.
 */
INLINE bool EggMesherEdge::
matches(const EggMesherEdge &other) const {
  return ((_vi_a == other._vi_a && _vi_b == other._vi_b) ||
          (_vi_b == other._vi_a && _vi_a == other._vi_b));
}

/**
 * Returns an arbitrary pointer that is used to represent both this edge and
 * its opposite.  this->common_ptr() is guaranteed to be the same as
 * this->_opposite->common_ptr().
 */
INLINE EggMesherEdge *EggMesherEdge::
common_ptr() {
  return std::min(this, _opposite);
}

/**
 *
 */
INLINE bool EggMesherEdge::
operator == (const EggMesherEdge &other) const {
  return _vi_a == other._vi_a && _vi_b == other._vi_b;
}

/**
 *
 */
INLINE bool EggMesherEdge::
operator != (const EggMesherEdge &other) const {
  return !operator == (other);
}

/**
 * Defines an arbitrary ordering for edges, used for putting edges in a sorted
 * container.
 */
INLINE bool EggMesherEdge::
operator < (const EggMesherEdge &other) const {
  if (_vi_a != other._vi_a) {
    return _vi_a < other._vi_a;
  }
  return _vi_b < other._vi_b;
}

/**
 * Returns the length of the edge in model units.
 */
INLINE double EggMesherEdge::
compute_length(const EggVertexPool *vertex_pool) const {
  LPoint3d a = vertex_pool->get_vertex(_vi_a)->get_pos3();
  LPoint3d b = vertex_pool->get_vertex(_vi_b)->get_pos3();
  return (a - b).length();
}

/**
 * Returns a 3-component vector that represents the lengths of the sides of
 * the smalled axis-aligned box that contains the edge.  That is, the
 * projection the edge onto each axis.
 */
INLINE LVecBase3d EggMesherEdge::
compute_box(const EggVertexPool *vertex_pool) const {
  LPoint3d a = vertex_pool->get_vertex(_vi_a)->get_pos3();
  LPoint3d b = vertex_pool->get_vertex(_vi_b)->get_pos3();
  LVector3d v = (a - b);
  return LVecBase3d(fabs(v[0]), fabs(v[1]), fabs(v[2]));
}
