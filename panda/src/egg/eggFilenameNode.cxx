// Filename: eggFilenameNode.cxx
// Created by:  drose (11Feb99)
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

#include "eggFilenameNode.h"

TypeHandle EggFilenameNode::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: EggFilenameNode::get_default_extension
//       Access: Public, Virtual
//  Description: Returns the default extension for this filename type.
////////////////////////////////////////////////////////////////////
string EggFilenameNode::
get_default_extension() const {
  return string();
}
