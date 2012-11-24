// Filename: bulletContactCallbackData.I
// Created by:  enn0x (22Nov12)
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
//     Function: BulletContactCallbackData::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BulletContactCallbackData::
BulletContactCallbackData(BulletManifoldPoint &mp, PandaNode *node0, PandaNode *node1) :
  _node0(node0),
  _node1(node1),
  _mp(mp) {

}

////////////////////////////////////////////////////////////////////
//     Function: BulletContactCallbackData::get_node_0
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PandaNode *BulletContactCallbackData::
get_node_0() const {

  return _node0;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletContactCallbackData::get_node_1
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PandaNode *BulletContactCallbackData::
get_node_1() const {

  return _node1;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletContactCallbackData::get_manifold
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BulletManifoldPoint &BulletContactCallbackData::
get_manifold() const {

  return _mp;
}

