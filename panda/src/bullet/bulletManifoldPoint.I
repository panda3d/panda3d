/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletManifoldPoint.I
 * @author enn0x
 * @date 2010-03-07
 */

/**
 *
 */
INLINE BulletManifoldPoint::
~BulletManifoldPoint() {

}

/**
 *
 */
INLINE void BulletManifoldPoint::
set_lateral_friction_initialized(bool value) {
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
INLINE bool BulletManifoldPoint::
get_lateral_friction_initialized() const {
#if BT_BULLET_VERSION >= 285
  return (_pt.m_contactPointFlags & BT_CONTACT_FLAG_LATERAL_FRICTION_INITIALIZED) != 0;
#else
  return _pt.m_lateralFrictionInitialized;
#endif
}

/**
 *
 */
INLINE void BulletManifoldPoint::
set_lateral_friction_dir1(const LVecBase3 &dir) {

  _pt.m_lateralFrictionDir1 = LVecBase3_to_btVector3(dir);
}

/**
 *
 */
INLINE LVector3 BulletManifoldPoint::
get_lateral_friction_dir1() const {

  return btVector3_to_LVector3(_pt.m_lateralFrictionDir1);
}

/**
 *
 */
INLINE void BulletManifoldPoint::
set_lateral_friction_dir2(const LVecBase3 &dir) {

  _pt.m_lateralFrictionDir2 = LVecBase3_to_btVector3(dir);
}

/**
 *
 */
INLINE LVector3 BulletManifoldPoint::
get_lateral_friction_dir2() const {

  return btVector3_to_LVector3(_pt.m_lateralFrictionDir2);
}

/**
 *
 */
INLINE void BulletManifoldPoint::
set_contact_motion1(PN_stdfloat value) {

  _pt.m_contactMotion1 = (btScalar)value;
}

/**
 *
 */
INLINE PN_stdfloat BulletManifoldPoint::
get_contact_motion1() const {

  return (PN_stdfloat)_pt.m_contactMotion1;
}

/**
 *
 */
INLINE void BulletManifoldPoint::
set_contact_motion2(PN_stdfloat value) {

  _pt.m_contactMotion2 = (btScalar)value;
}

/**
 *
 */
INLINE PN_stdfloat BulletManifoldPoint::
get_contact_motion2() const {

  return (PN_stdfloat)_pt.m_contactMotion2;
}

/**
 *
 */
INLINE void BulletManifoldPoint::
set_combined_friction(PN_stdfloat value) {

  _pt.m_combinedFriction = (btScalar)value;
}

/**
 *
 */
INLINE PN_stdfloat BulletManifoldPoint::
get_combined_friction() const {

  return (PN_stdfloat)_pt.m_combinedFriction;
}

/**
 *
 */
INLINE void BulletManifoldPoint::
set_combined_restitution(PN_stdfloat value) {

  _pt.m_combinedRestitution = (btScalar)value;
}

/**
 *
 */
INLINE PN_stdfloat BulletManifoldPoint::
get_combined_restitution() const {

  return (PN_stdfloat)_pt.m_combinedRestitution;
}

/**
 *
 */
INLINE void BulletManifoldPoint::
set_applied_impulse(PN_stdfloat value) {

  _pt.m_appliedImpulse = (btScalar)value;
}

/**
 *
 */
INLINE void BulletManifoldPoint::
set_applied_impulse_lateral1(PN_stdfloat value) {

  _pt.m_appliedImpulseLateral1 = (btScalar)value;
}

/**
 *
 */
INLINE PN_stdfloat BulletManifoldPoint::
get_applied_impulse_lateral1() const {

  return (PN_stdfloat)_pt.m_appliedImpulseLateral1;
}

/**
 *
 */
INLINE void BulletManifoldPoint::
set_applied_impulse_lateral2(PN_stdfloat value) {

  _pt.m_appliedImpulseLateral2 = (btScalar)value;
}

/**
 *
 */
INLINE PN_stdfloat BulletManifoldPoint::
get_applied_impulse_lateral2() const {

  return (PN_stdfloat)_pt.m_appliedImpulseLateral2;
}

/**
 *
 */
INLINE void BulletManifoldPoint::
set_contact_cfm1(PN_stdfloat value) {
#if BT_BULLET_VERSION < 285
  _pt.m_contactCFM1 = (btScalar)value;
#endif
}

/**
 *
 */
INLINE PN_stdfloat BulletManifoldPoint::
get_contact_cfm1() const {
#if BT_BULLET_VERSION < 285
  return (PN_stdfloat)_pt.m_contactCFM1;
#else
  return 0;
#endif
}

/**
 *
 */
INLINE void BulletManifoldPoint::
set_contact_cfm2(PN_stdfloat value) {
#if BT_BULLET_VERSION < 285
  _pt.m_contactCFM2 = (btScalar)value;
#endif
}

/**
 *
 */
INLINE PN_stdfloat BulletManifoldPoint::
get_contact_cfm2() const {
#if BT_BULLET_VERSION < 285
  return (PN_stdfloat)_pt.m_contactCFM2;
#else
  return 0;
#endif
}
