// Filename: texMatrixAttribute.cxx
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

#include "texMatrixAttribute.h"
#include "texMatrixTransition.h"

#include <graphicsStateGuardianBase.h>
#include <indent.h>

TypeHandle TexMatrixAttribute::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixAttribute::get_handle
//       Access: Public, Virtual
//  Description: Returns the handle of the associated transition.
////////////////////////////////////////////////////////////////////
TypeHandle TexMatrixAttribute::
get_handle() const {
  return TexMatrixTransition::get_class_type();
}

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixAttribute::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated TexMatrixAttribute just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeAttribute *TexMatrixAttribute::
make_copy() const {
  return new TexMatrixAttribute(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixAttribute::make_initial
//       Access: Public, Virtual
//  Description: Returns a newly allocated TexMatrixAttribute
//               corresponding to the default initial state.
////////////////////////////////////////////////////////////////////
NodeAttribute *TexMatrixAttribute::
make_initial() const {
  return new TexMatrixAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixAttribute::issue
//       Access: Public, Virtual
//  Description: This is called on scene graph rendering attributes
//               when it is time to issue the particular attribute to
//               the graphics engine.  It should call the appropriate
//               method on GraphicsStateGuardianBase.
////////////////////////////////////////////////////////////////////
void TexMatrixAttribute::
issue(GraphicsStateGuardianBase *gsgbase) {
  gsgbase->issue_tex_matrix(this);
}
