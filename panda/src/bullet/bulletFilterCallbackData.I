// Filename: bulletFilterCallbackData.I
// Created by:  enn0x (26Nov12)
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
//     Function: BulletFilterCallbackData::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BulletFilterCallbackData::
BulletFilterCallbackData(PandaNode *node0, PandaNode *node1) :
  _node0(node0),
  _node1(node1),
  _collide(false) {

}

////////////////////////////////////////////////////////////////////
//     Function: BulletFilterCallbackData::get_node_0
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PandaNode *BulletFilterCallbackData::
get_node_0() const {

  return _node0;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletFilterCallbackData::get_node_1
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PandaNode *BulletFilterCallbackData::
get_node_1() const {

  return _node1;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletFilterCallbackData::get_collide
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool BulletFilterCallbackData::
get_collide() const {

  return _collide;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletFilterCallbackData::set_collide
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void BulletFilterCallbackData::
set_collide(bool collide) {

  _collide = collide;
}

