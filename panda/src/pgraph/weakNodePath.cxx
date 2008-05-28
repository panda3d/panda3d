// Filename: weakNodePath.cxx
// Created by:  drose (29Sep04)
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

#include "weakNodePath.h"

////////////////////////////////////////////////////////////////////
//     Function: WeakNodePath::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void WeakNodePath::
output(ostream &out) const {
  if (was_deleted()) {
    out << "deleted";
  } else {
    get_node_path().output(out);
  }
}
