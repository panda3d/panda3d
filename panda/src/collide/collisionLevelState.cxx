// Filename: collisionLevelState.cxx
// Created by:  drose (24Apr00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "collide_headers.h"
#pragma hdrstop

////////////////////////////////////////////////////////////////////
//     Function: CollisionLevelState::clear
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void CollisionLevelState::
clear() {
  _colliders.clear();
  _local_bounds.clear();
  _current = 0;
  _colliders_with_geom = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionLevelState::reserve
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void CollisionLevelState::
reserve(int max_colliders) {
  _colliders.reserve(max_colliders);
  _local_bounds.reserve(max_colliders);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionLevelState::prepare_collider
//       Access: Public
//  Description: Adds the indicated Collider to the set of Colliders
//               in the current level state.
////////////////////////////////////////////////////////////////////
void CollisionLevelState::
prepare_collider(const ColliderDef &def) {
  int index = (int)_colliders.size();
  _colliders.push_back(def);

  CollisionSolid *collider = def._collider;
  const BoundingVolume &bv = collider->get_bound();
  if (!bv.is_of_type(GeometricBoundingVolume::get_class_type())) {
    _local_bounds.push_back((GeometricBoundingVolume *)NULL);
  } else {
    GeometricBoundingVolume *gbv;
    DCAST_INTO_V(gbv, bv.make_copy());
    gbv->xform(def._space);
    _local_bounds.push_back(gbv);
  }

  _current |= get_mask(index);

  if (def._node->get_collide_geom()) {
    _colliders_with_geom |= get_mask(index);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionLevelState::xform
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void CollisionLevelState::
xform(const LMatrix4f &mat) {
  BoundingVolumes new_bounds;

  int num_colliders = get_num_colliders();
  new_bounds.reserve(num_colliders);
  for (int c = 0; c < num_colliders; c++) {
    if (!has_collider(c) ||
        get_local_bound(c) == (GeometricBoundingVolume *)NULL) {
      new_bounds.push_back((GeometricBoundingVolume *)NULL);
    } else {
      const GeometricBoundingVolume *old_bound = get_local_bound(c);
      GeometricBoundingVolume *new_bound;
      DCAST_INTO_V(new_bound, old_bound->make_copy());
      new_bound->xform(mat);
      new_bounds.push_back(new_bound);
    }
  }

  _local_bounds = new_bounds;
}
