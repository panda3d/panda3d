/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletManifoldPoint.cxx
 * @author enn0x
 * @date 2010-03-07
 */

#include "bulletManifoldPoint.h"

#include "bulletWorld.h"

/**
 *
 */
BulletManifoldPoint::
BulletManifoldPoint(btManifoldPoint &pt)
 : _pt(pt) {

}

/**
 *
 */
BulletManifoldPoint::
BulletManifoldPoint(const BulletManifoldPoint &other)
 : _pt(other._pt) {

}

/**
 *
 */
BulletManifoldPoint& BulletManifoldPoint::
operator=(const BulletManifoldPoint& other) {

  this->_pt = other._pt;
  return *this;
}

/**
 *
 */
int BulletManifoldPoint::
get_life_time() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return _pt.getLifeTime();
}

/**
 *
 */
PN_stdfloat BulletManifoldPoint::
get_distance() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_pt.getDistance();
}

/**
 *
 */
PN_stdfloat BulletManifoldPoint::
get_applied_impulse() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_pt.getAppliedImpulse();
}

/**
 *
 */
LPoint3 BulletManifoldPoint::
get_position_world_on_a() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return btVector3_to_LPoint3(_pt.getPositionWorldOnA());
}

/**
 *
 */
LPoint3 BulletManifoldPoint::
get_position_world_on_b() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return btVector3_to_LPoint3(_pt.getPositionWorldOnB());
}

/**
 *
 */
LVector3 BulletManifoldPoint::
get_normal_world_on_b() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return btVector3_to_LVector3(_pt.m_normalWorldOnB);
}

/**
 *
 */
LPoint3 BulletManifoldPoint::
get_local_point_a() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return btVector3_to_LPoint3(_pt.m_localPointA);
}

/**
 *
 */
LPoint3 BulletManifoldPoint::
get_local_point_b() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return btVector3_to_LPoint3(_pt.m_localPointB);
}

/**
 *
 */
int BulletManifoldPoint::
get_part_id0() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return _pt.m_partId0;
}

/**
 *
 */
int BulletManifoldPoint::
get_part_id1() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return _pt.m_partId1;
}

/**
 *
 */
int BulletManifoldPoint::
get_index0() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return _pt.m_index0;
}

/**
 *
 */
int BulletManifoldPoint::
get_index1() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return _pt.m_index1;
}

/**
 *
 */
void BulletManifoldPoint::
set_lateral_friction_initialized(bool value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

#if BT_BULLET_VERSION >= 285
  if (value) {
    _pt.m_contactPointFlags |= BT_CONTACT_FLAG_LATERAL_FRICTION_INITIALIZED;
  } else {
    _pt.m_contactPointFlags &= ~BT_CONTACT_FLAG_LATERAL_FRICTION_INITIALIZED;
  }
#else
  _pt.m_lateralFrictionInitialized = value;
#endif
}

/**
 *
 */
bool BulletManifoldPoint::
get_lateral_friction_initialized() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

#if BT_BULLET_VERSION >= 285
  return (_pt.m_contactPointFlags & BT_CONTACT_FLAG_LATERAL_FRICTION_INITIALIZED) != 0;
#else
  return _pt.m_lateralFrictionInitialized;
#endif
}

/**
 *
 */
void BulletManifoldPoint::
set_lateral_friction_dir1(const LVecBase3 &dir) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _pt.m_lateralFrictionDir1 = LVecBase3_to_btVector3(dir);
}

/**
 *
 */
LVector3 BulletManifoldPoint::
get_lateral_friction_dir1() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return btVector3_to_LVector3(_pt.m_lateralFrictionDir1);
}

/**
 *
 */
void BulletManifoldPoint::
set_lateral_friction_dir2(const LVecBase3 &dir) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _pt.m_lateralFrictionDir2 = LVecBase3_to_btVector3(dir);
}

/**
 *
 */
LVector3 BulletManifoldPoint::
get_lateral_friction_dir2() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return btVector3_to_LVector3(_pt.m_lateralFrictionDir2);
}

/**
 *
 */
void BulletManifoldPoint::
set_contact_motion1(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _pt.m_contactMotion1 = (btScalar)value;
}

/**
 *
 */
PN_stdfloat BulletManifoldPoint::
get_contact_motion1() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_pt.m_contactMotion1;
}

/**
 *
 */
void BulletManifoldPoint::
set_contact_motion2(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _pt.m_contactMotion2 = (btScalar)value;
}

/**
 *
 */
PN_stdfloat BulletManifoldPoint::
get_contact_motion2() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_pt.m_contactMotion2;
}

/**
 *
 */
void BulletManifoldPoint::
set_combined_friction(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _pt.m_combinedFriction = (btScalar)value;
}

/**
 *
 */
PN_stdfloat BulletManifoldPoint::
get_combined_friction() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_pt.m_combinedFriction;
}

/**
 *
 */
void BulletManifoldPoint::
set_combined_restitution(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _pt.m_combinedRestitution = (btScalar)value;
}

/**
 *
 */
PN_stdfloat BulletManifoldPoint::
get_combined_restitution() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_pt.m_combinedRestitution;
}

/**
 *
 */
void BulletManifoldPoint::
set_applied_impulse(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _pt.m_appliedImpulse = (btScalar)value;
}

/**
 *
 */
void BulletManifoldPoint::
set_applied_impulse_lateral1(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _pt.m_appliedImpulseLateral1 = (btScalar)value;
}

/**
 *
 */
PN_stdfloat BulletManifoldPoint::
get_applied_impulse_lateral1() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_pt.m_appliedImpulseLateral1;
}

/**
 *
 */
void BulletManifoldPoint::
set_applied_impulse_lateral2(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _pt.m_appliedImpulseLateral2 = (btScalar)value;
}

/**
 *
 */
PN_stdfloat BulletManifoldPoint::
get_applied_impulse_lateral2() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_pt.m_appliedImpulseLateral2;
}

/**
 *
 */
void BulletManifoldPoint::
set_contact_cfm1(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

#if BT_BULLET_VERSION < 285
  _pt.m_contactCFM1 = (btScalar)value;
#endif
}

/**
 *
 */
PN_stdfloat BulletManifoldPoint::
get_contact_cfm1() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

#if BT_BULLET_VERSION < 285
  return (PN_stdfloat)_pt.m_contactCFM1;
#else
  return 0;
#endif
}

/**
 *
 */
void BulletManifoldPoint::
set_contact_cfm2(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

#if BT_BULLET_VERSION < 285
  _pt.m_contactCFM2 = (btScalar)value;
#endif
}

/**
 *
 */
PN_stdfloat BulletManifoldPoint::
get_contact_cfm2() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

#if BT_BULLET_VERSION < 285
  return (PN_stdfloat)_pt.m_contactCFM2;
#else
  return 0;
#endif
}
