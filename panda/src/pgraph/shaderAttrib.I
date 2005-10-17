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
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ShaderAttrib::
ShaderAttrib() {
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderAttrib::Copy Constructor
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ShaderAttrib::
ShaderAttrib(const ShaderAttrib &copy) :
  _shader(copy._shader),
  _shader_priority(copy._shader_priority),
  _has_shader(copy._has_shader),
  _inputs(copy._inputs)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderAttrib::has_shader
//       Access: Published
//  Description: If true, the shader field of this attribute overrides
//               the shader field of the parent attribute.
////////////////////////////////////////////////////////////////////
INLINE bool ShaderAttrib::
has_shader() const {
  return _has_shader;
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderAttrib::get_shader_priority
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int ShaderAttrib::
get_shader_priority() const {
  return _shader_priority;
}

