// Filename: colladaInput.I
// Created by:  rdb (23May11)
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


////////////////////////////////////////////////////////////////////
//     Function: ColladaInput::is_vertex_source
//  Description: Returns true if this has a <vertices> element as
//               source.
////////////////////////////////////////////////////////////////////
bool ColladaInput::
is_vertex_source() const {
  return (_semantic == "VERTEX");
}

////////////////////////////////////////////////////////////////////
//     Function: ColladaInput::get_offset
//  Description: Returns the offset associated with this input.
////////////////////////////////////////////////////////////////////
unsigned int ColladaInput::
get_offset() const {
  return _offset;
}
