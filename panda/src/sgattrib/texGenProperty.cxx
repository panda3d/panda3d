// Filename: texGenProperty.cxx
// Created by:  drose (22Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "texGenProperty.h"

ostream &
operator << (ostream &out, TexGenProperty::Mode mode) {
  switch (mode) {
  case TexGenProperty::M_none:
    return out << "none";

  case TexGenProperty::M_eye_linear:
    return out << "eye_linear";

  case TexGenProperty::M_texture_projector:
    return out << "texture_projector";

  case TexGenProperty::M_sphere_map:
    return out << "sphere_map";
  }

  return out << "**invalid**(" << (int)mode << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenProperty::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void TexGenProperty::
output(ostream &out) const {
  out << _mode;
}
