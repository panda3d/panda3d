// Filename: collisionHandlerPhysical.cxx
// Created by:  drose (03Jul00)
// 
////////////////////////////////////////////////////////////////////

#include "collisionHandlerPhysical.h"
#include "config_collide.h"

#include <renderRelation.h>
#include <transformTransition.h>

TypeHandle CollisionHandlerPhysical::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerPhysical::ColliderDef::get_mat
//       Access: Public
//  Description: Fills mat with the matrix representing the current
//               position and orientation of this collider.
////////////////////////////////////////////////////////////////////
void CollisionHandlerPhysical::ColliderDef::
get_mat(LMatrix4f &mat) const {
  if (_arc != (NodeRelation *)NULL) {
    TransformTransition *tt;
    if (get_transition_into(tt, _arc)) {
      mat = tt->get_matrix();
    } else {
      mat = LMatrix4f::ident_mat();
    }

  } else if (_drive_interface != (DriveInterface *)NULL) {
    mat = _drive_interface->get_mat();

  } else {
    collide_cat.error()
      << "Invalid CollisionHandlerPhysical::ColliderDef\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerPhysical::ColliderDef::set_mat
//       Access: Public
//  Description: Moves this collider to the position and orientation
//               indicated by the given transform.
////////////////////////////////////////////////////////////////////
void CollisionHandlerPhysical::ColliderDef::
set_mat(const LMatrix4f &mat) const {
  if (_arc != (NodeRelation *)NULL) {
    _arc->set_transition(new TransformTransition(mat));

  } else if (_drive_interface != (DriveInterface *)NULL) {
    _drive_interface->set_mat(mat);

  } else {
    collide_cat.error()
      << "Invalid CollisionHandlerPhysical::ColliderDef\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerPhysical::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
CollisionHandlerPhysical::
CollisionHandlerPhysical() {
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
    _from_entries[entry->get_from_node()].push_back(entry);
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
void CollisionHandlerPhysical::
end_group() {
  CollisionHandlerEvent::end_group();

  handle_entries();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerPhysical::add_collider
//       Access: Public
//  Description: Adds a new collider to the list with a DriveInterface
//               pointer that needs to be told about the collider's
//               new position, or updates the existing collider with a
//               new DriveInterface pointer.
////////////////////////////////////////////////////////////////////
void CollisionHandlerPhysical::
add_collider(CollisionNode *node, DriveInterface *drive_interface) {
  _colliders[node].set_drive_interface(drive_interface);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerPhysical::add_collider
//       Access: Public
//  Description: Adds a new collider to the list with a NodeRelation
//               pointer that will be updated with the collider's
//               new position, or updates the existing collider with a
//               new NodeRelation pointer.
////////////////////////////////////////////////////////////////////
void CollisionHandlerPhysical::
add_collider(CollisionNode *node, NodeRelation *arc) {
  _colliders[node].set_arc(arc);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerPhysical::remove_collider
//       Access: Public
//  Description: Removes the collider from the list of colliders that
//               this handler knows about.
////////////////////////////////////////////////////////////////////
bool CollisionHandlerPhysical::
remove_collider(CollisionNode *node) {
  Colliders::iterator ci = _colliders.find(node);
  if (ci == _colliders.end()) {
    return false;
  }
  _colliders.erase(ci);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerPhysical::has_collider
//       Access: Public
//  Description: Returns true if the handler knows about the indicated
//               collider, false otherwise.
////////////////////////////////////////////////////////////////////
bool CollisionHandlerPhysical::
has_collider(CollisionNode *node) const {
  Colliders::const_iterator ci = _colliders.find(node);
  return (ci != _colliders.end());
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerPhysical::clear_colliders
//       Access: Public
//  Description: Completely empties the list of colliders this handler
//               knows about.
////////////////////////////////////////////////////////////////////
void CollisionHandlerPhysical::
clear_colliders() {
  _colliders.clear();
}


