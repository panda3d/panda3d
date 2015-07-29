// Filename: configVariableColor.cxx
// Created by:  rdb (02Feb14)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "configVariableColor.h"
#include "pdtoa.h"

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableColor::set_default_value
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void ConfigVariableColor::
set_default_value(const LColor &default_value) {
  char buffer[128];
  char *p = buffer;
  pdtoa(default_value[0], p);

  p += strlen(p);
  *p++ = ' ';
  pdtoa(default_value[1], p);

  p += strlen(p);
  *p++ = ' ';
  pdtoa(default_value[2], p);

  p += strlen(p);
  *p++ = ' ';
  pdtoa(default_value[3], p);

  _core->set_default_value(buffer);
}
