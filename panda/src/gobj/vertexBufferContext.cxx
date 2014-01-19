// Filename: vertexBufferContext.cxx
// Created by:  drose (17Mar05)
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

#include "vertexBufferContext.h"
#include "config_gobj.h"

TypeHandle VertexBufferContext::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: VertexBufferContext::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void VertexBufferContext::
output(ostream &out) const {
  out << *get_data() << ", " << get_data_size_bytes();
}

////////////////////////////////////////////////////////////////////
//     Function: VertexBufferContext::write
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void VertexBufferContext::
write(ostream &out, int indent_level) const {
  SavedContext::write(out, indent_level);
}
