// Filename: doubleDataTransition.cxx
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

#if defined(WIN32_VC) && !defined(NO_PCH)
#include "dgraph_headers.h"
#endif

#pragma hdrstop

#if !defined(WIN32_VC) || defined(NO_PCH)
#include "doubleDataTransition.h"
#include "doubleDataAttribute.h"
#endif

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma implementation
#endif

TypeHandle DoubleDataTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: DoubleDataTransition::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
NodeTransition *DoubleDataTransition::
make_copy() const {
  return new DoubleDataTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: DoubleDataTransition::make_attrib
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
NodeAttribute *DoubleDataTransition::
make_attrib() const {
  return new DoubleDataAttribute;
}
