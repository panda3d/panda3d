// Filename: configVariableDouble.cxx
// Created by:  drose (20Oct04)
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

#include "configVariableDouble.h"
#include "string_utils.h"

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableDouble::set_default_value
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
void ConfigVariableDouble::
set_default_value(double default_value) {
  _core->set_default_value(format_string(default_value));
}
