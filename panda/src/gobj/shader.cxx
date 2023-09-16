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
#include "shaderModuleGlsl.h"
#include "shaderCompilerRegistry.h"
#include "shaderCompiler.h"

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
Shader(ShaderLanguage lang) :
  _mat_deps(0),
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
report_parameter_error(const InternalName *name, const ::ShaderType *type, const char *msg) {
  Filename fn = get_filename();
  shader_cat.error()
    << fn << ": " << *type << ' ' << *name << ": " << msg << "\n";
  return false;
}

/**
 * Make sure the provided parameter contains the specified number of words.
 * If not, print error message and return false.
 */
bool Shader::
expect_num_words(const InternalName *name, const ::ShaderType *type, size_t len) {
  const std::string &name_str = name->get_basename();
  size_t n = std::count(name_str.begin(), name_str.end(), '_');
  if (n + 1 != len) {
    std::string msg = "parameter name has wrong number of words, expected ";
    msg += format_string(len);
    return report_parameter_error(name, type, msg.c_str());
  }
  return true;
}

/**
 * Make sure the provided parameter has a floating point vector type.  If not,
 * print error message and return false.
 */
bool Shader::
expect_float_vector(const InternalName *name, const ::ShaderType *type, int lo, int hi) {
  int nfloat;
  ScalarType scalar_type = ScalarType::ST_unknown;
  if (const ::ShaderType::Scalar *scalar = type->as_scalar()) {
    nfloat = 1;
    scalar_type = scalar->get_scalar_type();
  }
  else if (const ::ShaderType::Vector *vector = type->as_vector()) {
    nfloat = vector->get_num_components();
    scalar_type = vector->get_scalar_type();
  }
  else {
    nfloat = 0;
  }
  if (scalar_type != ScalarType::ST_float || nfloat < lo || nfloat > hi) {
    return report_parameter_error(name, type, "expected floating-point vector");
  }
  return true;
}

/**
 * Make sure the provided parameter has a square floating-point matrix type.
 * Otherwise, prints an error message and returns false.
 */
bool Shader::
expect_float_matrix(const InternalName *name, const ::ShaderType *type, int lo, int hi) {
  ScalarType scalar_type = ScalarType::ST_unknown;
  uint32_t num_rows;
  uint32_t num_columns;
  if (const ::ShaderType::Matrix *matrix = type->as_matrix()) {
    num_rows = matrix->get_num_rows();
    num_columns = matrix->get_num_columns();
    scalar_type = matrix->get_scalar_type();
  }
  if ((scalar_type != ScalarType::ST_float && scalar_type != ScalarType::ST_double) ||
      (int)num_rows < lo || (int)num_rows > hi ||
      (int)num_columns < lo || (int)num_columns > hi) {

    std::string msg = "expected floating-point matrix of ";
    if (lo < hi) {
      msg += "at least ";
    }
    msg += "size ";
    msg += format_string(lo);
    return report_parameter_error(name, type, msg.c_str());
  }
  return true;
}

/**
 * Convert a single-word coordinate system name into a PART/ARG of a
 * ShaderMatSpec.
 */
bool Shader::
expect_coordinate_system(const InternalName *name, const ::ShaderType *type,
                         vector_string &pieces, int &next,
                         ShaderMatSpec &bind, bool fromflag) {

  if (pieces[next] == "" || pieces[next] == "to" || pieces[next] == "rel") {
    return report_parameter_error(name, type, "invalid coordinate system name");
  }

  string word1 = pieces[next++];
  if (pieces[next] == "of") {
    next++;
  }
  string word2;
  if (pieces[next] != "" && pieces[next] != "to" && pieces[next] != "rel") {
    word2 = pieces[next++];
  }

  ShaderMatInput from_single;
  ShaderMatInput from_double;
  ShaderMatInput to_single;
  ShaderMatInput to_double;

  if (word1 == "world") {
    from_single = SMO_world_to_view;
    from_double = SMO_INVALID;
    to_single   = SMO_view_to_world;
    to_double   = SMO_INVALID;
  }
  else if (word1 == "model") {
    from_single = SMO_model_to_view;
    from_double = SMO_view_x_to_view;
    to_single   = SMO_view_to_model;
    to_double   = SMO_view_to_view_x;
  }
  else if (word1 == "clip") {
    from_single = SMO_clip_to_view;
    from_double = SMO_clip_x_to_view;
    to_single   = SMO_view_to_clip;
    to_double   = SMO_view_to_clip_x;
  }
  else if (word1 == "view") {
    from_single = SMO_identity;
    from_double = SMO_view_x_to_view;
    to_single   = SMO_identity;
    to_double   = SMO_view_to_view_x;
  }
  else if (word1 == "apiview") {
    from_single = SMO_apiview_to_view;
    from_double = SMO_apiview_x_to_view;
    to_single   = SMO_view_to_apiview;
    to_double   = SMO_view_to_apiview_x;
  }
  else if (word1 == "apiclip") {
    from_single = SMO_apiclip_to_view;
    from_double = SMO_apiclip_x_to_view;
    to_single   = SMO_view_to_apiclip;
    to_double   = SMO_view_to_apiclip_x;
  }
  else {
    from_single = SMO_view_x_to_view;
    from_double = SMO_view_x_to_view;
    to_single   = SMO_view_to_view_x;
    to_double   = SMO_view_to_view_x;
    word2 = word1;
  }

  if (fromflag) {
    if (word2 == "") {
      bind._part[0] = from_single;
      bind._arg[0] = nullptr;
    } else {
      if (from_double == SMO_INVALID) {
        return report_parameter_error(name, type, "invalid coordinate system name");
      }
      bind._part[0] = from_double;
      bind._arg[0] = InternalName::make(word2);
    }
  } else {
    if (word2 == "") {
      bind._part[1] = to_single;
      bind._arg[1] = nullptr;
    } else {
      if (to_double == SMO_INVALID) {
        return report_parameter_error(name, type, "invalid coordinate system name");
      }
      bind._part[1] = to_double;
      bind._arg[1] = InternalName::make(word2);
    }
  }
  return true;
}

/**
 * Given ShaderMatInput, returns an indication of what part or parts of the
 * state_and_transform the ShaderMatInput depends upon.
 */
int Shader::
cp_dependency(ShaderMatInput inp) {

  int dep = SSD_general;

  if (inp == SMO_INVALID) {
    return SSD_NONE;
  }
  if (inp == SMO_attr_material || inp == SMO_attr_material2) {
    dep |= SSD_material | SSD_frame;
  }
  if (inp == SMO_attr_color || inp == SMO_attr_material2) {
    dep |= SSD_color;
  }
  if (inp == SMO_attr_colorscale) {
    dep |= SSD_colorscale;
  }
  if (inp == SMO_attr_fog || inp == SMO_attr_fogcolor) {
    dep |= SSD_fog | SSD_frame;
  }
  if ((inp == SMO_model_to_view) ||
      (inp == SMO_view_to_model) ||
      (inp == SMO_model_to_apiview) ||
      (inp == SMO_apiview_to_model)) {
    dep |= SSD_transform;
  }
  if ((inp == SMO_view_to_world) ||
      (inp == SMO_world_to_view) ||
      (inp == SMO_view_x_to_view) ||
      (inp == SMO_view_to_view_x) ||
      (inp == SMO_apiview_x_to_view) ||
      (inp == SMO_view_to_apiview_x) ||
      (inp == SMO_clip_x_to_view) ||
      (inp == SMO_view_to_clip_x) ||
      (inp == SMO_apiclip_x_to_view) ||
      (inp == SMO_view_to_apiclip_x) ||
      (inp == SMO_dlight_x) ||
      (inp == SMO_plight_x) ||
      (inp == SMO_slight_x)) {
    dep |= SSD_view_transform;
  }
  if ((inp == SMO_texpad_x) ||
      (inp == SMO_texpix_x) ||
      (inp == SMO_alight_x) ||
      (inp == SMO_dlight_x) ||
      (inp == SMO_plight_x) ||
      (inp == SMO_slight_x) ||
      (inp == SMO_satten_x) ||
      (inp == SMO_mat_constant_x) ||
      (inp == SMO_mat_constant_x_attrib) ||
      (inp == SMO_vec_constant_x) ||
      (inp == SMO_vec_constant_x_attrib) ||
      (inp == SMO_view_x_to_view) ||
      (inp == SMO_view_to_view_x) ||
      (inp == SMO_apiview_x_to_view) ||
      (inp == SMO_view_to_apiview_x) ||
      (inp == SMO_clip_x_to_view) ||
      (inp == SMO_view_to_clip_x) ||
      (inp == SMO_apiclip_x_to_view) ||
      (inp == SMO_view_to_apiclip_x)) {
    dep |= SSD_shaderinputs;

    if ((inp == SMO_texpad_x) ||
        (inp == SMO_texpix_x) ||
        (inp == SMO_alight_x) ||
        (inp == SMO_dlight_x) ||
        (inp == SMO_plight_x) ||
        (inp == SMO_slight_x) ||
        (inp == SMO_satten_x) ||
        (inp == SMO_mat_constant_x) ||
        (inp == SMO_mat_constant_x_attrib) ||
        (inp == SMO_vec_constant_x_attrib) ||
        (inp == SMO_view_x_to_view) ||
        (inp == SMO_view_to_view_x) ||
        (inp == SMO_apiview_x_to_view) ||
        (inp == SMO_view_to_apiview_x) ||
        (inp == SMO_clip_x_to_view) ||
        (inp == SMO_view_to_clip_x) ||
        (inp == SMO_apiclip_x_to_view) ||
        (inp == SMO_view_to_apiclip_x)) {
      // We can't track changes to these yet, so we have to assume that they
      // are modified every frame.
      dep |= SSD_frame;
    }
  }
  if ((inp == SMO_light_ambient) ||
      (inp == SMO_light_source_i_attrib) ||
      (inp == SMO_light_source_i_packed)) {
    dep |= SSD_light | SSD_frame;
  }
  if (inp == SMO_light_source_i_attrib ||
      inp == SMO_light_source_i_packed ||
      inp == SMO_mat_constant_x_attrib ||
      inp == SMO_vec_constant_x_attrib) {
    // Some light attribs (eg. position) need to be transformed to view space.
    dep |= SSD_view_transform;
  }
  if ((inp == SMO_light_product_i_ambient) ||
      (inp == SMO_light_product_i_diffuse) ||
      (inp == SMO_light_product_i_specular)) {
    dep |= (SSD_light | SSD_material);
  }
  if ((inp == SMO_clipplane_x) ||
      (inp == SMO_apiview_clipplane_i)) {
    dep |= SSD_clip_planes;
  }
  if (inp == SMO_texmat_i || inp == SMO_inv_texmat_i || inp == SMO_texscale_i) {
    dep |= SSD_tex_matrix;
  }
  if ((inp == SMO_window_size) ||
      (inp == SMO_pixel_size) ||
      (inp == SMO_frame_number) ||
      (inp == SMO_frame_time) ||
      (inp == SMO_frame_delta)) {
    dep |= SSD_frame;
  }
  if ((inp == SMO_clip_to_view) ||
      (inp == SMO_view_to_clip) ||
      (inp == SMO_apiclip_to_view) ||
      (inp == SMO_view_to_apiclip) ||
      (inp == SMO_apiview_to_apiclip) ||
      (inp == SMO_apiclip_to_apiview)) {
    dep |= SSD_projection;
  }
  if (inp == SMO_tex_is_alpha_i || inp == SMO_texcolor_i) {
    dep |= SSD_texture | SSD_frame;
  }
  if (inp == SMO_texconst_i) {
    dep |= SSD_tex_gen;
  }
  if (inp == SMO_attr_pointparams) {
    dep |= SSD_render_mode | SSD_transform | SSD_frame;
  }

  return dep;
}

/**
 * Adds the given ShaderMatSpec to the shader's mat spec table.
 */
void Shader::
cp_add_mat_spec(ShaderMatSpec &spec) {
  // We currently expect each ShaderMatSpec to map to one location.
  nassertv(spec._id._type->get_num_parameter_locations() == 1);

  // If we're composing with identity, simplify.

  if (spec._func == SMF_first) {
    spec._part[1] = SMO_INVALID;
    spec._arg[1] = nullptr;
  }
  if (spec._func == SMF_compose) {
    if (spec._part[1] == SMO_identity) {
      spec._func = SMF_first;
    }
  }
  if (spec._func == SMF_compose) {
    if (spec._part[0] == SMO_identity) {
      spec._func = SMF_first;
      spec._part[0] = spec._part[1];
      spec._arg[0] = spec._arg[1];
    }

    // More optimal combinations for common matrices.

    if (spec._part[0] == SMO_model_to_view &&
        spec._part[1] == SMO_view_to_apiclip) {
      spec._part[0] = SMO_model_to_apiview;
      spec._part[1] = SMO_apiview_to_apiclip;

    } else if (spec._part[0] == SMO_apiclip_to_view &&
               spec._part[1] == SMO_view_to_model) {
      spec._part[0] = SMO_apiclip_to_apiview;
      spec._part[1] = SMO_apiview_to_model;

    } else if (spec._part[0] == SMO_apiview_to_view &&
               spec._part[1] == SMO_view_to_apiclip) {
      spec._func = SMF_first;
      spec._part[0] = SMO_apiview_to_apiclip;
      spec._part[1] = SMO_identity;

    } else if (spec._part[0] == SMO_apiclip_to_view &&
               spec._part[1] == SMO_view_to_apiview) {
      spec._func = SMF_first;
      spec._part[0] = SMO_apiclip_to_apiview;
      spec._part[1] = SMO_identity;

    } else if (spec._part[0] == SMO_apiview_to_view &&
               spec._part[1] == SMO_view_to_model) {
      spec._func = SMF_first;
      spec._part[0] = SMO_apiview_to_model;
      spec._part[1] = SMO_identity;

    } else if (spec._part[0] == SMO_model_to_view &&
               spec._part[1] == SMO_view_to_apiview) {
      spec._func = SMF_first;
      spec._part[0] = SMO_model_to_apiview;
      spec._part[1] = SMO_identity;
    }
  }

  // If we're compiling a Cg shader, transpose the matrices, to account for the
  // differing matrix convention.
  if (_language == SL_Cg) {
    switch (spec._piece) {
    case SMP_whole: spec._piece = SMP_transpose; break;
    case SMP_transpose: spec._piece = SMP_whole; break;
    case SMP_upper3x3: spec._piece = SMP_transpose3x3; break;
    case SMP_transpose3x3: spec._piece = SMP_upper3x3; break;
    case SMP_upper3x4: spec._piece = SMP_transpose3x4; break;
    case SMP_transpose3x4: spec._piece = SMP_upper3x4; break;
    case SMP_upper4x3: spec._piece = SMP_transpose4x3; break;
    case SMP_transpose4x3: spec._piece = SMP_upper4x3; break;
    default: break;
    }
  }

  // Determine which part is an array, for determining which one the count and
  // index refer to.  (It can't be the case that both parts are arrays.)
  int begin[2] = {0, 0};
  int end[2] = {1, 1};
  if (spec._index > 0) {
    for (int i = 0; i < 2; ++i) {
      if (spec._part[i] == SMO_texmat_i ||
          spec._part[i] == SMO_inv_texmat_i ||
          spec._part[i] == SMO_light_source_i_attrib ||
          spec._part[i] == SMO_light_product_i_ambient ||
          spec._part[i] == SMO_light_product_i_diffuse ||
          spec._part[i] == SMO_light_product_i_specular ||
          spec._part[i] == SMO_apiview_clipplane_i ||
          spec._part[i] == SMO_tex_is_alpha_i ||
          spec._part[i] == SMO_transform_i ||
          spec._part[i] == SMO_slider_i ||
          spec._part[i] == SMO_light_source_i_packed ||
          spec._part[i] == SMO_texscale_i ||
          spec._part[i] == SMO_texcolor_i) {
        begin[i] = spec._index;
        end[i] = spec._index + 1;
      }
    }
    nassertv(end[0] == 1 || end[1] == 1);
  }

  // Make sure that we have a place in the part cache for both parts.
  int num_parts = (spec._func != SMF_first) ? 2 : 1;

  for (int p = 0; p < num_parts; ++p) {
    int dep = cp_dependency(spec._part[p]);
    spec._dep |= dep;

    // Do we already have a spot in the cache for this part?
    size_t i;
    size_t offset = 0;
    for (i = 0; i < _mat_parts.size(); ++i) {
      ShaderMatPart &part = _mat_parts[i];
      if (part._part == spec._part[p] && part._arg == spec._arg[p]) {
        int diff = end[p] - part._count;
        if (diff <= 0) {
          // The existing cache entry is big enough.
          break;
        } else {
          // It's not big enough.  Enlarge it, which means we have to change the
          // offset of some of the other spec entries.
          for (ShaderMatSpec &spec : _mat_spec) {
            if (spec._cache_offset[0] >= offset + part._count) {
              spec._cache_offset[0] += diff;
            }
            if (spec._cache_offset[1] >= offset + part._count) {
              spec._cache_offset[1] += diff;
            }
          }
          part._count = end[p];
          break;
        }
      }
      offset += part._count;
    }
    if (i == _mat_parts.size()) {
      // Didn't find this part yet, create a new one.
      ShaderMatPart part;
      part._part = spec._part[p];
      part._count = end[p];
      part._arg = spec._arg[p];
      part._dep = dep;
      _mat_parts.push_back(std::move(part));
    }
    spec._cache_offset[p] = offset + begin[p];
  }

  _mat_spec.push_back(spec);
  _mat_deps |= spec._dep;
}

/**
 * Returns the total size of the matrix part cache.
 */
size_t Shader::
cp_get_mat_cache_size() const {
  size_t size = 0;
  for (const ShaderMatPart &part : _mat_parts) {
    size += part._count;
  }
  return size;
}

/**
 *
 */
void Shader::
clear_parameters() {
  _mat_spec.clear();
  _var_spec.clear();
  _tex_spec.clear();
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
read(const ShaderFile &sfile, BamCacheRecord *record) {
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
        !do_read_source(Stage::vertex, sfile._vertex, record)) {
      return false;
    }
    if (!sfile._tess_control.empty() &&
        !do_read_source(Stage::tess_control, sfile._tess_control, record)) {
      return false;
    }
    if (!sfile._tess_evaluation.empty() &&
        !do_read_source(Stage::tess_evaluation, sfile._tess_evaluation, record)) {
      return false;
    }
    if (!sfile._geometry.empty() &&
        !do_read_source(Stage::geometry, sfile._geometry, record)) {
      return false;
    }
    if (!sfile._fragment.empty() &&
        !do_read_source(Stage::fragment, sfile._fragment, record)) {
      return false;
    }
    if (!sfile._compute.empty() &&
        !do_read_source(Stage::compute, sfile._compute, record)) {
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

    ShaderCompiler *compiler = get_compiler(SL_Cg);
    nassertr(compiler != nullptr, false);

    std::istringstream in(source);

    PT(ShaderModule) module;
    module = compiler->compile_now(Stage::vertex, in, fullpath, record);
    if (module == nullptr || !add_module(std::move(module))) {
      return false;
    }
    if (source.find("gshader") != string::npos) {
      in.clear();
      in.seekg(0);
      module = compiler->compile_now(Stage::geometry, in, fullpath, record);
      if (module == nullptr || !add_module(std::move(module))) {
        return false;
      }
    }
    in.clear();
    in.seekg(0);
    module = compiler->compile_now(Stage::fragment, in, fullpath, record);
    if (module == nullptr || !add_module(std::move(module))) {
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
load(const ShaderFile &sbody, BamCacheRecord *record) {
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
        !do_load_source(Stage::vertex, sbody._vertex, record)) {
      return false;
    }
    if (!sbody._tess_control.empty() &&
        !do_load_source(Stage::tess_control, sbody._tess_control, record)) {
      return false;
    }
    if (!sbody._tess_evaluation.empty() &&
        !do_load_source(Stage::tess_evaluation, sbody._tess_evaluation, record)) {
      return false;
    }
    if (!sbody._geometry.empty() &&
        !do_load_source(Stage::geometry, sbody._geometry, record)) {
      return false;
    }
    if (!sbody._fragment.empty() &&
        !do_load_source(Stage::fragment, sbody._fragment, record)) {
      return false;
    }
    if (!sbody._compute.empty() &&
        !do_load_source(Stage::compute, sbody._compute, record)) {
      return false;
    }

  } else if (_language == SL_Cg || _language == SL_none) {
    if (_language == SL_none && !has_cg_header(sbody._shared)) {
      shader_cat.error()
        << "Unable to determine shader language of created-shader\n";
      return false;
    }
    _language = SL_Cg;

    if (!do_load_source(Stage::vertex, sbody._shared, record)) {
      return false;
    }
    if (!do_load_source(Stage::fragment, sbody._shared, record)) {
      return false;
    }
    if (sbody._shared.find("gshader") != string::npos &&
        !do_load_source(Stage::geometry, sbody._shared, record)) {
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
do_read_source(Stage stage, const Filename &fn, BamCacheRecord *record) {
  ShaderCompiler *compiler = get_compiler(_language);
  nassertr(compiler != nullptr, false);

  PT(ShaderModule) module = compiler->compile_now(stage, fn, record);
  if (!module) {
    return false;
  }
  nassertr(stage == module->get_stage(), false);

  if (!add_module(module)) {
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
               const Filename &fullpath, BamCacheRecord *record) {
  ShaderCompiler *compiler = get_compiler(_language);
  nassertr(compiler != nullptr, false);

  PT(ShaderModule) module = compiler->compile_now(stage, in, fullpath, record);
  if (!module) {
    return false;
  }
  nassertr(stage == module->get_stage(), false);

  return add_module(module);
}

/**
 * Loads the shader file from the given string into the given string,
 * performing any pre-processing on it that may be necessary.
 *
 * Returns false if there was an error with this shader bad enough to consider
 * it 'invalid'.
 */
bool Shader::
do_load_source(ShaderModule::Stage stage, const std::string &source, BamCacheRecord *record) {
  std::istringstream in(source);
  return do_read_source(stage, in, Filename("created-shader"), record);
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
  BitArray used_locations;

  pmap<CPT_InternalName, const ::ShaderType *> spec_const_types;

  for (LinkedModule &linked_module : _modules) {
    const ShaderModule *module = linked_module._module.get_read_pointer();
    pmap<int, int> remap;

    for (const ShaderModule::Variable &var : module->_parameters) {
      Parameter param;
      param._name = var.name;
      param._type = var.type;
      param._location = var._location;
      param._stage_mask = (1 << (int)module->get_stage());

      auto result = parameters_by_name.insert({var.name, param});
      auto &it = result.first;

      if (!result.second) {
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
          if (other._location != param._location) {
            // Different location; need to remap this.
            remap[param._location] = other._location;
          }
          other._stage_mask |= param._stage_mask;
          continue;
        }
      }

      if (var.has_location()) {
        // Check whether the locations occupied by this variable are already in
        // use by another stage.
        int num_locations = var.type->get_num_parameter_locations();
        if (used_locations.has_any_of(var.get_location(), num_locations)) {
          // This location is already used.  Find another free location.
          int location = used_locations.find_off_range(num_locations);
          nassertr(location >= 0, false);
          used_locations.set_range(location, num_locations);
          remap[var.get_location()] = location;
          it->second._location = location;
        } else {
          used_locations.set_range(var.get_location(), num_locations);
        }
      }
      parameters.push_back(&(it->second));
    }

    if (!remap.empty()) {
      // We need to remap some locations.  Grab a writable pointer.  This will
      // make a unique copy of the module if it's also used by other shaders.
      PT(ShaderModule) module = linked_module._module.get_write_pointer();
      if (shader_cat.is_debug()) {
        std::ostream &out = shader_cat.debug()
          << "Remapping parameter locations for module " << *module << ":";

        pmap<int, int>::const_iterator it;
        for (it = remap.begin(); it != remap.end(); ++it) {
          out << ' ' << it->first << "->" << it->second;
        }
        out << "\n";
      }
      module->remap_parameter_locations(remap);
    }

    for (const ShaderModule::SpecializationConstant &spec_const : module->_spec_constants) {
      auto result = spec_const_types.insert({spec_const.name, spec_const.type});
      auto &it = result.first;

      if (!result.second) {
        // Another module has already defined a spec constant with this name.
        // Make sure they have the same type.
        const ::ShaderType *other_type = it->second;
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
    if (!bind_parameter(*param)) {
      success = false;
    }
  }

  return success;
}

/**
 * Binds a vertex input parameter with the given type.
 */
bool Shader::
bind_vertex_input(const InternalName *name, const ::ShaderType *type, int location) {
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
 * Binds a uniform parameter with the given type.
 */
bool Shader::
bind_parameter(const Parameter &param) {
  CPT(InternalName) name = param._name;
  const ::ShaderType *type = param._type;
  std::string name_str = name->get_name();

  // If this is an empty struct, we bind the individual members.
  const ::ShaderType::Struct *struct_type = type->as_struct();
  if (struct_type != nullptr && name_str.empty()) {
    bool success = true;

    int location = param._location;

    for (size_t i = 0; i < struct_type->get_num_members(); ++i) {
      const ::ShaderType::Struct::Member &member = struct_type->get_member(i);

      // Recurse.
      Parameter member_param(param);
      member_param._name = member.name;
      member_param._type = member.type;
      member_param._location = location;
      if (!bind_parameter(member_param)) {
        success = false;
      }

      location += member.type->get_num_parameter_locations();
    }

    return success;
  }

  if (shader_cat.is_debug()) {
    int num_locations = type->get_num_parameter_locations();
    if (num_locations < 2) {
      shader_cat.debug()
        << "Binding parameter " << name_str << " with type " << *type
        << " (location=" << param._location << ")\n";
    } else {
      shader_cat.debug()
        << "Binding parameter " << name_str << " with type " << *type
        << " (location=" << param._location << ".."
        << param._location + num_locations - 1 << ")\n";
    }
  }

  // Split it at the underscores.
  vector_string pieces;
  tokenize(name_str, pieces, "_");
  nassertr(!pieces.empty(), false);

  // Check if it has a p3d_ prefix - if so, assign special meaning.
  if (pieces[0] == "p3d" && _language == SL_GLSL) {
    // Check for matrix inputs.
    bool transpose = false;
    bool inverse = false;
    string matrix_name = pieces[1];

    // Check for and chop off any "Transpose" or "Inverse" suffix.
    if (matrix_name.size() > 6 + 9 &&
        matrix_name.compare(matrix_name.size() - 9, 9, "Transpose") == 0) {
      transpose = true;
      matrix_name = matrix_name.substr(0, matrix_name.size() - 9);
    }
    if (matrix_name.size() > 6 + 7 &&
        matrix_name.compare(matrix_name.size() - 7, 7, "Inverse") == 0) {
      inverse = true;
      matrix_name = matrix_name.substr(0, matrix_name.size() - 7);
    }

    // Now if the suffix that is left over is "Matrix", we know that it is
    // supposed to be a matrix input.
    if (matrix_name.size() > 6 &&
        matrix_name.compare(matrix_name.size() - 6, 6, "Matrix") == 0) {

      ShaderMatSpec bind;
      bind._id = param;
      bind._func = SMF_compose;
      bind._arg[0] = nullptr;
      bind._arg[1] = nullptr;

      if (!expect_float_matrix(name, type, 3, 4)) {
        return false;
      }

      const ::ShaderType::Matrix *matrix = type->as_matrix();
      if (matrix->get_num_rows() >= 4) {
        if (matrix->get_num_columns() >= 4) {
          bind._piece = transpose ? SMP_transpose : SMP_whole;
        } else {
          bind._piece = transpose ? SMP_transpose4x3 : SMP_upper4x3;
        }
      } else if (matrix->get_num_columns() >= 4) {
        bind._piece = transpose ? SMP_transpose3x4 : SMP_upper3x4;
      } else {
        bind._piece = transpose ? SMP_upper3x3 : SMP_transpose3x3;
      }
      bind._scalar_type = matrix->get_scalar_type();

      if (matrix_name == "ModelViewProjectionMatrix") {
        if (inverse) {
          bind._part[0] = SMO_apiclip_to_apiview;
          bind._part[1] = SMO_apiview_to_model;
        } else {
          bind._part[0] = SMO_model_to_apiview;
          bind._part[1] = SMO_apiview_to_apiclip;
        }
      }
      else if (matrix_name == "ModelViewMatrix") {
        bind._func = SMF_first;
        bind._part[0] = inverse ? SMO_apiview_to_model
                                : SMO_model_to_apiview;
        bind._part[1] = SMO_identity;
      }
      else if (matrix_name == "ProjectionMatrix") {
        bind._func = SMF_first;
        bind._part[0] = inverse ? SMO_apiclip_to_apiview
                                : SMO_apiview_to_apiclip;
        bind._part[1] = SMO_identity;
      }
      else if (matrix_name == "NormalMatrix") {
        // This is really the upper 3x3 of the ModelViewMatrixInverseTranspose.
        bind._func = SMF_first;
        bind._part[0] = inverse ? SMO_model_to_apiview
                                : SMO_apiview_to_model;
        bind._part[1] = SMO_identity;

        if (!expect_float_matrix(name, type, 3, 3)) {
          return false;
        }
      }
      else if (matrix_name == "ModelMatrix") {
        if (inverse) {
          bind._part[0] = SMO_world_to_view;
          bind._part[1] = SMO_view_to_model;
        } else {
          bind._part[0] = SMO_model_to_view;
          bind._part[1] = SMO_view_to_world;
        }
      }
      else if (matrix_name == "ViewMatrix") {
        if (inverse) {
          bind._part[0] = SMO_apiview_to_view;
          bind._part[1] = SMO_view_to_world;
        } else {
          bind._part[0] = SMO_world_to_view;
          bind._part[1] = SMO_view_to_apiview;
        }
      }
      else if (matrix_name == "ViewProjectionMatrix") {
        if (inverse) {
          bind._part[0] = SMO_apiclip_to_view;
          bind._part[1] = SMO_view_to_world;
        } else {
          bind._part[0] = SMO_world_to_view;
          bind._part[1] = SMO_view_to_apiclip;
        }
      }
      else if (matrix_name == "TextureMatrix") {
        // We may support 2-D texmats later, but let's make sure that people
        // don't think they can just use a mat3 to get the 2-D version.
        if (!expect_float_matrix(name, type, 4, 4)) {
          return false;
        }

        bind._func = SMF_first;
        bind._part[0] = inverse ? SMO_inv_texmat_i
                                : SMO_texmat_i;
        bind._part[1] = SMO_identity;

        // Add it once for each index.
//        for (bind._index = 0; bind._index < param_size; ++bind._index) {
//          // It was discovered in #846, that GLSL 4.10 and lower don't seem to
//          // guarantee that matrices occupy successive locations, and on macOS
//          // they indeed occupy four locations per element.
//          // As a big fat hack, we multiply by four on macOS, because this is
//          // hard to fix on the 1.10 branch.  We'll have a proper fix on the
//          // master branch.
//#ifdef __APPLE__
//          bind._id._location = p + bind._index * 4;
//#else
//          bind._id._location = p + bind._index;
//#endif
//          cp_add_mat_spec(bind);
//        }
      }
      else {
        return report_parameter_error(name, type, "unrecognized matrix name");
      }

      cp_add_mat_spec(bind);
      return true;
    }
    if (pieces[1].compare(0, 7, "Texture") == 0) {
      ShaderTexSpec bind;
      bind._id = param;

      const ::ShaderType *element_type;
      uint32_t num_elements;
      type->unwrap_array(element_type, num_elements);

      const ::ShaderType::SampledImage *sampled_image_type = element_type->as_sampled_image();
      if (sampled_image_type == nullptr) {
        return report_parameter_error(name, type, "expected sampled image");
      }
      bind._desired_type = sampled_image_type->get_texture_type();

      if (pieces[1].size() > 7 && isdigit(pieces[1][7])) {
        // p3d_Texture0, p3d_Texture1, etc.
        bind._part = Shader::STO_stage_i;

        string tail;
        bind._stage = string_to_int(pieces[1].substr(7), tail);
        if (!tail.empty()) {
          string msg = "unexpected '" + tail + "'";
          return report_parameter_error(name, type, msg.c_str());
        }

        _tex_spec.push_back(bind);
      }
      else {
        // p3d_Texture[] or p3d_TextureModulate[], etc.
        if (pieces[1].size() == 7) {
          bind._part = Shader::STO_stage_i;
        }
        else if (pieces[1].compare(7, string::npos, "FF") == 0) {
          bind._part = Shader::STO_ff_stage_i;
        }
        else if (pieces[1].compare(7, string::npos, "Modulate") == 0) {
          bind._part = Shader::STO_stage_modulate_i;
        }
        else if (pieces[1].compare(7, string::npos, "Add") == 0) {
          bind._part = Shader::STO_stage_add_i;
        }
        else if (pieces[1].compare(7, string::npos, "Normal") == 0) {
          bind._part = Shader::STO_stage_normal_i;
        }
        else if (pieces[1].compare(7, string::npos, "Height") == 0) {
          bind._part = Shader::STO_stage_height_i;
        }
        else if (pieces[1].compare(7, string::npos, "Selector") == 0) {
          bind._part = Shader::STO_stage_selector_i;
        }
        else if (pieces[1].compare(7, string::npos, "Gloss") == 0) {
          bind._part = Shader::STO_stage_gloss_i;
        }
        else if (pieces[1].compare(7, string::npos, "Emission") == 0) {
          bind._part = Shader::STO_stage_emission_i;
        }
        else {
          return report_parameter_error(name, type, "unrecognized parameter name");
        }

        for (bind._stage = 0; bind._stage < num_elements; ++bind._stage) {
          _tex_spec.push_back(bind);
        }
      }
      return true;
    }
    if (pieces[1] == "Material") {
      ShaderMatSpec bind;
      bind._id = param;
      bind._func = SMF_first;
      bind._part[0] = SMO_attr_material;
      bind._arg[0] = nullptr;
      bind._part[1] = SMO_identity;
      bind._arg[1] = nullptr;

      const ::ShaderType::Struct *struct_type = type->as_struct();
      if (struct_type == nullptr) {
        return report_parameter_error(name, type, "expected struct");
      }

      bool success = true;
      for (size_t i = 0; i < struct_type->get_num_members(); ++i) {
        const ::ShaderType::Struct::Member &member = struct_type->get_member(i);

        CPT(InternalName) fqname = ((InternalName *)name.p())->append(member.name);
        bind._id._location = param._location + i;
        bind._id._name = fqname->get_name();
        bind._id._type = member.type;

        if (member.name == "baseColor") {
          if (expect_float_vector(fqname, member.type, 4, 4)) {
            bind._part[0] = SMO_attr_material2;
            bind._piece = SMP_row0;
            cp_add_mat_spec(bind);
            continue;
          }
        } else if (member.name == "ambient") {
          if (expect_float_vector(fqname, member.type, 4, 4)) {
            bind._part[0] = SMO_attr_material;
            bind._piece = SMP_row0;
            cp_add_mat_spec(bind);
            continue;
          }
        } else if (member.name == "diffuse") {
          if (expect_float_vector(fqname, member.type, 4, 4)) {
            bind._part[0] = SMO_attr_material;
            bind._piece = SMP_row1;
            cp_add_mat_spec(bind);
            continue;
          }
        } else if (member.name == "emission") {
          if (expect_float_vector(fqname, member.type, 4, 4)) {
            bind._part[0] = SMO_attr_material;
            bind._piece = SMP_row2;
            cp_add_mat_spec(bind);
            continue;
          }
        } else if (member.name == "specular") {
          if (expect_float_vector(fqname, member.type, 3, 3)) {
            bind._part[0] = SMO_attr_material;
            bind._piece = SMP_row3x3;
            cp_add_mat_spec(bind);
            continue;
          }
        } else if (member.name == "shininess") {
          if (expect_float_vector(fqname, member.type, 1, 1)) {
            bind._part[0] = SMO_attr_material;
            bind._piece = SMP_cell15;
            cp_add_mat_spec(bind);
            continue;
          }
        } else if (member.name == "roughness") {
          if (expect_float_vector(fqname, member.type, 1, 1)) {
            bind._part[0] = SMO_attr_material2;
            bind._piece = SMP_cell15;
            cp_add_mat_spec(bind);
            continue;
          }
        } else if (member.name == "metallic") {
          if (member.type == ::ShaderType::bool_type ||
              member.type == ::ShaderType::float_type) {
            bind._part[0] = SMO_attr_material2;
            bind._piece = SMP_row3x1;
            cp_add_mat_spec(bind);
            continue;
          } else {
            report_parameter_error(fqname, member.type, "expected bool or float");
          }
        } else if (member.name == "refractiveIndex") {
          if (expect_float_vector(fqname, member.type, 1, 1)) {
            bind._part[0] = SMO_attr_material2;
            bind._piece = SMP_cell13;
            cp_add_mat_spec(bind);
            continue;
          }
        } else {
          report_parameter_error(fqname, member.type, "unrecognized material attribute");
        }
        success = false;
      }

      return success;
    }
    if (pieces[1] == "ColorScale") {
      if (!expect_float_vector(name, type, 3, 4)) {
        return false;
      }
      ShaderMatSpec bind;
      bind._id = param;
      bind._func = SMF_first;
      bind._part[0] = SMO_attr_colorscale;
      bind._arg[0] = nullptr;
      bind._part[1] = SMO_identity;
      bind._arg[1] = nullptr;

      if (type->as_vector()->get_num_components() == 3) {
        bind._piece = Shader::SMP_row3x3;
      } else {
        bind._piece = Shader::SMP_row3;
      }
      cp_add_mat_spec(bind);
      return true;
    }
    if (pieces[1] == "Color") {
      if (!expect_float_vector(name, type, 3, 4)) {
        return false;
      }
      ShaderMatSpec bind;
      bind._id = param;
      bind._func = Shader::SMF_first;
      bind._part[0] = Shader::SMO_attr_color;
      bind._arg[0] = nullptr;
      bind._part[1] = Shader::SMO_identity;
      bind._arg[1] = nullptr;

      if (type->as_vector()->get_num_components() == 3) {
        bind._piece = Shader::SMP_row3x3;
      } else {
        bind._piece = Shader::SMP_row3;
      }
      cp_add_mat_spec(bind);
      return true;
    }
    if (pieces[1] == "ClipPlane") {
      const ::ShaderType *element_type;
      uint32_t num_elements;
      type->unwrap_array(element_type, num_elements);
      if (!expect_float_vector(name, element_type, 4, 4)) {
        return false;
      }
      Shader::ShaderMatSpec bind;
      bind._id = param;
      bind._id._type = element_type;
      bind._piece = Shader::SMP_row3;
      bind._func = Shader::SMF_first;
      bind._part[0] = Shader::SMO_apiview_clipplane_i;
      bind._arg[0] = nullptr;
      bind._part[1] = Shader::SMO_identity;
      bind._arg[1] = nullptr;

      for (uint32_t i = 0; i < num_elements; ++i) {
        bind._index = i;
        cp_add_mat_spec(bind);
        ++bind._id._location;
      }
      return true;
    }
    if (pieces[1] == "TexAlphaOnly") {
      ShaderMatSpec bind;
      bind._id = param;
      bind._func = SMF_first;
      bind._index = 0;
      bind._part[0] = SMO_tex_is_alpha_i;
      bind._arg[0] = nullptr;
      bind._part[1] = SMO_identity;
      bind._arg[1] = nullptr;
      bind._piece = SMP_row3;
      cp_add_mat_spec(bind);
      return true;
    }
    if (pieces[1] == "Fog") {
      ShaderMatSpec bind;
      bind._id = param;
      bind._func = SMF_first;
      bind._arg[0] = nullptr;
      bind._part[1] = SMO_identity;
      bind._arg[1] = nullptr;

      const ::ShaderType::Struct *struct_type = type->as_struct();
      if (struct_type == nullptr) {
        return report_parameter_error(name, type, "expected struct");
      }

      bool success = true;
      for (size_t i = 0; i < struct_type->get_num_members(); ++i) {
        const ::ShaderType::Struct::Member &member = struct_type->get_member(i);

        CPT(InternalName) fqname = ((InternalName *)name.p())->append(member.name);
        bind._id._location = param._location + i;
        bind._id._name = fqname->get_name();
        bind._id._type = member.type;

        if (member.name == "color") {
          if (expect_float_vector(fqname, member.type, 3, 4)) {
            bind._part[0] = SMO_attr_fogcolor;
            if (member.type->as_vector()->get_num_components() == 3) {
              bind._piece = Shader::SMP_row3x3;
            } else {
              bind._piece = Shader::SMP_row3;
            }
            cp_add_mat_spec(bind);
            continue;
          }
        } else if (member.name == "density") {
          if (expect_float_vector(fqname, member.type, 1, 1)) {
            bind._part[0] = SMO_attr_fog;
            bind._piece = SMP_row3x1;
            cp_add_mat_spec(bind);
            continue;
          }
        } else if (member.name == "start") {
          if (expect_float_vector(fqname, member.type, 1, 1)) {
            bind._part[0] = SMO_attr_fog;
            bind._piece = SMP_cell13;
            cp_add_mat_spec(bind);
            continue;
          }
        } else if (member.name == "end") {
          if (expect_float_vector(fqname, member.type, 1, 1)) {
            bind._part[0] = SMO_attr_fog;
            bind._piece = SMP_cell14;
            cp_add_mat_spec(bind);
            continue;
          }
        } else if (member.name == "scale") {
          if (expect_float_vector(fqname, member.type, 1, 1)) {
            bind._part[0] = SMO_attr_fog;
            bind._piece = SMP_cell15;
            cp_add_mat_spec(bind);
            continue;
          }
        } else {
          report_parameter_error(fqname, member.type, "unrecognized fog attribute");
        }
        success = false;
      }

      return success;
    }
    if (pieces[1] == "LightModel") {
      const ::ShaderType::Struct *struct_type = type->as_struct();
      if (struct_type == nullptr || struct_type->get_num_members() > 1) {
        return report_parameter_error(name, type, "expected struct with 1 member");
      }

      if (struct_type->get_num_members() > 0) {
        const ::ShaderType::Struct::Member &member = struct_type->get_member(0);
        if (member.name != "ambient") {
          return report_parameter_error(name, type, "expected 'ambient' member");
        }

        CPT(InternalName) fqname = ((InternalName *)name.p())->append(member.name);
        if (!expect_float_vector(fqname, member.type, 3, 4)) {
          return false;
        }

        ShaderMatSpec bind;
        bind._id = param;
        bind._id._name = fqname;
        bind._id._type = member.type;
        bind._func = SMF_first;
        bind._part[0] = SMO_light_ambient;
        bind._arg[0] = nullptr;
        bind._part[1] = SMO_identity;
        bind._arg[1] = nullptr;

        if (member.type->as_vector()->get_num_components() == 3) {
          bind._piece = SMP_row3x3;
        } else {
          bind._piece = SMP_row3;
        }
        cp_add_mat_spec(bind);
      }

      return true;
    }
    if (pieces[1] == "LightSource") {
      const ::ShaderType::Array *array = type->as_array();
      if (array == nullptr) {
        return report_parameter_error(name, type, "expected array of structs");
      }

      const ::ShaderType::Struct *struct_type = array->get_element_type()->as_struct();
      if (struct_type == nullptr) {
        return report_parameter_error(name, type, "expected array of structs");
      }

      int location = param._location;

      size_t num_members = struct_type->get_num_members();
      for (size_t i = 0; i < num_members; ++i) {
        const ::ShaderType::Struct::Member &member = struct_type->get_member(i);

        CPT(InternalName) fqname = ((InternalName *)name.p())->append(member.name);

        if (member.name == "shadowMap") {
          if (member.type->as_sampled_image() == nullptr) {
            return report_parameter_error(name, type, "expected sampler2D");
          }
          ShaderTexSpec bind;
          bind._id = param;
          bind._id._name = fqname;
          bind._id._type = member.type;
          bind._id._location = location++;
          bind._part = STO_light_i_shadow_map;
          bind._desired_type = Texture::TT_2d_texture;
          for (bind._stage = 0; bind._stage < (int)array->get_num_elements(); ++bind._stage) {
            _tex_spec.push_back(bind);
            bind._id._location += num_members;
          }
        } else {
          ShaderMatSpec bind;
          bind._id = param;
          bind._id._name = fqname;
          bind._id._type = member.type;
          bind._id._location = location++;
          bind._func = SMF_first;
          bind._part[0] = SMO_light_source_i_attrib;
          bind._arg[0] = InternalName::make(member.name);
          bind._part[1] = SMO_identity;
          bind._arg[1] = nullptr;
          if (member.name == "shadowViewMatrix" || member.name == "shadowViewMatrixInverse") {
            if (!expect_float_matrix(fqname, member.type, 4, 4)) {
              return false;
            }
            bind._piece = SMP_whole;
            bind._scalar_type = member.type->as_matrix()->get_scalar_type();
          }
          else if (member.name == "shadowMatrix") {
            // Only supported for backward compatibility: includes the model
            // matrix.  Not very efficient to do this.
            if (!expect_float_matrix(fqname, member.type, 4, 4)) {
              return false;
            }

            bind._func = SMF_compose;
            bind._piece = SMP_whole;
            bind._part[0] = SMO_model_to_apiview;
            bind._arg[0] = nullptr;
            bind._part[1] = SMO_light_source_i_attrib;
            bind._arg[1] = InternalName::make("shadowViewMatrix");
            bind._scalar_type = member.type->as_matrix()->get_scalar_type();

            static bool warned = false;
            if (!warned) {
              warned = true;
              shader_cat.warning()
                << "p3d_LightSource[].shadowMatrix is deprecated; use "
                   "shadowViewMatrix instead, which transforms from view space "
                   "instead of model space.\n";
            }
          }
          else {
            if (!expect_float_vector(fqname, member.type, 1, 4)) {
              return false;
            }
            const ::ShaderType::Vector *vector = member.type->as_vector();
            if (vector == nullptr || vector->get_num_components() == 1) {
              bind._piece = SMP_row3x1;
            }
            else if (vector->get_num_components() == 2) {
              bind._piece = SMP_row3x2;
            }
            else if (vector->get_num_components() == 3) {
              bind._piece = SMP_row3x3;
            }
            else {
              bind._piece = SMP_row3;
            }
            bind._part[0] = SMO_light_source_i_attrib;
            bind._arg[0] = InternalName::make(member.name);
            bind._part[1] = SMO_identity;
            bind._arg[1] = nullptr;
            bind._scalar_type = ScalarType::ST_float;
          }
          for (bind._index = 0; bind._index < (int)array->get_num_elements(); ++bind._index) {
            cp_add_mat_spec(bind);
            bind._id._location += num_members;
          }
        }
      }

      return true;
    }
    if (pieces[1] == "TransformTable") {
      const ::ShaderType *element_type;
      uint32_t num_elements;
      type->unwrap_array(element_type, num_elements);

      const ::ShaderType::Matrix *matrix = element_type->as_matrix();
      if (matrix == nullptr ||
          matrix->get_num_rows() != 4 ||
          matrix->get_num_columns() != 4 ||
          matrix->get_scalar_type() != ScalarType::ST_float) {
        return report_parameter_error(name, type, "expected mat4[]");
      }

      _transform_table_loc = param._location;
      _transform_table_size = num_elements;
      _transform_table_reduced = false;
      return true;
    }
    if (pieces[1] == "SliderTable") {
      const ::ShaderType *element_type;
      uint32_t num_elements;
      type->unwrap_array(element_type, num_elements);

      if (element_type != ::ShaderType::float_type) {
        return report_parameter_error(name, type, "expected float");
      }

      _slider_table_loc = param._location;
      _slider_table_size = num_elements;
      return true;
    }

    return report_parameter_error(name, type, "unrecognized parameter name");
  }
  else if (pieces[0] == "osg" && _language == SL_GLSL) {
    if (!expect_num_words(name, type, 2)) {
      return false;
    }

    // These inputs are supported by OpenSceneGraph.  We can support them as
    // well, to increase compatibility.
    ShaderMatSpec bind;
    bind._id = param;
    bind._arg[0] = nullptr;
    bind._arg[1] = nullptr;

    if (pieces[1] == "ViewMatrix") {
      bind._piece = SMP_whole;
      bind._func = SMF_compose;
      bind._part[0] = SMO_world_to_view;
      bind._part[1] = SMO_view_to_apiview;
    }
    else if (pieces[1] == "InverseViewMatrix" || pieces[1] == "ViewMatrixInverse") {
      bind._piece = SMP_whole;
      bind._func = SMF_compose;
      bind._part[0] = SMO_apiview_to_view;
      bind._part[1] = SMO_view_to_world;
    }
    else if (pieces[1] == "FrameTime") {
      bind._piece = SMP_row3x1;
      bind._func = SMF_first;
      bind._part[0] = SMO_frame_time;
      bind._part[1] = SMO_identity;
    }
    else if (pieces[1] == "DeltaFrameTime") {
      bind._piece = SMP_row3x1;
      bind._func = SMF_first;
      bind._part[0] = SMO_frame_delta;
      bind._part[1] = SMO_identity;
    }
    else if (pieces[1] == "FrameNumber") {
      if (type == ::ShaderType::int_type) {
        _frame_number_loc = param._location;
        return true;
      } else {
        return report_parameter_error(name, type, "expected int");
      }
    }
    else {
      return report_parameter_error(name, type, "unrecognized parameter name");
    }

    cp_add_mat_spec(bind);
    return true;
  }

  // Check for mstrans, wstrans, vstrans, cstrans, mspos, wspos, vspos, cspos
  if (pieces[0].size() >= 5 &&
      (pieces[0].compare(1, std::string::npos, "strans") == 0 ||
       pieces[0].compare(1, std::string::npos, "spos") == 0)) {
    pieces.push_back("to");

    switch (pieces[0][0]) {
    case 'm':
      pieces.push_back("model");
      break;
    case 'w':
      pieces.push_back("world");
      break;
    case 'v':
      pieces.push_back("view");
      break;
    case 'c':
      pieces.push_back("clip");
      break;
    default:
      return false;
    }
    if (pieces[0].compare(1, std::string::npos, "strans") == 0) {
      pieces[0] = "trans";
    } else {
      pieces[0] = "row3";
    }
  }
  else if (pieces[0].size() == 3 && // mat_modelproj et al
           (pieces[0] == "mat" || pieces[0] == "inv" ||
            pieces[0] == "tps" || pieces[0] == "itp")) {
    std::string trans = pieces[0];
    std::string matrix = pieces[1];
    pieces.clear();
    if (matrix == "modelview") {
      tokenize("trans_model_to_apiview", pieces, "_");
    }
    else if (matrix == "projection") {
      tokenize("trans_apiview_to_apiclip", pieces, "_");
    }
    else if (matrix == "modelproj") {
      tokenize("trans_model_to_apiclip", pieces, "_");
    }
    else if (_language == SL_Cg && matrix == "shadow") {
      if (!expect_num_words(name, type, 3) ||
          !expect_float_matrix(name, type, 4, 4)) {
        return false;
      }
      ShaderMatSpec bind;
      bind._id = param;
      bind._piece = SMP_whole;
      bind._func = SMF_compose;
      bind._part[1] = SMO_light_source_i_attrib;
      bind._arg[1] = InternalName::make("shadowViewMatrix");
      bind._part[0] = SMO_view_to_apiview;
      bind._arg[0] = nullptr;
      bind._index = atoi(pieces[2].c_str());
      bind._scalar_type = type->as_matrix()->get_scalar_type();

      cp_add_mat_spec(bind);
      return true;
    }
    else {
      return report_parameter_error(name, type, "unrecognized matrix name");
    }
    if (trans == "mat") {
      pieces[0] = "trans";
    } else if (trans == "inv") {
      string t = pieces[1];
      pieces[1] = pieces[3];
      pieces[3] = t;
    } else if (trans == "tps") {
      pieces[0] = "tpose";
    } else if (trans == "itp") {
      string t = pieces[1];
      pieces[1] = pieces[3];
      pieces[3] = t;
      pieces[0] = "tpose";
    }
  }

  // Implement the Cg-style transform-matrix generator.
  if (pieces[0] == "trans" ||
      pieces[0] == "tpose" ||
      pieces[0] == "row0" ||
      pieces[0] == "row1" ||
      pieces[0] == "row2" ||
      pieces[0] == "row3" ||
      pieces[0] == "col0" ||
      pieces[0] == "col1" ||
      pieces[0] == "col2" ||
      pieces[0] == "col3") {

    ShaderMatSpec bind;
    bind._id = param;
    bind._func = SMF_compose;

    if (pieces[0] == "trans") {
      if (!expect_float_matrix(name, type, 3, 4)) {
        return false;
      }
      const ::ShaderType::Matrix *matrix = type->as_matrix();
      if (matrix->get_num_rows() >= 4) {
        if (matrix->get_num_columns() >= 4) {
          bind._piece = SMP_whole;
        } else {
          bind._piece = SMP_upper4x3;
        }
      } else if (matrix->get_num_columns() >= 4) {
        bind._piece = SMP_upper3x4;
      } else {
        bind._piece = SMP_upper3x3;
      }
      bind._scalar_type = matrix->get_scalar_type();
    }
    else if (pieces[0] == "tpose") {
      if (!expect_float_matrix(name, type, 3, 4)) {
        return false;
      }
      const ::ShaderType::Matrix *matrix = type->as_matrix();
      if (matrix->get_num_rows() >= 4) {
        if (matrix->get_num_columns() >= 4) {
          bind._piece = SMP_transpose;
        } else {
          bind._piece = SMP_transpose4x3;
        }
      } else if (matrix->get_num_columns() >= 4) {
        bind._piece = SMP_transpose3x4;
      } else {
        bind._piece = SMP_transpose3x3;
      }
      bind._scalar_type = matrix->get_scalar_type();
    }
    else if (pieces[0] == "row3") {
      // We can exceptionally support row3 to have any number of components.
      if (!expect_float_vector(name, type, 1, 4)) {
        return false;
      }
      const ::ShaderType::Vector *vector = type->as_vector();
      if (vector == nullptr || vector->get_num_components() == 1) {
        bind._piece = SMP_row3x1;
      }
      else if (vector->get_num_components() == 2) {
        bind._piece = SMP_row3x2;
      }
      else if (vector->get_num_components() == 3) {
        bind._piece = SMP_row3x3;
      }
      else {
        bind._piece = SMP_row3;
      }
      bind._scalar_type = ScalarType::ST_float;
    }
    else {
      if (!expect_float_vector(name, type, 4, 4)) {
        return false;
      }
      if (pieces[0][0] == 'r') {
        bind._piece = (ShaderMatPiece)(SMP_row0 + (pieces[0][3] - '0'));
      }
      else if (pieces[0][0] == 'c') {
        bind._piece = (ShaderMatPiece)(SMP_col0 + (pieces[0][3] - '0'));
      }
      else {
        nassertr(false, false);
      }
      bind._scalar_type = type->as_vector()->get_scalar_type();
    }

    int next = 1;
    pieces.push_back("");
    if (!expect_coordinate_system(name, type, pieces, next, bind, true)) {
      return false;
    }
    if (pieces[next] != "to" && pieces[next] != "rel") {
      return report_parameter_error(name, type, "expected 'to' or 'rel'");
    }
    ++next;
    if (!expect_coordinate_system(name, type, pieces, next, bind, false)) {
      return false;
    }
    if (pieces.size() > next + 1) {
      return report_parameter_error(name, type,
        "unexpected extra words after parameter name");
    }

    // clip == apiclip in OpenGL, and the apiclip matrices are cached.
    if (bind._part[0] == Shader::SMO_view_to_clip) {
      bind._part[0] = Shader::SMO_view_to_apiclip;
    } else if (bind._part[0] == Shader::SMO_clip_to_view) {
      bind._part[0] = Shader::SMO_apiclip_to_view;
    }
    if (bind._part[1] == Shader::SMO_view_to_clip) {
      bind._part[1] = Shader::SMO_view_to_apiclip;
    } else if (bind._part[1] == Shader::SMO_clip_to_view) {
      bind._part[1] = Shader::SMO_apiclip_to_view;
    }

    cp_add_mat_spec(bind);
    return true;
  }

  // Other Cg-specific inputs.
  //bool k_prefix = false;
  if (_language == SL_Cg) {
    if (name_str.size() >= 2 && name_str.substr(0, 2) == "__") {
      return true;
    }

    // Special parameter: attr_material or attr_color
    if (pieces[0] == "attr") {
      if (!expect_num_words(name, type,  2)) {
        return false;
      }
      ShaderMatSpec bind;
      bind._id = param;
      if (pieces[1] == "material") {
        if (!expect_float_matrix(name, type, 4, 4)) {
          return false;
        }
        bind._piece = SMP_transpose;
        bind._func = SMF_first;
        bind._part[0] = SMO_attr_material;
        bind._arg[0] = nullptr;
        bind._part[1] = SMO_identity;
        bind._arg[1] = nullptr;
        bind._scalar_type = type->as_matrix()->get_scalar_type();
      }
      else if (pieces[1] == "color") {
        if (!expect_float_vector(name, type, 3, 4)) {
          return false;
        }
        bind._piece = SMP_row3;
        bind._func = SMF_first;
        bind._part[0] = SMO_attr_color;
        bind._arg[0] = nullptr;
        bind._part[1] = SMO_identity;
        bind._arg[1] = nullptr;
      }
      else if (pieces[1] == "colorscale") {
        if (!expect_float_vector(name, type, 3, 4)) {
          return false;
        }
        bind._piece = SMP_row3;
        bind._func = SMF_first;
        bind._part[0] = SMO_attr_colorscale;
        bind._arg[0] = nullptr;
        bind._part[1] = SMO_identity;
        bind._arg[1] = nullptr;
      }
      else if (pieces[1] == "fog") {
        if (!expect_float_vector(name, type, 3, 4)) {
          return false;
        }
        bind._piece = SMP_row3;
        bind._func = SMF_first;
        bind._part[0] = SMO_attr_fog;
        bind._arg[0] = nullptr;
        bind._part[1] = SMO_identity;
        bind._arg[1] = nullptr;
      }
      else if (pieces[1] == "fogcolor") {
        if (!expect_float_vector(name, type, 3, 4)) {
          return false;
        }
        bind._piece = SMP_row3;
        bind._func = SMF_first;
        bind._part[0] = SMO_attr_fogcolor;
        bind._arg[0] = nullptr;
        bind._part[1] = SMO_identity;
        bind._arg[1] = nullptr;
      }
      else if (pieces[1] == "ambient") {
        if (!expect_float_vector(name, type, 3, 4)) {
          return false;
        }
        bind._piece = SMP_row3;
        bind._func = SMF_first;
        bind._part[0] = SMO_light_ambient;
        bind._arg[0] = nullptr;
        bind._part[1] = SMO_identity;
        bind._arg[1] = nullptr;
      }
      else if (pieces[1].compare(0, 5, "light") == 0) {
        if (!expect_float_matrix(name, type, 4, 4)) {
          return false;
        }
        bind._piece = SMP_transpose;
        bind._func = SMF_first;
        bind._part[0] = SMO_light_source_i_packed;
        bind._arg[0] = nullptr;
        bind._part[1] = SMO_identity;
        bind._arg[1] = nullptr;
        bind._index = atoi(pieces[1].c_str() + 5);
        bind._scalar_type = type->as_matrix()->get_scalar_type();
      }
      else if (pieces[1].compare(0, 5, "lspec") == 0) {
        if (!expect_float_vector(name, type, 3, 4)) {
          return false;
        }
        bind._piece = SMP_row3;
        bind._func = SMF_first;
        bind._part[0] = SMO_light_source_i_attrib;
        bind._arg[0] = InternalName::make("specular");
        bind._part[1] = SMO_identity;
        bind._arg[1] = nullptr;
        bind._index = atoi(pieces[1].c_str() + 5);
      }
      else if (pieces[1] == "pointparams") {
        if (!expect_float_vector(name, type, 3, 4)) {
          return false;
        }
        bind._func = SMF_first;
        bind._part[0] = SMO_attr_pointparams;
        bind._arg[0] = nullptr;
        bind._part[1] = SMO_identity;
        bind._arg[1] = nullptr;

        if (type->as_vector()->get_num_components() == 3) {
          bind._piece = Shader::SMP_row3x3;
        } else {
          bind._piece = Shader::SMP_row3;
        }
      }
      else {
        return report_parameter_error(name, type, "unrecognized parameter name");
      }

      cp_add_mat_spec(bind);
      return true;
    }

    // Keywords to access light properties.
    if (pieces[0] == "alight") {
      if (!expect_num_words(name, type, 2) ||
          !expect_float_vector(name, type, 3, 4)) {
        return false;
      }
      ShaderMatSpec bind;
      bind._id = param;
      bind._func = SMF_first;
      bind._part[0] = SMO_alight_x;
      bind._arg[0] = InternalName::make(pieces[1]);
      bind._part[1] = SMO_identity;
      bind._arg[1] = nullptr;

      if (type->as_vector()->get_num_components() == 3) {
        bind._piece = Shader::SMP_row3x3;
      } else {
        bind._piece = Shader::SMP_row3;
      }

      cp_add_mat_spec(bind);
      return true;
    }

    if (pieces[0] == "satten") {
      if (!expect_num_words(name, type, 2)||
          !expect_float_vector(name, type, 4, 4)) {
        return false;
      }
      ShaderMatSpec bind;
      bind._id = param;
      bind._piece = SMP_row3;
      bind._func = SMF_first;
      bind._part[0] = SMO_satten_x;
      bind._arg[0] = InternalName::make(pieces[1]);
      bind._part[1] = SMO_identity;
      bind._arg[1] = nullptr;

      cp_add_mat_spec(bind);
      return true;
    }

    if (pieces[0] == "dlight" || pieces[0] == "plight" || pieces[0] == "slight") {
      if (!expect_float_matrix(name, type, 4, 4)) {
        return false;
      }
      ShaderMatSpec bind;
      bind._id = param;
      bind._piece = SMP_transpose;
      int next = 1;
      pieces.push_back("");
      if (pieces[next] == "") {
        return report_parameter_error(name, type, "expected light input name");
      }
      if (pieces[0] == "dlight") {
        bind._func = SMF_transform_dlight;
        bind._part[0] = SMO_dlight_x;
      }
      else if (pieces[0] == "plight") {
        bind._func = SMF_transform_plight;
        bind._part[0] = SMO_plight_x;
      }
      else if (pieces[0] == "slight") {
        bind._func = SMF_transform_slight;
        bind._part[0] = SMO_slight_x;
      }
      bind._arg[0] = InternalName::make(pieces[next]);
      bind._scalar_type = type->as_matrix()->get_scalar_type();
      next += 1;
      if (pieces[next] != "to" && pieces[next] != "rel") {
        return report_parameter_error(name, type, "expected 'to' or 'rel'");
      }
      if (!expect_coordinate_system(name, type, pieces, next, bind, false)) {
        return false;
      }
      if (pieces.size() > next) {
        return report_parameter_error(name, type,
          "unexpected extra words after parameter name");
      }
      cp_add_mat_spec(bind);
      return true;
    }

    if (pieces[0] == "texmat") {
      if (!expect_num_words(name, type, 2) ||
          !expect_float_matrix(name, type, 4, 4)) {
        return false;
      }
      ShaderMatSpec bind;
      bind._id = param;
      bind._piece = SMP_whole;
      bind._func = SMF_first;
      bind._part[0] = SMO_texmat_i;
      bind._arg[0] = nullptr;
      bind._part[1] = SMO_identity;
      bind._arg[1] = nullptr;
      bind._index = atoi(pieces[1].c_str());
      bind._scalar_type = type->as_matrix()->get_scalar_type();

      cp_add_mat_spec(bind);
      return true;
    }

    if (pieces[0] == "texscale") {
      if (!expect_num_words(name, type, 2) ||
          !expect_float_vector(name, type, 3, 4)) {
        return false;
      }
      ShaderMatSpec bind;
      bind._id = param;
      bind._func = SMF_first;
      bind._part[0] = SMO_texscale_i;
      bind._arg[0] = nullptr;
      bind._part[1] = SMO_identity;
      bind._arg[1] = nullptr;
      bind._index = atoi(pieces[1].c_str());

      if (type->as_vector()->get_num_components() == 3) {
        bind._piece = Shader::SMP_row3x3;
      } else {
        bind._piece = Shader::SMP_row3;
      }

      cp_add_mat_spec(bind);
      return true;
    }

    if (pieces[0] == "texcolor") {
      if (!expect_num_words(name, type, 2) ||
          !expect_float_vector(name, type, 3, 4)) {
        return false;
      }
      ShaderMatSpec bind;
      bind._id = param;
      bind._func = SMF_first;
      bind._part[0] = SMO_texcolor_i;
      bind._arg[0] = nullptr;
      bind._part[1] = SMO_identity;
      bind._arg[1] = nullptr;
      bind._index = atoi(pieces[1].c_str());

      if (type->as_vector()->get_num_components() == 3) {
        bind._piece = Shader::SMP_row3x3;
      } else {
        bind._piece = Shader::SMP_row3;
      }

      cp_add_mat_spec(bind);
      return true;
    }

    if (pieces[0] == "texconst") {
      if (!expect_num_words(name, type, 2) ||
          !expect_float_vector(name, type, 3, 4)) {
        return false;
      }
      ShaderMatSpec bind;
      bind._id = param;
      bind._func = SMF_first;
      bind._part[0] = SMO_texconst_i;
      bind._arg[0] = nullptr;
      bind._part[1] = SMO_identity;
      bind._arg[1] = nullptr;
      bind._index = atoi(pieces[1].c_str());

      if (type->as_vector()->get_num_components() == 3) {
        bind._piece = Shader::SMP_row3x3;
      } else {
        bind._piece = Shader::SMP_row3;
      }

      cp_add_mat_spec(bind);
      return true;
    }

    if (pieces[0] == "plane") {
      if (!expect_num_words(name, type, 2) ||
          !expect_float_vector(name, type, 4, 4)) {
        return false;
      }
      ShaderMatSpec bind;
      bind._id = param;
      bind._piece = SMP_row3;
      bind._func = SMF_first;
      bind._part[0] = SMO_plane_x;
      bind._arg[0] = InternalName::make(pieces[1]);
      bind._part[1] = SMO_identity;
      bind._arg[1] = nullptr;

      cp_add_mat_spec(bind);
      return true;
    }

    if (pieces[0] == "clipplane") {
      if (!expect_num_words(name, type, 2) ||
          !expect_float_vector(name, type,  4, 4)) {
        return false;
      }
      ShaderMatSpec bind;
      bind._id = param;
      bind._piece = SMP_row3;
      bind._func = SMF_first;
      bind._part[0] = SMO_clipplane_x;
      bind._arg[0] = InternalName::make(pieces[1]);
      bind._part[1] = SMO_identity;
      bind._arg[1] = nullptr;

      cp_add_mat_spec(bind);
      return true;
    }

    // Keywords to access unusual parameters.
    if (pieces[0] == "sys") {
      if (!expect_num_words(name, type, 2)) {
        return false;
      }
      ShaderMatSpec bind;
      bind._id = param;
      bind._piece = SMP_row3;
      bind._func = SMF_first;
      bind._part[1] = SMO_identity;
      bind._arg[1] = nullptr;
      if (pieces[1] == "pixelsize") {
        if (!expect_float_vector(name, type, 2, 2)) {
          return false;
        }
        bind._part[0] = SMO_pixel_size;
        bind._arg[0] = nullptr;

      } else if (pieces[1] == "windowsize") {
        if (!expect_float_vector(name, type, 2, 2)) {
          return false;
        }
        bind._part[0] = SMO_window_size;
        bind._arg[0] = nullptr;

      } else if (pieces[1] == "time") {
        if (!expect_float_vector(name, type, 1, 1)) {
          return false;
        }
        bind._piece = SMP_row3x1;
        bind._part[0] = SMO_frame_time;
        bind._arg[0] = nullptr;

      } else {
        return report_parameter_error(name, type, "unrecognized parameter name");
      }

      cp_add_mat_spec(bind);
      return true;
    }

    // Keywords to access textures.

    if (pieces[0] == "tex") {
      if (pieces.size() != 2 && pieces.size() != 3) {
        return report_parameter_error(name, type, "unrecognized parameter name");
      }
      ShaderTexSpec bind;
      bind._id = param;
      bind._name = nullptr;
      bind._stage = atoi(pieces[1].c_str());
      bind._part = STO_stage_i;

      if (const ::ShaderType::SampledImage *image = type->as_sampled_image()) {
        bind._desired_type = image->get_texture_type();
      }
      else {
        return report_parameter_error(name, type, "expected sampler type");
      }
      if (pieces.size() == 3) {
        bind._suffix = InternalName::make(((string)"-") + pieces[2]);
        shader_cat.warning()
          << "Parameter " << name_str << ": use of a texture suffix is deprecated.\n";
      }
      _tex_spec.push_back(bind);
      return true;
    }

    if (pieces[0] == "shadow") {
      if (pieces.size() != 2) {
        return report_parameter_error(name, type, "unrecognized parameter name");
      }
      ShaderTexSpec bind;
      bind._id = param;
      bind._name = nullptr;
      bind._stage = atoi(pieces[1].c_str());
      bind._part = STO_light_i_shadow_map;

      if (const ::ShaderType::SampledImage *image = type->as_sampled_image()) {
        bind._desired_type = image->get_texture_type();
      }
      else {
        return report_parameter_error(name, type, "expected sampler type");
      }
      _tex_spec.push_back(bind);
      return true;
    }

    // Keywords to fetch texture parameter data.

    if (pieces[0] == "texpad") {
      if (!expect_num_words(name, type, 2) ||
          !expect_float_vector(name, type, 3, 4)) {
        return false;
      }
      ShaderMatSpec bind;
      bind._id = param;
      bind._func = SMF_first;
      bind._part[0] = SMO_texpad_x;
      bind._arg[0] = InternalName::make(pieces[1]);
      bind._part[1] = SMO_identity;
      bind._arg[1] = nullptr;

      if (type->as_vector()->get_num_components() == 3) {
        bind._piece = Shader::SMP_row3x3;
      } else {
        bind._piece = Shader::SMP_row3;
      }

      cp_add_mat_spec(bind);
      return true;
    }

    if (pieces[0] == "texpix") {
      if (!expect_num_words(name, type, 2) ||
          !expect_float_vector(name, type, 2, 4)) {
        return false;
      }
      ShaderMatSpec bind;
      bind._id = param;
      bind._piece = SMP_row3;
      bind._func = SMF_first;
      bind._part[0] = SMO_texpix_x;
      bind._arg[0] = InternalName::make(pieces[1]);
      bind._part[1] = SMO_identity;
      bind._arg[1] = nullptr;

      switch (type->as_vector()->get_num_components()) {
      case 2:
        bind._piece = Shader::SMP_row3x2;
        break;

      case 3:
        bind._piece = Shader::SMP_row3x3;
        break;

      case 4:
        bind._piece = Shader::SMP_row3;
        break;
      }

      cp_add_mat_spec(bind);
      return true;
    }

    if (pieces[0] == "tbl") {
      const ::ShaderType *element_type;
      uint32_t num_elements;
      if (!expect_num_words(name, type, 2) ||
          !type->unwrap_array(element_type, num_elements)) {
        return report_parameter_error(name, type, "expected array");
      }

      if (pieces[1] == "transforms") {
        const ::ShaderType::Matrix *matrix = element_type->as_matrix();
        if (matrix == nullptr ||
            matrix->get_num_rows() < 3 ||
            matrix->get_num_columns() != 4 ||
            matrix->get_scalar_type() != ScalarType::ST_float) {
          return report_parameter_error(name, type, "expected float3x4[] or float4x4[]");
        }

        _transform_table_loc = param._location;
        _transform_table_size = num_elements;
        _transform_table_reduced = (matrix->get_num_rows() == 3);
      }
      else if (pieces[1] == "sliders") {
        _slider_table_loc = param._location;
        _slider_table_size = num_elements;
      }
      else {
        return report_parameter_error(name, type, "unrecognized parameter name");
      }
      return true;
    }

    // Previously, custom shader inputs needed the k_ prefix, so we have to
    // strip it now.
    if (pieces[0] == "k") {
      //k_prefix = true;
      name_str = name_str.substr(2);
      name = InternalName::make(name_str);
    }
  }

  // If we get here, it's not a specially recognized input, but just a regular
  // user-defined input.
  if (const ::ShaderType::SampledImage *sampler = type->as_sampled_image()) {
    ShaderTexSpec bind;
    bind._id = param;
    bind._part = STO_named_input;
    bind._name = name;
    bind._desired_type = sampler->get_texture_type();
    bind._stage = 0;
    _tex_spec.push_back(bind);
    return true;
  }
  else if (const ::ShaderType::Image *image = type->as_image()) {
    ShaderImgSpec bind;
    bind._id = param;
    bind._name = name;
    bind._desired_type = image->get_texture_type();
    bind._writable = image->is_writable();
    _img_spec.push_back(bind);
    return true;
  }
  else if (const ::ShaderType::Matrix *matrix = type->as_matrix()) {
    if (matrix->get_num_columns() == 3 && matrix->get_num_rows() == 3) {
      ShaderMatSpec bind;
      bind._id = param;
      bind._piece = SMP_upper3x3;
      bind._func = SMF_first;
      bind._part[0] = SMO_mat_constant_x;
      bind._arg[0] = name;
      bind._part[1] = SMO_identity;
      bind._arg[1] = nullptr;
      bind._scalar_type = matrix->get_scalar_type();
      cp_add_mat_spec(bind);
      return true;
    }
    else if (matrix->get_num_columns() == 4 && matrix->get_num_rows() == 4) {
      ShaderMatSpec bind;
      bind._id = param;
      bind._piece = SMP_whole;
      bind._func = SMF_first;
      bind._part[0] = SMO_mat_constant_x;
      bind._arg[0] = name;
      bind._part[1] = SMO_identity;
      bind._arg[1] = nullptr;
      bind._scalar_type = matrix->get_scalar_type();
      cp_add_mat_spec(bind);
      return true;
    }
  }
  else if (const ::ShaderType::Struct *struct_type = type->as_struct()) {
    // Is this a struct?  If so, bind the individual members.
    bool success = true;

    int location = param._location;

    for (size_t i = 0; i < struct_type->get_num_members(); ++i) {
      const ::ShaderType::Struct::Member &member = struct_type->get_member(i);

      PT(InternalName) fqname = ((InternalName *)name.p())->append(member.name);

      // Numeric struct members under GLSL may need a special treatment.
      ScalarType scalar_type;
      uint32_t dim[3];
      if (_language == SL_GLSL &&
          member.type->as_scalar_type(scalar_type, dim[0], dim[1], dim[2]) &&
          (scalar_type == ScalarType::ST_float || scalar_type == ScalarType::ST_double) &&
          dim[0] == 1) {
        // It might be something like an attribute of a shader input, like a
        // light parameter.  It might also just be a custom struct parameter.
        // We can't know yet, so we always have to handle it specially.
        ShaderMatSpec bind;
        bind._id = param;
        bind._id._name = fqname;
        bind._id._type = member.type;
        bind._id._location = location;
        bind._scalar_type = scalar_type;
        if (member.name == "shadowMatrix" && dim[1] == 4 && dim[2] == 4) {
          // Special exception for shadowMatrix, which is deprecated because it
          // includes the model transformation.  It is far more efficient to do
          // that in the shader instead.
          static bool warned = false;
          if (!warned) {
            warned = true;
            shader_cat.warning()
              << "light.shadowMatrix inputs are deprecated; use "
                 "shadowViewMatrix instead, which transforms from view "
                 "space instead of model space.\n";
          }
          bind._piece = SMP_whole;
          bind._func = SMF_compose;
          bind._part[0] = SMO_model_to_apiview;
          bind._arg[0] = nullptr;
          bind._part[1] = SMO_mat_constant_x_attrib;
          bind._arg[1] = ((InternalName *)name.p())->append("shadowViewMatrix");
        }
        else {
          bind._func = SMF_first;
          if (dim[1] == 4) {
            bind._piece = SMP_whole;
            bind._part[0] = SMO_mat_constant_x_attrib;
          }
          else if (dim[1] == 3) {
            bind._piece = SMP_upper3x3;
            bind._part[0] = SMO_mat_constant_x_attrib;
          }
          else {
            bind._part[0] = SMO_vec_constant_x_attrib;
            if (dim[2] == 1) {
              bind._piece = SMP_row3x1;
            }
            else if (dim[2] == 2) {
              bind._piece = SMP_row3x2;
            }
            else if (dim[2] == 3) {
              bind._piece = SMP_row3x3;
            }
            else {
              bind._piece = SMP_row3;
            }
          }
          bind._arg[0] = fqname;
          bind._part[1] = SMO_identity;
          bind._arg[1] = nullptr;
        }
        cp_add_mat_spec(bind);
      }
      else {
        // Otherwise, recurse.
        Parameter member_param(param);
        member_param._name = fqname;
        member_param._type = member.type;
        member_param._location = location;
        if (!bind_parameter(member_param)) {
          success = false;
        }
      }
      location += member.type->get_num_parameter_locations();
    }

    return success;
  }
  else if (const ::ShaderType::Array *array_type = type->as_array()) {
    // Check if this is an array of structs.
    int location = param._location;
    if (const ::ShaderType::Struct *struct_type = array_type->get_element_type()->as_struct()) {
      bool success = true;

      // Generate names like structname[0].membername for every array element.
      // This is how GLSL has historically exposed these variables.
      size_t basename_size = name->get_basename().size();
      char *buffer = (char *)alloca(basename_size + 14);
      memcpy(buffer, name->get_basename().c_str(), basename_size);

      for (uint32_t ai = 0; ai < array_type->get_num_elements(); ++ai) {
        sprintf(buffer + basename_size, "[%d]", (int)ai);

        PT(InternalName) elemname = name->get_parent()->append(buffer);

        for (size_t mi = 0; mi < struct_type->get_num_members(); ++mi) {
          const ::ShaderType::Struct::Member &member = struct_type->get_member(mi);

          // Recurse.
          Parameter member_param(param);
          member_param._name = elemname->append(member.name);
          member_param._type = member.type;
          member_param._location = location;
          if (!bind_parameter(member_param)) {
            success = false;
          }

          location += member.type->get_num_parameter_locations();
        }
      }
      return success;
    }
  }

  ShaderPtrSpec bind;
  if (type->as_scalar_type(bind._type, bind._dim[0], bind._dim[1], bind._dim[2])) {
    bind._id = param;
    bind._arg = name;

    //if (k_prefix) {
    //  // Backward compatibility, disables certain checks.
    //  bind._dim[0] = -1;
    //}

    _ptr_spec.push_back(std::move(bind));
    return true;
  }

  shader_cat.error()
    << "Uniform parameter '" << name_str << "' has unsupported type "
    << *type << "\n";
  return false;
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
 * Find a ShaderCompiler in the global registry that makes the supplied language
 */
ShaderCompiler *Shader::
get_compiler(ShaderLanguage lang) const {
  ShaderCompilerRegistry *registry = ShaderCompilerRegistry::get_global_ptr();
  return registry->get_compiler_from_language(lang);
}

/**
 * Loads the shader with the given filename.
 */
PT(Shader) Shader::
load(const Filename &file, ShaderLanguage lang) {
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
  if (!shader->read(sfile)) {
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
load(ShaderLanguage lang, const Filename &vertex,
     const Filename &fragment, const Filename &geometry,
     const Filename &tess_control, const Filename &tess_evaluation) {
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
  if (!shader->read(sfile)) {
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
load_compute(ShaderLanguage lang, const Filename &fn) {
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
      if (shader->_module_mask == (1 << (int)Stage::compute)) {
        shader_cat.info()
          << "Compute shader " << fn << " was found in disk cache.\n";
        return shader;
      }
    }
  }

  PT(Shader) shader = new Shader(lang);
  if (!shader->read(sfile, record)) {
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
make(string body, ShaderLanguage lang) {
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
  if (!shader->load(sbody)) {
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
make(ShaderLanguage lang, string vertex, string fragment, string geometry,
     string tess_control, string tess_evaluation) {
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
  if (!shader->load(sbody)) {
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
make_compute(ShaderLanguage lang, string body) {
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
  if (!shader->load(sbody)) {
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

  int used_caps = module->get_used_capabilities();

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
  else if (stage == Stage::vertex) {
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

    Filename fn = module->get_source_filename();
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
  _language = (ShaderLanguage)scan.get_uint8();
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
      do_read_source(stage, fn, nullptr);
    }
    else {
      // This module was embedded.
      manager->read_pointer(scan);
    }
  }

  _prepare_shader_pcollector = PStatCollector(std::string("Draw:Prepare:Shader:") + _debug_name);
}
