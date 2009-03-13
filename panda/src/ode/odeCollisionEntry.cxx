// Filename: odeCollisionEntry.cxx
// Created by:  pro-rsoft (05Mar09)
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

#include "odeCollisionEntry.h"

TypeHandle OdeCollisionEntry::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: OdeCollisionEntry::Constructor
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
OdeCollisionEntry::
OdeCollisionEntry() {
}

////////////////////////////////////////////////////////////////////
//     Function: OdeCollisionEntry::get_geom1
//       Access: Published
//  Description: Returns the first geom in the collision.
////////////////////////////////////////////////////////////////////
const OdeGeom OdeCollisionEntry::
get_geom1() {
  return OdeGeom(_geom1);
}

////////////////////////////////////////////////////////////////////
//     Function: OdeCollisionEntry::get_geom2
//       Access: Published
//  Description: Returns the second geom in the collision.
////////////////////////////////////////////////////////////////////
const OdeGeom OdeCollisionEntry::
get_geom2() {
  return OdeGeom(_geom2);
}

////////////////////////////////////////////////////////////////////
//     Function: OdeCollisionEntry::get_body1
//       Access: Published
//  Description: Returns the first body in the collision.
////////////////////////////////////////////////////////////////////
const OdeBody OdeCollisionEntry::
get_body1() {
  return OdeBody(_body1);
}

////////////////////////////////////////////////////////////////////
//     Function: OdeCollisionEntry::get_body2
//       Access: Published
//  Description: Returns the second body in the collision.
////////////////////////////////////////////////////////////////////
const OdeBody OdeCollisionEntry::
get_body2() {
  return OdeBody(_body2);
}

