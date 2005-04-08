// Filename: qpgeomVertexColumn.cxx
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

#include "qpgeomVertexColumn.h"

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexColumn::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomVertexColumn::
qpGeomVertexColumn(const InternalName *name, int num_components,
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

  case NT_packed_dcba:
  case NT_packed_dabc:
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
//     Function: qpGeomVertexColumn::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexColumn::
output(ostream &out) const {
  out << *get_name() << "(" << get_num_components() << ")";
}

ostream &
operator << (ostream &out, qpGeomVertexColumn::NumericType numeric_type) {
  switch (numeric_type) {
  case qpGeomVertexColumn::NT_uint8:
    return out << "uint8";
    
  case qpGeomVertexColumn::NT_uint16:
    return out << "uint16";
    
  case qpGeomVertexColumn::NT_packed_dcba:
    return out << "packed_dcba";
    
  case qpGeomVertexColumn::NT_packed_dabc:
    return out << "packed_dabc";
    
  case qpGeomVertexColumn::NT_float32:
    return out << "float32";
  }

  return out << "**invalid numeric type (" << (int)numeric_type << ")**";
}

ostream &
operator << (ostream &out, qpGeomVertexColumn::Contents contents) {
  switch (contents) {
  case qpGeomVertexColumn::C_other:
    return out << "other";

  case qpGeomVertexColumn::C_point:
    return out << "point";

  case qpGeomVertexColumn::C_clip_point:
    return out << "clip_point";

  case qpGeomVertexColumn::C_vector:
    return out << "vector";

  case qpGeomVertexColumn::C_texcoord:
    return out << "texcoord";

  case qpGeomVertexColumn::C_color:
    return out << "color";

  case qpGeomVertexColumn::C_index:
    return out << "index";

  case qpGeomVertexColumn::C_morph_delta:
    return out << "morph_delta";
  }

  return out << "**invalid contents (" << (int)contents << ")**";
}
