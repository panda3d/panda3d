/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionHandlerGravity.cxx
 * @author drose
 * @date 2002-03-16
 */

#include "collisionHandlerGravity.h"
#include "collisionNode.h"
#include "collisionEntry.h"
#include "config_collide.h"
#include "collisionPlane.h"
#include "clockObject.h"

using std::cout;
using std::endl;

TypeHandle CollisionHandlerGravity::_type_handle;

/**
 *
 */
CollisionHandlerGravity::
CollisionHandlerGravity() {
  _offset = 0.0f;
  _reach = 1.0f;
  _airborne_height = 0.0f;
  _impact_velocity = 0.0f;
  _gravity = 32.174;
  _current_velocity = 0.0f;
  _max_velocity = 400.0f;
  _contact_normal = LVector3::zero();
  _legacy_mode = false;
}

/**
 *
 */
CollisionHandlerGravity::
~CollisionHandlerGravity() {
}

/**
 *
 */
#define OLD_COLLISION_HANDLER_GRAVITY 0
#if OLD_COLLISION_HANDLER_GRAVITY
PN_stdfloat CollisionHandlerGravity::
set_highest_collision(const NodePath &target_node_path, const NodePath &from_node_path, const Entries &entries) {
  // Get the maximum height for all collisions with this node.
  bool got_max = false;
  PN_stdfloat max_height = 0.0f;
  CollisionEntry *highest = nullptr;

  Entries::const_iterator ei;
  for (ei = entries.begin(); ei != entries.end(); ++ei) {
    CollisionEntry *entry = (*ei);
    nassertr(entry != nullptr, 0.0f);
    nassertr(from_node_path == entry->get_from_node_path(), 0.0f);

    if (entry->has_surface_point()) {
      LPoint3 point = entry->get_surface_point(target_node_path);
      if (collide_cat.is_debug()) {
        collide_cat.debug()
          << "Intersection point detected at " << point << "\n";
      }

      PN_stdfloat height = point[2];
      if (!got_max || height > max_height) {
        got_max = true;
        max_height = height;
        highest = entry;
      }
    }
  }
  // #*#_has_contact = got_max;

  #if 0
    cout<<"\ncolliding with:\n";
    for (Colliding::const_iterator i = _current_colliding.begin(); i != _current_colliding.end(); ++i) {
      (**i).write(cout, 2);
    }
    cout<<"\nhighest:\n";
    highest->write(cout, 2);
    cout<<endl;
  #endif

  if (_legacy_mode) {
    // We only collide with things we are impacting with.  Remove the
    // collisions:
    _current_colliding.clear();
    // Add only the one that we're impacting with:
    add_entry(highest);
  }

  return max_height;
}
#else
PN_stdfloat CollisionHandlerGravity::
set_highest_collision(const NodePath &target_node_path, const NodePath &from_node_path, const Entries &entries) {
  // Get the maximum height for all collisions with this node.  This is really
  // the distance to-the-ground, so it will be negative when the avatar is
  // above the ground.  Larger values (less negative) are higher elevation
  // (assuming the avatar is right-side-up (or the ray is plumb)).
  bool got_max = false;
  bool got_min = false;
  PN_stdfloat max_height = 0.0f;
  PN_stdfloat min_height = 0.0f;
  CollisionEntry *highest = nullptr;
  CollisionEntry *lowest = nullptr;

  pvector<PT(CollisionEntry)> valid_entries;

  Entries::const_iterator ei;
  for (ei = entries.begin(); ei != entries.end(); ++ei) {
    CollisionEntry *entry = (*ei);
    nassertr(entry != nullptr, 0.0f);
    nassertr(from_node_path == entry->get_from_node_path(), 0.0f);

    if (entry->has_surface_point()) {
      LPoint3 point = entry->get_surface_point(target_node_path);
      if (collide_cat.is_debug()) {
        collide_cat.debug()
          << "Intersection point detected at " << point << "\n";
      }
      PN_stdfloat height = point[2];
      if(height < _offset + _reach) {
        valid_entries.push_back(entry);
        if (!got_max || height > max_height) {
          got_max = true;
          max_height = height;
          highest = entry;
        }
      }
      if (!got_min || height < min_height) {
        got_min = true;
        min_height = height;
        lowest = entry;
      }
    }
  }
  if (!got_max && got_min) {
    // We've fallen through the world, but we're also under some walkable
    // geometry.  Move us up to the lowest surface:
    got_max = true;
    max_height = min_height;
    highest = lowest;
    valid_entries.push_back(lowest);
  }
  // #*#_has_contact = got_max;

  #if 0
    cout<<"\ncolliding with:\n";
    for (Colliding::const_iterator i = _current_colliding.begin(); i != _current_colliding.end(); ++i) {
      (**i).write(cout, 2);
    }
    cout<<"\nhighest:\n";
    highest->write(cout, 2);
    cout<<endl;
  #endif

  // We only collide with things we are impacting with.  Remove the
  // collisions:
  _current_colliding.clear();
  if (_legacy_mode) {
    // Add only the one that we're impacting with:
    add_entry(highest);
  } else {
    // Add all of them.
    pvector<PT(CollisionEntry)>::iterator vi;
    for (vi = valid_entries.begin(); vi != valid_entries.end(); ++vi) {
      add_entry(*vi);
    }
  }


  // Set the contact normal so that other code can make use of the surface
  // slope:
  if (highest->get_into()->is_of_type(CollisionPlane::get_class_type())) {
/*
 * This is asking: what is the normal of the plane that the avatar is
 * colliding with relative to the avatar.  A positive y valye means the avatar
 * is facing downhill and a negative y value means the avatar is facing
 * uphill.  _contact_normal = DCAST(CollisionPlane,
 * highest->get_into())->get_normal() *
 * from_node_path.get_mat(highest->get_into_node_path()); _contact_normal =
 * DCAST(CollisionPlane, highest->get_into())->get_normal(); This is asking:
 * what is the normal of the avatar that the avatar is colliding with relative
 * to the plane.
 */
    CPT(TransformState) transform = highest->get_into_node_path().get_transform(from_node_path);
    _contact_normal = DCAST(CollisionPlane, highest->get_into())->get_normal() * transform->get_mat();
  } else {
    _contact_normal = highest->get_surface_normal(from_node_path);
  }

  return max_height;
}
#endif

/**
 * Called by the parent class after all collisions have been detected, this
 * manages the various collisions and moves around the nodes as necessary.
 *
 * The return value is normally true, but it may be false to indicate the
 * CollisionTraverser should disable this handler from being called in the
 * future.
 */
bool CollisionHandlerGravity::
handle_entries() {
  bool okflag = true;

  FromEntries::const_iterator fi;
  for (fi = _from_entries.begin(); fi != _from_entries.end(); ++fi) {
    const NodePath &from_node_path = (*fi).first;
    const Entries &entries = (*fi).second;

    Colliders::iterator ci;
    ci = _colliders.find(from_node_path);
    if (ci == _colliders.end()) {
      // Hmm, someone added a CollisionNode to a traverser and gave it this
      // CollisionHandler pointer--but they didn't tell us about the node.
      collide_cat.error()
        << get_type() << " doesn't know about "
        << from_node_path << ", disabling.\n";
      okflag = false;
    } else {
      ColliderDef &def = (*ci).second;
      PN_stdfloat max_height = set_highest_collision(def._target, from_node_path, entries);

      // Now set our height accordingly.
      #if OLD_COLLISION_HANDLER_GRAVITY
      PN_stdfloat adjust = max_height + _offset;
      #else
      PN_stdfloat adjust = max_height + _offset;
      #endif
      if (_current_velocity > 0.0f || !IS_THRESHOLD_ZERO(adjust, 0.001)) {
        if (collide_cat.is_debug()) {
          collide_cat.debug()
            << "Adjusting height by " << adjust << "\n";
        }

        if (_current_velocity > 0.0f || adjust) {
          // ...we have a vertical thrust, ...or the node is above the floor,
          // so it is airborne.
          PN_stdfloat dt = ClockObject::get_global_clock()->get_dt();
          // Fyi, the sign of _gravity is reversed.  I think it makes the
          // get_*() set_*() more intuitive to do it this way.
          PN_stdfloat gravity_adjust = _current_velocity * dt + 0.5 * -_gravity * dt * dt;
          if (adjust > 0.0f) {
            // ...the node is under the floor, so it has landed.  Keep the
            // adjust to bring us up to the ground and then add the
            // gravity_adjust to get us airborne:
            adjust += std::max((PN_stdfloat)0.0, gravity_adjust);
          } else {
            // ...the node is above the floor, so it is airborne.
            adjust = std::max(adjust, gravity_adjust);
          }
          _current_velocity -= _gravity * dt;
          // Record the airborne height in case someone else needs it:
          _airborne_height = -(max_height + _offset) + adjust;
          assert(_airborne_height >= -0.001f);
        }

        if (_airborne_height < 0.001f && _current_velocity < 0.001f) {
          // ...the node is under the floor, so it has landed.
          _impact_velocity = _current_velocity;
          // These values are used by is_on_ground().
          _current_velocity = _airborne_height = 0.0f;
        } else if (_legacy_mode) {
          // ...we're airborne.
          _current_colliding.clear();
        }

        CPT(TransformState) trans = def._target.get_transform();
        LVecBase3 pos = trans->get_pos();
        pos[2] += adjust;
        def._target.set_transform(trans->set_pos(pos));
        def.updated_transform();

        apply_linear_force(def, LVector3(0.0f, 0.0f, adjust));
      } else {
        // _impact_velocity = _current_velocity;
        _current_velocity = _airborne_height = 0.0f;
        if (collide_cat.is_spam()) {
          collide_cat.spam()
            << "Leaving height unchanged.\n";
        }
      }
    }
  }

  return okflag;
}

/**
 *
 */
void CollisionHandlerGravity::
apply_linear_force(ColliderDef &def, const LVector3 &force) {
}
