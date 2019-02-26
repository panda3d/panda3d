/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderModuleGlsl.h
 * @author Mitchell Stokes
 * @date 2019-02-16
 */

#ifndef SHADERMODULEGLSL_H
#define SHADERMODULEGLSL_H

#include "shader.h"

/**
 * ShaderModule that contains raw GLSL code
 */
class EXPCL_PANDA_SHADERPIPELINE ShaderModuleGlsl : public ShaderModule {
public:
  ShaderModuleGlsl(Shader::ShaderType shader_type, std::string source);
  virtual ~ShaderModuleGlsl();

protected:
  Shader::ShaderType _shader_type;
  std::string _raw_source;

PUBLISHED:
  virtual std::string get_ir() const;
  virtual Shader::ShaderType get_shader_type() const;
};
#endif
