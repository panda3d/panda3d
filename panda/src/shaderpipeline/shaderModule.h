/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderModule.h
 * @author Mitchell Stokes
 * @date 2019-02-16
 */

#ifndef SHADERMODULE_H
#define SHADERMODULE_H

/**
 * This is the base class for the outputs of shaderCompilers
 */
class EXPCL_PANDA_SHADERPIPELINE ShaderModule : public TypedReferenceCount {
public:
  ShaderModule();
  virtual ~ShaderModule();

PUBLISHED:
  virtual std::string get_ir() const=0;
  virtual Shader::ShaderType get_shader_type() const=0;
};
#endif
