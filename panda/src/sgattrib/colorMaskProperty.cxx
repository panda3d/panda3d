// Filename: colorMaskProperty.cxx
// Created by:  drose (23Mar00)
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
