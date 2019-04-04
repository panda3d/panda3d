/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderCompilerCg.cxx
 * @author Mitchell Stokes
 * @date 2019-02-16
 */

#include "shaderCompilerCg.h"
#include "config_shaderpipeline.h"

#include "dcast.h"

TypeHandle ShaderCompilerCg::_type_handle;

/**
 *
 */
ShaderCompilerCg::
ShaderCompilerCg() {
}

/**
 *
 */
std::string ShaderCompilerCg::
get_name() const {
  return "Cg Compiler";
}

/**
 *
 */
ShaderLanguages ShaderCompilerCg::
get_languages() const {
  return {
    Shader::SL_Cg
  };
}

PT(ShaderModule) ShaderCompilerCg::
compile_now(ShaderModule::Stage stage, std::istream &in) const {
  return nullptr;
}
