// Filename: configVariableDouble.cxx
// Created by:  drose (20Oct04)
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

#include "configVariableDouble.h"

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableDouble::set_default_value
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
void ConfigVariableDouble::
set_default_value(double default_value) {
  ostringstream strm;
  strm << default_value;

  _core->set_default_value(strm.str());
}
