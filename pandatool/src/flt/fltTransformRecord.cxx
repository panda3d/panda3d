// Filename: fltTransformRecord.cxx
// Created by:  drose (24Aug00)
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

#include "fltTransformRecord.h"

TypeHandle FltTransformRecord::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: FltTransformRecord::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
FltTransformRecord::
FltTransformRecord(FltHeader *header) : FltRecord(header) {
  _matrix = LMatrix4d::ident_mat();
}

////////////////////////////////////////////////////////////////////
//     Function: FltTransformRecord::get_matrix
//       Access: Public
//  Description: Returns the transform matrix represented by this
//               particular component of the transform.
////////////////////////////////////////////////////////////////////
const LMatrix4d &FltTransformRecord::
get_matrix() const {
  return _matrix;
}
