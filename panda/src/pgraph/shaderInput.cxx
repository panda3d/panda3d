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
#include "paramNodePath.h"
#include "paramTexture.h"

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
//     Function: ShaderInput::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
ShaderInput::
ShaderInput(CPT_InternalName name, const NodePath &np, int priority) :
  _name(MOVE(name)),
  _type(M_nodepath),
  _priority(priority),
  _value(new ParamNodePath(np))
{
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
ShaderInput::
ShaderInput(CPT_InternalName name, Texture *tex, bool read, bool write, int z, int n, int priority) :
  _name(MOVE(name)),
  _type(M_texture_image),
  _priority(priority),
  _value(new ParamTextureImage(tex, read, write, z, n))
{
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
ShaderInput::
ShaderInput(CPT_InternalName name, Texture *tex, const SamplerState &sampler, int priority) :
  _name(MOVE(name)),
  _type(M_texture_sampler),
  _priority(priority),
  _value(new ParamTextureSampler(tex, sampler))
{
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::get_nodepath
//       Access: Published
//  Description: Warning: no error checking is done.  This *will*
//               crash if get_value_type() is not M_nodepath.
////////////////////////////////////////////////////////////////////
const NodePath &ShaderInput::
get_nodepath() const {
  return DCAST(ParamNodePath, _value)->get_value();
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::get_texture
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
Texture *ShaderInput::
get_texture() const {
  switch (_type) {
  case M_texture_sampler:
    return DCAST(ParamTextureSampler, _value)->get_texture();

  case M_texture_image:
    return DCAST(ParamTextureImage, _value)->get_texture();

  case M_texture:
    return DCAST(Texture, _value);

  default:
    return NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::get_sampler
//       Access: Published
//  Description: Warning: no error checking is done.
////////////////////////////////////////////////////////////////////
const SamplerState &ShaderInput::
get_sampler() const {
  return (_type == M_texture_sampler)
    ? DCAST(ParamTextureSampler, _value)->get_sampler()
    : get_texture()->get_default_sampler();
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
