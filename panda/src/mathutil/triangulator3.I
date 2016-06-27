/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file triangulator3.I
 * @author drose
 * @date 2013-01-03
 */

/**
 * Adds a new vertex to the vertex pool.  Returns the vertex index number.
 */
INLINE int Triangulator3::
add_vertex(double x, double y, double z) {
  return add_vertex(LPoint3d(x, y, z));
}

/**
 * Returns the number of vertices in the pool.  Note that the Triangulator
 * might append new vertices, in addition to those added by the user, if any
 * of the polygon is self-intersecting, or if any of the holes intersect some
 * part of the polygon edges.
 */
INLINE int Triangulator3::
get_num_vertices() const {
  return _vertices3.size();
}

/**
 * Returns the nth vertex.
 */
INLINE const LPoint3d &Triangulator3::
get_vertex(int n) const {
  nassertr(n >= 0 && n < (int)_vertices3.size(), LPoint3d::zero());
  return _vertices3[n];
}

/**
 * Returns the plane of the polygon.  This is only available after calling
 * triangulate().
 */
INLINE const LPlaned &Triangulator3::
get_plane() const {
  return _plane;
}
