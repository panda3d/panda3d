// Filename: texMatrixTransition.cxx
// Created by:  mike (19Jan99)
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

#include "texMatrixTransition.h"
#include "texMatrixAttribute.h"

#include <indent.h>

TypeHandle TexMatrixTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixTransition::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated TexMatrixTransition just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeTransition *TexMatrixTransition::
make_copy() const {
  return new TexMatrixTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixTransition::make_attrib
//       Access: Public, Virtual
//  Description: Returns a newly allocated TexMatrixAttribute.
////////////////////////////////////////////////////////////////////
NodeAttribute *TexMatrixTransition::
make_attrib() const {
  return new TexMatrixAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixTransition::make_with_matrix
//       Access: Protected, Virtual
//  Description: Returns a new transition with the indicated matrix.
////////////////////////////////////////////////////////////////////
MatrixTransition<LMatrix4f> *TexMatrixTransition::
make_with_matrix(const LMatrix4f &matrix) const {
  return new TexMatrixTransition(matrix);
}
