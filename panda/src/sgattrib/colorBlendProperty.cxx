// Filename: colorBlendProperty.cxx
// Created by:  drose (22Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "colorBlendProperty.h"

ostream &
operator << (ostream &out, ColorBlendProperty::Mode mode) {
  switch (mode) {
  case ColorBlendProperty::M_none:
    return out << "none";

  case ColorBlendProperty::M_multiply:
    return out << "multiply";

  case ColorBlendProperty::M_add:
    return out << "add";

  case ColorBlendProperty::M_multiply_add:
    return out << "multiply_add";

  case ColorBlendProperty::M_alpha:
    return out << "alpha";
  }

  return out << "**invalid**(" << (int)mode << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: ColorBlendProperty::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void ColorBlendProperty::
output(ostream &out) const {
  out << _mode;
}
