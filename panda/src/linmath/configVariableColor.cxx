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

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableColor::set_default_value
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void ConfigVariableColor::
set_default_value(const LColor &default_value) {
  ostringstream strm;
  strm << default_value;

  _core->set_default_value(strm.str());
}
