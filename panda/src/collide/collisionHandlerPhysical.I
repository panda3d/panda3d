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
//     Function: CollisionHandlerPhysical::ColliderDef::set_drive_interface
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CollisionHandlerPhysical::ColliderDef::
set_drive_interface(DriveInterface *drive_interface) {
  _drive_interface = drive_interface;
  _node.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerPhysical::ColliderDef::set_node
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CollisionHandlerPhysical::ColliderDef::
set_node(PandaNode *node) {
  _node = node;
  _drive_interface.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerPhysical::ColliderDef::set_target
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CollisionHandlerPhysical::ColliderDef::
set_target(const NodePath &target) {
  _target = target;
  if (!target.is_empty()) {
    _node = target.node();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerPhysical::ColliderDef::is_valid
//       Access: Public
//  Description: Returns true if this ColliderDef is still valid;
//               e.g. it refers to a valid node or drive interface.
////////////////////////////////////////////////////////////////////
INLINE bool CollisionHandlerPhysical::ColliderDef::
is_valid() const {
  return (_node != (PandaNode *)NULL) ||
    (_drive_interface != (DriveInterface *)NULL);
}
