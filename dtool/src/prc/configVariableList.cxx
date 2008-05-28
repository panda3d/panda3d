// Filename: configVariableList.cxx
// Created by:  drose (20Oct04)
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

#include "configVariableList.h"

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableList::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void ConfigVariableList::
output(ostream &out) const {
  out << get_num_values() << " values.";
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableList::write
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void ConfigVariableList::
write(ostream &out) const {
  int num_values = get_num_values();
  for (int i = 0; i < num_values; ++i) {
    out << get_string_value(i) << "\n";
  }
}
