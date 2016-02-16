/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file configVariableInt64.cxx
 * @author drose
 * @date 2007-12-19
 */

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
