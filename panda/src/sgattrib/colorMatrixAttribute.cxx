// Filename: colorMatrixAttribute.cxx
// Created by:  jason (01Aug00)
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

#include "colorMatrixAttribute.h"
#include "colorMatrixTransition.h"

#include <graphicsStateGuardianBase.h>
#include <indent.h>

TypeHandle ColorMatrixAttribute::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ColorMatrixAttribute::get_handle
//       Access: Public, Virtual
//  Description: Returns the handle of the associated transition.
////////////////////////////////////////////////////////////////////
TypeHandle ColorMatrixAttribute::
get_handle() const {
  return ColorMatrixTransition::get_class_type();
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMatrixAttribute::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated ColorMatrixAttribute just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeAttribute *ColorMatrixAttribute::
make_copy() const {
  return new ColorMatrixAttribute(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMatrixAttribute::make_initial
//       Access: Public, Virtual
//  Description: Returns a newly allocated ColorMatrixAttribute
//               corresponding to the default initial state.
////////////////////////////////////////////////////////////////////
NodeAttribute *ColorMatrixAttribute::
make_initial() const {
  return new ColorMatrixAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMatrixAttribute::issue
//       Access: Public, Virtual
//  Description: This is called on scene graph rendering attributes
//               when it is time to issue the particular attribute to
//               the graphics engine.  It should call the appropriate
//               method on GraphicsStateGuardianBase.
////////////////////////////////////////////////////////////////////
void ColorMatrixAttribute::
issue(GraphicsStateGuardianBase *gsgbase) {
  gsgbase->issue_color_transform(this);
}
