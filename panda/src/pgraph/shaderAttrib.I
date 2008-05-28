// Filename: shaderAttrib.I
// Created by:  sshodhan (10Jul04)
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
//     Function: ShaderAttrib::Constructor
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ShaderAttrib::
ShaderAttrib() :
  _shader(NULL),
  _shader_priority(0),
  _auto_shader(false),
  _has_shader(false),
  _flags(0),
  _has_flags(0)
{
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
  _auto_shader(copy._auto_shader),
  _has_shader(copy._has_shader),
  _flags(copy._flags),
  _has_flags(copy._has_flags),
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
//     Function: ShaderAttrib::auto_shader
//       Access: Published
//  Description: If true, then this ShaderAttrib does not contain an
//               explicit shader - instead, it requests the automatic
//               generation of a shader.
////////////////////////////////////////////////////////////////////
INLINE bool ShaderAttrib::
auto_shader() const {
  return _auto_shader;
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
//     Function: ShaderAttrib::get_flag
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool ShaderAttrib::
get_flag(int index) const {
  return (_flags & (1<<index)) ? true:false;
}

