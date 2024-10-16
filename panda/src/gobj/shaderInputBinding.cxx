/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderInputBinding.cxx
 * @author rdb
 * @date 2024-09-22
 */

#include "shaderInputBinding.h"

small_vector<ShaderInputBinding::BinderDef, 2> ShaderInputBinding::_binders;

/**
 * Creates the appropriate binding for the input with the given name and type.
 */
ShaderInputBinding *ShaderInputBinding::
make(Shader::ShaderLanguage language,
     const InternalName *name, const ShaderType *type) {

  for (const BinderDef &def : _binders) {
    if (def._lang == language) {
      ShaderInputBinding *binding = def._func(name, type);
      if (binding != nullptr) {
        return binding;
      }
    }
  }

  return nullptr;
}

/**
 * Returns a mask indicating which state changes should cause the parameter to
 * be respecified.
 */
int ShaderInputBinding::
get_state_dep() const {
  // Normally we respecify everything per frame.
  return Shader::D_frame;
}

/**
 * Sets up anything necessary to prepare this binding to be used with the given
 * shader.
 */
void ShaderInputBinding::
setup(Shader *shader) {
}

/**
 * Fetches the part of the shader input that is plain numeric data.
 * If packed is true, the data is tightly packed, even if the type originally
 * contained padding.
 */
void ShaderInputBinding::
fetch_data(const State &state, void *into, bool packed) const {
}

/**
 * Returns an opaque resource identifier that can later be used to fetch the
 * nth resource, which is of the given type.
 */
ShaderInputBinding::ResourceId ShaderInputBinding::
get_resource_id(int index, const ShaderType *type) const {
  return 0;
}

/**
 * Fetches the texture associated with this shader input.
 */
PT(Texture) ShaderInputBinding::
fetch_texture(const State &state, ResourceId resource_id, SamplerState &sampler, int &view) const {
  return nullptr;
}

/**
 * Fetches the texture that should be bound as a storage image.
 */
PT(Texture) ShaderInputBinding::
fetch_texture_image(const State &state, ResourceId resource_id, ShaderType::Access &access, int &z, int &n) const {
  return nullptr;
}

/**
 * Registers a factory function to create a binding.
 */
void ShaderInputBinding::
register_binder(ShaderEnums::ShaderLanguage language, int sort, Binder binder) {
  _binders.push_back({language, sort, binder});
  std::sort(_binders.begin(), _binders.end());
}
