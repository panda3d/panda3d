/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file navMeshPoly.I
 * @author Maxwell175
 * @date 2022-02-27
 */


/**
 * Gets the dtPolyRef that this object references.
 */
INLINE dtPolyRef NavMeshPoly::
get_poly_ref() const {
  return _polyRef;
}

/**
 * Gets the NavMesh that this NavMeshPoly is based on.
 */
INLINE PT(NavMesh) NavMeshPoly::
get_nav_mesh() const {
  return _navMesh;
}

INLINE bool NavMeshPoly::
operator == (const NavMeshPoly &other) const {
  return _navMesh == other.get_nav_mesh() && _polyRef == other.get_poly_ref();
}

INLINE bool NavMeshPoly::
operator != (const NavMeshPoly &other) const {
  return _navMesh != other.get_nav_mesh() || _polyRef != other.get_poly_ref();
}
