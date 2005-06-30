// Filename: geomEnums.cxx
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

#include "geomEnums.h"


ostream &
operator << (ostream &out, GeomEnums::NumericType numeric_type) {
  switch (numeric_type) {
  case GeomEnums::NT_uint8:
    return out << "uint8";
    
  case GeomEnums::NT_uint16:
    return out << "uint16";
    
  case GeomEnums::NT_uint32:
    return out << "uint32";
    
  case GeomEnums::NT_packed_dcba:
    return out << "packed_dcba";
    
  case GeomEnums::NT_packed_dabc:
    return out << "packed_dabc";
    
  case GeomEnums::NT_float32:
    return out << "float32";
  }

  return out << "**invalid numeric type (" << (int)numeric_type << ")**";
}

ostream &
operator << (ostream &out, GeomEnums::Contents contents) {
  switch (contents) {
  case GeomEnums::C_other:
    return out << "other";

  case GeomEnums::C_point:
    return out << "point";

  case GeomEnums::C_clip_point:
    return out << "clip_point";

  case GeomEnums::C_vector:
    return out << "vector";

  case GeomEnums::C_texcoord:
    return out << "texcoord";

  case GeomEnums::C_color:
    return out << "color";

  case GeomEnums::C_index:
    return out << "index";

  case GeomEnums::C_morph_delta:
    return out << "morph_delta";
  }

  return out << "**invalid contents (" << (int)contents << ")**";
}
