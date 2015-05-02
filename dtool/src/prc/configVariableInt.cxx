// Filename: configVariableInt.cxx
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

#include "configVariableInt.h"
#include "string_utils.h"

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableInt::set_default_value
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
void ConfigVariableInt::
set_default_value(int default_value) {
  _core->set_default_value(format_string(default_value));
}
