// Filename: transformAttribute.cxx
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

#include "transformAttribute.h"
#include "transformTransition.h"

#include <graphicsStateGuardianBase.h>
#include <indent.h>

TypeHandle TransformAttribute::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: TransformAttribute::get_handle
//       Access: Public, Virtual
//  Description: Returns the handle of the associated transition.
////////////////////////////////////////////////////////////////////
TypeHandle TransformAttribute::
get_handle() const {
  return TransformTransition::get_class_type();
}

////////////////////////////////////////////////////////////////////
//     Function: TransformAttribute::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated TransformAttribute just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeAttribute *TransformAttribute::
make_copy() const {
  return new TransformAttribute(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformAttribute::make_initial
//       Access: Public, Virtual
//  Description: Returns a newly allocated TransformAttribute
//               corresponding to the default initial state.
////////////////////////////////////////////////////////////////////
NodeAttribute *TransformAttribute::
make_initial() const {
  return new TransformAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformAttribute::issue
//       Access: Public, Virtual
//  Description: This is called on scene graph rendering attributes
//               when it is time to issue the particular attribute to
//               the graphics engine.  It should call the appropriate
//               method on GraphicsStateGuardianBase.
////////////////////////////////////////////////////////////////////
void TransformAttribute::
issue(GraphicsStateGuardianBase *gsgbase) {
  gsgbase->issue_transform(this);
}
