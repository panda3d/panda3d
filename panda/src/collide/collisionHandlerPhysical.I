// Filename: collisionHandlerPhysical.I
// Created by:  drose (03Jul00)
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

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerPhysical::ColliderDef::set_drive_interface
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CollisionHandlerPhysical::ColliderDef::
set_drive_interface(DriveInterface *drive_interface) {
  _drive_interface = drive_interface;
  _arc.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerPhysical::ColliderDef::set_arc
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CollisionHandlerPhysical::ColliderDef::
set_arc(NodeRelation *arc) {
  _arc = arc;
  _drive_interface.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerPhysical::ColliderDef::is_valid
//       Access: Public
//  Description: Returns true if this ColliderDef is still valid;
//               e.g. it refers to a valid arc or drive interface.
////////////////////////////////////////////////////////////////////
INLINE bool CollisionHandlerPhysical::ColliderDef::
is_valid() const {
  return (_arc != (NodeRelation *)NULL) ||
    (_drive_interface != (DriveInterface *)NULL);
}
