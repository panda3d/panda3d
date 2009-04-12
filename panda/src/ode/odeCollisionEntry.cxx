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
//     Function: OdeCollisionEntry::Destructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
OdeCollisionEntry::
~OdeCollisionEntry() {
  delete[] _contact_geoms;
}

