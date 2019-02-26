/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderModuleGlsl.cxx
 * @author Mitchell Stokes
 * @date 2019-02-16
 */

#include "shaderModuleGlsl.h"

ShaderModuleGlsl::
ShaderModuleGlsl(Shader::ShaderType shader_type, std::string source) :
  _shader_type(shader_type),
  _raw_source(source)
{
}

ShaderModuleGlsl::
~ShaderModuleGlsl() {
}

std::string ShaderModuleGlsl::
get_ir() const {
  return this->_raw_source;
}

Shader::ShaderType ShaderModuleGlsl::
get_shader_type() const {
  return this->_shader_type;
}
