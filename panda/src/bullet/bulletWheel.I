/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletWheel.I
 * @author enn0x
 * @date 2010-02-17
 */

/**
 *
 */
INLINE BulletWheel::
~BulletWheel() {

}

/**
 *
 */
INLINE BulletWheelRaycastInfo::
~BulletWheelRaycastInfo() {

}

/**
 * Named constructor intended to be used for asserts with have to return a
 * concrete value.
 */
INLINE BulletWheel BulletWheel::
empty() {

  btWheelInfoConstructionInfo ci;
  btWheelInfo info(ci);

  return BulletWheel(info);
}

/**
 *
 */
INLINE bool BulletWheelRaycastInfo::
is_in_contact() const {

  return _info.m_isInContact;
}

/**
 *
 */
INLINE PN_stdfloat BulletWheelRaycastInfo::
get_suspension_length() const {

  return _info.m_suspensionLength;
}

/**
 *
 */
INLINE LPoint3 BulletWheelRaycastInfo::
get_contact_point_ws() const {

  return btVector3_to_LPoint3(_info.m_contactPointWS);
}

/**
 *
 */
INLINE LPoint3 BulletWheelRaycastInfo::
get_hard_point_ws() const {

  return btVector3_to_LPoint3(_info.m_hardPointWS);
}

/**
 *
 */
INLINE LVector3 BulletWheelRaycastInfo::
get_contact_normal_ws() const {

  return btVector3_to_LVector3(_info.m_contactNormalWS);
}

/**
 *
 */
INLINE LVector3 BulletWheelRaycastInfo::
get_wheel_direction_ws() const {

  return btVector3_to_LVector3(_info.m_wheelDirectionWS);
}

/**
 *
 */
INLINE LVector3 BulletWheelRaycastInfo::
get_wheel_axle_ws() const {

  return btVector3_to_LVector3(_info.m_wheelAxleWS);
}

/**
 *
 */
INLINE PandaNode *BulletWheelRaycastInfo::
get_ground_object() const {

  return _info.m_groundObject ? (PandaNode *)_info.m_groundObject : NULL;
}
