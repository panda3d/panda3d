// Filename: collisionHandler.cxx
// Created by:  drose (24Apr00)
// 
////////////////////////////////////////////////////////////////////

#include "collisionHandler.h"

TypeHandle CollisionHandler::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandler::begin_group
//       Access: Public, Virtual
//  Description: Will be called by the CollisionTraverser before a new
//               traversal is begun.  It instructs the handler to
//               reset itself in preparation for a number of
//               CollisionEntries to be sent.
////////////////////////////////////////////////////////////////////
void CollisionHandler::
begin_group() {
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandler::add_entry
//       Access: Public, Virtual
//  Description: Called between a begin_group() .. end_group()
//               sequence for each collision that is detected.
////////////////////////////////////////////////////////////////////
void CollisionHandler::
add_entry(CollisionEntry *) {
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandler::end_group
//       Access: Public, Virtual
//  Description: Called by the CollisionTraverser at the completion of
//               all collision detections for this traversal.  It
//               should do whatever finalization is required for the
//               handler.
////////////////////////////////////////////////////////////////////
void CollisionHandler::
end_group() {
}
