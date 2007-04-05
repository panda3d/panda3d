// Filename: collisionLevelStateBase.cxx
// Created by:  drose (16Mar02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "collisionLevelStateBase.h"
#include "collisionSolid.h"
#include "collisionNode.h"
#include "dcast.h"

PStatCollector CollisionLevelStateBase::_node_volume_pcollector("Collision Volumes:PandaNode");

////////////////////////////////////////////////////////////////////
//     Function: CollisionLevelStateBase::clear
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void CollisionLevelStateBase::
clear() {
  _colliders.clear();
  _local_bounds.clear();
  _parent_bounds.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionLevelStateBase::reserve
//       Access: Public
//  Description: Indicates an intention to add the indicated number of
//               colliders to the level state.
////////////////////////////////////////////////////////////////////
void CollisionLevelStateBase::
reserve(int num_colliders) {
  _colliders.reserve(num_colliders);
  _local_bounds.reserve(num_colliders);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionLevelStateBase::prepare_collider
//       Access: Public
//  Description: Adds the indicated Collider to the set of Colliders
//               in the current level state.
////////////////////////////////////////////////////////////////////
void CollisionLevelStateBase::
prepare_collider(const ColliderDef &def, const NodePath &root) {
  _colliders.push_back(def);

  CollisionSolid *collider = def._collider;
  CPT(BoundingVolume) bv = collider->get_bounds();
  if (!bv->is_of_type(GeometricBoundingVolume::get_class_type())) {
    _local_bounds.push_back((GeometricBoundingVolume *)NULL);
  } else {
    GeometricBoundingVolume *gbv;
    DCAST_INTO_V(gbv, bv->make_copy());

    // TODO: we need to make this logic work in the new relative
    // world.  The bounding volume should be extended by the object's
    // motion relative to each object it is considering a collision
    // with.  That makes things complicated!
    /*
    if (def._delta != LVector3f::zero()) {
      // If the node has a delta, we have to include the starting
      // point in the volume as well.

      // Strictly speaking, we should actually transform gbv backward
      // by the delta(), and extend by *that* volume, instead of
      // just extending by the single point at -get_velocity().
      // However, assuming the solids within a moving CollisionNode
      // tend to be near the origin, this will generally produce the
      // same results, and is much easier to compute.
      gbv->extend_by(LPoint3f(-def._delta));
    }
    */

    CPT(TransformState) rel_transform = def._node_path.get_transform(root.get_parent());
    gbv->xform(rel_transform->get_mat());
    _local_bounds.push_back(gbv);
  }
  
  _parent_bounds = _local_bounds;
}
