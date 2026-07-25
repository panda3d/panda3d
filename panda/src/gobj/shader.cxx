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
#include "bamCacheRecord.h"
#include "datagram.h"
#include "string_utils.h"
#include "shaderCompilerRegistry.h"
#include "shaderCompiler.h"
#include "shaderInputBinding.h"
#include "shaderPool.h"

using std::istream;
using std::ostream;
using std::ostringstream;
using std::string;

TypeHandle Shader::_type_handle;
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
  _language(lang)
{
}

/**
 * Delete the compiled code, if it exists.
 */
Shader::
~Shader() {
  release_all();
}

/**
 * Returns a copy of this shader that shares the compiled modules with the
 * original, but is independent of the ShaderPool: modifying the copy never
 * affects other users of the original shader, and vice versa.  The prepared
 * contexts are not copied.
 */
PT(Shader) Shader::
make_copy() const {
  PT(Shader) copy = new Shader(_language);
  copy->_parameters = _parameters;
  copy->_vertex_inputs = _vertex_inputs;
  copy->_matrix_cache_desc = _matrix_cache_desc;
  copy->_matrix_cache_deps = _matrix_cache_deps;
  copy->_error_flag = _error_flag;
  copy->_text = _text;
  copy->_modules = _modules;
  copy->_module_mask = _module_mask;
  copy->_used_caps = _used_caps;
  copy->_subsumes_alpha_test = _subsumes_alpha_test;
  copy->_debug = _debug;
  copy->_filename = _filename;
  copy->_fullpath = _fullpath;
  copy->_source_files = _source_files;
  copy->_compiled_format = _compiled_format;
  copy->_compiled_binary = _compiled_binary;
  copy->_prepare_shader_pcollector = _prepare_shader_pcollector;
  copy->_debug_name = _debug_name;
  return copy;
}

/**
 * Used by SpirVDebugOutputPass, this ensures that the given assertion is
 * registered.
 */
size_t Shader::DebugInfo::
add_assert(std::string expression, Filename fn, int line, Stage stage) {
  LightMutexHolder holder(_lock);
  size_t assert_id = 0;
  Assertion assertion{std::move(expression), std::move(fn), line, std::move(stage)};
  for (assert_id = 0; assert_id < _asserts.size(); ++assert_id) {
    if (_asserts[assert_id]._expression == assertion._expression &&
        _asserts[assert_id]._file == assertion._file &&
        _asserts[assert_id]._line == assertion._line &&
        _asserts[assert_id]._stage == assertion._stage) {
      break;
    }
  }
  nassertr(assert_id < _assert_count, 0);
  if (assert_id == _asserts.size()) {
    _asserts.push_back(std::move(assertion));
  }
  return assert_id;
}

/**
 * Returns how many bytes have been consumed.
 */
size_t Shader::DebugInfo::
process_buffer(const unsigned char *data, size_t size, const Filename &fn) {
  LightMutexHolder holder(_lock);
  if (_asserts.empty() && _strings.is_empty()) {
    // Nothing to do.
    return 0;
  }

  const uint32_t *begin = (const uint32_t *)data;
  const uint32_t *end = (const uint32_t *)data + size;

  const uint32_t *ptr = begin;
  uint32_t count = *ptr++;

  if (shader_cat.is_debug()) {
    shader_cat.debug()
      << "Shader " << fn << " debug print log contains " << count << " words\n";
  }

  // Per assertion we store a count of how many times it triggered, as well as
  // the invocation id (or the equivalent for this shader type).
  const DebugInfo::Assertion *first_assert = nullptr;
  for (size_t i = 0; i < _assert_count; ++i) {
    if (i >= _asserts.size()) {
      ptr += 4;
      continue;
    }
    const DebugInfo::Assertion &assert = _asserts[i];
    uint32_t assert_count = *ptr++;
    union {
      float f;
      int32_t i;
      uint32_t u;
    } invoc_x, invoc_y, invoc_z;
    invoc_x.u = *ptr++;
    invoc_y.u = *ptr++;
    invoc_z.u = *ptr++;

    if (assert_count > 0) {
      if (first_assert == nullptr) {
        first_assert = &assert;
      }

      auto &out = shader_cat.error()
        << "Shader assertion " << assert._expression << " at "
        << assert._file << ":" << assert._line << " failed "
        << assert_count << " times";

      switch (assert._stage) {
      case Stage::VERTEX:
        out << "; first observed at vertex " << invoc_x.i << ", instance " << invoc_y.i << "\n";
        break;

      case Stage::GEOMETRY:
      case Stage::TESS_CONTROL:
        out << "; first observed at primitive " << invoc_x.i << ", invocation " << invoc_y.i << "\n";
        break;

      case Stage::TESS_EVALUATION:
        out << "; first observed at primitive " << invoc_x.i << ", coord (" << invoc_y.f << ", " << invoc_z.f << ")\n";
        break;

      case Stage::FRAGMENT:
        out << "; first observed at fragment (" << invoc_x.f << ", " << invoc_y.f << ") depth " << invoc_z.f << "\n";
        break;

      case Stage::COMPUTE:
        out << "; first observed at invocation (" << invoc_x.u << ", " << invoc_y.u << ", " << invoc_z.u << ")\n";
        break;

      default:
        out << "\n";
        break;
      }
    }
  }

  // Now handle the print statements.  If the shader produced more output than
  // fits in the buffer, the GPU dropped the excess; warn the user that some
  // output was lost and how to capture it next time.
  bool truncated = count > (uint32_t)(end - ptr);
  if (truncated) {
    shader_cat.warning()
      << "Shader " << fn << " produced " << count
      << " words of debug output, but only " << (end - ptr)
      << " fit in the debug buffer; output was truncated.  Increase "
         "shader-debug-buffer-size to capture more.\n";
  }

  char buf[1024];

  end = std::min(end, ptr + count);
  while (ptr != end) {
    uint32_t index = *ptr++;

    // We deliberately reserved string id 0 so that we can catch shaders
    // having incremented the count but not written the string
    if (index == 0 || index - 1 >= _strings.size()) {
      if (index != 0 || !truncated) {
        shader_cat.error()
          << "Shader debug log is corrupt, encountered invalid string index " << index << "\n";
      }
      break;
    }
    const std::string &string = _strings.get_key(index - 1);

    auto &out = nout;

    const char *fmt = string.c_str();
    while (*fmt != 0) {
      char c = *fmt++;
      if (c != '%') {
        out.put(c);
        continue;
      }

      c = *fmt++;
      if (c == '%' || c == 0) {
        out.put(c);
        continue;
      }

      std::string format = "%";
      while (!isalpha(c)) {
        format += c;
        c = *fmt++;
      }
      format += c;

      union {
        float f;
        int32_t i;
        uint32_t u;
      } arg;
      arg.u = *ptr++;

      // Note that this string will never contain vector specifiers, those
      // were expanded to individual arguments by SpirVDebugOutputPass.
      if (c == 'u' || c == 'x' || c == 'X' || c == 'o') {
        // Unsigned integer
        snprintf(buf, sizeof(buf), format.c_str(), (unsigned int)arg.u);
      }
      else if (c == 'i' || c == 'd' || c == 'c') {
        // Signed integer
        snprintf(buf, sizeof(buf), format.c_str(), (int)arg.i);
      }
      else if (c == 'a' || c == 'A' || c == 'e' || c == 'E' || c == 'f' || c == 'F' || c == 'g' || c == 'G') {
        // Floating-point
        snprintf(buf, sizeof(buf), format.c_str(), (double)arg.f);
      }
      else if (c == 's') {
        // %s got converted to the appropriate type, except for bool, which
        // doesn't have its own format code.
        out << (arg.u ? "true" : "false");
        continue;
      }
      out << buf;
    }
    out.put('\n');
  }
  nout.flush();

  if (first_assert != nullptr) {
    Notify::ptr()->assert_failure(first_assert->_expression,
                                  first_assert->_line,
                                  first_assert->_file.c_str());
  }

  return ptr - begin;
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
 * Called by the back-end when the shader has compiled data available.
 */
void Shader::
set_compiled(unsigned int format, const char *data, size_t length) {
  _compiled_format = format;
  _compiled_binary.assign(data, length);
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

    Filename fullpath = vf->get_filename();
    if (record != nullptr) {
      record->add_dependent_file(vf);
    }

    SourceFile sf;
    sf._fullpath = fullpath;
    sf._timestamp = vf->get_timestamp();
    sf._size = vf->get_file_size();
    _source_files.push_back(std::move(sf));

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

    ShaderCompilerRegistry *registry = ShaderCompilerRegistry::get_global_ptr();
    ShaderCompiler *compiler = registry->get_compiler_for_language(SL_Cg);
    nassertr(compiler != nullptr, false);

    PT(ShaderModule) vertex = ShaderPool::make_module(compiler, Stage::VERTEX, source, fullpath, std::string_view(), options, nullptr, record);
    if (vertex == nullptr || !add_module(std::move(vertex))) {
      return false;
    }
    if (source.find("gshader") != string::npos) {
      PT(ShaderModule) geometry = ShaderPool::make_module(compiler, Stage::GEOMETRY, source, fullpath, std::string_view(), options, nullptr, record);
      if (geometry == nullptr || !add_module(std::move(geometry))) {
        return false;
      }
    }
    PT(ShaderModule) fragment = ShaderPool::make_module(compiler, Stage::FRAGMENT, source, fullpath, std::string_view(), options, nullptr, record);
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
    if (sbody._shared.find("gshader") != string::npos &&
        !do_load_source(Stage::GEOMETRY, sbody._shared, options, record)) {
      return false;
    }
    if (!do_load_source(Stage::FRAGMENT, sbody._shared, options, record)) {
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

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  PT(VirtualFile) vf = vfs->find_file(fn, get_model_path());
  if (vf == nullptr) {
    shader_cat.error()
      << "Could not find shader file: " << fn << "\n";
    return false;
  }

  // Note the file's current timestamp before we start compiling, so that
  // check_modified() errs on the side of caution if it is being modified
  // while we are reading it.
  SourceFile sf;
  sf._fullpath = vf->get_filename();
  sf._timestamp = vf->get_timestamp();
  sf._size = vf->get_file_size();

  PT(ShaderModule) module = ShaderPool::load_module(compiler, stage, sf._fullpath, std::string_view(), options, nullptr, record);
  if (!module) {
    return false;
  }
  nassertr(stage == module->get_stage(), false);

  if (!add_module(std::move(module))) {
    return false;
  }
  _source_files.push_back(std::move(sf));

  if (!_debug_name.empty()) {
    _debug_name += '/';
  }
  _debug_name += fn.get_basename();

  return true;
}

/**
 * Compiles the shader module from the given source string and adds it to the
 * shader.
 *
 * Returns false if there was an error with this shader bad enough to consider
 * it 'invalid'.
 */
bool Shader::
do_load_source(ShaderModule::Stage stage, std::string_view source,
               const CompilerOptions &options, BamCacheRecord *record) {
  ShaderCompilerRegistry *registry = ShaderCompilerRegistry::get_global_ptr();
  ShaderCompiler *compiler = registry->get_compiler_for_language(_language);
  nassertr(compiler != nullptr, false);

  PT(ShaderModule) module = ShaderPool::make_module(
    compiler, stage, source, Filename("created-shader"), std::string_view(),
    options, nullptr, record);
  if (!module) {
    return false;
  }
  nassertr(stage == module->get_stage(), false);

  return add_module(std::move(module));
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

  VertexInputBinding bind;
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

  _vertex_inputs.push_back(bind);
  return true;
}

/**
 * Checks whether any of the files that this shader was loaded from, or any
 * of the files that its compilation depended on, were modified on disk.
 */
bool Shader::
check_modified() const {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  // The source files for the modules themselves are not checked by
  // ShaderModule::check_dependents_modified(), since the module cache is
  // keyed by content hash, so different files may have the same ShaderModule.
  for (const SourceFile &sf : _source_files) {
    PT(VirtualFile) file = vfs->get_file(sf._fullpath);
    if (file == nullptr) {
      // File no longer exists.
      if (sf._timestamp != 0) {
        return true;
      }
    } else {
      if (file->get_timestamp() != sf._timestamp ||
          file->get_file_size() != sf._size) {
        return true;
      }
    }
  }

  for (const LinkedModule &linked_module : _modules) {
    const ShaderModule *module = linked_module._module.get_read_pointer();

    if (module->check_dependents_modified()) {
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

  bool use_cache = (options.get_flags() & CompilerOptions::F_no_ram_cache) == 0;

  ShaderPool::ShaderModules modules;
  if (use_cache) {
    VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
    PT(VirtualFile) vf = vfs->find_file(file, get_model_path());

    Filename fullpath = file;
    time_t timestamp = 0;
    std::streamsize size = 0;
    if (vf != nullptr) {
      fullpath = vf->get_filename();
      timestamp = vf->get_timestamp();
      size = vf->get_file_size();
    } else {
      // The shader may have been registered with ShaderPool::add_shader()
      // under a filename that does not exist on disk, which resolves the
      // filename in the same manner.
      vfs->resolve_filename(fullpath, get_model_path());
    }

    // If the modules of this file are memoized with an unchanged stat, we can
    // find the previously linked shader without even reading the file.
    PT(Shader) shader = ShaderPool::find_shader(lang, fullpath, timestamp, size, options);
    if (shader != nullptr) {
      return shader;
    }

    if (vf == nullptr) {
      shader_cat.error()
        << "Could not find shader file: " << file << "\n";
      return nullptr;
    }

    if (lang == SL_Cg || lang == SL_none) {
      // Compile the modules up front, so that the tuple of canonical module
      // pointers can identify a previously linked shader in the ShaderPool.
      // This requires reading the file, since for a single-file (Cg) shader
      // the set of stages depends on its contents; a modified file simply
      // produces different modules, and thereby a different shader.
      std::string source;
      if (!vf->read_file(source, true)) {
        shader_cat.error()
          << "Could not read shader file: " << file << "\n";
        return nullptr;
      }

      if (lang == SL_none && !has_cg_header(source)) {
        shader_cat.error()
          << "Unable to determine shader language of " << file << "\n";
        return nullptr;
      }
      lang = SL_Cg;

      ShaderCompilerRegistry *registry = ShaderCompilerRegistry::get_global_ptr();
      ShaderCompiler *compiler = registry->get_compiler_for_language(SL_Cg);
      nassertr(compiler != nullptr, nullptr);

      Stage stages[3] = {Stage::VERTEX, Stage::GEOMETRY, Stage::FRAGMENT};
      for (Stage stage : stages) {
        if (stage == Stage::GEOMETRY && source.find("gshader") == string::npos) {
          continue;
        }
        PT(ShaderModule) module = ShaderPool::make_module(
          compiler, stage, source, fullpath, std::string_view(), options);
        if (module == nullptr) {
          return nullptr;
        }
        modules.push_back(std::move(module));
      }

      // Also memoize the modules by filename, so that the next load of this
      // file takes the stat-only path above instead of rereading the file.
      ShaderPool::record_file_modules(fullpath, timestamp, size, options, modules);

      shader = ShaderPool::find_shader(lang, modules);
      if (shader != nullptr) {
        return shader;
      }
    }
  }

  PT(Shader) shader = new Shader(lang);
  if (!shader->read(sfile, options)) {
    return nullptr;
  }

  if (use_cache && !modules.empty()) {
    return ShaderPool::record_shader(modules, shader);
  }
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
  if (lang == SL_none) {
    shader_cat.error()
      << "No shader language was specified!\n";
    return nullptr;
  }

  ShaderFile sfile(vertex, fragment, geometry, tess_control, tess_evaluation);

  bool use_cache = (options.get_flags() & CompilerOptions::F_no_ram_cache) == 0;

  // Load the module for each stage first; on a repeat load, these come out
  // of the ShaderPool's module memo without any file being read, and the
  // tuple of canonical module pointers identifies the linked shader.
  ShaderPool::ShaderModules modules;
  if (use_cache) {
    ShaderCompilerRegistry *registry = ShaderCompilerRegistry::get_global_ptr();
    ShaderCompiler *compiler = registry->get_compiler_for_language(lang);
    nassertr(compiler != nullptr, nullptr);

    for (Stage stage : {Stage::VERTEX, Stage::TESS_CONTROL, Stage::TESS_EVALUATION, Stage::GEOMETRY, Stage::FRAGMENT, Stage::COMPUTE}) {
      const std::string &fn = sfile.get_stage(stage);
      if (!fn.empty()) {
        PT(ShaderModule) module = ShaderPool::load_module(
          compiler, stage, fn, std::string_view(), options);
        if (module == nullptr) {
          return nullptr;
        }
        modules.push_back(std::move(module));
      }
    }

    PT(Shader) shader = ShaderPool::find_shader(lang, modules);
    if (shader != nullptr) {
      return shader;
    }
  }

  PT(Shader) shader = new Shader(lang);
  if (!shader->read(sfile, options)) {
    return nullptr;
  }

  if (use_cache) {
    return ShaderPool::record_shader(modules, shader);
  }
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

  bool use_cache = (options.get_flags() & CompilerOptions::F_no_ram_cache) == 0;

  ShaderPool::ShaderModules modules;
  if (use_cache) {
    ShaderCompilerRegistry *registry = ShaderCompilerRegistry::get_global_ptr();
    ShaderCompiler *compiler = registry->get_compiler_for_language(lang);
    nassertr(compiler != nullptr, nullptr);

    PT(ShaderModule) module = ShaderPool::load_module(
      compiler, Stage::COMPUTE, fn, std::string_view(), options);
    if (module == nullptr) {
      return nullptr;
    }
    modules.push_back(std::move(module));

    PT(Shader) shader = ShaderPool::find_shader(lang, modules);
    if (shader != nullptr) {
      return shader;
    }
  }

  PT(Shader) shader = new Shader(lang);
  if (!shader->read(sfile, options)) {
    return nullptr;
  }
  shader->_fullpath = std::move(fullpath);

  if (use_cache) {
    return ShaderPool::record_shader(modules, shader);
  }
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

  // If cache-generated-shaders is disabled, disable caching of the shader
  // and its modules entirely.
  CompilerOptions new_options(options);
  if (!cache_generated_shaders) {
    new_options.set_flags(new_options.get_flags() | CompilerOptions::F_no_cache);
  }
  bool use_cache = (new_options.get_flags() & CompilerOptions::F_no_ram_cache) == 0;

  // Compile (or fetch from the module cache) each of the stages that make up
  // this single-file Cg shader, so that the tuple of canonical module
  // pointers can identify a previously linked shader in the ShaderPool.
  ShaderPool::ShaderModules modules;
  if (use_cache) {
    ShaderCompilerRegistry *registry = ShaderCompilerRegistry::get_global_ptr();
    ShaderCompiler *compiler = registry->get_compiler_for_language(lang);
    nassertr(compiler != nullptr, nullptr);

    Stage stages[3] = {Stage::VERTEX, Stage::GEOMETRY, Stage::FRAGMENT};
    for (Stage stage : stages) {
      if (stage == Stage::GEOMETRY &&
          sbody._shared.find("gshader") == string::npos) {
        continue;
      }
      PT(ShaderModule) module = ShaderPool::make_module(
        compiler, stage, sbody._shared, Filename("created-shader"),
        std::string_view(), new_options);
      if (module == nullptr) {
        return nullptr;
      }
      modules.push_back(std::move(module));
    }

    PT(Shader) shader = ShaderPool::find_shader(lang, modules);
    if (shader != nullptr) {
      return shader;
    }
  }

  PT(Shader) shader = new Shader(lang);
  if (!shader->load(sbody, new_options)) {
    return nullptr;
  }

  if (dump_generated_shaders) {
    ostringstream fns;
    int index = _shaders_generated ++;
    fns << "genshader" << index;
    string fn = fns.str();
    shader_cat.warning() << "Dumping shader: " << fn << "\n";

    pofstream s;
    s.open(fn.c_str(), std::ios::out | std::ios::trunc);
    s << sbody._shared;
    s.close();
  }

  if (use_cache) {
    return ShaderPool::record_shader(modules, shader);
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

  // If cache-generated-shaders is disabled, disable caching of the shader
  // and its modules entirely.
  CompilerOptions new_options(options);
  if (!cache_generated_shaders) {
    new_options.set_flags(new_options.get_flags() | CompilerOptions::F_no_cache);
  }
  bool use_cache = (new_options.get_flags() & CompilerOptions::F_no_ram_cache) == 0;

  // Compile (or fetch from the module cache) the module for each stage; the
  // tuple of canonical module pointers identifies a previously linked shader
  // in the ShaderPool, even one whose modules were loaded from files with
  // the same contents.
  ShaderPool::ShaderModules modules;
  if (use_cache) {
    ShaderCompilerRegistry *registry = ShaderCompilerRegistry::get_global_ptr();
    ShaderCompiler *compiler = registry->get_compiler_for_language(lang);
    nassertr(compiler != nullptr, nullptr);

    for (Stage stage : {Stage::VERTEX, Stage::TESS_CONTROL, Stage::TESS_EVALUATION, Stage::GEOMETRY, Stage::FRAGMENT, Stage::COMPUTE}) {
      const std::string &body = sbody.get_stage(stage);
      if (!body.empty()) {
        PT(ShaderModule) module = ShaderPool::make_module(
          compiler, stage, body, Filename("created-shader"),
          std::string_view(), new_options);
        if (module == nullptr) {
          return nullptr;
        }
        modules.push_back(std::move(module));
      }
    }

    PT(Shader) shader = ShaderPool::find_shader(lang, modules);
    if (shader != nullptr) {
      return shader;
    }
  }

  PT(Shader) shader = new Shader(lang);
  shader->_filename = ShaderFile("created-shader");
  if (!shader->load(sbody, new_options)) {
    return nullptr;
  }

  if (use_cache) {
    return ShaderPool::record_shader(modules, shader);
  }
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

  // If cache-generated-shaders is disabled, disable caching of the shader
  // and its modules entirely.
  CompilerOptions new_options(options);
  if (!cache_generated_shaders) {
    new_options.set_flags(new_options.get_flags() | CompilerOptions::F_no_cache);
  }
  bool use_cache = (new_options.get_flags() & CompilerOptions::F_no_ram_cache) == 0;

  ShaderPool::ShaderModules modules;
  if (use_cache) {
    ShaderCompilerRegistry *registry = ShaderCompilerRegistry::get_global_ptr();
    ShaderCompiler *compiler = registry->get_compiler_for_language(lang);
    nassertr(compiler != nullptr, nullptr);

    PT(ShaderModule) module = ShaderPool::make_module(
      compiler, Stage::COMPUTE, sbody._compute, Filename("created-shader"),
      std::string_view(), new_options);
    if (module == nullptr) {
      return nullptr;
    }
    modules.push_back(std::move(module));

    PT(Shader) shader = ShaderPool::find_shader(lang, modules);
    if (shader != nullptr) {
      return shader;
    }
  }

  PT(Shader) shader = new Shader(lang);
  shader->_filename = ShaderFile("created-shader");
  if (!shader->load(sbody, new_options)) {
    return nullptr;
  }

  if (use_cache) {
    return ShaderPool::record_shader(modules, shader);
  }
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
  uint32_t assert_count = module->_assert_count;

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
  _debug._assert_count += assert_count;
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

  uint64_t supported_caps = gsg->get_supported_shader_capabilities();
  uint64_t unsupported_caps = _used_caps & ~supported_caps;
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
      ShaderModule *module = DCAST(ShaderModule, p_list[pi++]);
      _debug._assert_count += module->_assert_count;
      add_module(module);
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

  _debug._assert_count = 0;
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
