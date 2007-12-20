// Filename: configVariableInt64.cxx
// Created by:  drose (19Dec07)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// aPN_int64 with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "configVariableInt64.h"

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableInt64::set_default_value
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
void ConfigVariableInt64::
set_default_value(PN_int64 default_value) {
  ostringstream strm;
  strm << default_value;

  _core->set_default_value(strm.str());
}
