/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file clientButtonDevice.cxx
 * @author drose
 * @date 2001-01-26
 */

#include "clientButtonDevice.h"

#include "indent.h"

using std::ostream;

TypeHandle ClientButtonDevice::_type_handle;

/**
 *
 */
ClientButtonDevice::
ClientButtonDevice(ClientBase *client, const std::string &device_name):
  ClientDevice(client, get_class_type(), device_name)
{
}

/**
 *
 */
void ClientButtonDevice::
output(ostream &out) const {
  out << get_type() << " " << get_name() << " (";
  output_buttons(out);
  out << ")";
}

/**
 *
 */
void ClientButtonDevice::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << get_type() << " " << get_name() << ":\n";
  write_buttons(out, indent_level + 2);
}
