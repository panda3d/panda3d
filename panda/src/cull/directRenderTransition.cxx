// Filename: directRenderTransition.cxx
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

#include "directRenderTransition.h"

#include <indent.h>

TypeHandle DirectRenderTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: DirectRenderTransition::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated DirectRenderTransition just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeTransition *DirectRenderTransition::
make_copy() const {
  return new DirectRenderTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: DirectRenderTransition::has_sub_render
//       Access: Public, Virtual
//  Description: DirectRenderTransition doesn't actually have a
//               sub_render() function, but it might as well, because
//               it's treated as a special case.  We set this function
//               to return true so GraphReducer will behave correctly.
////////////////////////////////////////////////////////////////////
bool DirectRenderTransition::
has_sub_render() const {
  return true;
}
