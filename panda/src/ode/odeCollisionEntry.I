// Filename: odeCollisionEntry.cxx
// Created by:  pro-rsoft (13Mar09)
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
//     Function: OdeCollisionEntry::Constructor
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE OdeCollisionEntry::
OdeCollisionEntry() {
}

////////////////////////////////////////////////////////////////////
//     Function: OdeCollisionEntry::get_geom1
//       Access: Published
//  Description: Returns the first geom in the collision.
////////////////////////////////////////////////////////////////////
INLINE const OdeGeom OdeCollisionEntry::
get_geom1() {
  return OdeGeom(_geom1);
}

////////////////////////////////////////////////////////////////////
//     Function: OdeCollisionEntry::get_geom2
//       Access: Published
//  Description: Returns the second geom in the collision.
////////////////////////////////////////////////////////////////////
INLINE const OdeGeom OdeCollisionEntry::
get_geom2() {
  return OdeGeom(_geom2);
}

////////////////////////////////////////////////////////////////////
//     Function: OdeCollisionEntry::get_body1
//       Access: Published
//  Description: Returns the first body in the collision.
////////////////////////////////////////////////////////////////////
INLINE const OdeBody OdeCollisionEntry::
get_body1() {
  return OdeBody(_body1);
}

////////////////////////////////////////////////////////////////////
//     Function: OdeCollisionEntry::get_body2
//       Access: Published
//  Description: Returns the second body in the collision.
////////////////////////////////////////////////////////////////////
INLINE const OdeBody OdeCollisionEntry::
get_body2() {
  return OdeBody(_body2);
}

////////////////////////////////////////////////////////////////////
//     Function: OdeCollisionEntry::get_num_contact_points
//       Access: Published
//  Description: Returns the number of contact points.
////////////////////////////////////////////////////////////////////
INLINE const size_t OdeCollisionEntry::
get_num_contact_points() {
  return _num_points;
}

////////////////////////////////////////////////////////////////////
//     Function: OdeCollisionEntry::get_contact_point
//       Access: Published
//  Description: Returns the nth contact point in the collision.
////////////////////////////////////////////////////////////////////
INLINE const LPoint3f OdeCollisionEntry::
get_contact_point(size_t n) {
  nassertr(n >= 0 && n < _num_points, LPoint3f::zero());
  return _points[n];
}

