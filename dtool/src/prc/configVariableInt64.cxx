// Filename: configVariableInt64.cxx
// Created by:  drose (19Dec07)
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

#include "configVariableInt64.h"
#include "string_utils.h"

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableInt64::set_default_value
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
void ConfigVariableInt64::
set_default_value(PN_int64 default_value) {
  _core->set_default_value(format_string(default_value));
}
