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

////////////////////////////////////////////////////////////////////
//     Function: BulletManifoldPoint::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
BulletManifoldPoint::
BulletManifoldPoint(btManifoldPoint &pt) 
 : _pt(pt) {

}

////////////////////////////////////////////////////////////////////
//     Function: BulletManifoldPoint::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
BulletManifoldPoint::
BulletManifoldPoint(const BulletManifoldPoint &other) 
 : _pt(other._pt) {

}

////////////////////////////////////////////////////////////////////
//     Function: BulletManifoldPoint::Copy Assignment
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
BulletManifoldPoint& BulletManifoldPoint::
operator=(const BulletManifoldPoint& other) {
 
  this->_pt = other._pt;
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletManifoldPoint::get_lift_time
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
int BulletManifoldPoint::
get_life_time() const {

  return _pt.getLifeTime();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletManifoldPoint::get_distance
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PN_stdfloat BulletManifoldPoint::
get_distance() const {

  return (PN_stdfloat)_pt.getDistance();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletManifoldPoint::get_applied_impulse
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PN_stdfloat BulletManifoldPoint::
get_applied_impulse() const {

  return (PN_stdfloat)_pt.getAppliedImpulse();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletManifoldPoint::get_position_world_on_a
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LPoint3 BulletManifoldPoint::
get_position_world_on_a() const {

  return btVector3_to_LPoint3(_pt.getPositionWorldOnA());
}

////////////////////////////////////////////////////////////////////
//     Function: BulletManifoldPoint::get_position_world_on_b
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LPoint3 BulletManifoldPoint::
get_position_world_on_b() const {

  return btVector3_to_LPoint3(_pt.getPositionWorldOnB());
}

////////////////////////////////////////////////////////////////////
//     Function: BulletManifoldPoint::get_normal_world_on_b
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LPoint3 BulletManifoldPoint::
get_normal_world_on_b() const {

  return btVector3_to_LPoint3(_pt.m_normalWorldOnB);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletManifoldPoint::get_local_point_a
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LPoint3 BulletManifoldPoint::
get_local_point_a() const {

  return btVector3_to_LPoint3(_pt.m_localPointA);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletManifoldPoint::get_local_point_b
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LPoint3 BulletManifoldPoint::
get_local_point_b() const {

  return btVector3_to_LPoint3(_pt.m_localPointB);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletManifoldPoint::get_part_id0
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
int BulletManifoldPoint::
get_part_id0() const {

  return _pt.m_partId0;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletManifoldPoint::get_part_id1
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
int BulletManifoldPoint::
get_part_id1() const {

  return _pt.m_partId1;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletManifoldPoint::get_index0
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
int BulletManifoldPoint::
get_index0() const {

  return _pt.m_index0;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletManifoldPoint::get_index1
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
int BulletManifoldPoint::
get_index1() const {

  return _pt.m_index1;
}

