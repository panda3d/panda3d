/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionLevelStateBase.cxx
 * @author drose
 * @date 2002-03-16
 */

#include "collisionLevelStateBase.h"
#include "collisionSolid.h"
#include "collisionNode.h"
#include "config_collide.h"
#include "dcast.h"

PStatCollector CollisionLevelStateBase::_node_volume_pcollector("Collision Volumes:PandaNode");

TypeHandle CollisionLevelStateBase::_type_handle;

/**
 *
 */
void CollisionLevelStateBase::
clear() {
  _colliders.clear();
  _local_bounds.clear();
  _parent_bounds.clear();
}

/**
 * Indicates an intention to add the indicated number of colliders to the
 * level state.
 */
void CollisionLevelStateBase::
reserve(int num_colliders) {
  _colliders.reserve(num_colliders);
  _local_bounds.reserve(num_colliders);
}

/**
 * Adds the indicated Collider to the set of Colliders in the current level
 * state.
 */
void CollisionLevelStateBase::
prepare_collider(const ColliderDef &def, const NodePath &root) {
  _colliders.push_back(def);

  const CollisionSolid *collider = def._collider;
  CPT(BoundingVolume) bv = collider->get_bounds();
  if (!bv->is_of_type(GeometricBoundingVolume::get_class_type())) {
    _local_bounds.push_back(nullptr);
  } else {
    // We can use a plain pointer, rather than a PT() here, because we know we
    // are going to save the volume in the vector, below.
    GeometricBoundingVolume *gbv;
    DCAST_INTO_V(gbv, bv->make_copy());

    // TODO: we need to make this logic work in the new relative world.  The
    // bounding volume should be extended by the object's motion relative to
    // each object it is considering a collision with.  That makes things
    // complicated!
    if (bv->as_bounding_sphere()) {
      LPoint3 pos_delta = def._node_path.get_pos_delta(root);

      // LVector3 cap(pos_delta); if(cap.length()>fluid_cap_amount) {
      // pos_delta=LPoint3(capcap.length())*fluid_cap_amount; }
      if (pos_delta != LVector3::zero()) {
        // If the node has a delta, we have to include the starting position
        // in the volume as well.  We only do this for bounding spheres, since
        // (a) other kinds of volumes may not extend so well, and (b) we've
        // only implemented fluid-motion detection for CollisionSpheres
        // anyway.
        LMatrix4 inv_trans = LMatrix4::translate_mat(-pos_delta);
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
