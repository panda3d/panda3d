// Filename: glCgShaderContext_src.h
// Created by: rdb (27Jun14)
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

#ifndef OPENGLES_1

////////////////////////////////////////////////////////////////////
//     Function: GLCgShaderContext::valid
//       Access: Public
//  Description: Returns true if the shader is "valid", ie, if the
//               compilation was successful.  The compilation could
//               fail if there is a syntax error in the shader, or
//               if the current video card isn't shader-capable,
//               or if no shader languages are compiled into panda.
////////////////////////////////////////////////////////////////////
INLINE bool CLP(CgShaderContext)::
valid() {
  if (_shader->get_error_flag()) return false;
  if (_shader->get_language() != Shader::SL_Cg) return false;
  return (_cg_program != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: GLCgShaderContext::uses_standard_vertex_arrays
//       Access: Public
//  Description: Returns true if the shader may need to access
//               standard vertex attributes as passed by
//               glVertexPointer and the like.
////////////////////////////////////////////////////////////////////
INLINE bool CLP(CgShaderContext)::
uses_standard_vertex_arrays() {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: GLCgShaderContext::uses_custom_vertex_arrays
//       Access: Public
//  Description: Always true, for now.
////////////////////////////////////////////////////////////////////
INLINE bool CLP(CgShaderContext)::
uses_custom_vertex_arrays() {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GLCgShaderContext::uses_custom_texture_bindings
//       Access: Public
//  Description: Always true, for now.
////////////////////////////////////////////////////////////////////
INLINE bool CLP(CgShaderContext)::
uses_custom_texture_bindings() {
  return true;
}

#endif  // OPENGLES_1

