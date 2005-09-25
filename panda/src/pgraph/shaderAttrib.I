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
//     Function: ShaderAttrib::get_shader
//       Access: Published
//  Description: Returns the shader object associated with the node.
//               If get_override returns true, but get_shader 
//               returns NULL, that means that this attribute should
//               disable the shader.
////////////////////////////////////////////////////////////////////
INLINE const Shader *ShaderAttrib::
get_shader() const {
  return _shader;
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

////////////////////////////////////////////////////////////////////
//     Function: ShaderAttrib::get_input
//       Access: Published
//  Description: Returns the ShaderInput of the given name.  If
//               no such name is found, this function does not return
//               NULL --- it returns the "blank" ShaderInput.
////////////////////////////////////////////////////////////////////
INLINE const ShaderInput *ShaderAttrib::
get_input(const InternalName *id) const {
  Inputs::const_iterator i = _inputs.find(id);
  if (i == _inputs.end()) {
    return ShaderInput::get_blank();
  } else {
    return (*i).second;
  }
}

