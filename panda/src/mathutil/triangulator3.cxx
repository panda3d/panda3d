/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file triangulator3.cxx
 * @author drose
 * @date 2013-01-03
 */

#include "triangulator3.h"
#include "look_at.h"

/**

 */
Triangulator3::
Triangulator3() {
}

/**
 * Removes all vertices and polygon specifications from the Triangulator, and
 * prepares it to start over.
 */
void Triangulator3::
clear() {
  _vertices3.clear();
  _plane = LPlaned();
  Triangulator::clear();
}

/**
 * Adds a new vertex to the vertex pool.  Returns the vertex index number.
 */
int Triangulator3::
add_vertex(const LPoint3d &point) {
  int index = (int)_vertices3.size();
  _vertices3.push_back(point);
  return index;
}

/**
 * Does the work of triangulating the specified polygon.  After this call, you
 * may retrieve the new triangles one at a time by iterating through
 * get_triangle_v0/1/2().
 */
void Triangulator3::
triangulate() {
  _result.clear();

  if (_polygon.size() < 3) {
    // Degenerate case.
    return;
  }

  // First, determine the polygon normal.
  LNormald normal = LNormald::zero();

  // Project the polygon into each of the three major planes and
  // calculate the area of each 2-d projection.  This becomes the
  // polygon normal.  This works because the ratio between these
  // different areas corresponds to the angle at which the polygon is
  // tilted toward each plane.
  size_t num_verts = _polygon.size();
  for (size_t i = 0; i < num_verts; i++) {
    int i0 = _polygon[i];
    int i1 = _polygon[(i + 1) % num_verts];;;;
    nassertv(i0 >= 0 && i0 < (int)_vertices3.size() &&
             i1 >= 0 && i1 < (int)_vertices3.size());
    const LPoint3d &p0 = _vertices3[i0];
    const LPoint3d &p1 = _vertices3[i1];
    normal[0] += p0[1] * p1[2] - p0[2] * p1[1];
    normal[1] += p0[2] * p1[0] - p0[0] * p1[2];
    normal[2] += p0[0] * p1[1] - p0[1] * p1[0];
  }

  if (!normal.normalize()) {
    // The polygon is degenerate: it has zero area in each plane.  In
    // this case, the triangulation result produces no triangles
    // anyway.
    return;
  }

  _plane = LPlaned(normal, _vertices3[0]);

  // Now determine the matrix to project each of the vertices into
  // this 2-d plane.
  LMatrix4d mat;
  heads_up(mat, _vertices3[1] - _vertices3[2], normal, CS_zup_right);
  mat.set_row(3, _vertices3[0]);
  mat.invert_in_place();

  _vertices.clear();
  for (size_t i = 0; i < _vertices3.size(); i++) {
    LPoint3d p = _vertices3[i] * mat;
    _vertices.push_back(LPoint2d(p[0], p[1]));
  }
  Triangulator::triangulate();
}
