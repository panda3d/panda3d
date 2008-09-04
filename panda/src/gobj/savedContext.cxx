// Filename: savedContext.cxx
// Created by:  drose (11Jun01)
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

#include "savedContext.h"
#include "indent.h"

TypeHandle SavedContext::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: SavedContext::output
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void SavedContext::
output(ostream &out) const {
  out << "SavedContext " << this;
}

////////////////////////////////////////////////////////////////////
//     Function: SavedContext::write
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void SavedContext::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << *this << "\n";
}
