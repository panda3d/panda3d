// Filename: collisionLevelState.cxx
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

#include "collisionLevelState.h"
#include "collisionSolid.h"
#include "dcast.h"

////////////////////////////////////////////////////////////////////
//     Function: CollisionLevelState::clear
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void CollisionLevelState::
clear() {
  _colliders.clear();
  _local_bounds.clear();
  _parent_bounds.clear();
  _current = 0;
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

    gbv->xform(def._node_path.get_net_transform()->get_mat());
    _local_bounds.push_back(gbv);
  }

  _current |= get_mask(index);

  _parent_bounds = _local_bounds;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionLevelState::any_in_bounds
//       Access: Public
//  Description: Checks the bounding volume of the current node
//               against each of our colliders.  Eliminates from the
//               current collider list any that are outside of the
//               bounding volume.  Returns true if any colliders
//               remain, false if all of them fall outside this node's
//               bounding volume.
////////////////////////////////////////////////////////////////////
bool CollisionLevelState::
any_in_bounds() {
#ifndef NDEBUG
  int indent_level = 0;
  if (collide_cat.is_spam()) {
    indent_level = _node_path.get_num_nodes() * 2;
    collide_cat.spam();
    indent(collide_cat.spam(false), indent_level)
      << "Considering " << _node_path.get_node_path() << "\n";
  }
#endif  // NDEBUG

  const BoundingVolume &node_bv = node()->get_bound();
  if (node_bv.is_of_type(GeometricBoundingVolume::get_class_type())) {
    const GeometricBoundingVolume *node_gbv;
    DCAST_INTO_R(node_gbv, &node_bv, false);

    int num_colliders = get_num_colliders();
    for (int c = 0; c < num_colliders; c++) {
      if (has_collider(c)) {
        CollisionNode *cnode = get_collider_node(c);
        bool is_in = false;

        // Don't even bother testing the bounding volume if there are
        // no collide bits in common between our collider and this
        // node.
        CollideMask from_mask = cnode->get_from_collide_mask();
        if (cnode->get_collide_geom() ||
            (from_mask & node()->get_net_collide_mask()) != 0) {
          // There are bits in common, so go ahead and try the
          // bounding volume.
          const GeometricBoundingVolume *col_gbv =
            get_local_bound(c);
          if (col_gbv != (GeometricBoundingVolume *)NULL) {
            is_in = (node_gbv->contains(col_gbv) != 0);

#ifndef NDEBUG
            if (collide_cat.is_spam()) {
              collide_cat.spam();
              indent(collide_cat.spam(false), indent_level)
                << "Comparing " << c << ": " << *col_gbv
                << " to " << *node_gbv << ", is_in = " << is_in << "\n";
            }
#endif  // NDEBUG
          }
        }

        if (!is_in) {
          // This collider cannot intersect with any geometry at
          // this node or below.
          omit_collider(c);
        }
      }
    }
  }

#ifndef NDEBUG
  if (collide_cat.is_spam()) {
    int num_active_colliders = 0;
    int num_colliders = get_num_colliders();
    for (int c = 0; c < num_colliders; c++) {
      if (has_collider(c)) {
        num_active_colliders++;
      }
    }

    collide_cat.spam();
    indent(collide_cat.spam(false), indent_level)
      << _node_path.get_node_path() << " has " << num_active_colliders
      << " interested colliders.\n";
  }
#endif  // NDEBUG
  return has_any_collider();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionLevelState::apply_transform
//       Access: Public
//  Description: Applies the inverse transform from the current node,
//               if any, onto all the colliders in the level state.
////////////////////////////////////////////////////////////////////
void CollisionLevelState::
apply_transform() {
  // The "parent" bounds list remembers the bounds list of the
  // previous node.
  _parent_bounds = _local_bounds;

  // Recompute the bounds list of this node (if we have a transform).
  const TransformState *node_transform = node()->get_transform();
  if (!node_transform->is_identity()) {
    CPT(TransformState) inv_transform = 
      node_transform->invert_compose(TransformState::make_identity());
    const LMatrix4f &mat = inv_transform->get_mat();

    // Now build the new bounding volumes list.
    BoundingVolumes new_bounds;

    int num_colliders = get_num_colliders();
    new_bounds.reserve(num_colliders);
    for (int c = 0; c < num_colliders; c++) {
      if (!has_collider(c) ||
          get_local_bound(c) == (GeometricBoundingVolume *)NULL) {
        new_bounds.push_back((GeometricBoundingVolume *)NULL);
      } else {
        const GeometricBoundingVolume *old_bound = get_local_bound(c);
        GeometricBoundingVolume *new_bound = 
          DCAST(GeometricBoundingVolume, old_bound->make_copy());
        new_bound->xform(mat);
        new_bounds.push_back(new_bound);
      }
    }
    
    _local_bounds = new_bounds;
  }    
}
