// Filename: colorMaskProperty.cxx
// Created by:  drose (23Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "colorMaskProperty.h"

#include <stdio.h>

////////////////////////////////////////////////////////////////////
//     Function: ColorMaskProperty::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void ColorMaskProperty::
output(ostream &out) const {
  char hex[20];
  sprintf(hex, "%02x", _mask);
  nassertv(strlen(hex) < 20);

  out << "0x" << hex << "(";
  if ((_mask & M_r) != 0) {
    out << "r";
  }
  if ((_mask & M_g) != 0) {
    out << "g";
  }
  if ((_mask & M_b) != 0) {
    out << "b";
  }
  if ((_mask & M_a) != 0) {
    out << "a";
  }
  out << ")";
}
