// Filename: bulletGhostNode.I
// Created by:  enn0x (19Nov10)
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
//     Function: BulletGhostNode::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BulletGhostNode::
~BulletGhostNode() {

  delete _ghost;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletGhostNode::get_num_overlapping_nodes
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int BulletGhostNode::
get_num_overlapping_nodes() const {

  return _ghost->getNumOverlappingObjects();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletGhostNode::get_overlapping_node
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PandaNode *BulletGhostNode::
get_overlapping_node(int idx) const {

  nassertr(idx >=0 && idx < _ghost->getNumOverlappingObjects(), NULL);

  btCollisionObject *object = _ghost->getOverlappingObject(idx);
  return (object) ? (PandaNode *)object->getUserPointer() : NULL;
}

