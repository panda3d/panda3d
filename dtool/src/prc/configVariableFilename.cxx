// Filename: configVariableFilename.cxx
// Created by:  drose (22Nov04)
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

#include "configVariableFilename.h"

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableFilename::reload_value
//       Access: Private
//  Description: Recopies the config variable into the Filename for
//               returning its value.
////////////////////////////////////////////////////////////////////
void ConfigVariableFilename::
reload_value() {
  _value = Filename::expand_from(get_string_value());

  _value_seq = _core->get_value_seq();
  _value_stale = false;
}
