// Filename: doublePtrDataTransition.cxx
// Created by:  jason (07Aug00)
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


#include "doublePtrDataTransition.h"
#include "doublePtrDataAttribute.h"

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma implementation
#endif

TypeHandle DoublePtrDataTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: DoublePtrDataTransition::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
NodeTransition *DoublePtrDataTransition::
make_copy() const {
  return new DoublePtrDataTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: DoublePtrDataTransition::make_attrib
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
NodeAttribute *DoublePtrDataTransition::
make_attrib() const {
  return new DoublePtrDataAttribute;
}
