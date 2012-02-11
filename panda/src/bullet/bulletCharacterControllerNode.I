// Filename: bulletCharacterControllerNode.I
// Created by:  enn0x (21Nov10)
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
//     Function: BulletCharacterControllerNode::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BulletCharacterControllerNode::
~BulletCharacterControllerNode() {

}

////////////////////////////////////////////////////////////////////
//     Function: BulletCharacterControllerNode::get_ghost
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE btPairCachingGhostObject *BulletCharacterControllerNode::
get_ghost() const {

  return _ghost;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletCharacterControllerNode::get_character
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE btCharacterControllerInterface *BulletCharacterControllerNode::
get_character() const {

  return _character;
}

