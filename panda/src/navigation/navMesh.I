/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file navMesh.I
 * @author Maxwell175
 * @date 2022-02-21
 */


/**
 * Returns the vertex count in the general navigation mesh.
 */
INLINE int NavMesh::
get_vert_count() const {
  return _params.vertCount;
}

/**
 * Returns the polygon count in the general navigation mesh.
 */
INLINE int NavMesh::
get_poly_count() const {
  return _params.polyCount;
}

/**
 * Returns the vertex count in the detailed navigation mesh.
 */
INLINE int NavMesh::
get_detail_vert_count() const {
  return _params.detailVertsCount;
}

/**
 * Returns the tri count in the detailed navigation mesh.
 */
INLINE int NavMesh::
get_detail_tri_count() const {
  return _params.detailTriCount;
}

/**
 * Returns the color that the specified detour Poly will show() as.
 *
 * Defaults to blue.
 */
INLINE LColor NavMesh::
get_poly_debug_color(dtPolyRef poly) const {
  auto it = _debug_colors.find(poly);
  if (it == _debug_colors.end()) {
    return {0, 0, 1, 1};
  } else {
    return (*it).second;
  }
}

/**
 * Sets the color that the specified detour Poly will show() as.
 */
INLINE void NavMesh::
set_poly_debug_color(dtPolyRef poly, LColor color) {
  _debug_colors[poly] = color;
}

/**
 * Clears any debug colors that have been set on this NavMesh.
 */
INLINE void NavMesh::
reset_debug_colors() {
  _debug_colors.clear();
}
