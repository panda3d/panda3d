// Filename: collisionHandlerPhysical.I
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


////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerPhysical::ColliderDef::set_target
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CollisionHandlerPhysical::ColliderDef::
set_target(const NodePath &target, DriveInterface *drive_interface) {
  _target = target;
  _drive_interface = drive_interface;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerPhysical::ColliderDef::updated_transform
//       Access: Public
//  Description: Called by the handler when it has changed the
//               transform on the target node, this applies the change
//               to the drive interface if one is specified.
////////////////////////////////////////////////////////////////////
INLINE void CollisionHandlerPhysical::ColliderDef::
updated_transform() {
  if (_drive_interface != (DriveInterface *)NULL) {
    _drive_interface->set_mat(_target.get_mat());
    _drive_interface->force_dgraph();
  }
}
