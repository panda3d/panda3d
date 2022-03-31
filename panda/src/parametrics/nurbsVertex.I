/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file nurbsVertex.I
 * @author drose
 * @date 2002-12-04
 */

/**
 *
 */
INLINE NurbsVertex::
NurbsVertex() {
  _vertex.set(0, 0, 0, 1);
}

/**
 *
 */
INLINE NurbsVertex::
NurbsVertex(const NurbsVertex &copy) :
  _vertex(copy._vertex),
  _space(copy._space),
  _space_path(copy._space_path)
{
}

/**
 *
 */
INLINE void NurbsVertex::
operator = (const NurbsVertex &copy) {
  _vertex = copy._vertex;
  _space = copy._space;
  _space_path = copy._space_path;
}

/**
 *
 */
INLINE NurbsVertex::
~NurbsVertex() {
}

/**
 *
 */
INLINE void NurbsVertex::
set_vertex(const LVecBase4 &vertex) {
  _vertex = vertex;
}

/**
 *
 */
INLINE const LVecBase4 &NurbsVertex::
get_vertex() const {
  return _vertex;
}

/**
 * Sets the space of this vertex as a fixed NodePath.
 */
INLINE void NurbsVertex::
set_space(const NodePath &space) {
  _space = space;
  _space_path = std::string();
}

/**
 * Sets the space of this vertex as a relative path from the rel_to node.
 */
INLINE void NurbsVertex::
set_space(const std::string &space) {
  _space = NodePath();
  _space_path = space;
}

/**
 *
 */
INLINE NodePath NurbsVertex::
get_space(const NodePath &rel_to) const {
  if (_space_path.empty()) {
    return _space;
  } else {
    return rel_to.find(_space_path);
  }
}
