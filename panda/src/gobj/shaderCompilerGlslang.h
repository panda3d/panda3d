/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderCompilerGlslang.h
 * @author rdb
 * @date 2020-01-02
 */

#ifndef SHADERCOMPILERGLSLANG_H
#define SHADERCOMPILERGLSLANG_H

#include "pandabase.h"

#include "shaderCompiler.h"
#include "shaderModuleSpirV.h"

/**
 * ShaderCompiler implementation that uses the libglslang library to compile
 * GLSL shaders to SPIR-V.
 */
class EXPCL_PANDA_GOBJ ShaderCompilerGlslang : public ShaderCompiler {
public:
  ShaderCompilerGlslang();

  virtual std::string get_name() const override;
  virtual ShaderLanguages get_languages() const override;
  virtual PT(ShaderModule) compile_now(Stage stage, std::istream &in,
                                       const Filename &fullpath,
                                       BamCacheRecord *record = nullptr) const override;

private:
  static bool check_cg_header(const vector_uchar &code);
  static bool preprocess_glsl(vector_uchar &code, int &glsl_version,
                              const Filename &source_filename,
                              pset<Filename> &once_files,
                              BamCacheRecord *record = nullptr);
  static bool postprocess_glsl150(ShaderModuleSpirV::InstructionStream &stream);
  static bool postprocess_cg(ShaderModuleSpirV::InstructionStream &stream);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ShaderCompiler::init_type();
    register_type(_type_handle, "ShaderCompilerGlslang",
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
