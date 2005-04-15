// Filename: qpgeomEnums.cxx
// Created by:  drose (14Apr05)
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

#include "qpgeomEnums.h"


ostream &
operator << (ostream &out, qpGeomEnums::NumericType numeric_type) {
  switch (numeric_type) {
  case qpGeomEnums::NT_uint8:
    return out << "uint8";
    
  case qpGeomEnums::NT_uint16:
    return out << "uint16";
    
  case qpGeomEnums::NT_uint32:
    return out << "uint32";
    
  case qpGeomEnums::NT_packed_dcba:
    return out << "packed_dcba";
    
  case qpGeomEnums::NT_packed_dabc:
    return out << "packed_dabc";
    
  case qpGeomEnums::NT_float32:
    return out << "float32";
  }

  return out << "**invalid numeric type (" << (int)numeric_type << ")**";
}

ostream &
operator << (ostream &out, qpGeomEnums::Contents contents) {
  switch (contents) {
  case qpGeomEnums::C_other:
    return out << "other";

  case qpGeomEnums::C_point:
    return out << "point";

  case qpGeomEnums::C_clip_point:
    return out << "clip_point";

  case qpGeomEnums::C_vector:
    return out << "vector";

  case qpGeomEnums::C_texcoord:
    return out << "texcoord";

  case qpGeomEnums::C_color:
    return out << "color";

  case qpGeomEnums::C_index:
    return out << "index";

  case qpGeomEnums::C_morph_delta:
    return out << "morph_delta";
  }

  return out << "**invalid contents (" << (int)contents << ")**";
}
