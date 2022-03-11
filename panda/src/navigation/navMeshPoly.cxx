/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file navMeshPoly.cxx
 * @author Maxwell175
 * @date 2022-02-27
 */

#include "navMeshPoly.h"
#include "navMesh.h"


/**
 * Makes a NavMeshPoly that wraps the given NavMesh and dtPolyRef
 */
NavMeshPoly::NavMeshPoly(PT(NavMesh) navMesh, dtPolyRef polyRef) :
    _navMesh(navMesh), _polyRef(polyRef) { }

/**
 * Copies an existing NavMeshPoly object.
 */
NavMeshPoly::NavMeshPoly(const NavMeshPoly &copy) :
    _navMesh(copy.get_nav_mesh()), _polyRef(copy.get_poly_ref()) { }

/**
 * Sets the passed in flags on the current NavMesh polygon.
 */
void NavMeshPoly::set_flags(BitMask16 mask) {
  _navMesh->get_nav_mesh()->setPolyFlags(_polyRef, mask.get_word());
}

/**
 * Sets the passed in flags on the current NavMesh polygon.
 */
BitMask16 NavMeshPoly::get_flags() {
  uint16_t flags = 0;
  if (_navMesh->get_nav_mesh()->getPolyFlags(_polyRef, &flags)) {
    return {flags};
  } else {
    return BitMask16::all_off();
  }
}

/**
 * Returns the center point of the NavMesh polygon.
 */
LPoint3 NavMeshPoly::get_center() {
  const dtPoly *poly;
  const dtMeshTile *tile;
  if (_navMesh->get_nav_mesh()->getTileAndPolyByRef(_polyRef, &tile, &poly)) {
    float x = 0;
    float y = 0;
    float z = 0;
    for (unsigned char i = 0; i < poly->vertCount; i++) {
      x += tile->verts[poly->verts[i] * 3];
      y += tile->verts[poly->verts[i] * 3 + 1];
      z += tile->verts[poly->verts[i] * 3 + 2];
    }
    x /= poly->vertCount;
    y /= poly->vertCount;
    z /= poly->vertCount;
    return mat_from_y.xform_point({ x, y, z }); // convert back from y-up system
  } else {
    return {0, 0, 0};
  }
}
