/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderCompiler.cxx
 * @author Mitchell Stokes
 * @date 2019-02-16
 */

#include "shaderCompiler.h"
#include "virtualFileSystem.h"
#include "config_putil.h"
#include "bamCacheRecord.h"

TypeHandle ShaderCompiler::_type_handle;

/**
 *
 */
ShaderCompiler::
ShaderCompiler() {
}

/**
 *
 */
ShaderCompiler::
~ShaderCompiler() {
}

/**
 * Loads and compiles the code from the given shader file, producing a
 * ShaderModule on success.
 */
PT(ShaderModule) ShaderCompiler::
compile_now(ShaderModule::Stage stage, const Filename &fn, BamCacheRecord *record) const {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  PT(VirtualFile) vf = vfs->find_file(fn, get_model_path());
  if (vf == nullptr) {
    shader_cat.error()
      << "Could not find shader file: " << fn << "\n";
    return nullptr;
  }

  std::istream *in = vf->open_read_file(true);
  if (vf == nullptr) {
    shader_cat.error()
      << "Could not open shader file for reading: " << fn << "\n";
    return nullptr;
  }

  // The default implementation calls the version that takes an istream.
  PT(ShaderModule) module = compile_now(stage, *in, vf->get_filename(), record);
  vf->close_read_file(in);
  return module;
}
