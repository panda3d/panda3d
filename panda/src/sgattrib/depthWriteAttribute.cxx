// Filename: depthWriteAttribute.cxx
// Created by:  drose (31Mar00)
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

#include "depthWriteAttribute.h"
#include "depthWriteTransition.h"

#include <graphicsStateGuardianBase.h>

TypeHandle DepthWriteAttribute::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: DepthWriteAttribute::get_handle
//       Access: Public, Virtual
//  Description: Returns the handle of the associated transition.
////////////////////////////////////////////////////////////////////
TypeHandle DepthWriteAttribute::
get_handle() const {
  return DepthWriteTransition::get_class_type();
}

////////////////////////////////////////////////////////////////////
//     Function: DepthWriteAttribute::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated DepthWriteAttribute just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeAttribute *DepthWriteAttribute::
make_copy() const {
  return new DepthWriteAttribute(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: DepthWriteAttribute::make_initial
//       Access: Public, Virtual
//  Description: Returns a newly allocated DepthWriteAttribute
//               corresponding to the default initial state.
////////////////////////////////////////////////////////////////////
NodeAttribute *DepthWriteAttribute::
make_initial() const {
  return new DepthWriteAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: DepthWriteAttribute::issue
//       Access: Public, Virtual
//  Description: This is called on scene graph rendering attributes
//               when it is time to issue the particular attribute to
//               the graphics engine.  It should call the appropriate
//               method on GraphicsStateGuardianBase.
////////////////////////////////////////////////////////////////////
void DepthWriteAttribute::
issue(GraphicsStateGuardianBase *gsgbase) {
  gsgbase->issue_depth_write(this);
}
