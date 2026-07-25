/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderPool.cxx
 * @author aignacio
 * @date 2006-03
 */

#include "shaderPool.h"
#include "bamCache.h"
#include "bamCacheRecord.h"
#include "config_putil.h"
#include "configVariableBool.h"
#include "datagram.h"
#include "lightMutexHolder.h"
#include "shaderCompiler.h"
#include "virtualFileSystem.h"
#include "string_utils.h"

ShaderPool *ShaderPool::_global_ptr = nullptr;

/**
 * Ordering operator for the composition table.  The modules are compared by
 * pointer, since the pool guarantees a canonical pointer per module.
 */
bool ShaderPool::LinkedShaderKey::
operator < (const LinkedShaderKey &other) const {
  if (_language != other._language) {
    return _language < other._language;
  }
  if (_modules.size() != other._modules.size()) {
    return _modules.size() < other._modules.size();
  }
  for (size_t i = 0; i < _modules.size(); ++i) {
    if (_modules[i] != other._modules[i]) {
      return _modules[i] < other._modules[i];
    }
  }
  return false;
}

/**
 * Lists the contents of the shader pool to the indicated output stream.
 */
void ShaderPool::
write(std::ostream &out) {
  get_ptr()->ns_list_contents(out);
}

/**
 * Loads and compiles the code from the given shader file using the given
 * compiler, producing a ShaderModule on success.
 *
 * The result may be shared with other shaders compiled from identical source
 * code with identical settings, and may have been retrieved from the model
 * cache on disk.  This may be bypassed using the F_no_ram_cache and
 * F_no_disk_cache flags in the CompilerOptions.
 */
PT(ShaderModule) ShaderPool::
load_module(const ShaderCompiler *compiler, Shader::Stage stage,
            const Filename &fn, std::string_view entry_point,
            const CompilerOptions &options, std::ostream *output_log,
            BamCacheRecord *record) {
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

  int flags = options.get_flags();

  // Note the file's current stat before we start reading, so that the lookup
  // errs on the side of caution if the file is being modified while we are
  // reading it.
  HashVal options_hash = options.get_cache_key();
  ModuleFileKey key {vf->get_filename(), stage, std::string(entry_point),
                     options_hash};
  time_t timestamp = vf->get_timestamp();
  std::streamsize size = vf->get_file_size();

  if ((flags & CompilerOptions::F_no_ram_cache) == 0) {
    ShaderPool *pool = get_ptr();
    LightMutexHolder holder(pool->_lock);

    ModulesByFile::iterator it = pool->_file_modules.find(key);
    if (it != pool->_file_modules.end()) {
      const ModuleFile &entry = it->second;
      if (entry._timestamp == timestamp && entry._size == size &&
           !entry._module->check_dependents_modified()) {
        if (shader_cat.is_debug()) {
          shader_cat.debug()
            << "Shader module " << key._fullpath
            << " was found in module cache.\n";
        }
        PT(ShaderModule) module = entry._module;
        if (record != nullptr) {
          module->record_dependent_files(record);
        }
        return module;
      }

      // The file or one of its includes was modified; evict the stale entry
      // and fall through to a full load.
      shader_cat.info()
        << "Shader module " << key._fullpath
        << " was modified on disk, reloading.\n";
      pool->_file_modules.erase(it);
    }
  }

  std::string source;
  if (!vf->read_file(source, true)) {
    shader_cat.error()
      << "Could not read shader file: " << fn << "\n";
    return nullptr;
  }

  PT(ShaderModule) module =
    do_make_module(compiler, stage, source, key._fullpath, entry_point,
                   options, options_hash, output_log, record);

  if (module != nullptr && (flags & CompilerOptions::F_no_ram_cache) == 0) {
    ShaderPool *pool = get_ptr();
    LightMutexHolder holder(pool->_lock);
    pool->_file_modules[std::move(key)] =
      ModuleFile {module, timestamp, size};
  }
  return module;
}

/**
 * Compiles the source code from the given string using the given compiler,
 * producing a ShaderModule on success.
 *
 * The result may be shared with other shaders compiled from identical source
 * code with identical settings, and may have been retrieved from the model
 * cache on disk.  This may be bypassed using the F_no_ram_cache and
 * F_no_disk_cache flags in the CompilerOptions.
 */
PT(ShaderModule) ShaderPool::
make_module(const ShaderCompiler *compiler, Shader::Stage stage,
            std::string_view source, const Filename &fullpath,
            std::string_view entry_point, const CompilerOptions &options,
            std::ostream *output_log, BamCacheRecord *record) {
  HashVal options_hash = options.get_cache_key();
  return do_make_module(compiler, stage, source, fullpath, entry_point,
                        options, options_hash, output_log, record);
}

/**
 * Checks the memory and disk caches for a module with the same source code,
 * stage, entry point and settings, invoking the compiler only if it was not
 * found.
 */
PT(ShaderModule) ShaderPool::
do_make_module(const ShaderCompiler *compiler, Shader::Stage stage,
               std::string_view source, const Filename &fullpath,
               std::string_view entry_point, const CompilerOptions &options,
               const HashVal &options_hash, std::ostream *output_log,
               BamCacheRecord *record) {
  // Make the cache key, which is a hash of the source code plus everything
  // that may affect the result of the compilation.
  HashVal hash;
  {
    Datagram dg;
    dg.add_string(compiler->get_type().get_name());
    compiler->write_cache_key(dg);
    dg.add_uint8((int)stage);
    dg.add_string(entry_point);
    options_hash.write_datagram(dg);
    dg.add_string(fullpath.get_dirname());
    dg.append_data(source.data(), source.size());

    hash.hash_buffer((const char *)dg.get_data(), dg.get_length());
  }

  int flags = options.get_flags();

  if ((flags & CompilerOptions::F_no_ram_cache) == 0) {
    PT(ShaderModule) module = find_module(hash);
    if (module != nullptr) {
      if (shader_cat.is_debug()) {
        shader_cat.debug()
          << "Shader module " << fullpath << " (" << hash
          << ") was found in module cache.\n";
      }
      if (record != nullptr) {
        module->record_dependent_files(record);
      }
      return module;
    }
  }

  BamCache *cache = BamCache::get_global_ptr();
  PT(BamCacheRecord) record2;
  if ((flags & CompilerOptions::F_no_disk_cache) == 0 &&
      cache->get_cache_shaders()) {
    // The source of a compiled module is identified in the cache purely by
    // the hash of its contents, not by the file it may have come from.
    record2 = cache->lookup_hash(hash, "smo");
    if (record2 != nullptr && record2->has_data()) {
      PT(ShaderModule) module = DCAST(ShaderModule, record2->get_data());

      shader_cat.info()
        << "Shader module " << fullpath << " (" << hash
        << ") found in disk cache.\n";

      if (record != nullptr) {
        module->record_dependent_files(record);
      }
      if ((flags & CompilerOptions::F_no_ram_cache) == 0) {
        module = record_module(hash, module);
      }
      return module;
    }
  }

  shader_cat.info()
    << "Compiling " << stage << " shader module: " << fullpath << "\n";

  PT(ShaderModule) module =
    compiler->compile_now(stage, source, fullpath, entry_point, options,
                          output_log, record2);
  if (module == nullptr) {
    return nullptr;
  }

  module->set_source_filename(fullpath);
  module->set_source_entry_point(std::string(entry_point));
  if (record != nullptr) {
    module->record_dependent_files(record);
  }

  if (record2 != nullptr) {
    // Note that the source file itself is deliberately not added as a
    // dependent of this record: it is keyed by the source contents, so a
    // change to the file simply produces a different key, whereas recording
    // it here would spuriously invalidate the entry for every other file or
    // string that shares the same contents.  Only the #included files, which
    // the compiler added to the record, determine its continued validity.
    record2->set_data(module, module);
    cache->store(record2);
  }

  if ((flags & CompilerOptions::F_no_ram_cache) == 0) {
    // If another thread beat us to it, this returns that module instead, so
    // that all shaders end up sharing the same copy.
    module = record_module(hash, module);
  }
  return module;
}

/**
 * Returns the shader that was previously linked from the given combination
 * of modules with no specialization constants set, or null if there is none.
 *
 * There is no staleness check at this level: when a source file changes, the
 * module loading layer produces different module pointers, which simply form
 * a different key.
 */
PT(Shader) ShaderPool::
find_shader(Shader::SourceLanguage lang, const ShaderModules &modules) {
  ShaderPool *pool = get_ptr();
  LightMutexHolder holder(pool->_lock);

  LinkedShaderKey key;
  key._language = lang;
  key._modules = modules;

  LinkedShaders::const_iterator it = pool->_linked_shaders.find(key);
  if (it == pool->_linked_shaders.end()) {
    return nullptr;
  }
  if (shader_cat.is_debug()) {
    shader_cat.debug()
      << "Shader " << it->second->get_debug_name()
      << " was found in shader pool.\n";
  }
  return it->second;
}

/**
 * Returns the shader whose modules were all cached under the given file
 * with the given options and an unchanged stat, or null if there is none.
 *
 * The given timestamp and size must be those of the file as it is now; pass
 * 0 for both if the file does not exist, which matches a shader that was
 * registered with add_shader() under a filename that is not on disk.  If the
 * cached entries are stale, they are evicted and null is returned.
 *
 * If lang is SL_none, a shader in any language is accepted.
 */
PT(Shader) ShaderPool::
find_shader(Shader::SourceLanguage lang, const Filename &fullpath,
            time_t timestamp, std::streamsize size,
            const CompilerOptions &options) {
  HashVal options_hash = options.get_cache_key();

  ShaderPool *pool = get_ptr();
  LightMutexHolder holder(pool->_lock);

  // Gather the modules cached under this filename with these options.  The
  // entries for a file are adjacent and ordered by stage, which is also the
  // order in which a shader links its modules.
  ShaderModules modules;
  bool stale = false;
  ModulesByFile::iterator it = pool->find_module_by_file(fullpath);
  while (it != pool->_file_modules.end() && it->first._fullpath == fullpath) {
    if (it->first._options_hash == options_hash) {
      const ModuleFile &entry = it->second;
      if (entry._timestamp != timestamp || entry._size != size ||
          entry._module->check_dependents_modified()) {
        stale = true;
        break;
      }
      modules.push_back(entry._module.p());
    }
    ++it;
  }

  if (stale) {
    // Evict every entry for this file with these options, so that the
    // subsequent load repopulates them wholesale.
    shader_cat.info()
      << "Shader file " << fullpath << " (" << options_hash
      << ") was modified on disk, reloading.\n";
    it = pool->find_module_by_file(fullpath);
    while (it != pool->_file_modules.end() && it->first._fullpath == fullpath) {
      if (it->first._options_hash == options_hash) {
        it = pool->_file_modules.erase(it);
      } else {
        ++it;
      }
    }
    return nullptr;
  }
  if (modules.empty()) {
    return nullptr;
  }

  // Now find a shader that links exactly these modules.  If the file was
  // only ever loaded as an individual stage of a multi-file shader, there is
  // usually no such shader, and this misses.
  LinkedShaderKey key;
  key._modules = std::move(modules);

  // The language cannot be derived from the modules, so if none was
  // specified, simply try each of the possible values in turn.
  Shader::SourceLanguage languages[3] = {Shader::SL_Cg, Shader::SL_GLSL,
                                         Shader::SL_HLSL};
  for (Shader::SourceLanguage candidate : languages) {
    if (lang != Shader::SL_none && lang != candidate) {
      continue;
    }
    key._language = candidate;
    LinkedShaders::const_iterator li = pool->_linked_shaders.find(key);
    if (li != pool->_linked_shaders.end()) {
      if (shader_cat.is_debug()) {
        shader_cat.debug()
          << "Shader " << fullpath << " (" << options_hash
          << ") was found in shader pool.\n";
      }
      return li->second;
    }
  }
  return nullptr;
}

/**
 * Records a shader that was linked from the given combination of modules.
 * If an equivalent shader was recorded by another thread in the meantime,
 * that one is returned instead, so that all callers end up sharing the same
 * shader; otherwise, returns the shader that was just recorded.
 */
PT(Shader) ShaderPool::
record_shader(const ShaderModules &modules, Shader *shader) {
  ShaderPool *pool = get_ptr();
  LightMutexHolder holder(pool->_lock);

  if (shader->_modules.size() != modules.size()) {
    // Quick sanity check; this is admittedly incomplete.
    return shader;
  }

  LinkedShaderKey key;
  key._language = shader->get_language();
  key._modules = modules;

  auto result = pool->_linked_shaders.insert({std::move(key), shader});
  if (!result.second) {
    return result.first->second;
  }
  return shader;
}

/**
 * Caches the modules of a shader that was linked as a whole from the given
 * file, with the stat the file had before it was read, such that a
 * subsequent find_shader() with the same filename resolves to the linked
 * shader without reading the file.
 */
void ShaderPool::
record_file_modules(const Filename &fullpath, time_t timestamp,
                    std::streamsize size, const CompilerOptions &options,
                    const ShaderModules &modules) {
  HashVal options_hash = options.get_cache_key();

  ShaderPool *pool = get_ptr();
  LightMutexHolder holder(pool->_lock);

  for (const CPT(ShaderModule) &module : modules) {
    ModuleFileKey key {fullpath, module->get_stage(),
                       module->get_source_entry_point(), options_hash};
    pool->_file_modules[std::move(key)] =
      ModuleFile {const_cast<ShaderModule *>(module.p()), timestamp, size};
  }
}

/**
 * Returns the compiled module that was previously recorded with the given
 * key, or null if there is none.  If any of the files it was compiled from
 * (such as included files) have since been modified on disk, the stale entry
 * is evicted and null is returned.
 */
PT(ShaderModule) ShaderPool::
find_module(const HashVal &key) {
  ShaderPool *pool = get_ptr();
  LightMutexHolder holder(pool->_lock);

  ModulesByHash::iterator it = pool->_hash_modules.find(key);
  if (it == pool->_hash_modules.end()) {
    return nullptr;
  }
  if (it->second->check_dependents_modified()) {
    pool->_hash_modules.erase(it);
    return nullptr;
  }
  return it->second;
}

/**
 * Records a compiled module with the given key.  If an equivalent module was
 * recorded by another thread in the meantime, that one is returned instead,
 * so that all shaders end up sharing the same module; otherwise, returns the
 * module that was just recorded.
 */
PT(ShaderModule) ShaderPool::
record_module(const HashVal &key, ShaderModule *module) {
  ShaderPool *pool = get_ptr();
  LightMutexHolder holder(pool->_lock);

  auto result = pool->_hash_modules.insert({key, module});
  if (!result.second) {
    if (!result.first->second->check_dependents_modified()) {
      return result.first->second;
    }
    result.first->second = module;
  }
  return module;
}

/**
 * The nonstatic implementation of has_shader().
 */
bool ShaderPool::
ns_has_shader(const Filename &orig_filename) {
  Filename filename;
  resolve_filename(filename, orig_filename);

  LightMutexHolder holder(_lock);

  ModulesByFile::iterator it = find_module_by_file(filename);
  if (it != _file_modules.end() && it->first._fullpath == filename) {
    return true;
  }

  // The cache does not cover every module (shaders created from strings, or
  // entries evicted after a modification on disk), so also check the source
  // filenames recorded on the modules.
  for (const auto &item : _hash_modules) {
    if (item.second->get_source_filename() == filename) {
      return true;
    }
  }
  for (const auto &item : _linked_shaders) {
    for (const CPT(ShaderModule) &module : item.first._modules) {
      if (module->get_source_filename() == filename) {
        return true;
      }
    }
  }
  return false;
}

/**
 * The nonstatic implementation of load_shader().
 */
CPT(Shader) ShaderPool::
ns_load_shader(const Filename &orig_filename) {
  Filename filename;
  resolve_filename(filename, orig_filename);

  Shader::SourceLanguage lang = Shader::SL_none;

  // Do some guesswork to see if we can figure out the shader language from
  // the file extension.  This is really just guesswork - there are no
  // standardized extensions for shaders, especially for GLSL. These are the
  // ones that appear to be closest to "standard".
  std::string ext = downcase(filename.get_extension());
  if (ext == "cg" || ext == "sha") {
    // "sha" is for historical reasons.
    lang = Shader::SL_Cg;

  } else if (ext == "glsl" || ext == "vert" || ext == "frag" ||
             ext == "geom" || ext == "tesc" || ext == "tese" ||
             ext == "comp") {
    lang = Shader::SL_GLSL;
  }

  // Shader::load() itself takes care of checking and updating the pool.
  return Shader::load(filename, lang);
}

/**
 * The nonstatic implementation of add_shader().
 */
void ShaderPool::
ns_add_shader(const Filename &orig_filename, Shader *shader) {
  Filename filename;
  resolve_filename(filename, orig_filename);

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  PT(VirtualFile) vf = vfs->get_file(filename);
  time_t timestamp = 0;
  std::streamsize size = 0;
  if (vf != nullptr) {
    timestamp = vf->get_timestamp();
    size = vf->get_file_size();
  }

  static HashVal options_hash = CompilerOptions().get_cache_key();

  LightMutexHolder holder(_lock);

  // We blow away whatever was registered under this filename previously.
  ModulesByFile::iterator it = find_module_by_file(filename);
  while (it != _file_modules.end() && it->first._fullpath == filename) {
    it = _file_modules.erase(it);
  }

  // Cache each of the shader's modules under this filename, so that a
  // subsequent load of the file with default options resolves to the same
  // modules, and through them to this shader's composition.
  LinkedShaderKey key;
  key._language = shader->get_language();
  for (const Shader::LinkedModule &linked_module : shader->_modules) {
    CPT(ShaderModule) module = linked_module._module.get_read_pointer();

    ModuleFileKey mkey {filename, module->get_stage(),
                        module->get_source_entry_point(), options_hash};
    _file_modules[std::move(mkey)] = ModuleFile {
      const_cast<ShaderModule *>(module.p()), timestamp, size};

    key._modules.push_back(std::move(module));
  }

  // Overwrite any linked shader previously recorded for this combination of
  // modules, so that the newly added shader is the one that is returned.
  _linked_shaders[std::move(key)] = shader;
}

/**
 * The nonstatic implementation of release_shader().
 */
void ShaderPool::
ns_release_shader(const Filename &orig_filename) {
  Filename filename;
  resolve_filename(filename, orig_filename);

  LightMutexHolder holder(_lock);

  // Erase the cache entries for this file.
  {
    ModulesByFile::iterator it = find_module_by_file(filename);
    while (it != _file_modules.end() && it->first._fullpath == filename) {
      it = _file_modules.erase(it);
    }
  }

  // Erase the modules that were compiled from this file.  Note that a module
  // records only the first file it was compiled from, so a twin file with
  // identical contents does not release the shared module.
  for (ModulesByHash::iterator it = _hash_modules.begin(); it != _hash_modules.end();) {
    if (it->second->get_source_filename() == filename) {
      it = _hash_modules.erase(it);
    } else {
      ++it;
    }
  }

  // Erase the shaders that link any module compiled from this file.
  for (LinkedShaders::iterator ci = _linked_shaders.begin();
       ci != _linked_shaders.end();) {
    bool match = false;
    for (const CPT(ShaderModule) &module : ci->first._modules) {
      if (module->get_source_filename() == filename) {
        match = true;
        break;
      }
    }
    if (match) {
      ci = _linked_shaders.erase(ci);
    } else {
      ++ci;
    }
  }
}

/**
 * The nonstatic implementation of release_all_shaders().
 */
void ShaderPool::
ns_release_all_shaders() {
  LightMutexHolder holder(_lock);
  _file_modules.clear();
  _hash_modules.clear();
  _linked_shaders.clear();
}

/**
 * The nonstatic implementation of garbage_collect().
 */
int ShaderPool::
ns_garbage_collect() {
  LightMutexHolder holder(_lock);

  int num_released = 0;

  {
    // Release the shaders that are not referenced outside of the pool.
    LinkedShaders new_linked_shaders;
    for (auto &item : _linked_shaders) {
      if (item.second->get_ref_count() == 1) {
        ++num_released;
      } else {
        new_linked_shaders.insert(new_linked_shaders.end(), item);
      }
    }
    _linked_shaders.swap(new_linked_shaders);

    // After the swap, the local holds the old table, so the released shaders
    // are destructed as this scope closes.  This must happen before the
    // module sweep below, so that it sees their module references released.
  }

  // Also garbage collect the modules whose only remaining references are the
  // ones held by the pool tables themselves.  Modules referenced by a
  // surviving shader, or by the composition key of one, stay put.
  pmap<const ShaderModule *, int> pool_refs;
  for (const auto &item : _hash_modules) {
    ++pool_refs[item.second.p()];
  }
  for (const auto &item : _file_modules) {
    ++pool_refs[item.second._module.p()];
  }

  {
    ModulesByHash new_hash_modules;
    for (auto &item : _hash_modules) {
      if (item.second->get_ref_count() != pool_refs[item.second.p()]) {
        new_hash_modules.insert(new_hash_modules.end(), item);
      }
    }
    _hash_modules.swap(new_hash_modules);

    ModulesByFile new_file_modules;
    for (auto &item : _file_modules) {
      const ShaderModule *module = item.second._module.p();
      if (module->get_ref_count() != pool_refs[module]) {
        new_file_modules.insert(new_file_modules.end(), item);
      }
    }
    _file_modules.swap(new_file_modules);
  }

  return num_released;
}

/**
 * The nonstatic implementation of list_contents().
 */
void ShaderPool::
ns_list_contents(std::ostream &out) const {
  LightMutexHolder holder(_lock);

  out << _linked_shaders.size() << " linked shaders:\n";
  for (const auto &item : _linked_shaders) {
    out << "  " << item.second->get_debug_name()
        << " (count = " << item.second->get_ref_count() << ")\n";
  }

  out << _file_modules.size() << " modules loaded from file:\n";
  for (const auto &item : _file_modules) {
    out << "  " << item.first._fullpath << " (" << item.first._stage
        << ") (count = " << item.second._module->get_ref_count() << ")\n";
  }

  out << _hash_modules.size() << " compiled modules:\n";
  for (const auto &item : _hash_modules) {
    out << "  " << item.first << " " << *item.second
        << " (count = " << item.second->get_ref_count() << ")\n";
  }
}

/**
 * Searches for the indicated filename along the model path.
 */
void ShaderPool::
resolve_filename(Filename &new_filename, const Filename &orig_filename) {
  new_filename = orig_filename;
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  vfs->resolve_filename(new_filename, get_model_path());
}

/**
 * Initializes and/or returns the global pointer to the one ShaderPool object
 * in the system.
 */
ShaderPool *ShaderPool::
get_ptr() {
  if (_global_ptr == nullptr) {
    _global_ptr = new ShaderPool;
  }
  return _global_ptr;
}
