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
