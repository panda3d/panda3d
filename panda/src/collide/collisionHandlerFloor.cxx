// Filename: collisionHandlerFloor.cxx
// Created by:  drose (16Mar02)
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

#include "collisionHandlerFloor.h"
#include "collisionNode.h"
#include "collisionEntry.h"
#include "config_collide.h"

#include "clockObject.h"

TypeHandle CollisionHandlerFloor::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerFloor::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CollisionHandlerFloor::
CollisionHandlerFloor() {
  _offset = 0.0f;
  _max_velocity = 0.0f;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerFloor::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CollisionHandlerFloor::
~CollisionHandlerFloor() {
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerFloor::handle_entries
//       Access: Protected, Virtual
//  Description: Called by the parent class after all collisions have
//               been detected, this manages the various collisions
//               and moves around the nodes as necessary.
//
//               The return value is normally true, but it may be
//               false to indicate the CollisionTraverser should
//               disable this handler from being called in the future.
////////////////////////////////////////////////////////////////////
bool CollisionHandlerFloor::
handle_entries() {
  bool okflag = true;

  // Reset the set of things our parent class CollisionHandlerEvent
  // recorded a collision with.  We'll only consider ourselves
  // collided with the topmost object for each from_entry.
  _current_colliding.clear();

  FromEntries::const_iterator fi;
  for (fi = _from_entries.begin(); fi != _from_entries.end(); ++fi) {
    const NodePath &from_node_path = (*fi).first;
    const Entries &entries = (*fi).second;

    Colliders::iterator ci;
    ci = _colliders.find(from_node_path);
    if (ci == _colliders.end()) {
      // Hmm, someone added a CollisionNode to a traverser and gave
      // it this CollisionHandler pointer--but they didn't tell us
      // about the node.
      collide_cat.error()
        << get_type() << " doesn't know about "
        << from_node_path << ", disabling.\n";
      okflag = false;
    } else {
      ColliderDef &def = (*ci).second;
      {
        // Get the maximum height for all collisions with this node.
        bool got_max = false;
        float max_height = 0.0f;
        CollisionEntry *max_entry = NULL;
        
        Entries::const_iterator ei;
        for (ei = entries.begin(); ei != entries.end(); ++ei) {
          CollisionEntry *entry = (*ei);
          nassertr(entry != (CollisionEntry *)NULL, false);
          nassertr(from_node_path == entry->get_from_node_path(), false);
          
          if (entry->has_surface_point()) {
            LPoint3f point = entry->get_surface_point(def._target);
            if (collide_cat.is_debug()) {
              collide_cat.debug()
                << "Intersection point detected at " << point << "\n";
            }
            
            float height = point[2];
            if (!got_max || height > max_height) {
              got_max = true;
              max_height = height;
              max_entry = entry;
            }
          }
        }

        // Record a collision with the topmost element for the
        // CollisionHandlerEvent base class.
        _current_colliding.insert(max_entry);

        // Now set our height accordingly.
        float adjust = max_height + _offset;
        if (!IS_THRESHOLD_ZERO(adjust, 0.001)) {
          if (collide_cat.is_debug()) {
            collide_cat.debug()
              << "Adjusting height by " << adjust << "\n";
          }
          
          if (adjust < 0.0f && _max_velocity != 0.0f) {
            float max_adjust =
              _max_velocity * ClockObject::get_global_clock()->get_dt();
            adjust = max(adjust, -max_adjust);
          }

          CPT(TransformState) trans = def._target.get_transform();
          LVecBase3f pos = trans->get_pos();
          pos[2] += adjust;
          def._target.set_transform(trans->set_pos(pos));
          def.updated_transform();

          apply_linear_force(def, LVector3f(0.0f, 0.0f, adjust));
        } else {
          if (collide_cat.is_spam()) {
            collide_cat.spam()
              << "Leaving height unchanged.\n";
          }
        }
      }
    }
  }

  return okflag;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerFloor::apply_linear_force
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void CollisionHandlerFloor::
apply_linear_force(ColliderDef &def, const LVector3f &force) {
}
