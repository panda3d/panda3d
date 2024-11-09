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
#include "bamCache.h"
#include "bamCacheRecord.h"

// Use different model cache extensions for different stages, since the same
// shader file might have been loaded more than once for different stages.
const char *const cache_extensions[] = {
  "vert.smo",
  "tesc.smo",
  "tese.smo",
  "geom.smo",
  "frag.smo",
  "comp.smo",
};

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
compile_now(Stage stage, const Filename &fn, BamCacheRecord *record) const {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  PT(VirtualFile) vf = vfs->find_file(fn, get_model_path());
  if (vf == nullptr) {
    shader_cat.error()
      << "Could not find shader file: " << fn << "\n";
    return nullptr;
  }
  if (record != nullptr) {
    record->add_dependent_file(vf);
  }

  // Try to read from the compiled shader module cache first.
  Filename fullpath = vf->get_filename();
  BamCache *cache = BamCache::get_global_ptr();

  PT(BamCacheRecord) record2;
  if (cache->get_cache_compiled_shaders()) {
    record2 = cache->lookup(fullpath, cache_extensions[(int)stage]);
    if (record2 != nullptr && record2->has_data()) {
      PT(ShaderModule) module = DCAST(ShaderModule, record2->get_data());

      shader_cat.info()
        << "Shader module " << fn << " found in disk cache.\n";
      return module;
    }
  }

  if (record2 != nullptr) {
    record2->add_dependent_file(vf);
  }

  std::istream *in = vf->open_read_file(true);
  if (vf == nullptr) {
    shader_cat.error()
      << "Could not open shader file for reading: " << fn << "\n";
    return nullptr;
  }

  shader_cat.info()
    << "Compiling " << stage << " shader module: " << fn << "\n";

  // The default implementation calls the version that takes an istream.
  PT(ShaderModule) module = compile_now(stage, *in, fullpath, record2);
  vf->close_read_file(in);

  if (module != nullptr) {
    module->set_source_filename(fullpath);

    if (record2 != nullptr && module != nullptr) {
      // Update the compiled shader module cache.
      record2->set_data(module, module);
      cache->store(record2);
    }
  }

  return module;
}
