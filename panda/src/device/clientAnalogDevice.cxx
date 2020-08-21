/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file clientAnalogDevice.cxx
 * @author drose
 * @date 2001-01-26
 */

#include "clientAnalogDevice.h"

#include "indent.h"

TypeHandle ClientAnalogDevice::_type_handle;

/**
 *
 */
void ClientAnalogDevice::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level) << get_type() << " " << get_name() << ":\n";
  write_axes(out, indent_level + 2);
}
