// Filename: decalAttribute.cxx
// Created by:  drose (17Apr00)
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

#include "decalAttribute.h"
#include "decalTransition.h"

#include <graphicsStateGuardianBase.h>

TypeHandle DecalAttribute::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: DecalAttribute::get_handle
//       Access: Public, Virtual
//  Description: Returns the handle of the associated transition.
////////////////////////////////////////////////////////////////////
TypeHandle DecalAttribute::
get_handle() const {
  return DecalTransition::get_class_type();
}

////////////////////////////////////////////////////////////////////
//     Function: DecalAttribute::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated DecalAttribute just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeAttribute *DecalAttribute::
make_copy() const {
  return new DecalAttribute(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: DecalAttribute::make_initial
//       Access: Public, Virtual
//  Description: Returns a newly allocated DecalAttribute
//               corresponding to the default initial state.
////////////////////////////////////////////////////////////////////
NodeAttribute *DecalAttribute::
make_initial() const {
  return new DecalAttribute;
}
