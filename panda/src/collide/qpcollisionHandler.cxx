// Filename: qpcollisionHandler.cxx
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

#include "qpcollisionHandler.h"

TypeHandle qpCollisionHandler::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: qpCollisionHandler::begin_group
//       Access: Public, Virtual
//  Description: Will be called by the CollisionTraverser before a new
//               traversal is begun.  It instructs the handler to
//               reset itself in preparation for a number of
//               CollisionEntries to be sent.
////////////////////////////////////////////////////////////////////
void qpCollisionHandler::
begin_group() {
}

////////////////////////////////////////////////////////////////////
//     Function: qpCollisionHandler::add_entry
//       Access: Public, Virtual
//  Description: Called between a begin_group() .. end_group()
//               sequence for each collision that is detected.
////////////////////////////////////////////////////////////////////
void qpCollisionHandler::
add_entry(qpCollisionEntry *) {
}

////////////////////////////////////////////////////////////////////
//     Function: qpCollisionHandler::end_group
//       Access: Public, Virtual
//  Description: Called by the CollisionTraverser at the completion of
//               all collision detections for this traversal.  It
//               should do whatever finalization is required for the
//               handler.
//
//               The return value is normally true, but if this
//               returns value, the CollisionTraverser will remove the
//               handler from its list, allowing the qpCollisionHandler
//               itself to determine when it is no longer needed.
////////////////////////////////////////////////////////////////////
bool qpCollisionHandler::
end_group() {
  return true;
}
