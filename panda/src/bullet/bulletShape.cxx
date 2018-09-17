/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletShape.cxx
 * @author enn0x
 * @date 2010-01-23
 */

#include "bulletShape.h"

#include "bulletWorld.h"

#include "bullet_utils.h"

TypeHandle BulletShape::_type_handle;

/**
 *
 */
const char *BulletShape::
get_name() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return ptr()->getName();
}

/**
 *
 */
PN_stdfloat BulletShape::
get_margin() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return ptr()->getMargin();
}

/**
 *
 */
void BulletShape::
set_margin(PN_stdfloat margin) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  ptr()->setMargin(margin);
}

/**
 *
 */
LVecBase3 BulletShape::
get_local_scale() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return btVector3_to_LVecBase3(ptr()->getLocalScaling());
}

/**
 * Assumes the lock(bullet global lock) is held by the caller
 */
void BulletShape::
do_set_local_scale(const LVecBase3 &scale) {

  nassertv(!scale.is_nan());
  ptr()->setLocalScaling(LVecBase3_to_btVector3(scale));
}

/**
 *
 */
void BulletShape::
set_local_scale(const LVecBase3 &scale) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  do_set_local_scale(scale);
}

/**
 * Returns the current bounds of this collision shape.
 */
BoundingSphere BulletShape::
get_shape_bounds() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

/*
  btTransform tr;
  tr.setIdentity();
  btVector3 aabbMin,aabbMax;
  ptr()->getAabb(tr,aabbMin,aabbMax);
  btVector3 o = tr.getOrigin();
cout << "aabbMin " << aabbMin.x() << " " << aabbMin.y() << " " << aabbMin.z() << endl;
cout << "aabbMax " << aabbMax.x() << " " << aabbMax.y() << " " << aabbMax.z() << endl;
cout << "origin " << aabbMin.x() << " " << aabbMin.y() << " " << aabbMin.z() << endl;
*/

  btVector3 center;
  btScalar radius;

  ptr()->getBoundingSphere(center, radius);
  BoundingSphere bounds(btVector3_to_LPoint3(center), (PN_stdfloat)radius);

  return bounds;
}

/**
 *
 */
bool BulletShape::
is_polyhedral() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return ptr()->isPolyhedral();
}

/**
 *
 */
bool BulletShape::
is_convex() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return ptr()->isConvex();
}

/**
 *
 */
bool BulletShape::
is_convex_2d() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return ptr()->isConvex2d();
}

/**
 *
 */
bool BulletShape::
is_concave() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return ptr()->isConcave();
}

/**
 *
 */
bool BulletShape::
is_infinite() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return ptr()->isInfinite();
}

/**
 *
 */
bool BulletShape::
is_non_moving() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return ptr()->isNonMoving();
}

/**
 *
 */
bool BulletShape::
is_soft_body() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return ptr()->isSoftBody();
}
