// Filename: shaderInput.cxx
// Created by: jyelon (01Sep05)
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

#include "shaderInput.h"

TypeHandle ShaderInput::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::get_blank
//       Access: Public, Static
//  Description: Returns a static ShaderInput object with
//               name NULL, priority zero, type INVALID, and
//               all value-fields cleared.
////////////////////////////////////////////////////////////////////
const ShaderInput *ShaderInput::
get_blank() {
  static CPT(ShaderInput) blank;
  if (blank == 0) {
    blank = new ShaderInput(NULL, 0);
  }
  return blank;
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::register_with_read_factory
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
void ShaderInput::
register_with_read_factory() {
  // IMPLEMENT ME
}

