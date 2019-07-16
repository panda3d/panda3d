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

class ShaderModuleGlsl;

/**
 * This defines the compiler interface to read GLSL files and pre-process them
 */
class EXPCL_PANDA_SHADERPIPELINE ShaderCompilerGlslPreProc : public ShaderCompiler {
public:
  ShaderCompilerGlslPreProc();

  virtual std::string get_name() const override;
  virtual ShaderLanguages get_languages() const override;
  virtual PT(ShaderModule) compile_now(Stage stage, std::istream &in,
                                       const std::string &filename = "created-shader",
                                       BamCacheRecord *record = nullptr) const override;

private:
  bool r_preprocess_include(ShaderModuleGlsl *module,
                            std::ostream &out, const std::string &filename,
                            const Filename &source_dir,
                            std::set<Filename> &open_files,
                            BamCacheRecord *record, int depth) const;
  bool r_preprocess_source(ShaderModuleGlsl *module,
                           std::ostream &out, std::istream &in,
                           const std::string &fn, const Filename &full_fn,
                           std::set<Filename> &open_files,
                           BamCacheRecord *record,
                           int fileno = 0, int depth = 0) const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ShaderCompiler::init_type();
    register_type(_type_handle, "ShaderCompilerGlslPreProc",
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
