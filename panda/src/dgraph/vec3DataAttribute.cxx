// Filename: vec3DataAttribute.cxx
// Created by:  drose (27Mar00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "vec3DataAttribute.h"
#include "vec3DataTransition.h"

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma implementation
#endif

TypeHandle Vec3DataAttribute::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: Vec3DataAttribute::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
NodeAttribute *Vec3DataAttribute::
make_copy() const {
  return new Vec3DataAttribute(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: Vec3DataAttribute::make_initial
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
NodeAttribute *Vec3DataAttribute::
make_initial() const {
  return new Vec3DataAttribute;
}
