// Filename: vec4DataTransition.cxx
// Created by:  jason (03Aug00)
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
#include "vec4DataTransition.h"
#include "vec4DataAttribute.h"
#endif

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma implementation
#endif

TypeHandle Vec4DataTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: Vec4DataTransition::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
NodeTransition *Vec4DataTransition::
make_copy() const {
  return new Vec4DataTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: Vec4DataTransition::make_attrib
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
NodeAttribute *Vec4DataTransition::
make_attrib() const {
  return new Vec4DataAttribute;
}
