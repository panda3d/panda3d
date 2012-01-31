// Filename: collisionHandlerPhysical.cxx
// Created by:  drose (16Mar02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "collisionHandlerPhysical.h"
#include "config_collide.h"

#include "transformState.h"

TypeHandle CollisionHandlerPhysical::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerPhysical::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CollisionHandlerPhysical::
CollisionHandlerPhysical() {
  _has_contact = false;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerPhysical::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CollisionHandlerPhysical::
~CollisionHandlerPhysical() {
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerPhysical::begin_group
//       Access: Public, Virtual
//  Description: Will be called by the CollisionTraverser before a new
//               traversal is begun.  It instructs the handler to
//               reset itself in preparation for a number of
//               CollisionEntries to be sent.
////////////////////////////////////////////////////////////////////
void CollisionHandlerPhysical::
begin_group() {
  CollisionHandlerEvent::begin_group();
  _from_entries.clear();
  _has_contact = false;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerPhysical::add_entry
//       Access: Public, Virtual
//  Description: Called between a begin_group() .. end_group()
//               sequence for each collision that is detected.
////////////////////////////////////////////////////////////////////
void CollisionHandlerPhysical::
add_entry(CollisionEntry *entry) {
  nassertv(entry != (CollisionEntry *)NULL);
  CollisionHandlerEvent::add_entry(entry);

  if (entry->get_from()->is_tangible() &&
      (!entry->has_into() || entry->get_into()->is_tangible())) {

    if (has_center()) {
      // If a center is specified, we have to make sure the surface is
      // more-or-less facing it.
      if (!entry->has_surface_point() || !entry->has_surface_normal()) {
        return;
      }

      LPoint3 point = entry->get_surface_point(_center);
      LVector3 normal = entry->get_surface_normal(_center);
       if (point.dot(normal) > 0) {
         return;
       }
    }

    _from_entries[entry->get_from_node_path()].push_back(entry);
    _has_contact = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerPhysical::end_group
//       Access: Public, Virtual
//  Description: Called by the CollisionTraverser at the completion of
//               all collision detections for this traversal.  It
//               should do whatever finalization is required for the
//               handler.
////////////////////////////////////////////////////////////////////
bool CollisionHandlerPhysical::
end_group() {
  bool result = handle_entries();
  CollisionHandlerEvent::end_group();

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerPhysical::add_collider
//       Access: Published
//  Description: Adds a new collider to the list with a NodePath
//               that will be updated with the collider's new
//               position, or updates the existing collider with a new
//               NodePath object.
////////////////////////////////////////////////////////////////////
void CollisionHandlerPhysical::
add_collider(const NodePath &collider, const NodePath &target) {
  nassertv(!collider.is_empty() && collider.node()->is_collision_node());
  nassertv(validate_target(target));
  _colliders[collider].set_target(target);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerPhysical::add_collider
//       Access: Published
//  Description: Adds a new collider to the list with a NodePath
//               that will be updated with the collider's new
//               position, or updates the existing collider with a new
//               NodePath object.
//
//               The indicated DriveInterface will also be updated
//               with the target's new transform each frame.  This
//               method should be used when the target is directly
//               controlled by a DriveInterface.
////////////////////////////////////////////////////////////////////
void CollisionHandlerPhysical::
add_collider(const NodePath &collider, const NodePath &target,
             DriveInterface *drive_interface) {
  nassertv(!collider.is_empty() && collider.node()->is_collision_node());
  nassertv(validate_target(target));
  _colliders[collider].set_target(target, drive_interface);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerPhysical::remove_collider
//       Access: Published
//  Description: Removes the collider from the list of colliders that
//               this handler knows about.
////////////////////////////////////////////////////////////////////
bool CollisionHandlerPhysical::
remove_collider(const NodePath &collider) {
  Colliders::iterator ci = _colliders.find(collider);
  if (ci == _colliders.end()) {
    return false;
  }
  _colliders.erase(ci);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerPhysical::has_collider
//       Access: Published
//  Description: Returns true if the handler knows about the indicated
//               collider, false otherwise.
////////////////////////////////////////////////////////////////////
bool CollisionHandlerPhysical::
has_collider(const NodePath &target) const {
  Colliders::const_iterator ci = _colliders.find(target);
  return (ci != _colliders.end());
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerPhysical::clear_colliders
//       Access: Published
//  Description: Completely empties the list of colliders this handler
//               knows about.
////////////////////////////////////////////////////////////////////
void CollisionHandlerPhysical::
clear_colliders() {
  _colliders.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerPhysical::validate_target
//       Access: Protected, Virtual
//  Description: Called internally to validate the target passed to
//               add_collider().  Returns true if acceptable, false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool CollisionHandlerPhysical::
validate_target(const NodePath &target) {
  nassertr_always(!target.is_empty(), false);
  return true;
}
