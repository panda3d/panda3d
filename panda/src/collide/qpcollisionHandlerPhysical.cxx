// Filename: qpcollisionHandlerPhysical.cxx
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

#include "qpcollisionHandlerPhysical.h"
#include "config_collide.h"

#include "transformState.h"

TypeHandle qpCollisionHandlerPhysical::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: qpCollisionHandlerPhysical::ColliderDef::get_mat
//       Access: Public
//  Description: Fills mat with the matrix representing the current
//               position and orientation of this collider.
////////////////////////////////////////////////////////////////////
void qpCollisionHandlerPhysical::ColliderDef::
get_mat(LMatrix4f &mat) const {
  if (_node != (PandaNode *)NULL) {
    mat = _node->get_transform()->get_mat();

  } else if (_drive_interface != (qpDriveInterface *)NULL) {
    mat = _drive_interface->get_mat();

  } else {
    collide_cat.error()
      << "Invalid qpCollisionHandlerPhysical::ColliderDef\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpCollisionHandlerPhysical::ColliderDef::set_mat
//       Access: Public
//  Description: Moves this collider to the position and orientation
//               indicated by the given transform.
////////////////////////////////////////////////////////////////////
void qpCollisionHandlerPhysical::ColliderDef::
set_mat(const LMatrix4f &mat) {
  if (_node != (PandaNode *)NULL) {
    _node->set_transform(TransformState::make_mat(mat));

  } else if (_drive_interface != (qpDriveInterface *)NULL) {
    _drive_interface->set_mat(mat);
    _drive_interface->force_dgraph();

  } else {
    collide_cat.error()
      << "Invalid qpCollisionHandlerPhysical::ColliderDef\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpCollisionHandlerPhysical::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
qpCollisionHandlerPhysical::
qpCollisionHandlerPhysical() {
}

////////////////////////////////////////////////////////////////////
//     Function: qpCollisionHandlerPhysical::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
qpCollisionHandlerPhysical::
~qpCollisionHandlerPhysical() {
}

////////////////////////////////////////////////////////////////////
//     Function: qpCollisionHandlerPhysical::begin_group
//       Access: Public, Virtual
//  Description: Will be called by the CollisionTraverser before a new
//               traversal is begun.  It instructs the handler to
//               reset itself in preparation for a number of
//               CollisionEntries to be sent.
////////////////////////////////////////////////////////////////////
void qpCollisionHandlerPhysical::
begin_group() {
  qpCollisionHandlerEvent::begin_group();
  _from_entries.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: qpCollisionHandlerPhysical::add_entry
//       Access: Public, Virtual
//  Description: Called between a begin_group() .. end_group()
//               sequence for each collision that is detected.
////////////////////////////////////////////////////////////////////
void qpCollisionHandlerPhysical::
add_entry(qpCollisionEntry *entry) {
  nassertv(entry != (qpCollisionEntry *)NULL);
  qpCollisionHandlerEvent::add_entry(entry);

  if (entry->get_from()->is_tangible() &&
      (!entry->has_into() || entry->get_into()->is_tangible())) {
    _from_entries[entry->get_from_node()].push_back(entry);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpCollisionHandlerPhysical::end_group
//       Access: Public, Virtual
//  Description: Called by the CollisionTraverser at the completion of
//               all collision detections for this traversal.  It
//               should do whatever finalization is required for the
//               handler.
////////////////////////////////////////////////////////////////////
bool qpCollisionHandlerPhysical::
end_group() {
  qpCollisionHandlerEvent::end_group();

  return handle_entries();
}

////////////////////////////////////////////////////////////////////
//     Function: qpCollisionHandlerPhysical::add_collider
//       Access: Public
//  Description: Adds a new collider to the list with a qpDriveInterface
//               pointer that needs to be told about the collider's
//               new position, or updates the existing collider with a
//               new qpDriveInterface pointer.
////////////////////////////////////////////////////////////////////
void qpCollisionHandlerPhysical::
add_collider(qpCollisionNode *node, qpDriveInterface *drive_interface) {
  _colliders[node].set_drive_interface(drive_interface);
}

////////////////////////////////////////////////////////////////////
//     Function: qpCollisionHandlerPhysical::add_collider
//       Access: Public
//  Description: Adds a new collider to the list with a PandaNode
//               pointer that will be updated with the collider's
//               new position, or updates the existing collider with a
//               new PandaNode pointer.
////////////////////////////////////////////////////////////////////
void qpCollisionHandlerPhysical::
add_collider(qpCollisionNode *node, PandaNode *target) {
  _colliders[node].set_node(target);
}

////////////////////////////////////////////////////////////////////
//     Function: qpCollisionHandlerPhysical::remove_collider
//       Access: Public
//  Description: Removes the collider from the list of colliders that
//               this handler knows about.
////////////////////////////////////////////////////////////////////
bool qpCollisionHandlerPhysical::
remove_collider(qpCollisionNode *node) {
  Colliders::iterator ci = _colliders.find(node);
  if (ci == _colliders.end()) {
    return false;
  }
  _colliders.erase(ci);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: qpCollisionHandlerPhysical::has_collider
//       Access: Public
//  Description: Returns true if the handler knows about the indicated
//               collider, false otherwise.
////////////////////////////////////////////////////////////////////
bool qpCollisionHandlerPhysical::
has_collider(qpCollisionNode *node) const {
  Colliders::const_iterator ci = _colliders.find(node);
  return (ci != _colliders.end());
}

////////////////////////////////////////////////////////////////////
//     Function: qpCollisionHandlerPhysical::clear_colliders
//       Access: Public
//  Description: Completely empties the list of colliders this handler
//               knows about.
////////////////////////////////////////////////////////////////////
void qpCollisionHandlerPhysical::
clear_colliders() {
  _colliders.clear();
}


