// Filename: bulletWheel.I
// Created by:  enn0x (17Feb10)
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
//     Function: BulletWheel::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BulletWheel::
~BulletWheel() {

}

////////////////////////////////////////////////////////////////////
//     Function: BulletWheelRaycastInfo::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BulletWheelRaycastInfo::
~BulletWheelRaycastInfo() {

}

////////////////////////////////////////////////////////////////////
//     Function: BulletWheel::empty
//       Access: Public
//  Description: Named constructor intended to be used for asserts
//               with have to return a concrete value.
////////////////////////////////////////////////////////////////////
INLINE BulletWheel BulletWheel::
empty() {

  btWheelInfoConstructionInfo ci;
  btWheelInfo info(ci);

  return BulletWheel(info);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletWheelRaycastInfo::is_in_contact
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool BulletWheelRaycastInfo::
is_in_contact() const {

  return _info.m_isInContact;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletWheelRaycastInfo::get_suspension_length
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat BulletWheelRaycastInfo::
get_suspension_length() const {

  return _info.m_suspensionLength;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletWheelRaycastInfo::get_contact_point_ws
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE LPoint3 BulletWheelRaycastInfo::
get_contact_point_ws() const {

  return btVector3_to_LPoint3(_info.m_contactPointWS);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletWheelRaycastInfo::get_hard_point_ws
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE LPoint3 BulletWheelRaycastInfo::
get_hard_point_ws() const {

  return btVector3_to_LPoint3(_info.m_hardPointWS);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletWheelRaycastInfo::get_contact_normal_ws
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE LVector3 BulletWheelRaycastInfo::
get_contact_normal_ws() const {

  return btVector3_to_LVector3(_info.m_contactNormalWS);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletWheelRaycastInfo::get_wheel_direction_ws
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE LVector3 BulletWheelRaycastInfo::
get_wheel_direction_ws() const {

  return btVector3_to_LVector3(_info.m_wheelDirectionWS);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletWheelRaycastInfo::get_wheel_axle_ws
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE LVector3 BulletWheelRaycastInfo::
get_wheel_axle_ws() const {

  return btVector3_to_LVector3(_info.m_wheelAxleWS);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletWheelRaycastInfo::get_ground_object
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE PandaNode *BulletWheelRaycastInfo::
get_ground_object() const {

  return _info.m_groundObject ? (PandaNode *)_info.m_groundObject : NULL;
}

