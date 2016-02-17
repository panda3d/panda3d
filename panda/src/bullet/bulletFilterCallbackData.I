/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletFilterCallbackData.I
 * @author enn0x
 * @date 2012-11-26
 */

/**
 *
 */
INLINE BulletFilterCallbackData::
BulletFilterCallbackData(PandaNode *node0, PandaNode *node1) :
  _node0(node0),
  _node1(node1),
  _collide(false) {

}

/**
 *
 */
INLINE PandaNode *BulletFilterCallbackData::
get_node_0() const {

  return _node0;
}

/**
 *
 */
INLINE PandaNode *BulletFilterCallbackData::
get_node_1() const {

  return _node1;
}

/**
 *
 */
INLINE bool BulletFilterCallbackData::
get_collide() const {

  return _collide;
}

/**
 *
 */
INLINE void BulletFilterCallbackData::
set_collide(bool collide) {

  _collide = collide;
}
