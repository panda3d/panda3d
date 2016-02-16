// Filename: clientButtonDevice.cxx
// Created by:  drose (26Jan01)
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


#include "clientButtonDevice.h"

#include "indent.h"

TypeHandle ClientButtonDevice::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ClientButtonDevice::Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
ClientButtonDevice::
ClientButtonDevice(ClientBase *client, const string &device_name):
  ClientDevice(client, get_class_type(), device_name)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ClientButtonDevice::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void ClientButtonDevice::
output(ostream &out) const {
  out << get_type() << " " << get_name() << " (";
  output_buttons(out);
  out << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: ClientButtonDevice::write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void ClientButtonDevice::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << get_type() << " " << get_name() << ":\n";
  write_buttons(out, indent_level + 2);
}
