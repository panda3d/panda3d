// Filename: CollisionHandlerGravity.cxx
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

#include "collisionHandlerGravity.h"
#include "collisionNode.h"
#include "collisionEntry.h"
#include "config_collide.h"

#include "clockObject.h"

TypeHandle CollisionHandlerGravity::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerGravity::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CollisionHandlerGravity::
CollisionHandlerGravity() {
  _offset = 0.0f;
  _airborne_height = 0.0f;
  _impact_velocity = 0.0f;
  _gravity = 32.174f;
  _current_velocity = 0.0f;
  _max_velocity = 400.0f;
  _outer_space = false;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerGravity::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CollisionHandlerGravity::
~CollisionHandlerGravity() {
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerGravity::handle_entries
//       Access: Protected, Virtual
//  Description: Called by the parent class after all collisions have
//               been detected, this manages the various collisions
//               and moves around the nodes as necessary.
//
//               The return value is normally true, but it may be
//               false to indicate the CollisionTraverser should
//               disable this handler from being called in the future.
////////////////////////////////////////////////////////////////////
bool CollisionHandlerGravity::
handle_entries() {
  bool okflag = true;
  _outer_space = true;

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
        
        if (!entries.empty()) {
          _outer_space = false;
        }
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
            }
          }
        }
        
        // Now set our height accordingly.
        float adjust = max_height + _offset;
        if (_current_velocity > 0.0f || !IS_THRESHOLD_ZERO(adjust, 0.001)) {
          if (collide_cat.is_debug()) {
            collide_cat.debug()
              << "Adjusting height by " << adjust << "\n";
          }
          
          if (_current_velocity > 0.0f || adjust < -0.001f) {
            // ...we have a vertical thrust,
            // ...or the node is above the floor, so it is airborne.
            float dt = ClockObject::get_global_clock()->get_dt();
            // The sign in this equation is reversed from normal.  This is
            // because _current_velocity is a scaler and the equation normally
            // has a vector.  I suppose the sign of _gravity could have been
            // reversed, but I think it makes the get_*() set_*()
            // more intuitive to do it this way.
            float gravity_adjust = _current_velocity * dt - 0.5 * _gravity * dt * dt;
            if (adjust > 0.0f) {
              // ...the node is under the floor, so it has landed.
              // Keep the adjust to bring us up to the ground and
              // then add the gravity_adjust to get us airborne:
              adjust += max(0.0f, gravity_adjust);
            } else {
              // ...the node is above the floor, so it is airborne.
              adjust = max(adjust, gravity_adjust);
            }
            _current_velocity -= _gravity * dt;
            // Record the airborne height in case someone else needs it: 
            _airborne_height = -max_height + adjust;
          }
          
          if (_airborne_height < 0.001f && _current_velocity < 0.001f) {
            // ...the node is under the floor, so it has landed.
            _impact_velocity = _current_velocity;
            // These values are used by is_on_ground().
            _current_velocity = _airborne_height = 0.0f;
          }

          CPT(TransformState) trans = def._target.get_transform();
          LVecBase3f pos = trans->get_pos();
          pos[2] += adjust;
          def._target.set_transform(trans->set_pos(pos));
          def.updated_transform();

          apply_linear_force(def, LVector3f(0.0f, 0.0f, adjust));
        } else {
          _current_velocity = _airborne_height = 0.0f;
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
//     Function: CollisionHandlerGravity::apply_linear_force
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void CollisionHandlerGravity::
apply_linear_force(ColliderDef &def, const LVector3f &force) {
}
