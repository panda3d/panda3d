// Filename: doublePtrDataAttribute.cxx
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

#include "doublePtrDataAttribute.h"
#include "doublePtrDataTransition.h"

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma implementation
#endif

TypeHandle DoublePtrDataAttribute::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: DoublePtrDataAttribute::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
NodeAttribute *DoublePtrDataAttribute::
make_copy() const {
  return new DoublePtrDataAttribute(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: DoublePtrDataAttribute::make_initial
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
NodeAttribute *DoublePtrDataAttribute::
make_initial() const {
  return new DoublePtrDataAttribute;
}
