// Filename: linesmoothAttribute.cxx
// Created by:  drose (24Mar00)
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

#include "linesmoothAttribute.h"
#include "linesmoothTransition.h"

#include <graphicsStateGuardianBase.h>

TypeHandle LinesmoothAttribute::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LinesmoothAttribute::get_handle
//       Access: Public, Virtual
//  Description: Returns the handle of the associated transition.
////////////////////////////////////////////////////////////////////
TypeHandle LinesmoothAttribute::
get_handle() const {
  return LinesmoothTransition::get_class_type();
}

////////////////////////////////////////////////////////////////////
//     Function: LinesmoothAttribute::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated LinesmoothAttribute just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeAttribute *LinesmoothAttribute::
make_copy() const {
  return new LinesmoothAttribute(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: LinesmoothAttribute::make_initial
//       Access: Public, Virtual
//  Description: Returns a newly allocated LinesmoothAttribute
//               corresponding to the default initial state.
////////////////////////////////////////////////////////////////////
NodeAttribute *LinesmoothAttribute::
make_initial() const {
  return new LinesmoothAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: LinesmoothAttribute::issue
//       Access: Public, Virtual
//  Description: This is called on scene graph rendering attributes
//               when it is time to issue the particular attribute to
//               the graphics engine.  It should call the appropriate
//               method on GraphicsStateGuardianBase.
////////////////////////////////////////////////////////////////////
void LinesmoothAttribute::
issue(GraphicsStateGuardianBase *gsgbase) {
  gsgbase->issue_linesmooth(this);
}
