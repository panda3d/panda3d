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
BulletContactCallbackData(BulletManifoldPoint &mp, PandaNode *node0, PandaNode *node1, int id0, int id1, int index0, int index1) :
  _mp(mp),
  _node0(node0),
  _node1(node1),
  _id0(id0),
  _id1(id1),
  _index0(index0),
  _index1(index1) {

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

////////////////////////////////////////////////////////////////////
//     Function: BulletContactCallbackData::get_node0
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PandaNode *BulletContactCallbackData::
get_node0() const {

  return _node0;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletContactCallbackData::get_node1
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PandaNode *BulletContactCallbackData::
get_node1() const {

  return _node1;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletContactCallbackData::get_part_id0
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int BulletContactCallbackData::
get_part_id0() const {

  return _id0;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletContactCallbackData::get_part_id1
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int BulletContactCallbackData::
get_part_id1() const {

  return _id1;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletContactCallbackData::get_index0
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int BulletContactCallbackData::
get_index0() const {

  return _index0;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletContactCallbackData::get_index1
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int BulletContactCallbackData::
get_index1() const {

  return _index1;
}

