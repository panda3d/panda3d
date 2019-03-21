/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderCompilerGlslPreProc.cxx
 * @author Mitchell Stokes
 * @date 2019-02-16
 */

#include "shaderCompilerGlslPreProc.h"
#include "shaderModuleGlsl.h"
#include "config_shaderpipeline.h"

#include "dcast.h"

TypeHandle ShaderCompilerGlslPreProc::_type_handle;

/**
 *
 */
ShaderCompilerGlslPreProc::
ShaderCompilerGlslPreProc() {
}

/**
 *
 */
std::string ShaderCompilerGlslPreProc::
get_name() const {
  return "GLSL Pre-Processing Compiler";
}

/**
 *
 */
ShaderLanguages ShaderCompilerGlslPreProc::
get_languages() const {
  return {
    Shader::SL_GLSL
  };
}

PT(ShaderModule) ShaderCompilerGlslPreProc::
compile_now(ShaderModule::Stage stage, std::istream &in) const {
  return new ShaderModuleGlsl(stage);
}
