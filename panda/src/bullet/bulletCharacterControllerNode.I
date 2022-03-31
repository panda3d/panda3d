/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletCharacterControllerNode.I
 * @author enn0x
 * @date 2010-11-21
 */

/**
 *
 */
INLINE BulletCharacterControllerNode::
~BulletCharacterControllerNode() {

  delete _character;
  delete _ghost;
}

/**
 *
 */
INLINE btPairCachingGhostObject *BulletCharacterControllerNode::
get_ghost() const {

  return _ghost;
}

/**
 *
 */
INLINE btCharacterControllerInterface *BulletCharacterControllerNode::
get_character() const {

  return _character;
}
