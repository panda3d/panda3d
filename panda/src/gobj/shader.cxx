/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shader.cxx
 * @author jyelon
 * @date 2005-09-01
 * @author fperazzi, PandaSE
 * @date 2010-04-06
 * @author fperazzi, PandaSE
 * @date 2010-04-29
 */

#include "pandabase.h"
#include "shader.h"
#include "preparedGraphicsObjects.h"
#include "virtualFileSystem.h"
#include "config_putil.h"
#include "bamCache.h"
#include "string_utils.h"
#include "shaderCompilerRegistry.h"
#include "shaderCompiler.h"
#include "shaderInputBinding.h"

using std::istream;
using std::ostream;
using std::ostringstream;
using std::string;

TypeHandle Shader::_type_handle;
Shader::ShaderTable Shader::_load_table;
Shader::ShaderTable Shader::_make_table;
int Shader::_shaders_generated;

/**
 * Determine whether the source file hints at being a Cg shader
 */
static bool has_cg_header(const std::string &shader_text) {
  size_t newline_pos = shader_text.find('\n');
  std::string search_str = shader_text.substr(0, newline_pos);
  return search_str.rfind("//Cg") != std::string::npos;
}

/**
 * Construct a Shader that will be filled in using fillin() or read() later.
 */
Shader::
Shader(SourceLanguage lang) :
  _error_flag(false),
  _language(lang),
  _cache_compiled_shader(false)
{
}

/**
 * Delete the compiled code, if it exists.
 */
Shader::
~Shader() {
  release_all();
  // Note: don't try to erase ourselves from the table.  It currently keeps a
  // reference forever, and so the only place where this constructor is called
  // is in the destructor of the table itself.
  /*if (_loaded) {
    _load_table.erase(_filename);
  } else {
    _make_table.erase(_text);
  }*/
}

/**
 * Generate an error message including a description of the specified
 * parameter.  Always returns false.
 */
bool Shader::
report_parameter_error(const InternalName *name, const ShaderType *type, const char *msg) {
  Filename fn = get_filename();
  shader_cat.error()
    << fn << ": " << *type << ' ' << *name << ": " << msg << "\n";
  return false;
}

/**
 * Adds a part to the matrix cache, if it doesn't already exist, and returns
 * the index.
 */
size_t Shader::
add_matrix_cache_item(StateMatrix input, const InternalName *arg, int dep) {
  // Do we already have a spot in the cache for this part?
  size_t index;
  for (index = 0; index < _matrix_cache_desc.size(); ++index) {
    MatrixCacheItem &part = _matrix_cache_desc[index];
    if (part._part == input && part._arg == arg) {
      part._dep |= dep;
      break;
    }
  }
  if (index == _matrix_cache_desc.size()) {
    // Didn't find this part yet, create a new one.
    MatrixCacheItem part;
    part._part = input;
    part._dep = dep;
    part._arg = arg;

    _matrix_cache_deps |= part._dep;
    _matrix_cache_desc.push_back(std::move(part));
  }
  return index;
}

/**
 * Returns the total size of the matrix part cache in terms of number of
 * matrices.
 */
size_t Shader::
get_matrix_cache_size() const {
  return _matrix_cache_desc.size();
}

/**
 *
 */
void Shader::
clear_parameters() {
  _parameters.clear();
  _var_spec.clear();
}

/**
 * Called by the back-end when the shader has compiled data available.
 */
void Shader::
set_compiled(unsigned int format, const char *data, size_t length) {
  _compiled_format = format;
  _compiled_binary.assign(data, length);

  // Store the compiled shader in the cache.
  /*if (_cache_compiled_shader && !_record.is_null()) {
    _record->set_data(this);

    BamCache *cache = BamCache::get_global_ptr();
    cache->store(_record);
  }*/
}

/**
 * Called by the back-end to retrieve compiled data.
 */
bool Shader::
get_compiled(unsigned int &format, string &binary) const {
  format = _compiled_format;
  binary = _compiled_binary;
  return !binary.empty();
}

/**
 * Reads the shader from the given filename(s). Returns a boolean indicating
 * success or failure.
 */
bool Shader::
read(const ShaderFile &sfile, const CompilerOptions &options, BamCacheRecord *record) {
  _text._separate = sfile._separate;

  PT(BamCacheRecord) record2;
  BamCache *cache;

  if (sfile._separate) {
    if (_language == SL_none) {
      shader_cat.error()
        << "No shader language was specified!\n";
      return false;
    }

    // Read the various stages in order.
    if (!sfile._vertex.empty() &&
        !do_read_source(Stage::VERTEX, sfile._vertex, options, record)) {
      return false;
    }
    if (!sfile._tess_control.empty() &&
        !do_read_source(Stage::TESS_CONTROL, sfile._tess_control, options, record)) {
      return false;
    }
    if (!sfile._tess_evaluation.empty() &&
        !do_read_source(Stage::TESS_EVALUATION, sfile._tess_evaluation, options, record)) {
      return false;
    }
    if (!sfile._geometry.empty() &&
        !do_read_source(Stage::GEOMETRY, sfile._geometry, options, record)) {
      return false;
    }
    if (!sfile._fragment.empty() &&
        !do_read_source(Stage::FRAGMENT, sfile._fragment, options, record)) {
      return false;
    }
    if (!sfile._compute.empty() &&
        !do_read_source(Stage::COMPUTE, sfile._compute, options, record)) {
      return false;
    }
    _filename = sfile;
  }
  else if (_language == SL_Cg || _language == SL_none) {
    // For historical reasons, we have to open up this file early to determine
    // some things about it.
    Filename fn = sfile._shared;
    VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
    PT(VirtualFile) vf = vfs->find_file(fn, get_model_path());
    if (vf == nullptr) {
      shader_cat.error()
        << "Could not find shader file: " << fn << "\n";
      return false;
    }

    // Single-file shaders are cached in their linked form in one big file.
    Filename fullpath = vf->get_filename();
    cache = BamCache::get_global_ptr();
    if (cache->get_cache_compiled_shaders()) {
      record2 = cache->lookup(fullpath, "sho");
      if (record2 != nullptr && record2->has_data()) {
        PT(Shader) shader = DCAST(Shader, record2->get_data());

        if (!shader->_modules.empty()) {
          shader_cat.info()
            << "Shader " << fn << " found in disk cache.\n";

          *this = *shader;
          _debug_name = fullpath.get_basename();
          _prepare_shader_pcollector = PStatCollector(std::string("Draw:Prepare:Shader:") + _debug_name);
          return true;
        }
      }
    }

    std::string source;
    if (!vf->read_file(source, true)) {
      shader_cat.error()
        << "Could not read shader file: " << fn << "\n";
      return false;
    }

    if (_language == SL_none && !has_cg_header(source)) {
      shader_cat.error()
        << "Unable to determine shader language of " << fn << "\n";
      return false;
    }
    _language = SL_Cg;
    _filename = sfile;

    shader_cat.info()
      << "Compiling Cg shader: " << fn << "\n";

    ShaderCompilerRegistry *registry = ShaderCompilerRegistry::get_global_ptr();
    ShaderCompiler *compiler = registry->get_compiler_for_language(SL_Cg);
    nassertr(compiler != nullptr, false);

    CompilerOptions options;
    std::istringstream in(source);

    PT(ShaderModule) vertex = compiler->compile_now(Stage::VERTEX, in, fullpath, options, nullptr, record);
    if (vertex == nullptr || !add_module(std::move(vertex))) {
      return false;
    }
    if (source.find("gshader") != string::npos) {
      in.clear();
      in.seekg(0);
      PT(ShaderModule) geometry = compiler->compile_now(Stage::GEOMETRY, in, fullpath, options, nullptr, record);
      if (geometry == nullptr || !add_module(std::move(geometry))) {
        return false;
      }
    }
    in.clear();
    in.seekg(0);
    PT(ShaderModule) fragment = compiler->compile_now(Stage::FRAGMENT, in, fullpath, options, nullptr, record);
    if (fragment == nullptr || !add_module(std::move(fragment))) {
      return false;
    }

    _debug_name = fullpath.get_basename();
  }
  else {
    shader_cat.error()
      << "GLSL shaders must have separate shader bodies!\n";
    return false;
  }

  if (!link()) {
    return false;
  }

  _prepare_shader_pcollector = PStatCollector(std::string("Draw:Prepare:Shader:") + _debug_name);

  if (record2 != nullptr) {
    // Note that we will call link() again after loading from the cache, but
    // putting this here will make sure that we checked that it links correctly
    // before we write it to the cache.
    record2->set_data(this, this);
    cache->store(record2);
  }

  return true;
}

/**
 * Loads the shader from the given string(s). Returns a boolean indicating
 * success or failure.
 */
bool Shader::
load(const ShaderFile &sbody, const CompilerOptions &options, BamCacheRecord *record) {
  _filename = ShaderFile("created-shader");
  _fullpath = Filename();
  _text._separate = sbody._separate;

  if (sbody._separate) {
    if (_language == SL_none) {
      shader_cat.error()
        << "No shader language was specified!\n";
      return false;
    }

    if (!sbody._vertex.empty() &&
        !do_load_source(Stage::VERTEX, sbody._vertex, options, record)) {
      return false;
    }
    if (!sbody._tess_control.empty() &&
        !do_load_source(Stage::TESS_CONTROL, sbody._tess_control, options, record)) {
      return false;
    }
    if (!sbody._tess_evaluation.empty() &&
        !do_load_source(Stage::TESS_EVALUATION, sbody._tess_evaluation, options, record)) {
      return false;
    }
    if (!sbody._geometry.empty() &&
        !do_load_source(Stage::GEOMETRY, sbody._geometry, options, record)) {
      return false;
    }
    if (!sbody._fragment.empty() &&
        !do_load_source(Stage::FRAGMENT, sbody._fragment, options, record)) {
      return false;
    }
    if (!sbody._compute.empty() &&
        !do_load_source(Stage::COMPUTE, sbody._compute, options, record)) {
      return false;
    }

  } else if (_language == SL_Cg || _language == SL_none) {
    if (_language == SL_none && !has_cg_header(sbody._shared)) {
      shader_cat.error()
        << "Unable to determine shader language of created-shader\n";
      return false;
    }
    _language = SL_Cg;

    if (!do_load_source(Stage::VERTEX, sbody._shared, options, record)) {
      return false;
    }
    if (!do_load_source(Stage::FRAGMENT, sbody._shared, options, record)) {
      return false;
    }
    if (sbody._shared.find("gshader") != string::npos &&
        !do_load_source(Stage::GEOMETRY, sbody._shared, options, record)) {
      return false;
    }

  } else {
    shader_cat.error()
      << "GLSL shaders must have separate shader bodies!\n";
    return false;
  }

  if (!link()) {
    shader_cat.error()
      << "Failed to link shader.\n";
    return false;
  }

  _debug_name = "created-shader";
  _prepare_shader_pcollector = PStatCollector("Draw:Prepare:Shader:created-shader");
  return true;
}

/**
 * Reads the shader file from the given path into the given string.
 *
 * Returns false if there was an error with this shader bad enough to consider
 * it 'invalid'.
 */
bool Shader::
do_read_source(Stage stage, const Filename &fn, const CompilerOptions &options, BamCacheRecord *record) {
  ShaderCompilerRegistry *registry = ShaderCompilerRegistry::get_global_ptr();
  ShaderCompiler *compiler = registry->get_compiler_for_language(_language);
  nassertr(compiler != nullptr, false);

  PT(ShaderModule) module = compiler->compile_now(stage, fn, options, nullptr, record);
  if (!module) {
    return false;
  }
  nassertr(stage == module->get_stage(), false);

  if (!add_module(std::move(module))) {
    return false;
  }

  if (!_debug_name.empty()) {
    _debug_name += '/';
  }
  _debug_name += fn.get_basename();

  return true;
}

/**
 * Loads the shader file from the given string into the given string,
 * performing any pre-processing on it that may be necessary.
 *
 * Returns false if there was an error with this shader bad enough to consider
 * it 'invalid'.
 */
bool Shader::
do_read_source(ShaderModule::Stage stage, std::istream &in,
               const Filename &fullpath, const CompilerOptions &options,
               BamCacheRecord *record) {
  ShaderCompilerRegistry *registry = ShaderCompilerRegistry::get_global_ptr();
  ShaderCompiler *compiler = registry->get_compiler_for_language(_language);
  nassertr(compiler != nullptr, false);

  PT(ShaderModule) module = compiler->compile_now(stage, in, fullpath, options, nullptr, record);
  if (!module) {
    return false;
  }
  nassertr(stage == module->get_stage(), false);

  return add_module(std::move(module));
}

/**
 * Loads the shader file from the given string into the given string,
 * performing any pre-processing on it that may be necessary.
 *
 * Returns false if there was an error with this shader bad enough to consider
 * it 'invalid'.
 */
bool Shader::
do_load_source(ShaderModule::Stage stage, const std::string &source,
               const CompilerOptions &options, BamCacheRecord *record) {
  std::istringstream in(source);
  return do_read_source(stage, in, Filename("created-shader"), options, record);
}

/**
 * Completes the binding between the different shader stages.
 */
bool Shader::
link() {
  nassertr(!_modules.empty(), false);

  // Go through all the modules to fetch the parameters.
  pmap<CPT_InternalName, Parameter> parameters_by_name;
  pvector<Parameter *> parameters;

  pmap<CPT_InternalName, const ShaderType *> spec_const_types;

  for (LinkedModule &linked_module : _modules) {
    const ShaderModule *module = linked_module._module.get_read_pointer();
    pmap<int, int> remap;

    for (const ShaderModule::Variable &var : module->_parameters) {
      Parameter param;
      param._name = var.name;
      param._type = var.type;
      param._stage_mask = (1 << (int)module->get_stage());

      auto result = parameters_by_name.insert({var.name, param});
      auto &it = result.first;

      if (result.second) {
        parameters.push_back(&(it->second));
      } else {
        // A variable by this name was already added by another stage.  Check
        // that it has the same type and location.
        Parameter &other = it->second;
        if (other._type != var.type) {
          shader_cat.error()
            << "Parameter " << *var.name << " in module " << *module
            << " is declared in another stage with a mismatching type!\n";
          return false;
        }

        // Aggregate types don't seem to work properly when sharing uniforms
        // between shader stages.  Needs revisiting.
        if (!var.type->is_aggregate_type()) {
          other._stage_mask |= param._stage_mask;
          continue;
        }
      }
    }

    for (const ShaderModule::SpecializationConstant &spec_const : module->_spec_constants) {
      auto result = spec_const_types.insert({spec_const.name, spec_const.type});
      auto &it = result.first;

      if (!result.second) {
        // Another module has already defined a spec constant with this name.
        // Make sure they have the same type.
        const ShaderType *other_type = it->second;
        if (spec_const.type != other_type) {
          shader_cat.error()
            << "Specialization constant " << *spec_const.name << " in module "
            << *module << " is declared in another stage with a mismatching type!\n";
          return false;
        }
      }
    }
  }

  // Now bind all of the parameters.
  bool success = true;
  for (const Parameter *param : parameters) {
    ShaderInputBinding *binding = ShaderInputBinding::make(_language, param->_name, param->_type);
    _parameters.push_back(*param);
    if (binding != nullptr) {
      binding->setup(this);
      _parameters.back()._binding = binding;
    } else {
      shader_cat.error()
        << "Failed to bind parameter " << *param->_name << " with type "
        << *param->_type << "\n";
      success = false;
    }
  }

  return success;
}

void Shader::
add_parameter(const InternalName *name, const ShaderType *type, int location) {
  {
    Shader::Parameter param;
    param._name = name;
    param._type = type;
    param._binding = ShaderInputBinding::make(_language, name, type);
    param._location = location;
    _parameters.push_back(std::move(param));
  }

  Shader::Parameter &param = _parameters.back();
  if (param._binding != nullptr) {
    param._binding->setup(this);
  } else {
    shader_cat.error()
      << "Failed to bind parameter " << *name << " with type "
      << *type << "\n";
  }
}

/**
 * Binds a vertex input parameter with the given type.
 */
bool Shader::
bind_vertex_input(const InternalName *name, const ShaderType *type, int location) {
  std::string name_str = name->get_name();

  Shader::ShaderVarSpec bind;
  bind._id._name = name_str;
  bind._id._type = type;
  bind._id._location = location;
  bind._name = nullptr;
  bind._append_uv = -1;

  uint32_t dim[3];
  if (!type->as_scalar_type(bind._scalar_type, dim[0], dim[1], dim[2])) {
    shader_cat.error()
      << "Unrecognized type " << *type << " for vertex input " << *name << "\n";
  }
  bind._elements = dim[0] * dim[1];

  if (shader_cat.is_debug()) {
    shader_cat.debug()
      << "Binding vertex input " << name_str << " with type " << *type
      << " and location " << location << "\n";
  }

  // Check if it has a p3d_ prefix - if so, assign special meaning.
  if (_language == Shader::SL_GLSL && name_str.compare(0, 4, "p3d_") == 0) {
    // GLSL-style vertex input.
    if (name_str == "p3d_Vertex") {
      bind._name = InternalName::get_vertex();
    }
    else if (name_str == "p3d_Normal") {
      bind._name = InternalName::get_normal();
    }
    else if (name_str == "p3d_Color") {
      bind._name = InternalName::get_color();
    }
    else if (name_str.compare(4, 7, "Tangent") == 0) {
      bind._name = InternalName::get_tangent();
      if (name_str.size() > 11) {
        bind._append_uv = atoi(name_str.substr(11).c_str());
      }
    }
    else if (name_str.compare(4, 8, "Binormal") == 0) {
      bind._name = InternalName::get_binormal();
      if (name_str.size() > 12) {
        bind._append_uv = atoi(name_str.substr(12).c_str());
      }
    }
    else if (name_str.compare(4, 13, "MultiTexCoord") == 0 && name_str.size() > 17) {
      bind._name = InternalName::get_texcoord();
      bind._append_uv = atoi(name_str.substr(17).c_str());
    }
    else if (name_str == "p3d_InstanceMatrix") {
      bind._name = InternalName::get_instance_matrix();

      if (dim[1] != 4 || dim[2] != 3) {
        return report_parameter_error(name, type, "expected mat4x3");
      }
    }
    else {
      shader_cat.error()
        << "Unrecognized built-in vertex input name '" << name_str << "'!\n";
      return false;
    }
  }
  else if (_language == Shader::SL_Cg && name_str.compare(0, 4, "vtx_") == 0) {
    // Cg-style vertex input.
    if (name_str == "vtx_position") {
      bind._name = InternalName::get_vertex();
      bind._append_uv = -1;
    }
    else if (name_str.substr(4, 8) == "texcoord") {
      bind._name = InternalName::get_texcoord();
      if (name_str.size() > 12) {
        bind._append_uv = atoi(name_str.c_str() + 12);
      }
    }
    else if (name_str.substr(4, 7) == "tangent") {
      bind._name = InternalName::get_tangent();
      if (name_str.size() > 11) {
        bind._append_uv = atoi(name_str.c_str() + 11);
      }
    }
    else if (name_str.substr(4, 8) == "binormal") {
      bind._name = InternalName::get_binormal();
      if (name_str.size() > 11) {
        bind._append_uv = atoi(name_str.c_str() + 11);
      }
    }
    else {
      bind._name = InternalName::make(name_str.substr(4));
    }
  }
  else {
    // Arbitrarily named attribute.
    bind._name = InternalName::make(name_str);
  }

  _var_spec.push_back(bind);
  return true;
}

/**
 * Checks whether the shader or any of its dependent files were modified on
 * disk.
 */
bool Shader::
check_modified() const {
  for (const LinkedModule &linked_module : _modules) {
    const ShaderModule *module = linked_module._module.get_read_pointer();

    if (module->_record != nullptr && !module->_record->dependents_unchanged()) {
      return true;
    }
  }
  return false;
}

/**
 * Loads the shader with the given filename.
 */
PT(Shader) Shader::
load(const Filename &file, SourceLanguage lang, const CompilerOptions &options) {
  ShaderFile sfile(file);
  ShaderTable::const_iterator i = _load_table.find(sfile);
  if (i != _load_table.end() && (lang == SL_none || lang == i->second->_language)) {
    // But check that someone hasn't modified it in the meantime.
    if (i->second->check_modified()) {
      shader_cat.info()
        << "Shader " << file << " was modified on disk, reloading.\n";
    } else {
      if (shader_cat.is_debug()) {
        shader_cat.debug()
          << "Shader " << file << " was found in shader cache.\n";
      }
      return i->second;
    }
  }

  PT(Shader) shader = new Shader(lang);
  if (!shader->read(sfile, options)) {
    return nullptr;
  }

  _load_table[sfile] = shader;

  /*if (cache_generated_shaders) {
    ShaderTable::const_iterator i = _make_table.find(shader->_text);
    if (i != _make_table.end() && (lang == SL_none || lang == i->second->_language)) {
      return i->second;
    }
    _make_table[shader->_text] = shader;
  }*/
  return shader;
}

/**
 * This variant of Shader::load loads all shader programs separately.
 */
PT(Shader) Shader::
load(SourceLanguage lang, const Filename &vertex,
     const Filename &fragment, const Filename &geometry,
     const Filename &tess_control, const Filename &tess_evaluation,
     const CompilerOptions &options) {
  ShaderFile sfile(vertex, fragment, geometry, tess_control, tess_evaluation);
  ShaderTable::const_iterator i = _load_table.find(sfile);
  if (i != _load_table.end() && (lang == SL_none || lang == i->second->_language)) {
    // But check that someone hasn't modified it in the meantime.
    if (i->second->check_modified()) {
      shader_cat.info()
        << "Shader was modified on disk, reloading.\n";
    } else {
      if (shader_cat.is_debug()) {
        shader_cat.debug()
          << "Shader was found in shader cache.\n";
      }
      return i->second;
    }
  }

  PT(Shader) shader = new Shader(lang);
  if (!shader->read(sfile, options)) {
    return nullptr;
  }

  _load_table[sfile] = shader;

  /*if (cache_generated_shaders) {
    ShaderTable::const_iterator i = _make_table.find(shader->_text);
    if (i != _make_table.end() && (lang == SL_none || lang == i->second->_language)) {
      return i->second;
    }
    _make_table[shader->_text] = shader;
  }*/
  return shader;
}

/**
 * Loads a compute shader.
 */
PT(Shader) Shader::
load_compute(SourceLanguage lang, const Filename &fn,
             const CompilerOptions &options) {
  if (lang != SL_GLSL) {
    shader_cat.error()
      << "Only GLSL compute shaders are currently supported.\n";
    return nullptr;
  }

  Filename fullpath(fn);
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  if (!vfs->resolve_filename(fullpath, get_model_path())) {
    shader_cat.error()
      << "Could not find compute shader file: " << fn << "\n";
    return nullptr;
  }

  ShaderFile sfile;
  sfile._separate = true;
  sfile._compute = fn;

  ShaderTable::const_iterator i = _load_table.find(sfile);
  if (i != _load_table.end() && (lang == SL_none || lang == i->second->_language)) {
    // But check that someone hasn't modified it in the meantime.
    if (i->second->check_modified()) {
      shader_cat.info()
        << "Compute shader " << fn << " was modified on disk, reloading.\n";
    } else {
      if (shader_cat.is_debug()) {
        shader_cat.debug()
          << "Compute shader " << fn << " was found in shader cache.\n";
      }
      return i->second;
    }
  }

  BamCache *cache = BamCache::get_global_ptr();
  PT(BamCacheRecord) record = cache->lookup(fullpath, "sho");
  if (record != nullptr) {
    if (record->has_data()) {
      PT(Shader) shader = DCAST(Shader, record->get_data());
      if (shader->_module_mask == (1 << (int)Stage::COMPUTE)) {
        shader_cat.info()
          << "Compute shader " << fn << " was found in disk cache.\n";
        return shader;
      }
    }
  }

  PT(Shader) shader = new Shader(lang);
  if (!shader->read(sfile, options, record)) {
    return nullptr;
  }

  _load_table[sfile] = shader;

  /*if (cache_generated_shaders) {
    ShaderTable::const_iterator i = _make_table.find(shader->_text);
    if (i != _make_table.end() && (lang == SL_none || lang == i->second->_language)) {
      return i->second;
    }
    _make_table[shader->_text] = shader;
  }*/

  // It makes little sense to cache the shader before compilation, so we keep
  // the record for when we have the compiled the shader.
  //shader->_record = std::move(record);
  shader->_cache_compiled_shader = BamCache::get_global_ptr()->get_cache_compiled_shaders();
  shader->_fullpath = std::move(fullpath);
  return shader;
}

/**
 * Loads the shader, using the string as shader body.
 */
PT(Shader) Shader::
make(string body, SourceLanguage lang, const CompilerOptions &options) {
  if (lang == SL_GLSL) {
    shader_cat.error()
      << "GLSL shaders must have separate shader bodies!\n";
    return nullptr;

  } else if (lang == SL_none) {
    shader_cat.warning()
      << "Shader::make() now requires an explicit shader language.  Assuming Cg.\n";
    lang = SL_Cg;
  }

  ShaderFile sbody(std::move(body));

  if (cache_generated_shaders) {
    ShaderTable::const_iterator i = _make_table.find(sbody);
    if (i != _make_table.end() && (lang == SL_none || lang == i->second->_language)) {
      // But check that someone hasn't modified its includes in the meantime.
      if (!i->second->check_modified()) {
        return i->second;
      }
    }
  }

  PT(Shader) shader = new Shader(lang);
  if (!shader->load(sbody, options)) {
    return nullptr;
  }

  /*if (cache_generated_shaders) {
    ShaderTable::const_iterator i = _make_table.find(shader->_text);
    if (i != _make_table.end() && (lang == SL_none || lang == i->second->_language)) {
      shader = i->second;
    } else {
      _make_table[shader->_text] = shader;
    }
    _make_table[std::move(sbody)] = shader;
  }*/

  if (dump_generated_shaders) {
    ostringstream fns;
    int index = _shaders_generated ++;
    fns << "genshader" << index;
    string fn = fns.str();
    shader_cat.warning() << "Dumping shader: " << fn << "\n";

    pofstream s;
    s.open(fn.c_str(), std::ios::out | std::ios::trunc);
    s << shader->get_text();
    s.close();
  }
  return shader;
}

/**
 * Loads the shader, using the strings as shader bodies.
 */
PT(Shader) Shader::
make(SourceLanguage lang, string vertex, string fragment, string geometry,
     string tess_control, string tess_evaluation, const CompilerOptions &options) {
  if (lang == SL_none) {
    shader_cat.error()
      << "Shader::make() requires an explicit shader language.\n";
    return nullptr;
  }

  ShaderFile sbody(std::move(vertex), std::move(fragment), std::move(geometry),
                   std::move(tess_control), std::move(tess_evaluation));

  if (cache_generated_shaders) {
    ShaderTable::const_iterator i = _make_table.find(sbody);
    if (i != _make_table.end() && (lang == SL_none || lang == i->second->_language)) {
      // But check that someone hasn't modified its includes in the meantime.
      if (!i->second->check_modified()) {
        return i->second;
      }
    }
  }

  PT(Shader) shader = new Shader(lang);
  shader->_filename = ShaderFile("created-shader");
  if (!shader->load(sbody, options)) {
    return nullptr;
  }

  /*if (cache_generated_shaders) {
    ShaderTable::const_iterator i = _make_table.find(shader->_text);
    if (i != _make_table.end() && (lang == SL_none || lang == i->second->_language)) {
      shader = i->second;
    } else {
      _make_table[shader->_text] = shader;
    }
    _make_table[std::move(sbody)] = shader;
  }*/

  return shader;
}

/**
 * Loads the compute shader from the given string.
 */
PT(Shader) Shader::
make_compute(SourceLanguage lang, string body, const CompilerOptions &options) {
  if (lang != SL_GLSL) {
    shader_cat.error()
      << "Only GLSL compute shaders are currently supported.\n";
    return nullptr;
  }

  ShaderFile sbody;
  sbody._separate = true;
  sbody._compute = std::move(body);

  if (cache_generated_shaders) {
    ShaderTable::const_iterator i = _make_table.find(sbody);
    if (i != _make_table.end() && (lang == SL_none || lang == i->second->_language)) {
      // But check that someone hasn't modified its includes in the meantime.
      if (!i->second->check_modified()) {
        return i->second;
      }
    }
  }

  PT(Shader) shader = new Shader(lang);
  shader->_filename = ShaderFile("created-shader");
  if (!shader->load(sbody, options)) {
    return nullptr;
  }

  /*if (cache_generated_shaders) {
    ShaderTable::const_iterator i = _make_table.find(shader->_text);
    if (i != _make_table.end() && (lang == SL_none || lang == i->second->_language)) {
      shader = i->second;
    } else {
      _make_table[shader->_text] = shader;
    }
    _make_table[std::move(sbody)] = shader;
  }*/

  return shader;
}

/**
 * Adds a module to a shader. Returns true if it was added successfully, false
 * if there was a problem. Modules must be added in increasing stage order and
 * only one module of a given stage type may be added.
 *
 * If the ShaderModule is already used in a different Shader object, this may
 * create a unique copy of it so that it may be modified to make it compatible
 * with the other modules in this shader.
 */
bool Shader::
add_module(PT(ShaderModule) module) {
  nassertr(module != nullptr, false);

  Stage stage = module->get_stage();
  if (has_stage(stage)) {
    shader_cat.error()
      << "Shader already has a module with stage " << stage << ".\n";
    return false;
  }

  if (_module_mask > (1u << (uint32_t)stage)) {
    shader_cat.error()
      << "Shader modules must be loaded in increasing stage order.\n";
    return false;
  }

  uint64_t used_caps = module->get_used_capabilities();

  // Make sure that any modifications made to the module will not affect other
  // Shader objects, by storing it as copy-on-write.
  COWPT(ShaderModule) cow_module = std::move(module);

  if (!_modules.empty()) {
    // Link its inputs up with the previous stage.
    pmap<int, int> location_remap;
    {
      CPT(ShaderModule) module = cow_module.get_read_pointer();
      if (!module->link_inputs(_modules.back()._module.get_read_pointer(), location_remap)) {
        shader_cat.error()
          << "Unable to match shader module interfaces.\n";
        return false;
      }
    }

    if (!location_remap.empty()) {
      // Make sure we have a unique copy so that we can do the remappings.
      PT(ShaderModule) module = cow_module.get_write_pointer();
      module->remap_input_locations(location_remap);
    }
  }
  else if (stage == Stage::VERTEX) {
    // Bind vertex inputs right away.
    CPT(ShaderModule) module = cow_module.get_read_pointer();
    bool success = true;
    for (const ShaderModule::Variable &var : module->_inputs) {
      if (!bind_vertex_input(var.name, var.type, var.get_location())) {
        success = false;
      }
    }
    if (!success) {
      shader_cat.error()
        << "Failed to bind vertex inputs.\n";
      return false;
    }
  }

  _modules.push_back(std::move(cow_module));
  _module_mask |= (1u << (uint32_t)stage);
  _used_caps |= used_caps;
  return true;
}

/**
 * Sets an unsigned integer value for the specialization constant with the
 * indicated name.  All modules containing a specialization constant with
 * this name will be given this value.
 *
 * Returns true if there was a specialization constant with this name on any of
 * the modules, false otherwise.
 */
bool Shader::
set_constant(CPT_InternalName name, unsigned int value) {
  bool any_changed = false;
  bool any_found = false;

  // Set the value on all modules containing a spec constant with this name.
  for (LinkedModule &linked_module : _modules) {
    const ShaderModule *module = linked_module._module.get_read_pointer();

    for (const ShaderModule::SpecializationConstant &spec_const : module->_spec_constants) {
      if (spec_const.name == name) {
        // Found one.
        if (linked_module._consts.set_constant(spec_const.id, value)) {
          any_changed = true;
        }
        any_found = true;
        break;
      }
    }
  }

  if (any_changed) {
    if (shader_cat.is_debug()) {
      shader_cat.debug()
        << "Specialization constant value changed, forcing shader to "
        << "re-prepare.\n";
    }
    // Force the shader to be re-prepared so the value change is picked up.
    release_all();
  }

  return any_found;
}

/**
 * Indicates that the shader should be enqueued to be prepared in the
 * indicated prepared_objects at the beginning of the next frame.  This will
 * ensure the texture is already loaded into texture memory if it is expected
 * to be rendered soon.
 *
 * Use this function instead of prepare_now() to preload textures from a user
 * interface standpoint.
 */
PT(AsyncFuture) Shader::
prepare(PreparedGraphicsObjects *prepared_objects) {
  return prepared_objects->enqueue_shader_future(this);
}

/**
 * Returns true if the shader has already been prepared or enqueued for
 * preparation on the indicated GSG, false otherwise.
 */
bool Shader::
is_prepared(PreparedGraphicsObjects *prepared_objects) const {
  Contexts::const_iterator ci;
  ci = _contexts.find(prepared_objects);
  if (ci != _contexts.end()) {
    return true;
  }
  return prepared_objects->is_shader_queued(this);
}

/**
 * Frees the texture context only on the indicated object, if it exists there.
 * Returns true if it was released, false if it had not been prepared.
 */
bool Shader::
release(PreparedGraphicsObjects *prepared_objects) {
  Contexts::iterator ci;
  ci = _contexts.find(prepared_objects);
  if (ci != _contexts.end()) {
    ShaderContext *sc = (*ci).second;
    if (sc != nullptr) {
      prepared_objects->release_shader(sc);
    } else {
      _contexts.erase(ci);
    }
    return true;
  }

  // Maybe it wasn't prepared yet, but it's about to be.
  return prepared_objects->dequeue_shader(this);
}

/**
 * Creates a context for the shader on the particular GSG, if it does not
 * already exist.  Returns the new (or old) ShaderContext.  This assumes that
 * the GraphicsStateGuardian is the currently active rendering context and
 * that it is ready to accept new textures.  If this is not necessarily the
 * case, you should use prepare() instead.
 *
 * Normally, this is not called directly except by the GraphicsStateGuardian;
 * a shader does not need to be explicitly prepared by the user before it may
 * be rendered.
 */
ShaderContext *Shader::
prepare_now(PreparedGraphicsObjects *prepared_objects,
            GraphicsStateGuardianBase *gsg) {
  Contexts::const_iterator ci;
  ci = _contexts.find(prepared_objects);
  if (ci != _contexts.end()) {
    return (*ci).second;
  }

  int supported_caps = gsg->get_supported_shader_capabilities();
  int unsupported_caps = _used_caps & ~supported_caps;
  if (unsupported_caps != 0) {
    std::ostream &out = shader_cat.error()
      << "Cannot load shader because the graphics back-end does not support ";

    if (supported_caps == 0) {
      out << "shaders.";
    } else {
      out << "these capabilities: ";
      ShaderModule::output_capabilities(out, unsupported_caps);
    }
    out << std::endl;

    // Insert nullptr so that we don't spam this error next time.
    _contexts[prepared_objects] = nullptr;

    return nullptr;
  }

  ShaderContext *tc = prepared_objects->prepare_shader_now(this, gsg);
  _contexts[prepared_objects] = tc;

  return tc;
}

/**
 * Removes the indicated PreparedGraphicsObjects table from the Shader's
 * table, without actually releasing the texture.  This is intended to be
 * called only from PreparedGraphicsObjects::release_texture(); it should
 * never be called by user code.
 */
void Shader::
clear_prepared(PreparedGraphicsObjects *prepared_objects) {
  Contexts::iterator ci;
  ci = _contexts.find(prepared_objects);
  if (ci != _contexts.end()) {
    _contexts.erase(ci);
  } else {
    // If this assertion fails, clear_prepared() was given a prepared_objects
    // which the texture didn't know about.
    nassert_raise("unknown PreparedGraphicsObjects");
  }
}

/**
 * Frees the context allocated on all objects for which the texture has been
 * declared.  Returns the number of contexts which have been freed.
 */
int Shader::
release_all() {
  // We have to traverse a copy of the _contexts list, because the
  // PreparedGraphicsObjects object will call clear_prepared() in response to
  // each release_texture(), and we don't want to be modifying the _contexts
  // list while we're traversing it.
  Contexts temp = _contexts;
  int num_freed = (int)_contexts.size();

  Contexts::const_iterator ci;
  for (ci = temp.begin(); ci != temp.end(); ++ci) {
    PreparedGraphicsObjects *prepared_objects = (*ci).first;
    ShaderContext *sc = (*ci).second;
    if (sc != nullptr) {
      prepared_objects->release_shader(sc);
    }
  }

  // There might still be some outstanding contexts in the map, if there were
  // any NULL pointers there.  Eliminate them.
  _contexts.clear();

  return num_freed;
}

/**
 * Tells the BamReader how to create objects of type Shader.
 */
void Shader::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void Shader::
write_datagram(BamWriter *manager, Datagram &dg) {
  dg.add_uint8(_language);

  dg.add_uint32(_compiled_format);
  dg.add_string(_compiled_binary);

  dg.add_uint32(_module_mask);

  for (const LinkedModule &linked_module : _modules) {
    CPT(ShaderModule) module = linked_module._module.get_read_pointer();

    Filename fn;
    //Filename fn = module->get_source_filename();
    dg.add_string(fn);

    if (fn.empty()) {
      // This module was not read from a file, so write the module itself too.
      manager->write_pointer(dg, module);
    }
  }
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type Shader is encountered in the Bam file.  It should create the Shader
 * and extract its information from the file.
 */
TypedWritable *Shader::
make_from_bam(const FactoryParams &params) {
  Shader *shader = new Shader(SL_none);
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  shader->fillin(scan, manager);

  manager->register_finalize(shader);

  return shader;
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int Shader::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = TypedWritableReferenceCount::complete_pointers(p_list, manager);

  if (_modules.empty()) {
    int num_modules = count_bits_in_word(_module_mask);
    _module_mask = 0u;

    for (int i = 0; i < num_modules; ++i) {
      add_module(DCAST(ShaderModule, p_list[pi++]));
    }
  }

  return pi;
}

/**
 * Some objects require all of their nested pointers to have been completed
 * before the objects themselves can be completed.  If this is the case,
 * override this method to return true, and be careful with circular
 * references (which would make the object unreadable from a bam file).
 */
bool Shader::
require_fully_complete() const {
  return true;
}

/**
 * Called by the BamReader to perform any final actions needed for setting up
 * the object after all objects have been read and all pointers have been
 * completed.
 */
void Shader::
finalize(BamReader *manager) {
  // Since the shader modules may have changed since last time, we have to
  // re-link the shader (which also binds all of the parameters).
  if (!link()) {
    _modules.clear();
    _module_mask = 0u;
  }
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new Shader.
 */
void Shader::
fillin(DatagramIterator &scan, BamReader *manager) {
  _language = (SourceLanguage)scan.get_uint8();
  _debug_name = std::string();
  _module_mask = 0u;
  _modules.clear();

  _compiled_format = scan.get_uint32();
  _compiled_binary = scan.get_string();

  uint32_t mask = scan.get_uint32();
  _module_mask = mask;
  int num_modules = count_bits_in_word(mask);

  for (int i = 0; i < num_modules; ++i) {
    Stage stage = (Stage)get_lowest_on_bit(mask);
    mask &= ~(1u << (uint32_t)stage);

    Filename fn = scan.get_string();
    if (!fn.empty()) {
      // Compile this module from a source file.
      //FIXME: store options
      do_read_source(stage, fn, CompilerOptions(), nullptr);
    }
    else {
      // This module was embedded.
      manager->read_pointer(scan);
    }
  }

  _prepare_shader_pcollector = PStatCollector(std::string("Draw:Prepare:Shader:") + _debug_name);
}
