/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionHandlerHighestEvent.cxx
 * @author drose
 * @date 2002-03-16
 */

#include "collisionHandlerHighestEvent.h"
#include "config_collide.h"

#include "eventParameter.h"
#include "throw_event.h"


TypeHandle CollisionHandlerHighestEvent::_type_handle;

/**
 * The default CollisionHandlerEvent will throw no events.  Its pattern
 * strings must first be set via a call to add_in_pattern() and/or
 * add_out_pattern().
 */
CollisionHandlerHighestEvent::
CollisionHandlerHighestEvent() {
}

/**
 * Will be called by the CollisionTraverser before a new traversal is begun.
 * It instructs the handler to reset itself in preparation for a number of
 * CollisionEntries to be sent.
 */
void CollisionHandlerHighestEvent::
begin_group() {
  if (collide_cat.is_spam()) {
    collide_cat.spam()
      << "begin_group.\n";
  }
  _last_colliding.clear();
  if (_closest_collider) {
    _last_colliding.insert(_closest_collider);
  }
  _current_colliding.clear();
  _collider_distance = 0;
  _closest_collider = nullptr;
}

/**
 * Called between a begin_group() .. end_group() sequence for each collision
 * that is detected.
 */
void CollisionHandlerHighestEvent::
add_entry(CollisionEntry *entry) {
  nassertv(entry != nullptr);
  LVector3 vec =
    entry->get_surface_point(entry->get_from_node_path()) -
    entry->get_from()->get_collision_origin();
  double dist = vec.length_squared();
  if (_closest_collider == nullptr || dist < _collider_distance) {
    _collider_distance = dist;
    _closest_collider = entry;
  }
}

/**
 * Called by the CollisionTraverser at the completion of all collision
 * detections for this traversal.  It should do whatever finalization is
 * required for the handler.
 */
bool CollisionHandlerHighestEvent::
end_group() {
  if (_closest_collider) {
    _current_colliding.insert(_closest_collider);
  }
  return CollisionHandlerEvent::end_group();
}
