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

TypeHandle CollisionLevelStateBase::_type_handle;

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

  const CollisionSolid *collider = def._collider;
  CPT(BoundingVolume) bv = collider->get_bounds();
  if (!bv->is_of_type(GeometricBoundingVolume::get_class_type())) {
    _local_bounds.push_back((GeometricBoundingVolume *)NULL);
  } else {
    // We can use a plain pointer, rather than a PT() here, because we
    // know we are going to save the volume in the vector, below.
    GeometricBoundingVolume *gbv;
    DCAST_INTO_V(gbv, bv->make_copy());

    // TODO: we need to make this logic work in the new relative
    // world.  The bounding volume should be extended by the object's
    // motion relative to each object it is considering a collision
    // with.  That makes things complicated!
    if (bv->as_bounding_sphere()) {
      LPoint3f pos_delta = def._node_path.get_pos_delta(root);
      if (pos_delta != LVector3f::zero()) {
        // If the node has a delta, we have to include the starting
        // position in the volume as well.  We only do this for bounding
        // spheres, since (a) other kinds of volumes may not extend so
        // well, and (b) we've only implemented fluid-motion detection
        // for CollisionSpheres anyway.
        LMatrix4f inv_trans = LMatrix4f::translate_mat(-pos_delta);
        PT(GeometricBoundingVolume) gbv_prev;
        gbv_prev = DCAST(GeometricBoundingVolume, bv->make_copy());
        gbv_prev->xform(inv_trans);
        gbv->extend_by(gbv_prev);
      }
    }

    CPT(TransformState) rel_transform = def._node_path.get_transform(root.get_parent());
    gbv->xform(rel_transform->get_mat());
    _local_bounds.push_back(gbv);
  }
  
  _parent_bounds = _local_bounds;
}
