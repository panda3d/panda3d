// Filename: clientAnalogDevice.cxx
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


#include "clientAnalogDevice.h"

#include "indent.h"

TypeHandle ClientAnalogDevice::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ClientAnalogDevice::write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void ClientAnalogDevice::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << get_type() << " " << get_name() << ":\n";
  write_controls(out, indent_level + 2);
}
