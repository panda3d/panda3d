// Filename: bulletManifoldPoint.cxx
// Created by:  enn0x (07Mar10)
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

#include "bulletManifoldPoint.h"

////////////////////////////////////////////////////////////////////
//     Function: BulletManifoldPoint::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
BulletManifoldPoint::
BulletManifoldPoint(btManifoldPoint &pt) : _pt(pt) {

}

////////////////////////////////////////////////////////////////////
//     Function: BulletManifoldPoint::get_lift_time
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
int BulletManifoldPoint::
get_lift_time() const {

  return _pt.getLifeTime();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletManifoldPoint::get_distance
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
float BulletManifoldPoint::
get_distance() const {

  return _pt.getDistance();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletManifoldPoint::get_applied_impulse
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
float BulletManifoldPoint::
get_applied_impulse() const {

  return _pt.getAppliedImpulse();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletManifoldPoint::get_position_world_on_a
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LPoint3f BulletManifoldPoint::
get_position_world_on_a() const {

  return btVector3_to_LPoint3f(_pt.getPositionWorldOnA());
}

////////////////////////////////////////////////////////////////////
//     Function: BulletManifoldPoint::get_position_world_on_b
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LPoint3f BulletManifoldPoint::
get_position_world_on_b() const {

  return btVector3_to_LPoint3f(_pt.getPositionWorldOnB());
}

////////////////////////////////////////////////////////////////////
//     Function: BulletManifoldPoint::get_local_point_a
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LPoint3f BulletManifoldPoint::
get_local_point_a() const {

  return btVector3_to_LPoint3f(_pt.m_localPointA);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletManifoldPoint::get_local_point_b
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LPoint3f BulletManifoldPoint::
get_local_point_b() const {

  return btVector3_to_LPoint3f(_pt.m_localPointB);
}

