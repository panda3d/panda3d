/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderPool.h
 * @author aignacio
 * @date 2006-03
 */

#ifndef SHADERPOOL_H
#define SHADERPOOL_H

#include "pandabase.h"
#include "shader.h"
#include "filename.h"
#include "hashVal.h"
#include "lightMutex.h"
#include "pmap.h"
#include "pvector.h"

class ShaderCompiler;

/**
 * This class unifies references to the same shader, similar to what ModelPool
 * and TexturePool do for models and textures.  The unit of caching is the
 * ShaderModule: compiled modules are shared between all shaders that use the
 * same source code, whether it came from a file or from a string, and linked
 * Shader objects are shared between all users of the same combination of
 * modules.  It underpins the in-memory caching done by Shader::load() and
 * Shader::make(), and offers management of the resulting tables.
 *
 * There are three layers of caching.  The first maps a module filename and
 * options to a compiled ShaderModule.  The second maps a hash of the module's
 * source (and options) to a compiled ShaderModule.  The third maps a
 * combination of ShaderModule pointers to a linked Shader object.  The third
 * only exists for pointer identity of the Shader objects and this cache isn't
 * part of the disk cache.
 *
 * Passing CompilerOptions with the F_no_ram_cache flag to Shader::load() or
 * Shader::make() bypasses this pool entirely.
 */
class EXPCL_PANDA_GOBJ ShaderPool {
PUBLISHED:
  INLINE static bool has_shader(const Filename &filename);
  INLINE static bool verify_shader(const Filename &filename);
  BLOCKING INLINE static CPT(Shader) load_shader(const Filename &filename);
  INLINE static void add_shader(const Filename &filename, Shader *shader);
  INLINE static void release_shader(const Filename &filename);
  INLINE static void release_all_shaders();

  INLINE static int garbage_collect();

  INLINE static void list_contents(std::ostream &out);
  static void write(std::ostream &out);

public:
  // An ordered tuple of modules making up a linked shader.
  typedef pvector<CPT(ShaderModule)> ShaderModules;

  // Compiles a shader module using the given compiler, unless an equivalent
  // module was already present in the memory or disk cache.
  static PT(ShaderModule) load_module(const ShaderCompiler *compiler,
                                      Shader::Stage stage, const Filename &fn,
                                      std::string_view entry_point,
                                      const CompilerOptions &options,
                                      std::ostream *output_log = nullptr,
                                      BamCacheRecord *record = nullptr);
  static PT(ShaderModule) make_module(const ShaderCompiler *compiler,
                                      Shader::Stage stage,
                                      std::string_view source,
                                      const Filename &fullpath,
                                      std::string_view entry_point,
                                      const CompilerOptions &options,
                                      std::ostream *output_log = nullptr,
                                      BamCacheRecord *record = nullptr);

  // The interface below is used by Shader::load() and Shader::make() to
  // unify shaders that link the same combination of modules.
  static PT(Shader) find_shader(Shader::SourceLanguage lang,
                                const ShaderModules &modules);
  static PT(Shader) find_shader(Shader::SourceLanguage lang,
                                const Filename &fullpath, time_t timestamp,
                                std::streamsize size,
                                const CompilerOptions &options);
  static PT(Shader) record_shader(const ShaderModules &modules, Shader *shader);
  static void record_file_modules(const Filename &fullpath, time_t timestamp,
                                  std::streamsize size,
                                  const CompilerOptions &options,
                                  const ShaderModules &modules);

  static PT(ShaderModule) find_module(const HashVal &key);
  static PT(ShaderModule) record_module(const HashVal &key, ShaderModule *module);

private:
  INLINE ShaderPool();

  static PT(ShaderModule) do_make_module(const ShaderCompiler *compiler,
                                         Shader::Stage stage,
                                         std::string_view source,
                                         const Filename &fullpath,
                                         std::string_view entry_point,
                                         const CompilerOptions &options,
                                         const HashVal &options_hash,
                                         std::ostream *output_log,
                                         BamCacheRecord *record);

  bool ns_has_shader(const Filename &orig_filename);
  CPT(Shader) ns_load_shader(const Filename &orig_filename);
  void ns_add_shader(const Filename &orig_filename, Shader *shader);
  void ns_release_shader(const Filename &orig_filename);
  void ns_release_all_shaders();
  int ns_garbage_collect();
  void ns_list_contents(std::ostream &out) const;

  void resolve_filename(Filename &new_filename, const Filename &orig_filename);

  static ShaderPool *get_ptr();
  static ShaderPool *_global_ptr;

  LightMutex _lock;

  // Compiled modules loaded from file, keyed by filename/options.
  struct ModuleFileKey {
    INLINE bool operator < (const ModuleFileKey &other) const;

    Filename _fullpath;
    Shader::Stage _stage;
    std::string _source_entry_point;
    HashVal _options_hash;
  };
  struct ModuleFile {
    PT(ShaderModule) _module;
    time_t _timestamp;
    std::streamsize _size;
  };
  typedef pmap<ModuleFileKey, ModuleFile> ModulesByFile;
  ModulesByFile _file_modules;

  INLINE ModulesByFile::iterator find_module_by_file(const Filename &fullpath);

  // Compiled modules, keyed by a hash of their source code.
  typedef pmap<HashVal, PT(ShaderModule) > ModulesByHash;
  ModulesByHash _hash_modules;

  // Linked shaders, keyed by a combination of module pointers.
  struct LinkedShaderKey {
    bool operator < (const LinkedShaderKey &other) const;

    Shader::SourceLanguage _language;
    ShaderModules _modules;
  };
  typedef pmap<LinkedShaderKey, PT(Shader) > LinkedShaders;
  LinkedShaders _linked_shaders;
};

#include "shaderPool.I"

#endif
