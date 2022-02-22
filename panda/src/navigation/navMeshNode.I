/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file navMeshNode.I
 * @author Maxwell175
 * @date 2022-02-21
 */


/**
 * Returns the underlying NavMesh object.
 */
INLINE PT(NavMesh) NavMeshNode::
get_nav_mesh() {
  return _nav_mesh;
}

/**
 * Returns a GeomNode that visually represents the NavMesh.
 */
INLINE PT(GeomNode) NavMeshNode::
draw_nav_mesh_geom() {
  return _nav_mesh->draw_nav_mesh_geom();
}
