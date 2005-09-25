// Filename: shaderInput.I
// Created by: jyelon (01Sep05)
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
//     Function: ShaderInput::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ShaderInput::
~ShaderInput()
{
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ShaderInput::
ShaderInput(InternalName *name, int priority) :
  _type(M_invalid),
  _name(name),
  _priority(priority),
  _stored_texture(NULL),
  _stored_nodepath(NodePath()),
  _stored_vector(LVector4f(0,0,0,1))
{
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ShaderInput::
ShaderInput(InternalName *name, Texture *tex, int priority) :
  _type(M_texture),
  _name(name),
  _priority(priority),
  _stored_texture(tex),
  _stored_nodepath(NodePath()),
  _stored_vector(LVector4f(0,0,0,1))
{
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ShaderInput::
ShaderInput(InternalName *name, const NodePath &np, int priority) :
  _type(M_nodepath),
  _name(name),
  _priority(priority),
  _stored_texture(NULL),
  _stored_nodepath(np),
  _stored_vector(LVector4f(0,0,0,1))
{
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ShaderInput::
ShaderInput(InternalName *name, const LVector4f &vec, int priority) :
  _type(M_vector),
  _name(name),
  _priority(priority),
  _stored_texture(NULL),
  _stored_nodepath(NodePath()),
  _stored_vector(vec)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::get_name
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE InternalName *ShaderInput::
get_name() const {
  return _name;
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::get_value_type
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int ShaderInput::
get_value_type() const {
  return _type;
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::get_priority
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int ShaderInput::
get_priority() const {
  return _priority;
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::get_texture
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE Texture *ShaderInput::
get_texture() const {
  return _stored_texture;
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::get_nodepath
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE const NodePath &ShaderInput::
get_nodepath() const {
  return _stored_nodepath;
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::get_vector
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE const LVector4f &ShaderInput::
get_vector() const {
  return _stored_vector;
}

