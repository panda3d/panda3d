// Filename: qpgeomVertexAnimationSpec.cxx
// Created by:  drose (29Mar05)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "qpgeomVertexAnimationSpec.h"

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexAnimationSpec::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexAnimationSpec::
output(ostream &out) const {
  switch (_animation_type) {
  case AT_none:
    out << "none";
    break;

  case AT_panda:
    out << "panda";
    break;

  case AT_hardware:
    out << "hardware(" << _num_transforms << ", " 
        << _indexed_transforms << ")";
    break;
  }
}
