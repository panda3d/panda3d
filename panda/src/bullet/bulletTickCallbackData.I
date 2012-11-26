// Filename: bulletTickCallbackData.I
// Created by:  enn0x (26Nov12)
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
//     Function: BulletTickCallbackData::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BulletTickCallbackData::
BulletTickCallbackData(btScalar timestep) :
  _timestep(timestep) {

}

////////////////////////////////////////////////////////////////////
//     Function: BulletTickCallbackData::get_timestep
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat BulletTickCallbackData::
get_timestep() const {

  return (PN_stdfloat)_timestep;
}

