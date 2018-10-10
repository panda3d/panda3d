/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletSoftBodyShape.cxx
 * @author enn0x
 * @date 2010-05-06
 */

#include "bulletSoftBodyShape.h"

#include "bulletSoftBodyNode.h"
#include "bulletWorld.h"

TypeHandle BulletSoftBodyShape::_type_handle;

/**
 *
 */
BulletSoftBodyShape::
BulletSoftBodyShape(btSoftBodyCollisionShape *shapePtr) {

  _shape = shapePtr;
  _shape->setUserPointer(this);
}

/**
 *
 */
btCollisionShape *BulletSoftBodyShape::
ptr() const {

  return _shape;
}

/**
 *
 */
BulletSoftBodyNode *BulletSoftBodyShape::
get_body() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  if (_shape->m_body) {
    return (BulletSoftBodyNode *)_shape->m_body->getUserPointer();
  }
  else
  {
    return nullptr;
  }
}
