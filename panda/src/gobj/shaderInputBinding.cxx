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
 * Registers a factory function to create a binding.
 */
void ShaderInputBinding::
register_binder(ShaderEnums::ShaderLanguage language, int sort, Binder binder) {
  _binders.push_back({language, sort, binder});
  std::sort(_binders.begin(), _binders.end());
}
