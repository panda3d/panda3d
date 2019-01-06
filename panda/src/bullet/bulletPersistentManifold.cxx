/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletPersistentManifold.cxx
 * @author enn0x
 * @date 2010-03-07
 */

#include "bulletPersistentManifold.h"

#include "bulletManifoldPoint.h"
#include "bulletWorld.h"

/**
 *
 */
BulletPersistentManifold::
BulletPersistentManifold(btPersistentManifold *manifold) : _manifold(manifold) {

}

/**
 *
 */
PN_stdfloat BulletPersistentManifold::
get_contact_breaking_threshold() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_manifold->getContactBreakingThreshold();
}

/**
 *
 */
PN_stdfloat BulletPersistentManifold::
get_contact_processing_threshold() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_manifold->getContactProcessingThreshold();
}

/**
 *
 */
void BulletPersistentManifold::
clear_manifold() {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _manifold->clearManifold();
}

/**
 *
 */
PandaNode *BulletPersistentManifold::
get_node0() {
  LightMutexHolder holder(BulletWorld::get_global_lock());

#if BT_BULLET_VERSION >= 281
  const btCollisionObject *obj = _manifold->getBody0();
#else
  const btCollisionObject *obj = (btCollisionObject *)_manifold->getBody0();
#endif

  return (obj) ? (PandaNode *)obj->getUserPointer(): nullptr;
}

/**
 *
 */
PandaNode *BulletPersistentManifold::
get_node1() {
  LightMutexHolder holder(BulletWorld::get_global_lock());

#if BT_BULLET_VERSION >= 281
  const btCollisionObject *obj = _manifold->getBody1();
#else
  const btCollisionObject *obj = (btCollisionObject *)_manifold->getBody1();
#endif

  return (obj) ? (PandaNode *)obj->getUserPointer(): nullptr;
}

/**
 *
 */
int BulletPersistentManifold::
get_num_manifold_points() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return _manifold->getNumContacts();
}

/**
 *
 */
BulletManifoldPoint *BulletPersistentManifold::
get_manifold_point(int idx) const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertr(idx < _manifold->getNumContacts(), nullptr)

  return new BulletManifoldPoint(_manifold->getContactPoint(idx));
}
