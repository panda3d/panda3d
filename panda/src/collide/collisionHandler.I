// Filename: collisionHandler.I
// Created by:  WDIG (15Aug07)
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

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandler::wants_all_potential_collidees
//       Access: Private
//  Description: Returns true if handler wants to know about all
//               solids that are within the collider's bounding
//               volume
////////////////////////////////////////////////////////////////////
INLINE bool CollisionHandler::
wants_all_potential_collidees() const {
  return _wants_all_potential_collidees;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandler::set_root
//       Access: Private
//  Description: Sets the root of the collision traversal. Only set
//               if wants_all_potential_collidees is true
////////////////////////////////////////////////////////////////////
INLINE void CollisionHandler::
set_root(const NodePath &root) {
  _root = &root;
}
