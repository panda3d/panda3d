/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderCompilerCg.h
 * @author Mitchell Stokes
 * @date 2019-04-03
 */

#ifndef SHADERCOMPILERCG_H
#define SHADERCOMPILERCG_H

#include "pandabase.h"

#include "shaderCompiler.h"


/**
 * This defines the compiler interface to read Cg shaders
 */
class EXPCL_PANDA_SHADERPIPELINE ShaderCompilerCg : public ShaderCompiler {
public:
  ShaderCompilerCg();

  void get_profile_from_header(const Shader::ShaderFile &shader_file, Shader::ShaderCaps &caps) const;

PUBLISHED:
  virtual std::string get_name() const override;
  virtual ShaderLanguages get_languages() const override;
  virtual PT(ShaderModule) compile_now(Stage stage, std::istream &in) const override;


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ShaderCompiler::init_type();
    register_type(_type_handle, "ShaderCompilerCg",
                  ShaderCompiler::get_class_type());
  }
  virtual TypeHandle get_type() const override {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() override {
    init_type();
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

#endif
