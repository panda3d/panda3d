// Filename: shaderAttrib.I
// Created by:  sshodhan (10Jul04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: ShaderAttrib::Constructor
//       Access: Private
//  Description: xyz
////////////////////////////////////////////////////////////////////
INLINE ShaderAttrib::
ShaderAttrib() {
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderAttrib::is_off
//       Access: Published
//  Description: Returns true if the ShaderAttrib is an 'off'
//               ShaderAttrib, indicating that it should disable
//               the previous shader profiles
////////////////////////////////////////////////////////////////////
INLINE bool ShaderAttrib::
is_off() const {
  return _shader_mode == (ShaderMode *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderAttrib::get_shader_mode
//       Access: Published
//  Description: If the ShaderAttrib is not an 'off' ShaderAttrib,
//               returns the ShaderMode that is associated.  Otherwise,
//               return NULL.
////////////////////////////////////////////////////////////////////
INLINE ShaderMode *ShaderAttrib::
get_shader_mode() const {
  return _shader_mode;
}

