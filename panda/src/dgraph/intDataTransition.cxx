// Filename: intDataTransition.cxx
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


#include "intDataTransition.h"
#include "intDataAttribute.h"

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma implementation
#endif

TypeHandle IntDataTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: IntDataTransition::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
NodeTransition *IntDataTransition::
make_copy() const {
  return new IntDataTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: IntDataTransition::make_attrib
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
NodeAttribute *IntDataTransition::
make_attrib() const {
  return new IntDataAttribute;
}
