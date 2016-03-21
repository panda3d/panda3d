/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletGhostNode.I
 * @author enn0x
 * @date 2010-11-19
 */

/**
 *
 */
INLINE BulletGhostNode::
~BulletGhostNode() {

  delete _ghost;
}

/**
 *
 */
INLINE int BulletGhostNode::
get_num_overlapping_nodes() const {

  return _ghost->getNumOverlappingObjects();
}

/**
 *
 */
INLINE PandaNode *BulletGhostNode::
get_overlapping_node(int idx) const {

  nassertr(idx >=0 && idx < _ghost->getNumOverlappingObjects(), NULL);

  btCollisionObject *object = _ghost->getOverlappingObject(idx);
  return (object) ? (PandaNode *)object->getUserPointer() : NULL;
}
