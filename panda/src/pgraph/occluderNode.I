/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file occluderNode.I
 * @author jenes
 * @date 2011-03-11
 */

/**
 * Replaces the four vertices of the occluder polygon.  The vertices should be
 * defined in a counterclockwise orientation when looking at the face of the
 * occluder.
 */
INLINE void OccluderNode::
set_vertices(const LPoint3 &v0, const LPoint3 &v1,
             const LPoint3 &v2, const LPoint3 &v3) {
  _vertices.clear();
  _vertices.reserve(4);
  _vertices.push_back(v0);
  _vertices.push_back(v1);
  _vertices.push_back(v2);
  _vertices.push_back(v3);
}

/**
 * Returns the number of vertices in the occluder polygon.  This should always
 * return 4.
 */
INLINE size_t OccluderNode::
get_num_vertices() const {
  return _vertices.size();
}

/**
 * Returns the nth vertex of the occluder polygon.
 */
INLINE const LPoint3 &OccluderNode::
get_vertex(size_t n) const {
  nassertr(n < _vertices.size(), LPoint3::zero());
  return _vertices[n];
}

/**
 * Sets the nth vertex of the occluder polygon.
 */
INLINE void OccluderNode::
set_vertex(size_t n, const LPoint3 &v) {
  nassertv(n < _vertices.size());
  _vertices[n] = v;
}

/**
 * If true, the back-face will also be used to occlude
 */
INLINE void OccluderNode::set_double_sided(bool value) {
  _double_sided = value;
}

/**
 * Is this occluder double-sided
 */
INLINE bool OccluderNode::is_double_sided() {
  return _double_sided;
}

/**
 * Minimum screen coverage needed before occluder used.  Range should be 0 to
 * 1. For example, setting to 0.2 would mean that the occluder needs to cover
 * 20% of the screen to be considered.
 */
INLINE void OccluderNode::set_min_coverage(PN_stdfloat value) {
  _min_coverage = value;
}

/**
 * Returns the minimum screen coverage.
 */
INLINE PN_stdfloat OccluderNode::get_min_coverage() {
  return _min_coverage;
}
