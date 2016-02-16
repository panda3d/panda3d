/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file configVariableInt.cxx
 * @author drose
 * @date 2004-10-20
 */

#include "configVariableInt.h"
#include "string_utils.h"

/**

 */
void ConfigVariableInt::
set_default_value(int default_value) {
  _core->set_default_value(format_string(default_value));
}
