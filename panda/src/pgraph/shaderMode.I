// Filename: shaderMode.I
// Heavily Modified:  jyelon (01Sep05)
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
//  Function: ShaderModeArg::Constructor
//  Access: Public
//  Description: initialize a shader arg storage structure.
////////////////////////////////////////////////////////////////////
INLINE ShaderModeArg::
ShaderModeArg(void) {
  _type = SAT_INVALID;
}
  
////////////////////////////////////////////////////////////////////
//  Function: ShaderModeArg::Destructor
//  Access: Public
//  Description: delete a shader arg storage structure.
////////////////////////////////////////////////////////////////////
INLINE ShaderModeArg::
~ShaderModeArg(void) {
}
 
////////////////////////////////////////////////////////////////////
//  Function: ShaderMode::get_shader
//  Access: Public
//  Description: Return the Shader associated with this shader mode.
////////////////////////////////////////////////////////////////////
INLINE Shader *ShaderMode::
get_shader(void) {
  return _shader;
}

