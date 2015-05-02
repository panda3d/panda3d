// Filename: glShaderContext_src.h
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


////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::valid
//       Access: Public
//  Description: Returns true if the shader is "valid", ie, if the
//               compilation was successful.  The compilation could
//               fail if there is a syntax error in the shader, or
//               if the current video card isn't shader-capable,
//               or if no shader languages are compiled into panda.
////////////////////////////////////////////////////////////////////
INLINE bool CLP(ShaderContext)::
valid() {
  if (_shader->get_error_flag()) return false;
  if (_shader->get_language() != Shader::SL_GLSL) {
    return false;
  }
  if (_glsl_program != 0) {
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::uses_standard_vertex_arrays
//       Access: Public
//  Description: Returns true if the shader may need to access
//               standard vertex attributes as passed by
//               glVertexPointer and the like.
////////////////////////////////////////////////////////////////////
INLINE bool CLP(ShaderContext)::
uses_standard_vertex_arrays() {
  return _uses_standard_vertex_arrays;
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::uses_custom_vertex_arrays
//       Access: Public
//  Description: Always true, for now.
////////////////////////////////////////////////////////////////////
INLINE bool CLP(ShaderContext)::
uses_custom_vertex_arrays() {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::uses_custom_texture_bindings
//       Access: Public
//  Description: Always true, for now.
////////////////////////////////////////////////////////////////////
INLINE bool CLP(ShaderContext)::
uses_custom_texture_bindings() {
  return true;
}
