// Filename: qpgeomVertexDataType.cxx
// Created by:  drose (06Mar05)
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

#include "qpgeomVertexDataType.h"

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexDataType::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomVertexDataType::
qpGeomVertexDataType(const InternalName *name, int num_components,
                     NumericType numeric_type, Contents contents,
                     int start) :
  _name(name),
  _num_components(num_components),
  _num_values(num_components),
  _numeric_type(numeric_type),
  _contents(contents),
  _start(start)
{
  nassertv(num_components > 0 && start >= 0);

  switch (numeric_type) {
  case NT_uint16:
    _component_bytes = 2;  // sizeof(PN_uint16)
    break;

  case NT_uint8:
    _component_bytes = 1;
    break;

  case NT_packed_8888:
    _component_bytes = 4;  // sizeof(PN_uint32)
    _num_values *= 4;
    break;

  case NT_float32:
    _component_bytes = 4;  // sizeof(PN_float32)
    break;
  }

  _total_bytes = _component_bytes * _num_components;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexDataType::error named constructor
//       Access: Published
//  Description: Returns a data type specifically to represent an
//               error condition.
////////////////////////////////////////////////////////////////////
const qpGeomVertexDataType &qpGeomVertexDataType::
error() {
  static qpGeomVertexDataType error_result
    (InternalName::get_error(), 1, NT_uint8, C_other, 0);
  return error_result;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexDataType::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexDataType::
output(ostream &out) const {
  out << *get_name() << "(" << get_num_components() << ")";
}
