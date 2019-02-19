/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderInput.cxx
 * @author jyelon
 * @date 2005-09-01
 */

#include "shaderInput.h"
#include "paramNodePath.h"
#include "paramTexture.h"

/**
 * Returns a static ShaderInput object with name NULL, priority zero, type
 * INVALID, and all value-fields cleared.
 */
const ShaderInput &ShaderInput::
get_blank() {
  static ShaderInput blank(nullptr, 0);
  return blank;
}

/**
 *
 */
ShaderInput::
ShaderInput(CPT_InternalName name, const NodePath &np, int priority) :
  _name(std::move(name)),
  _type(M_nodepath),
  _priority(priority),
  _value(new ParamNodePath(np))
{
}

/**
 *
 */
ShaderInput::
ShaderInput(CPT_InternalName name, Texture *tex, bool read, bool write, int z, int n, int priority) :
  _name(std::move(name)),
  _type(M_texture_image),
  _priority(priority),
  _value(new ParamTextureImage(tex, read, write, z, n))
{
}

/**
 *
 */
ShaderInput::
ShaderInput(CPT_InternalName name, Texture *tex, const SamplerState &sampler, int priority) :
  _name(std::move(name)),
  _type(M_texture_sampler),
  _priority(priority),
  _value(new ParamTextureSampler(tex, sampler))
{
}

/**
 *
 */
size_t ShaderInput::
add_hash(size_t hash) const {
  hash = int_hash::add_hash(hash, _type);
  hash = pointer_hash::add_hash(hash, _name);
  hash = int_hash::add_hash(hash, _priority);

  switch (_type) {
  case M_invalid:
    return hash;

  case M_vector:
    return _stored_vector.add_hash(hash);

  case M_numeric:
    return pointer_hash::add_hash(hash, _stored_ptr._ptr);

  default:
    return pointer_hash::add_hash(hash, _value);
  }
}

/**
 * Warning: no error checking is done.  This *will* crash if get_value_type()
 * is not M_nodepath.
 */
NodePath ShaderInput::
get_nodepath() const {
  return DCAST(ParamNodePath, _value)->get_value();
}

/**
 *
 */
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
    return nullptr;
  }
}

/**
 * Warning: no error checking is done.
 */
const SamplerState &ShaderInput::
get_sampler() const {
  if (_type == M_texture_sampler) {
    return DCAST(ParamTextureSampler, _value)->get_sampler();

  } else if (!_value.is_null()) {
    return get_texture()->get_default_sampler();

  } else {
    return SamplerState::get_default();
  }
}

/**
 *
 */
void ShaderInput::
register_with_read_factory() {
  // IMPLEMENT ME
}
