/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderCompilerGlslPreProc.h
 * @author Mitchell Stokes
 * @date 2019-02-16
 */

#ifndef SHADERCOMPILERGLSLPREPROC_H
#define SHADERCOMPILERGLSLPREPROC_H

#include "pandabase.h"

#include "shaderCompiler.h"

/**
 * This defines the compiler interface to read GLSL files and pre-process them
 */
class EXPCL_PANDA_SHADERPIPELINE ShaderCompilerGlslPreProc : public ShaderCompiler {
public:
  ShaderCompilerGlslPreProc();

PUBLISHED:
  virtual std::string get_name() const;
  virtual ShaderLanguages get_languages() const;
  virtual PT(ShaderModule) compile_now(Shader::ShaderType shader_type, std::string source) const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ShaderCompiler::init_type();
    register_type(_type_handle, "ShaderCompilerGlslPreProc",
                  ShaderCompiler::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif
