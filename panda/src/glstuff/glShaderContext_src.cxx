/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file glShaderContext_src.cxx
 * @author jyelon
 * @date 2005-09-01
 * @author fperazzi, PandaSE
 * @date 2010-04-29
 *   parameter types only supported under Cg)
 */

#ifndef OPENGLES_1

#include "pStatGPUTimer.h"

#include "alphaTestAttrib.h"
#include "colorAttrib.h"
#include "colorScaleAttrib.h"
#include "materialAttrib.h"
#include "shaderAttrib.h"
#include "fogAttrib.h"
#include "lightAttrib.h"
#include "clipPlaneAttrib.h"
#include "renderModeAttrib.h"
#include "bamCache.h"
#include "shaderModuleGlsl.h"
#include "shaderModuleSpirV.h"
#include "sparseArray.h"
#include "spirVTransformer.h"
#include "spirVInjectAlphaTestPass.h"
#include "spirVEmulateTextureQueriesPass.h"

#define SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS
#include <spirv_cross/spirv_glsl.hpp>

using std::dec;
using std::hex;
using std::max;
using std::min;
using std::string;

// Inject alpha test into generated shaders.
class Compiler final : public spirv_cross::CompilerGLSL {
public:
  explicit Compiler(std::vector<uint32_t> spirv_,
                    RenderAttrib::PandaCompareFunc alpha_test_mode)
    : CompilerGLSL(std::move(spirv_)),
      _alpha_test_mode(alpha_test_mode) {
  }

private:
  virtual void emit_fixup() override {
    switch (_alpha_test_mode) {
    case RenderAttrib::M_never:
      statement("discard;");
      break;
    case RenderAttrib::M_less:
      statement("if (o0.a >= aref) discard;");
      break;
    case RenderAttrib::M_equal:
      statement("if (o0.a != aref) discard;");
      break;
    case RenderAttrib::M_less_equal:
      statement("if (o0.a > aref) discard;");
      break;
    case RenderAttrib::M_greater:
      statement("if (o0.a <= aref) discard;");
      break;
    case RenderAttrib::M_not_equal:
      statement("if (o0.a == aref) discard;");
      break;
    case RenderAttrib::M_greater_equal:
      statement("if (o0.a < aref) discard;");
      break;
    case RenderAttrib::M_none:
    case RenderAttrib::M_always:
      break;
    }
    CompilerGLSL::emit_fixup();
  }

  RenderAttrib::PandaCompareFunc _alpha_test_mode;
};

TypeHandle CLP(ShaderContext)::_type_handle;

/**
 * xyz
 */
CLP(ShaderContext)::
CLP(ShaderContext)(CLP(GraphicsStateGuardian) *glgsg, Shader *s) : ShaderContext(s) {
  _glgsg = glgsg;
  _uses_standard_vertex_arrays = false;
  _enabled_attribs.clear();
  _color_attrib_index = -1;
  _validated = !gl_validate_shaders;
  _is_legacy = s->_modules[0]._module.get_read_pointer()->is_of_type(ShaderModuleGlsl::get_class_type());

  _matrix_cache = pvector<LMatrix4>(s->_matrix_cache_desc.size(), LMatrix4::ident_mat());
  _matrix_cache_deps = s->_matrix_cache_deps;

  _scratch_space_size = 16;

  // Do we have a p3d_Color attribute?
  for (Shader::ShaderVarSpec &spec : s->_var_spec) {
    if (spec._name == InternalName::get_color()) {
      _color_attrib_index = spec._id._location;
      break;
    }
  }

  if (!_is_legacy) {
    // We may need to generate a different program for each different possible
    // alpha test mode that may be applied.  This doesn't apply to the legacy
    // pipeline since we can't modify shaders there.
    if (s->_module_mask & (1u << (uint32_t)Shader::Stage::FRAGMENT)) {
      _inject_alpha_test = !s->_subsumes_alpha_test && !glgsg->has_fixed_function_pipeline();
    }

    // Ignoring any locations that may have already been set, we assign a new
    // unique location to each parameter.
    // We can't specify any locations or bindings up front for legacy shaders,
    // in part because we don't know the parameters until after we compiled it.
    // We reserve location 0 for the alpha test ref.
    GLint next_location = (GLint)_inject_alpha_test;
    GLint next_ssbo_binding = 0;
#ifdef OPENGLES
    GLint next_image_binding = 0;
#endif
    for (const Shader::Parameter &param : _shader->_parameters) {
      GLint num_locations = 0;
      GLint num_ssbo_bindings = 0;
      GLint num_image_bindings = 0;
      r_count_locations_bindings(param._type, num_locations, num_ssbo_bindings,
                                 num_image_bindings);

      if (num_locations > 0) {
        _locations[param._name] = next_location;
        next_location += num_locations;
      }
      if (num_ssbo_bindings > 0) {
        _bindings[param._name] = next_ssbo_binding;
        next_ssbo_binding += num_ssbo_bindings;
      }
#ifdef OPENGLES
      // For OpenGL ES, we can't specify image bindings after the fact, so we
      // have to specify them up front.
      if (num_image_bindings > 0) {
        //TODO: what if there is a struct with mixed samplers and images?
        _bindings[param._name] = next_image_binding;
        next_image_binding += num_image_bindings;
      }
#endif
    }
  }

  GLuint program = 0;
  if (!_inject_alpha_test) {
    program = _glgsg->_glCreateProgram();
    _program = program;
  }

  // Compile all the modules now, except for the fragment module if we will be
  // generating a different fragment module per alpha test mode later.
  bool valid = true;
  size_t mi = 0;
  for (Shader::LinkedModule &linked_module : _shader->_modules) {
    CPT(ShaderModule) module = linked_module._module.get_read_pointer();

    if (!_inject_alpha_test || module->get_stage() != Shader::Stage::FRAGMENT) {
      GLuint handle = create_shader(program, module, mi, linked_module._consts, RenderAttrib::M_none);
      if (handle != 0) {
        _modules.push_back({module->get_stage(), handle});

        if (program != 0) {
          _glgsg->_glAttachShader(program, handle);
        }
      } else {
        valid = false;
      }
    }

    ++mi;
  }

  // Now compile the individual shaders, unless we loaded them as SPIR-V.
  // NVIDIA drivers seem to cope better when we compile them all in one go.
  if (_is_legacy || !_glgsg->_supports_spir_v) {
    for (Module &module : _modules) {
      _glgsg->_glCompileShader(module._handle);
    }
  }

  if (!valid) {
    _shader->_error_flag = true;
  }

#ifdef DO_PSTATS
  _compute_dispatch_pcollector = PStatCollector(_glgsg->_compute_dispatch_pcollector, s->get_debug_name());
#endif
}

/**
 * Returns true if the shader is "valid", ie, if the compilation was
 * successful.  The compilation could fail if there is a syntax error in the
 * shader, or if the current video card isn't shader-capable, or if no shader
 * languages are compiled into panda.
 */
bool CLP(ShaderContext)::
valid() {
  if (_shader->get_error_flag()) {
    return false;
  }
  return _program != 0 || _inject_alpha_test;
}

/**
 * This function is to be called to enable a new shader.  It also initializes
 * all of the shader's input parameters.
 */
bool CLP(ShaderContext)::
bind(CLP(GraphicsStateGuardian) *glgsg,
     RenderAttrib::PandaCompareFunc alpha_test_mode) {
  _glgsg = glgsg;
  /*if (!_validated) {
    _glgsg->_glValidateProgram(_glsl_program);
    report_program_errors(_glsl_program, false);
    _validated = true;
  }*/

  if (!_inject_alpha_test) {
    alpha_test_mode = RenderAttrib::M_none;
  }

  GLuint program = _linked_programs[alpha_test_mode];
  if (program != 0) {
    if (!_shader->get_error_flag()) {
      _glgsg->_glUseProgram(program);
    } else {
      return false;
    }
  } else {
    if (!compile_for(alpha_test_mode)) {
      return false;
    }
  }

  _alpha_test_mode = alpha_test_mode;

  if (GLCAT.is_spam()) {
    GLCAT.spam() << "glUseProgram(" << program << "): "
                 << _shader->get_filename() << " with alpha test "
                 << alpha_test_mode << "\n";
  }

  _glgsg->report_my_gl_errors();
  return true;
}

/**
 * This function disables a currently-bound shader.
 */
void CLP(ShaderContext)::
unbind() {
  if (GLCAT.is_spam()) {
    GLCAT.spam() << "glUseProgram(0)\n";
  }

  _glgsg->_glUseProgram(0);
  _glgsg->report_my_gl_errors();
}

/**
 * Compiles the shader for the given alpha test mode.  Also binds it if there
 * were no errors.
 */
bool CLP(ShaderContext)::
compile_for(RenderAttrib::PandaCompareFunc alpha_test_mode) {
  nassertr(alpha_test_mode < RenderAttrib::M_always, false);

  if (GLCAT.is_debug()) {
    GLCAT.debug()
      << "Compiling shader " << _shader->get_filename()
      << " with alpha test mode " << alpha_test_mode << "\n";
  }

  GLuint program = compile_and_link(alpha_test_mode);
  if (program == 0) {
    release_resources(_glgsg);
    _shader->_error_flag = true;
    return false;
  }
  _linked_programs[alpha_test_mode] = program;
  _alpha_test_ref_locations[alpha_test_mode] = -1;

  // Bind the program, so that we can call glUniform1i for the textures.
  _glgsg->_glUseProgram(program);

  // If this is a legacy GLSL shader, we don't have the parameter definitions
  // yet, so we need to perform reflection on the shader.
  SparseArray active_locations;
  if (_is_legacy) {
    nassertr(alpha_test_mode == RenderAttrib::M_none, false);
    reflect_program(program, active_locations);
  }
  else if (!_remap_locations) {
    // We still need to query which uniform locations are actually in use,
    // because the GL driver may have optimized some out.
    GLint num_active_uniforms = 0;
    _glgsg->_glGetProgramInterfaceiv(program, GL_UNIFORM, GL_ACTIVE_RESOURCES, &num_active_uniforms);

    for (GLint i = 0; i < num_active_uniforms; ++i) {
      GLenum props[2] = {GL_LOCATION, GL_ARRAY_SIZE};
      GLint values[2];
      _glgsg->_glGetProgramResourceiv(program, GL_UNIFORM, i, 2, props, 2, nullptr, values);
      GLint location = values[0];
      if (location >= 0) {
        GLint array_size = values[1];
        active_locations.set_range(location, array_size);
      }
    }

    // We reserved location 0 for the alpha test reference value.
    if (alpha_test_mode != RenderAttrib::M_none && active_locations.get_bit(0) && _inject_alpha_test) {
      _alpha_test_ref_locations[alpha_test_mode] = 0;
    }
  }
  else if (alpha_test_mode != RenderAttrib::M_none && _inject_alpha_test) {
    // We couldn't map the alpha ref to 0, ask the driver for the location.
    _alpha_test_ref_locations[alpha_test_mode] = _glgsg->_glGetUniformLocation(program, "aref");

    if (GLCAT.is_debug()) {
      GLCAT.debug()
        << "Injected alpha test reference is bound to location "
        << _alpha_test_ref_locations[alpha_test_mode] << "\n";
    }
  }

  for (const Shader::Parameter &param : _shader->_parameters) {
    if (param._binding == nullptr) {
      continue;
    }

    int block_index = -1;
    for (size_t i = 0; i < _uniform_blocks.size(); ++i) {
      if (_uniform_blocks[i]._bindings[0]._binding == param._binding) {
        block_index = (int)i;
        break;
      }
    }

    UniformBlock new_block;
    UniformBlock &block = block_index >= 0 ? _uniform_blocks[block_index] : new_block;

    // We chose a location earlier, but if remap_locations was set to true,
    // we have to map this to the actual location chosen by the driver.
    int chosen_location = -1;
    int actual_location = -1;
    {
      auto it = _locations.find(param._name);
      if (it != _locations.end()) {
        chosen_location = it->second;
        if (!_remap_locations) {
          actual_location = it->second;
        }
      }
    }

    // This could be an image binding or an SSBO binding, but an SSBO can't be
    // in a struct and an image can't be in an SSBO, so we're fine with only
    // using a single counter.
    int binding = -1;
    {
      auto it = _bindings.find(param._name);
      if (it != _bindings.end()) {
        binding = it->second;
      }
    }

    // Though the code is written to take advantage of UBOs, we're not using
    // UBOs yet, so we instead have the parameters copied to a scratch space.
    // Now make a list of the individual uniform calls we will have to do.
    int resource_index = 0;
    std::string name = param._name->get_name();
    char sym_buffer[16];
    sym_buffer[0] = 0;
    if (_remap_locations && chosen_location >= 0) {
      sprintf(sym_buffer, "p%d", chosen_location);
    }

    if (alpha_test_mode >= block._calls.size()) {
      block._calls.resize(alpha_test_mode + 1);
    }

    UniformCalls &calls = block._calls[alpha_test_mode];
    r_collect_uniforms(alpha_test_mode, param, calls, param._type, name.c_str(),
                       sym_buffer, actual_location, active_locations,
                       resource_index, binding);

    if (block_index < 0 && (!calls._matrices.empty() || !calls._vectors.empty())) {
      block._dep = param._binding->get_state_dep();
      block._bindings.push_back({param._binding, 0});

      _uniform_data_deps |= block._dep;
      _uniform_blocks.push_back(std::move(block));

      // We ideally want the tightly packed size, since we are not using UBOs
      // and the regular glUniform calls use tight packing.
      uint32_t size;
      ShaderType::ScalarType scalar_type;
      uint32_t num_elements;
      uint32_t num_rows;
      uint32_t num_cols;
      if (param._type->as_scalar_type(scalar_type, num_elements, num_rows, num_cols)) {
        size = num_elements * num_rows * num_cols * ShaderType::get_scalar_size_bytes(scalar_type);
      } else {
        // If it's a struct, we just use the regular size.  It's too much, but
        // since we're using the original offsets from the struct, I can't be
        // bothered right now to write code to repack the entire struct.
        size = param._type->get_size_bytes();
      }

      size = (size + 15) & ~15;
      _scratch_space_size = std::max(_scratch_space_size, (size_t)size);
    }
  }

  if (_image_units.size() > (size_t)_glgsg->_max_image_units) {
    _image_units.resize((size_t)_glgsg->_max_image_units);
  }

  //_glgsg->report_my_gl_errors();
  return true;
}

/**
 * Counts the number of uniform locations and shader storage buffer bindings,
 * which are separate because OpenGL doesn't use locations for SSBOs (unlike
 * textures).
 */
void CLP(ShaderContext)::
r_count_locations_bindings(const ShaderType *type,
                           GLint &num_locations, GLint &num_ssbo_bindings,
                           GLint &num_image_bindings) {

  if (const ShaderType::Array *array_type = type->as_array()) {
    GLint element_locs = 0, element_ssbo_binds = 0, element_img_binds = 0;
    r_count_locations_bindings(array_type->get_element_type(), element_locs, element_ssbo_binds, element_img_binds);
    num_locations += element_locs * array_type->get_num_elements();
    num_ssbo_bindings += element_ssbo_binds * array_type->get_num_elements();
    num_image_bindings += element_img_binds * array_type->get_num_elements();
    return;
  }
  if (const ShaderType::Struct *struct_type = type->as_struct()) {
    for (uint32_t i = 0; i < struct_type->get_num_members(); ++i) {
      const ShaderType::Struct::Member &member = struct_type->get_member(i);

      r_count_locations_bindings(member.type, num_locations, num_ssbo_bindings, num_image_bindings);
    }
    return;
  }

  ShaderType::ScalarType scalar_type;
  uint32_t num_elements;
  uint32_t num_rows;
  uint32_t num_cols;
  if (type->as_scalar_type(scalar_type, num_elements, num_rows, num_cols)) {
    num_locations += num_elements;
    return;
  }

  if (type->as_image() != nullptr) {
    ++num_locations;
    ++num_image_bindings;
    return;
  }

  if (type->as_sampled_image() != nullptr || type == ShaderType::void_type) {
    ++num_locations;
    return;
  }

  if (type->as_storage_buffer()) {
    ++num_ssbo_bindings;
    return;
  }
}

/**
 * For UBO emulation, expand the individual uniforms within aggregate types and
 * make a list of individual glUniform calls, also querying their location from
 * the driver if necessary.
 * Also finds all resources and adds them to the respective arrays.
 */
void CLP(ShaderContext)::
r_collect_uniforms(RenderAttrib::PandaCompareFunc alpha_test_mode,
                   const Shader::Parameter &param, UniformCalls &calls,
                   const ShaderType *type, const char *name, const char *sym,
                   int &cur_location, const SparseArray &active_locations,
                   int &resource_index, int &cur_binding, size_t offset) {

  GLuint program = _linked_programs[alpha_test_mode];

  ShaderType::ScalarType scalar_type;
  uint32_t num_elements;
  uint32_t num_rows;
  uint32_t num_cols;
  if (type->as_scalar_type(scalar_type, num_elements, num_rows, num_cols)) {
    int location = cur_location;
    if (location < 0) {
      location = _glgsg->_glGetUniformLocation(program, _is_legacy ? name : sym);
      if (location < 0) {
        return;
      }
    } else {
      cur_location += num_elements;
      if (!active_locations.get_bit(location)) {
        return;
      }
    }
    if (GLCAT.is_debug()) {
      GLCAT.debug()
        << "Active uniform " << name << " with type " << *type
        << " is bound to location " << location << "\n";
    }

    UniformCall call;
    call._location = location;
    call._count = num_elements;
    call._offset = offset;

    if (num_rows > 1) {
      if (num_rows == 3) {
        if (num_cols == 3) {
          call._func = (void *)_glgsg->_glUniformMatrix3fv;
        } else {
          call._func = (void *)_glgsg->_glUniformMatrix3x4fv;
        }
      } else {
        if (num_cols == 3) {
          call._func = (void *)_glgsg->_glUniformMatrix4x3fv;
        } else {
          call._func = (void *)_glgsg->_glUniformMatrix4fv;
        }
      }
      calls._matrices.push_back(std::move(call));
    } else {
      switch (scalar_type) {
      case ShaderType::ST_float:
        if (num_cols == 1) {
          call._func = (void *)_glgsg->_glUniform1fv;
        }
        else if (num_cols == 2) {
          call._func = (void *)_glgsg->_glUniform2fv;
        }
        else if (num_cols == 3) {
          call._func = (void *)_glgsg->_glUniform3fv;
        }
        else if (num_cols == 4) {
          call._func = (void *)_glgsg->_glUniform4fv;
        }
        break;

#ifndef OPENGLES
      case ShaderType::ST_double:
        if (num_cols == 1) {
          call._func = (void *)_glgsg->_glUniform1dv;
        }
        else if (num_cols == 2) {
          call._func = (void *)_glgsg->_glUniform2dv;
        }
        else if (num_cols == 3) {
          call._func = (void *)_glgsg->_glUniform3dv;
        }
        else if (num_cols == 4) {
          call._func = (void *)_glgsg->_glUniform4dv;
        }
        break;
#endif

      case ShaderType::ST_int:
        if (num_cols == 1) {
          call._func = (void *)_glgsg->_glUniform1iv;
        }
        else if (num_cols == 2) {
          call._func = (void *)_glgsg->_glUniform2iv;
        }
        else if (num_cols == 3) {
          call._func = (void *)_glgsg->_glUniform3iv;
        }
        else if (num_cols == 4) {
          call._func = (void *)_glgsg->_glUniform4iv;
        }
        break;

      default:
        if (num_cols == 1) {
          call._func = (void *)_glgsg->_glUniform1uiv;
        }
        else if (num_cols == 2) {
          call._func = (void *)_glgsg->_glUniform2uiv;
        }
        else if (num_cols == 3) {
          call._func = (void *)_glgsg->_glUniform3uiv;
        }
        else if (num_cols == 4) {
          call._func = (void *)_glgsg->_glUniform4uiv;
        }
        break;
      }
      calls._vectors.push_back(std::move(call));
    }
    return;
  }
  if (const ShaderType::Array *array_type = type->as_array()) {
    // Recurse.
    char *name_buffer = (char *)alloca(strlen(name) + 14);
    char *sym_buffer = (char *)alloca(strlen(sym) + 14);
    const ShaderType *element_type = array_type->get_element_type();
    size_t stride = (size_t)array_type->get_stride_bytes();

    for (uint32_t i = 0; i < array_type->get_num_elements(); ++i) {
      sprintf(name_buffer, "%s[%u]", name, i);
      sprintf(sym_buffer, "%s[%u]", sym, i);
      r_collect_uniforms(alpha_test_mode, param, calls, element_type, name_buffer, sym_buffer,
                         cur_location, active_locations, resource_index, cur_binding,
                         offset);
      offset += stride;
    }
    return;
  }
  if (const ShaderType::Struct *struct_type = type->as_struct()) {
    char *sym_buffer = (char *)alloca(strlen(sym) + 14);

    for (uint32_t i = 0; i < struct_type->get_num_members(); ++i) {
      const ShaderType::Struct::Member &member = struct_type->get_member(i);

      std::string qualname(name);
      qualname += "." + member.name;

      // We have named struct members m0, m1, etc. in declaration order.
      sprintf(sym_buffer, "%s.m%u", sym, i);
      r_collect_uniforms(alpha_test_mode, param, calls, member.type, qualname.c_str(), sym_buffer,
                         cur_location, active_locations, resource_index, cur_binding,
                         offset + member.offset);
    }
    return;
  }
  if (type == ShaderType::void_type) {
    // We use this as a placeholder to advance the location by one.
    ++cur_location;
  }

  if (type->as_storage_buffer() != nullptr) {
    // These are an exception, they do not have locations but bindings.
    GLint binding = cur_binding;
    if (binding < 0) {
      // We have to look this one up.
      GLuint index = _glgsg->_glGetProgramResourceIndex(program, GL_SHADER_STORAGE_BLOCK, name);
      const GLenum props[] = {GL_BUFFER_BINDING};
      _glgsg->_glGetProgramResourceiv(program, GL_SHADER_STORAGE_BLOCK, index, 1, props, 1, nullptr, &binding);
    } else {
      ++cur_binding;
    }

    if ((_storage_block_bindings & (1 << binding)) == 0) {
      if (GLCAT.is_debug()) {
        GLCAT.debug()
          << "Storage block " << name << " with type " << *type
          << " is bound at binding " << binding << "\n";
      }

      StorageBlock block;
      block._binding = param._binding;
      block._resource_id = param._binding->get_resource_id(resource_index++);
      block._binding_index = binding;
      _storage_blocks.push_back(std::move(block));
      _storage_block_bindings |= (1 << binding);
    }
    return;
  }

  int location = cur_location;
  if (location < 0) {
    location = _glgsg->_glGetUniformLocation(program, _is_legacy ? name : sym);
  } else {
    ++cur_location;
    if (!active_locations.get_bit(location)) {
      location = -1;
    }
  }
  int size_location = -1;
  if (_emulated_caps & (Shader::C_image_query_size | Shader::C_texture_query_size | Shader::C_texture_query_levels)) {
    // Do we have a separate size input?
    size_t sym_len = strlen(sym);
    char *size_name_buffer = (char *)alloca(sym_len + 3);
    char *p = size_name_buffer;
    for (size_t i = 0; i < sym_len; ++i) {
      if (sym[i] == '[' || sym[i] == '.') {
        *p++ = '_';
      }
      else if (sym[i] != 'm' && sym[i] != ']') {
        *p++ = sym[i];
      }
    }
    *p++ = '_';
    *p++ = 's';
    *p = '\0';
    size_location = _glgsg->_glGetUniformLocation(program, size_name_buffer);
  }
  if (location < 0 && size_location < 0) {
    return;
  }

  if (GLCAT.is_debug()) {
    GLCAT.debug()
      << "Active uniform " << name << " with type " << *type
      << " is bound to location " << location << "\n";
  }

  if (const ShaderType::SampledImage *sampler = type->as_sampled_image()) {
    TextureUnit unit;
    unit._binding = param._binding;
    unit._resource_id = param._binding->get_resource_id(resource_index++);
    unit._target = _glgsg->get_texture_target(sampler->get_texture_type());

    for (int i = 0; i < RenderAttrib::M_always; ++i) {
      unit._size_loc[i] = -1;
    }

    if (size_location >= 0) {
      unit._size_loc[alpha_test_mode] = size_location;
    }

    // Check if we already have a unit with these properties.  If so, we alias
    // the binding.  This will also prevent duplicating texture units when the
    // shader is compiled multiple times, for different alpha test modes.
    GLint binding = -1;
    for (size_t i = 0; i < _texture_units.size(); ++i) {
      TextureUnit &other_unit = _texture_units[i];
      if (other_unit._binding == unit._binding &&
          other_unit._resource_id == unit._resource_id &&
          other_unit._target == unit._target) {
        binding = (GLint)i;
        if (unit._size_loc[alpha_test_mode] >= 0) {
          other_unit._size_loc[alpha_test_mode] = unit._size_loc[alpha_test_mode];
        }
        break;
      }
    }

    if (binding < 0) {
      binding = (GLint)_texture_units.size();
      _texture_units.push_back(std::move(unit));
    }
    if (location >= 0) {
      _glgsg->_glUniform1i(location, binding);
    }
  }
  else if (const ShaderType::Image *image = type->as_image()) {
    // In OpenGL ES, we can't specify a binding index after the fact.
    // We try to specify it up front, but if we couldn't, we have to rely on
    // the driver (or the user) providing a unique one.
    GLint binding = -1;
#ifdef OPENGLES
    if (location < 0) {
      // There's an edge case here if we use imageSize without any other
      // accesses to the image, and the image itself is optimized out.
      // I don't think it's very realistic, so I haven't bothered with it.
      return;
    }
    glGetUniformiv(program, location, &binding);
    if (binding < 0) {
      return;
    }
    if ((size_t)binding >= _image_units.size()) {
      _image_units.resize(binding + 1);
    }
    ImageUnit &unit = _image_units[binding];
#else
    ImageUnit unit;
#endif
    unit._binding = param._binding;
    unit._resource_id = param._binding->get_resource_id(resource_index++);
    unit._access = image->get_access();
    unit._written = false;

    for (int i = 0; i < RenderAttrib::M_always; ++i) {
      unit._size_loc[i] = -1;
    }

    if (size_location >= 0) {
      unit._size_loc[alpha_test_mode] = size_location;
    }

#ifndef OPENGLES
    // See note above in the SampledImage case.
    for (size_t i = 0; i < _image_units.size(); ++i) {
      ImageUnit &other_unit = _image_units[i];
      if (other_unit._binding == unit._binding &&
          other_unit._resource_id == unit._resource_id &&
          other_unit._access == unit._access) {
        binding = (GLint)i;
        if (unit._size_loc[alpha_test_mode] >= 0) {
          other_unit._size_loc[alpha_test_mode] = unit._size_loc[alpha_test_mode];
        }
        break;
      }
    }
    if (binding < 0) {
      binding = (GLint)_image_units.size();
      _image_units.push_back(std::move(unit));
    }
    if (location >= 0) {
      _glgsg->_glUniform1i(location, binding);
    }
#endif
  }
  else if (type->as_resource()) {
    resource_index++;
  }
}

/**
 * Analyzes the uniforms, attributes, etc. of a shader that was not already
 * reflected.  Also sets active_locations to all ranges of uniforms that are
 * active, and anything that is known about top-level uniform names (that are
 * not in a struct or array) is also already filled into the maps.  The rest
 * will have to be queried later.
 */
void CLP(ShaderContext)::
reflect_program(GLuint program, SparseArray &active_locations) {
  // Process the vertex attributes first.
  GLint param_count = 0;
  GLint name_buflen = 0;
  _glgsg->_glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &param_count);
  _glgsg->_glGetProgramiv(program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &name_buflen);
  name_buflen = max(64, name_buflen);
  char *name_buffer = (char *)alloca(name_buflen);

  _shader->_var_spec.clear();
  for (int i = 0; i < param_count; ++i) {
    reflect_attribute(program, i, name_buffer, name_buflen);
  }

  /*if (gl_fixed_vertex_attrib_locations) {
    // Relink the shader for glBindAttribLocation to take effect.
    _glgsg->_glLinkProgram(program);
  }*/

  // Create a buffer the size of the longest uniform name.  Note that Intel HD
  // drivers report values that are too low.
  name_buflen = 0;
  _glgsg->_glGetProgramiv(program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &name_buflen);
  name_buflen = max(64, name_buflen);
  name_buffer = (char *)alloca(name_buflen);

  // Get the used uniform blocks.
  if (_glgsg->_supports_uniform_buffers) {
    GLint block_count = 0, block_maxlength = 0;
    _glgsg->_glGetProgramiv(program, GL_ACTIVE_UNIFORM_BLOCKS, &block_count);

    // Intel HD drivers report GL_INVALID_ENUM here.  They reportedly fixed
    // it, but I don't know in which driver version the fix is.
    if (_glgsg->_gl_vendor != "Intel") {
      _glgsg->_glGetProgramiv(program, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, &block_maxlength);
      block_maxlength = max(64, block_maxlength);
    } else {
      block_maxlength = 1024;
    }

    char *block_name_cstr = (char *)alloca(block_maxlength);

    for (int i = 0; i < block_count; ++i) {
      block_name_cstr[0] = 0;
      _glgsg->_glGetActiveUniformBlockName(program, i, block_maxlength, nullptr, block_name_cstr);

      reflect_uniform_block(program, i, block_name_cstr, name_buffer, name_buflen);
    }
  }

  _shader->_parameters.clear();

#ifndef OPENGLES_1
  // Get the used shader storage blocks.
  if (_glgsg->_supports_shader_buffers) {
    GLint block_count = 0, block_maxlength = 0;

    _glgsg->_glGetProgramInterfaceiv(program, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &block_count);
    _glgsg->_glGetProgramInterfaceiv(program, GL_SHADER_STORAGE_BLOCK, GL_MAX_NAME_LENGTH, &block_maxlength);

    block_maxlength = max(64, block_maxlength);
    char *block_name_cstr = (char *)alloca(block_maxlength);

#ifndef OPENGLES
    BitArray bindings;
#endif

    // OpenGL exposes SSBO arrays as individual members named name[0][1],
    // name[0][2], etc. with potential gaps for unused SSBOs.  We have to
    // untangle this.
    struct SSBO {
      pvector<uint32_t> _array_sizes;
    };
    pmap<CPT_InternalName, SSBO> ssbos;

    for (int i = 0; i < block_count; ++i) {
      block_name_cstr[0] = 0;
      _glgsg->_glGetProgramResourceName(program, GL_SHADER_STORAGE_BLOCK, i, block_maxlength, nullptr, block_name_cstr);

      const GLenum props[] = {GL_BUFFER_BINDING, GL_BUFFER_DATA_SIZE};
      GLint values[2];
      _glgsg->_glGetProgramResourceiv(program, GL_SHADER_STORAGE_BLOCK, i, 2, props, 2, nullptr, values);

#ifndef OPENGLES
      if (bindings.get_bit(values[0])) {
        // Binding index already in use, assign a different one.
        values[0] = bindings.get_lowest_off_bit();
        _glgsg->_glShaderStorageBlockBinding(program, i, values[0]);
      }
      bindings.set_bit(values[0]);
#endif

      if (GLCAT.is_debug()) {
        GLCAT.debug()
          << "Active shader storage block " << block_name_cstr
          << " with size " << values[1] << " is bound to binding "
          << values[0] << "\n";
      }

      // Parse the array elements off the end of the name.
      char *p = strchr(block_name_cstr, '[');
      if (p != nullptr) {
        // It's an array, this is a bit annoying.
        CPT_InternalName name = InternalName::make(std::string(block_name_cstr, p - block_name_cstr));
        SSBO &ssbo = ssbos[name];
        size_t i = 0;
        do {
          ++p;
          uint32_t count = strtoul(p, &p, 10) + 1;
          if (*p == ']') {
            if (i >= ssbo._array_sizes.size()) {
              ssbo._array_sizes.resize(i + 1, 0);
            }
            if (count > ssbo._array_sizes[i]) {
              ssbo._array_sizes[i] = count;
            }
            ++i;
            ++p;
          }
        } while (*p == '[');
      } else {
        // Simple case.  We can already jot down the binding, so we don't have
        // to query it later.
        CPT(InternalName) name = InternalName::make(std::string(block_name_cstr));
        _bindings[name] = values[0];
        ssbos[std::move(name)];
      }
    }

    for (auto &item : ssbos) {
      const InternalName *name = item.first.p();
      SSBO &ssbo = item.second;

      //TODO: write code to actually query the block variables.
      const ShaderType *struct_type = ShaderType::register_type(ShaderType::Struct());
      const ShaderType *type = ShaderType::register_type(ShaderType::StorageBuffer(struct_type, ShaderType::Access::READ_WRITE));

      std::reverse(ssbo._array_sizes.begin(), ssbo._array_sizes.end());
      for (uint32_t count : ssbo._array_sizes) {
        type = ShaderType::register_type(ShaderType::Array(type, count));
      }
      _shader->add_parameter(name, type);
    }
  }
#endif

  // Analyze the uniforms.  All the structs and arrays thereof are unrolled,
  // so we need to spend some effort to work out the original structures.
  param_count = 0;
  _glgsg->_glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &param_count);

  // First gather the uniforms and sort them by location.
  struct Uniform {
    std::string name;
    GLint size;
    GLenum type;
  };
  pmap<GLint, Uniform> uniforms;

  for (int i = 0; i < param_count; ++i) {
    // Get the name, location, type and size of this uniform.
    GLint param_size;
    GLenum param_type;
    name_buffer[0] = 0;
    _glgsg->_glGetActiveUniform(program, i, name_buflen, nullptr, &param_size, &param_type, name_buffer);
    GLint loc = _glgsg->_glGetUniformLocation(program, name_buffer);

    /*if (GLCAT.is_debug()) {
      GLCAT.debug()
        << "Active uniform " << name_buffer << " with size " << param_size
        << " and type 0x" << hex << param_type << dec
        << " is bound to location " << loc << "\n";
    }*/

    // Some NVidia drivers (361.43 for example) (incorrectly) include "internal"
    // uniforms in the list starting with "_main_" (for example,
    // "_main_0_gp5fp[0]") we need to skip those, because we don't know anything
    // about them
    if (strncmp(name_buffer, "_main_", 6) == 0) {
      if (GLCAT.is_debug()) {
        GLCAT.debug()
          << "Ignoring uniform " << name_buffer
          << " which may be generated by buggy Nvidia driver.\n";
      }
      continue;
    }

    if (loc < 0) {
      // Special meaning, or it's in a uniform block.  Let it go.
      continue;
    }

    active_locations.set_range(loc, param_size);

    uniforms.insert({loc, {std::string(name_buffer), param_size, param_type}});
  }

  struct StackItem {
    std::string name;
    int num_elements; // 0 means not an array
    ShaderType::Struct type;

    const ShaderType *make_type() const {
      const ShaderType *new_type = ShaderType::register_type(std::move(type));
      if (num_elements > 0) {
        new_type = ShaderType::register_type(ShaderType::Array(new_type, num_elements));
      }
      return new_type;
    }
  };

  pdeque<StackItem> struct_stack;

  for (auto &uniform_item : uniforms) {
    GLint loc = uniform_item.first;
    Uniform &param = uniform_item.second;

    vector_string parts;
    tokenize(param.name, parts, ".");

    // Extract any [n] suffix on each part.
    vector_int sizes(parts.size(), 0);
    for (size_t i = 0; i < parts.size(); ++i) {
      std::string &part = parts[i];
      size_t p = part.find_last_of('[');
      if (p != std::string::npos) {
        sizes[i] = atoi(&part[p + 1]) + 1;
        part.resize(p);
      }
    }

    // Count the number of parts in the name that match the current stack.
    size_t i = 0;
    while (i < struct_stack.size() && i < parts.size() - 1 &&
           struct_stack[i].name == parts[i]) {
      if (sizes[i] > struct_stack[i].num_elements) {
        struct_stack[i].num_elements = sizes[i];
      }
      ++i;
    }

    // Pop everything else from the end of the stack.
    while (i < struct_stack.size()) {
      StackItem item = std::move(struct_stack.back());
      struct_stack.pop_back();
      const ShaderType *type = item.make_type();

      if (struct_stack.empty()) {
        // Don't record location, the driver may omit members of individual
        // structs in a struct array, we'll re-query them later
        CPT(InternalName) name = InternalName::make(item.name);
        _shader->add_parameter(std::move(name), type);
      } else {
        // Add as nested struct member
        struct_stack.back().type.merge_member_by_name(item.name, type);
      }
    }
    // Push the remaining parts (except the last) onto the stack.
    while (struct_stack.size() < parts.size() - 1) {
      struct_stack.push_back({parts[struct_stack.size()], sizes[struct_stack.size()], {}});
    }

    const ShaderType *type = get_param_type(param.type);
    if (sizes.back() > 0 || param.size > 1) {
      type = ShaderType::register_type(ShaderType::Array(type, param.size));
    }

    if (struct_stack.empty()) {
      // Add as top-level.  Recording the location here saves a
      // glGetUniformLocation call later on.
      assert(parts.size() == 1);
      CPT(InternalName) name = InternalName::make(parts[0]);
      _locations[name] = loc;
      _shader->add_parameter(std::move(name), type, loc);
    } else {
      // Add as struct member
      assert(parts.size() > 1);
      struct_stack.back().type.merge_member_by_name(parts.back(), type);
    }
  }

  while (!struct_stack.empty()) {
    StackItem item = std::move(struct_stack.back());
    struct_stack.pop_back();
    const ShaderType *type = item.make_type();

    if (struct_stack.empty()) {
      // Add struct as top-level.
      // Don't record location, the driver may omit members of individual
      // structs in a struct array, we'll re-query them later
      CPT(InternalName) name = InternalName::make(item.name);
      _shader->add_parameter(std::move(name), type);
    } else {
      // Add as nested struct member
      struct_stack.back().type.merge_member_by_name(item.name, type);
    }
  }

  _matrix_cache = pvector<LMatrix4>(_shader->_matrix_cache_desc.size(), LMatrix4::ident_mat());
  _matrix_cache_deps = _shader->_matrix_cache_deps;
}

/**
 * Analyzes the vertex attribute and stores the information it needs to
 * remember.
 */
void CLP(ShaderContext)::
reflect_attribute(GLuint program, int i, char *name_buffer, GLsizei name_buflen) {
  GLint param_size;
  GLenum param_type;

  // Get the name, size, and type of this attribute.
  name_buffer[0] = 0;
  _glgsg->_glGetActiveAttrib(program, i, name_buflen, nullptr,
                             &param_size, &param_type, name_buffer);

  // Get the attrib location.
  GLint p = _glgsg->_glGetAttribLocation(program, name_buffer);

  if (GLCAT.is_debug()) {
    GLCAT.debug()
      << "Active attribute " << name_buffer << " with size " << param_size
      << " and type 0x" << hex << param_type << dec
      << " is bound to location " << p << "\n";
  }

  if (p == -1 || strncmp(name_buffer, "gl_", 3) == 0) {
    // A gl_ attribute such as gl_Vertex requires us to pass the standard
    // vertex arrays as we would do without shader.  Not all drivers return -1
    // in glGetAttribLocation for gl_ prefixed attributes, so we check the
    // prefix of the input ourselves, just to be sure.
    _uses_standard_vertex_arrays = true;
    return;
  }

  if (strcmp(name_buffer, "p3d_Color") == 0) {
    // Save the index, so we can apply special handling to this attrib.
    _color_attrib_index = p;
  }

  CPT(InternalName) name = InternalName::make(name_buffer);
  _shader->bind_vertex_input(name, get_param_type(param_type), p);
  //FIXME matrices
}

/**
 * Analyzes the uniform block and stores its format.
 */
void CLP(ShaderContext)::
reflect_uniform_block(GLuint program, int i, const char *name, char *name_buffer, GLsizei name_buflen) {
 // GLint offset = 0;

  GLint data_size = 0;
  GLint param_count = 0;
  _glgsg->_glGetActiveUniformBlockiv(program, i, GL_UNIFORM_BLOCK_DATA_SIZE, &data_size);
  _glgsg->_glGetActiveUniformBlockiv(program, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &param_count);

  if (param_count <= 0) {
    return;
  }

  // We use a GeomVertexArrayFormat to describe the uniform buffer layout.
  // GeomVertexArrayFormat block_format; block_format.set_pad_to(data_size);

  // Get an array containing the indices of all the uniforms in this block.
  GLuint *indices = (GLuint *)alloca(param_count * sizeof(GLint));
  _glgsg->_glGetActiveUniformBlockiv(program, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, (GLint *)indices);

  // Acquire information about the way the uniforms in this block are packed.
  GLint *offsets = (GLint *)alloca(param_count * sizeof(GLint));
  GLint *mstrides = (GLint *)alloca(param_count * sizeof(GLint));
  GLint *astrides = (GLint *)alloca(param_count * sizeof(GLint));
  _glgsg->_glGetActiveUniformsiv(program, param_count, indices, GL_UNIFORM_OFFSET, offsets);
  _glgsg->_glGetActiveUniformsiv(program, param_count, indices, GL_UNIFORM_MATRIX_STRIDE, mstrides);
  _glgsg->_glGetActiveUniformsiv(program, param_count, indices, GL_UNIFORM_ARRAY_STRIDE, astrides);

  for (int ui = 0; ui < param_count; ++ui) {
    name_buffer[0] = 0;
    GLint param_size;
    GLenum param_type;
    _glgsg->_glGetActiveUniform(program, indices[ui], name_buflen, nullptr, &param_size, &param_type, name_buffer);

    // Strip off [0] suffix that some drivers append to arrays.
    size_t size = strlen(name_buffer);
    if (size > 3 && strncmp(name_buffer + (size - 3), "[0]", 3) == 0) {
      name_buffer[size - 3] = 0;
    }

    GeomEnums::NumericType numeric_type;
    GeomEnums::Contents contents = GeomEnums::C_other;
    int num_components = 1;

    switch (param_type) {
    case GL_INT:
    case GL_INT_VEC2:
    case GL_INT_VEC3:
    case GL_INT_VEC4:
      numeric_type = GeomEnums::NT_int32;
      break;

    case GL_BOOL:
    case GL_BOOL_VEC2:
    case GL_BOOL_VEC3:
    case GL_BOOL_VEC4:
    case GL_UNSIGNED_INT:
    case GL_UNSIGNED_INT_VEC2:
    case GL_UNSIGNED_INT_VEC3:
    case GL_UNSIGNED_INT_VEC4:
      numeric_type = GeomEnums::NT_uint32;
      break;

    case GL_FLOAT:
    case GL_FLOAT_VEC2:
    case GL_FLOAT_VEC3:
    case GL_FLOAT_VEC4:
    case GL_FLOAT_MAT2:
    case GL_FLOAT_MAT3:
    case GL_FLOAT_MAT4:
      numeric_type = GeomEnums::NT_float32;
      break;

#ifndef OPENGLES
    case GL_DOUBLE:
    case GL_DOUBLE_VEC2:
    case GL_DOUBLE_VEC3:
    case GL_DOUBLE_VEC4:
    case GL_DOUBLE_MAT2:
    case GL_DOUBLE_MAT3:
    case GL_DOUBLE_MAT4:
      numeric_type = GeomEnums::NT_float64;
      break;
#endif

    default:
      GLCAT.info() << "Ignoring uniform '" << name_buffer
        << "' with unsupported type 0x" << hex << param_type << dec << "\n";
      continue;
    }

    switch (param_type) {
    case GL_INT_VEC2:
    case GL_BOOL_VEC2:
    case GL_UNSIGNED_INT_VEC2:
    case GL_FLOAT_VEC2:
#ifndef OPENGLES
    case GL_DOUBLE_VEC2:
#endif
      num_components = 2;
      break;

    case GL_INT_VEC3:
    case GL_BOOL_VEC3:
    case GL_UNSIGNED_INT_VEC3:
    case GL_FLOAT_VEC3:
#ifndef OPENGLES
    case GL_DOUBLE_VEC3:
#endif
      num_components = 3;
      break;

    case GL_INT_VEC4:
    case GL_BOOL_VEC4:
    case GL_UNSIGNED_INT_VEC4:
    case GL_FLOAT_VEC4:
#ifndef OPENGLES
    case GL_DOUBLE_VEC4:
#endif
      num_components = 4;
      break;

    case GL_FLOAT_MAT3:
#ifndef OPENGLES
    case GL_DOUBLE_MAT3:
#endif
      num_components = 3;
      contents = GeomEnums::C_matrix;
      nassertd(param_size <= 1 || astrides[ui] == mstrides[ui] * 3) continue;
      param_size *= 3;
      break;

    case GL_FLOAT_MAT4:
#ifndef OPENGLES
    case GL_DOUBLE_MAT4:
#endif
      num_components = 4;
      contents = GeomEnums::C_matrix;
      nassertd(param_size <= 1 || astrides[ui] == mstrides[ui] * 4) continue;
      param_size *= 4;
      break;
    }

    (void)numeric_type;
    (void)contents;
    (void)num_components;
    // GeomVertexColumn column(InternalName::make(name_buffer),
    // num_components, numeric_type, contents, offsets[ui], 4, param_size,
    // astrides[ui]); block_format.add_column(column);
  }

  // if (GLCAT.is_debug()) { GLCAT.debug() << "Active uniform block " << name
  // << " has format:\n"; block_format.write(GLCAT.debug(false), 2); }

  // UniformBlock block; block._name = InternalName::make(name); block._format
  // = GeomVertexArrayFormat::register_format(&block_format); block._buffer =
  // 0;

  // _uniform_blocks.push_back(block);
}

/**
 * Converts an OpenGL type enum to a ShaderType.
 */
const ShaderType *CLP(ShaderContext)::
get_param_type(GLenum param_type) {
  switch (param_type) {
  case GL_FLOAT:
    return ShaderType::float_type;

  case GL_FLOAT_VEC2:
    return ShaderType::register_type(ShaderType::Vector(ShaderType::ST_float, 2));

  case GL_FLOAT_VEC3:
    return ShaderType::register_type(ShaderType::Vector(ShaderType::ST_float, 3));

  case GL_FLOAT_VEC4:
    return ShaderType::register_type(ShaderType::Vector(ShaderType::ST_float, 4));

  case GL_FLOAT_MAT2:
    return ShaderType::register_type(ShaderType::Matrix(ShaderType::ST_float, 2, 2));

  case GL_FLOAT_MAT3:
    return ShaderType::register_type(ShaderType::Matrix(ShaderType::ST_float, 3, 3));

  case GL_FLOAT_MAT4:
    return ShaderType::register_type(ShaderType::Matrix(ShaderType::ST_float, 4, 4));

  case GL_FLOAT_MAT2x3:
    return ShaderType::register_type(ShaderType::Matrix(ShaderType::ST_float, 2, 3));

  case GL_FLOAT_MAT2x4:
    return ShaderType::register_type(ShaderType::Matrix(ShaderType::ST_float, 2, 4));

  case GL_FLOAT_MAT3x2:
    return ShaderType::register_type(ShaderType::Matrix(ShaderType::ST_float, 3, 2));

  case GL_FLOAT_MAT3x4:
    return ShaderType::register_type(ShaderType::Matrix(ShaderType::ST_float, 3, 4));

  case GL_FLOAT_MAT4x2:
    return ShaderType::register_type(ShaderType::Matrix(ShaderType::ST_float, 4, 2));

  case GL_FLOAT_MAT4x3:
    return ShaderType::register_type(ShaderType::Matrix(ShaderType::ST_float, 4, 3));

  case GL_INT:
    return ShaderType::int_type;

  case GL_INT_VEC2:
    return ShaderType::register_type(ShaderType::Vector(ShaderType::ST_int, 2));

  case GL_INT_VEC3:
    return ShaderType::register_type(ShaderType::Vector(ShaderType::ST_int, 3));

  case GL_INT_VEC4:
    return ShaderType::register_type(ShaderType::Vector(ShaderType::ST_int, 4));

  case GL_BOOL:
    return ShaderType::bool_type;

  case GL_BOOL_VEC2:
    return ShaderType::register_type(ShaderType::Vector(ShaderType::ST_bool, 2));

  case GL_BOOL_VEC3:
    return ShaderType::register_type(ShaderType::Vector(ShaderType::ST_bool, 3));

  case GL_BOOL_VEC4:
    return ShaderType::register_type(ShaderType::Vector(ShaderType::ST_bool, 4));

  case GL_UNSIGNED_INT:
    return ShaderType::uint_type;

  case GL_UNSIGNED_INT_VEC2:
    return ShaderType::register_type(ShaderType::Vector(ShaderType::ST_uint, 2));

  case GL_UNSIGNED_INT_VEC3:
    return ShaderType::register_type(ShaderType::Vector(ShaderType::ST_uint, 3));

  case GL_UNSIGNED_INT_VEC4:
    return ShaderType::register_type(ShaderType::Vector(ShaderType::ST_uint, 4));

#ifndef OPENGLES
  case GL_DOUBLE:
    return ShaderType::double_type;

  case GL_DOUBLE_VEC2:
    return ShaderType::register_type(ShaderType::Vector(ShaderType::ST_double, 2));

  case GL_DOUBLE_VEC3:
    return ShaderType::register_type(ShaderType::Vector(ShaderType::ST_double, 3));

  case GL_DOUBLE_VEC4:
    return ShaderType::register_type(ShaderType::Vector(ShaderType::ST_double, 4));

  case GL_DOUBLE_MAT2:
    return ShaderType::register_type(ShaderType::Matrix(ShaderType::ST_double, 2, 2));

  case GL_DOUBLE_MAT3:
    return ShaderType::register_type(ShaderType::Matrix(ShaderType::ST_double, 3, 3));

  case GL_DOUBLE_MAT4:
    return ShaderType::register_type(ShaderType::Matrix(ShaderType::ST_double, 4, 4));

  case GL_DOUBLE_MAT2x3:
    return ShaderType::register_type(ShaderType::Matrix(ShaderType::ST_double, 2, 3));

  case GL_DOUBLE_MAT2x4:
    return ShaderType::register_type(ShaderType::Matrix(ShaderType::ST_double, 2, 4));

  case GL_DOUBLE_MAT3x2:
    return ShaderType::register_type(ShaderType::Matrix(ShaderType::ST_double, 3, 2));

  case GL_DOUBLE_MAT3x4:
    return ShaderType::register_type(ShaderType::Matrix(ShaderType::ST_double, 3, 4));

  case GL_DOUBLE_MAT4x2:
    return ShaderType::register_type(ShaderType::Matrix(ShaderType::ST_double, 4, 2));

  case GL_DOUBLE_MAT4x3:
    return ShaderType::register_type(ShaderType::Matrix(ShaderType::ST_double, 4, 3));
#endif

#ifndef OPENGLES
  case GL_SAMPLER_1D:
  case GL_SAMPLER_1D_SHADOW:
    return ShaderType::register_type(ShaderType::SampledImage(Texture::TT_1d_texture, ShaderType::ST_float));

  case GL_INT_SAMPLER_1D:
    return ShaderType::register_type(ShaderType::SampledImage(Texture::TT_1d_texture, ShaderType::ST_int));

  case GL_UNSIGNED_INT_SAMPLER_1D:
    return ShaderType::register_type(ShaderType::SampledImage(Texture::TT_1d_texture, ShaderType::ST_uint));

  case GL_SAMPLER_1D_ARRAY:
    return ShaderType::register_type(ShaderType::SampledImage(Texture::TT_1d_texture_array, ShaderType::ST_float));

  case GL_INT_SAMPLER_1D_ARRAY:
    return ShaderType::register_type(ShaderType::SampledImage(Texture::TT_1d_texture_array, ShaderType::ST_int));

  case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
    return ShaderType::register_type(ShaderType::SampledImage(Texture::TT_1d_texture_array, ShaderType::ST_uint));
#endif

  case GL_SAMPLER_2D:
  case GL_SAMPLER_2D_SHADOW:
    return ShaderType::register_type(ShaderType::SampledImage(Texture::TT_2d_texture, ShaderType::ST_float));

  case GL_INT_SAMPLER_2D:
    return ShaderType::register_type(ShaderType::SampledImage(Texture::TT_2d_texture, ShaderType::ST_int));

  case GL_UNSIGNED_INT_SAMPLER_2D:
    return ShaderType::register_type(ShaderType::SampledImage(Texture::TT_2d_texture, ShaderType::ST_uint));

  case GL_SAMPLER_3D:
    return ShaderType::register_type(ShaderType::SampledImage(Texture::TT_3d_texture, ShaderType::ST_float));

  case GL_INT_SAMPLER_3D:
    return ShaderType::register_type(ShaderType::SampledImage(Texture::TT_3d_texture, ShaderType::ST_int));

  case GL_UNSIGNED_INT_SAMPLER_3D:
    return ShaderType::register_type(ShaderType::SampledImage(Texture::TT_3d_texture, ShaderType::ST_uint));

  case GL_SAMPLER_CUBE:
  case GL_SAMPLER_CUBE_SHADOW:
    return ShaderType::register_type(ShaderType::SampledImage(Texture::TT_cube_map, ShaderType::ST_float));

  case GL_INT_SAMPLER_CUBE:
    return ShaderType::register_type(ShaderType::SampledImage(Texture::TT_cube_map, ShaderType::ST_int));

  case GL_UNSIGNED_INT_SAMPLER_CUBE:
    return ShaderType::register_type(ShaderType::SampledImage(Texture::TT_cube_map, ShaderType::ST_uint));

  case GL_SAMPLER_2D_ARRAY:
  case GL_SAMPLER_2D_ARRAY_SHADOW:
    return ShaderType::register_type(ShaderType::SampledImage(Texture::TT_2d_texture_array, ShaderType::ST_float));

  case GL_INT_SAMPLER_2D_ARRAY:
    return ShaderType::register_type(ShaderType::SampledImage(Texture::TT_2d_texture_array, ShaderType::ST_int));

  case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
    return ShaderType::register_type(ShaderType::SampledImage(Texture::TT_2d_texture_array, ShaderType::ST_uint));

  case GL_SAMPLER_CUBE_MAP_ARRAY:
  case GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW:
    return ShaderType::register_type(ShaderType::SampledImage(Texture::TT_cube_map_array, ShaderType::ST_float));

  case GL_INT_SAMPLER_CUBE_MAP_ARRAY:
    return ShaderType::register_type(ShaderType::SampledImage(Texture::TT_cube_map_array, ShaderType::ST_int));

  case GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY:
    return ShaderType::register_type(ShaderType::SampledImage(Texture::TT_cube_map_array, ShaderType::ST_uint));

  case GL_SAMPLER_BUFFER:
    return ShaderType::register_type(ShaderType::SampledImage(Texture::TT_buffer_texture, ShaderType::ST_float));

  case GL_INT_SAMPLER_BUFFER:
    return ShaderType::register_type(ShaderType::SampledImage(Texture::TT_buffer_texture, ShaderType::ST_int));

  case GL_UNSIGNED_INT_SAMPLER_BUFFER:
    return ShaderType::register_type(ShaderType::SampledImage(Texture::TT_buffer_texture, ShaderType::ST_uint));

#ifndef OPENGLES
  case GL_IMAGE_1D:
    return ShaderType::register_type(ShaderType::Image(Texture::TT_1d_texture, ShaderType::ST_float, ShaderType::Access::READ_WRITE));

  case GL_INT_IMAGE_1D:
    return ShaderType::register_type(ShaderType::Image(Texture::TT_1d_texture, ShaderType::ST_int, ShaderType::Access::READ_WRITE));

  case GL_UNSIGNED_INT_IMAGE_1D:
    return ShaderType::register_type(ShaderType::Image(Texture::TT_1d_texture, ShaderType::ST_uint, ShaderType::Access::READ_WRITE));

  case GL_IMAGE_1D_ARRAY:
    return ShaderType::register_type(ShaderType::Image(Texture::TT_1d_texture_array, ShaderType::ST_float, ShaderType::Access::READ_WRITE));

  case GL_INT_IMAGE_1D_ARRAY:
    return ShaderType::register_type(ShaderType::Image(Texture::TT_1d_texture_array, ShaderType::ST_int, ShaderType::Access::READ_WRITE));

  case GL_UNSIGNED_INT_IMAGE_1D_ARRAY:
    return ShaderType::register_type(ShaderType::Image(Texture::TT_1d_texture_array, ShaderType::ST_uint, ShaderType::Access::READ_WRITE));
#endif

  case GL_IMAGE_2D:
    return ShaderType::register_type(ShaderType::Image(Texture::TT_2d_texture, ShaderType::ST_float, ShaderType::Access::READ_WRITE));

  case GL_INT_IMAGE_2D:
    return ShaderType::register_type(ShaderType::Image(Texture::TT_2d_texture, ShaderType::ST_int, ShaderType::Access::READ_WRITE));

  case GL_UNSIGNED_INT_IMAGE_2D:
    return ShaderType::register_type(ShaderType::Image(Texture::TT_2d_texture, ShaderType::ST_uint, ShaderType::Access::READ_WRITE));

  case GL_IMAGE_2D_ARRAY:
    return ShaderType::register_type(ShaderType::Image(Texture::TT_2d_texture_array, ShaderType::ST_float, ShaderType::Access::READ_WRITE));

  case GL_INT_IMAGE_2D_ARRAY:
    return ShaderType::register_type(ShaderType::Image(Texture::TT_2d_texture_array, ShaderType::ST_int, ShaderType::Access::READ_WRITE));

  case GL_UNSIGNED_INT_IMAGE_2D_ARRAY:
    return ShaderType::register_type(ShaderType::Image(Texture::TT_2d_texture_array, ShaderType::ST_uint, ShaderType::Access::READ_WRITE));

  case GL_IMAGE_3D:
    return ShaderType::register_type(ShaderType::Image(Texture::TT_3d_texture, ShaderType::ST_float, ShaderType::Access::READ_WRITE));

  case GL_INT_IMAGE_3D:
    return ShaderType::register_type(ShaderType::Image(Texture::TT_3d_texture, ShaderType::ST_int, ShaderType::Access::READ_WRITE));

  case GL_UNSIGNED_INT_IMAGE_3D:
    return ShaderType::register_type(ShaderType::Image(Texture::TT_3d_texture, ShaderType::ST_uint, ShaderType::Access::READ_WRITE));

  case GL_IMAGE_CUBE:
    return ShaderType::register_type(ShaderType::Image(Texture::TT_cube_map, ShaderType::ST_float, ShaderType::Access::READ_WRITE));

  case GL_INT_IMAGE_CUBE:
    return ShaderType::register_type(ShaderType::Image(Texture::TT_cube_map, ShaderType::ST_int, ShaderType::Access::READ_WRITE));

  case GL_UNSIGNED_INT_IMAGE_CUBE:
    return ShaderType::register_type(ShaderType::Image(Texture::TT_cube_map, ShaderType::ST_uint, ShaderType::Access::READ_WRITE));

  case GL_IMAGE_CUBE_MAP_ARRAY:
    return ShaderType::register_type(ShaderType::Image(Texture::TT_cube_map_array, ShaderType::ST_float, ShaderType::Access::READ_WRITE));

  case GL_INT_IMAGE_CUBE_MAP_ARRAY:
    return ShaderType::register_type(ShaderType::Image(Texture::TT_cube_map_array, ShaderType::ST_int, ShaderType::Access::READ_WRITE));

  case GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY:
    return ShaderType::register_type(ShaderType::Image(Texture::TT_cube_map_array, ShaderType::ST_uint, ShaderType::Access::READ_WRITE));

  case GL_IMAGE_BUFFER:
    return ShaderType::register_type(ShaderType::Image(Texture::TT_buffer_texture, ShaderType::ST_float, ShaderType::Access::READ_WRITE));

  case GL_INT_IMAGE_BUFFER:
    return ShaderType::register_type(ShaderType::Image(Texture::TT_buffer_texture, ShaderType::ST_int, ShaderType::Access::READ_WRITE));

  case GL_UNSIGNED_INT_IMAGE_BUFFER:
    return ShaderType::register_type(ShaderType::Image(Texture::TT_buffer_texture, ShaderType::ST_uint, ShaderType::Access::READ_WRITE));
  }

  GLCAT.error()
    << "Encountered unknown shader variable type "
    << std::hex << param_type << std::dec << "\n";
  return nullptr;
}

/**
 * Returns the texture type required for the given GL sampler type.  Returns
 * false if unsupported.
 */
bool CLP(ShaderContext)::
get_sampler_texture_type(int &out, GLenum param_type) {
  switch (param_type) {
#ifndef OPENGLES
  case GL_SAMPLER_1D_SHADOW:
    if (!_glgsg->_supports_shadow_filter) {
      GLCAT.error()
        << "GLSL shader uses shadow sampler, which is unsupported by the driver.\n";
      return false;
    }
    // Fall through
  case GL_INT_SAMPLER_1D:
  case GL_UNSIGNED_INT_SAMPLER_1D:
  case GL_SAMPLER_1D:
    out = Texture::TT_1d_texture;
    return true;

  case GL_INT_SAMPLER_1D_ARRAY:
  case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
  case GL_SAMPLER_1D_ARRAY:
    out = Texture::TT_1d_texture_array;
    return true;
#endif

  case GL_INT_SAMPLER_2D:
  case GL_UNSIGNED_INT_SAMPLER_2D:
  case GL_SAMPLER_2D:
    out = Texture::TT_2d_texture;
    return true;

  case GL_SAMPLER_2D_SHADOW:
    out = Texture::TT_2d_texture;
    if (!_glgsg->_supports_shadow_filter) {
      GLCAT.error()
        << "GLSL shader uses shadow sampler, which is unsupported by the driver.\n";
      return false;
    }
    return true;

  case GL_INT_SAMPLER_3D:
  case GL_UNSIGNED_INT_SAMPLER_3D:
  case GL_SAMPLER_3D:
    out = Texture::TT_3d_texture;
    if (_glgsg->_supports_3d_texture) {
      return true;
    } else {
      GLCAT.error()
        << "GLSL shader uses 3D texture, which is unsupported by the driver.\n";
      return false;
    }

  case GL_SAMPLER_CUBE_SHADOW:
    if (!_glgsg->_supports_shadow_filter) {
      GLCAT.error()
        << "GLSL shader uses shadow sampler, which is unsupported by the driver.\n";
      return false;
    }
    // Fall through
  case GL_INT_SAMPLER_CUBE:
  case GL_UNSIGNED_INT_SAMPLER_CUBE:
  case GL_SAMPLER_CUBE:
    out = Texture::TT_cube_map;
    if (!_glgsg->_supports_cube_map) {
      GLCAT.error()
        << "GLSL shader uses cube map, which is unsupported by the driver.\n";
      return false;
    }
    return true;

  case GL_SAMPLER_2D_ARRAY_SHADOW:
    if (!_glgsg->_supports_shadow_filter) {
      GLCAT.error()
        << "GLSL shader uses shadow sampler, which is unsupported by the driver.\n";
      return false;
    }
    // Fall through
  case GL_INT_SAMPLER_2D_ARRAY:
  case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
  case GL_SAMPLER_2D_ARRAY:
    out = Texture::TT_2d_texture_array;
    if (_glgsg->_supports_2d_texture_array) {
      return true;
    } else {
      GLCAT.error()
        << "GLSL shader uses 2D texture array, which is unsupported by the driver.\n";
      return false;
    }

  case GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW:
    if (!_glgsg->_supports_shadow_filter) {
      GLCAT.error()
        << "GLSL shader uses shadow sampler, which is unsupported by the driver.\n";
      return false;
    }
    // Fall through
  case GL_INT_SAMPLER_CUBE_MAP_ARRAY:
  case GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY:
  case GL_SAMPLER_CUBE_MAP_ARRAY:
    out = Texture::TT_cube_map_array;
    if (_glgsg->_supports_cube_map_array) {
      return true;
    } else {
      GLCAT.error()
        << "GLSL shader uses cube map array, which is unsupported by the driver.\n";
      return false;

    }

  case GL_INT_SAMPLER_BUFFER:
  case GL_UNSIGNED_INT_SAMPLER_BUFFER:
  case GL_SAMPLER_BUFFER:
    out = Texture::TT_buffer_texture;
    if (_glgsg->_supports_buffer_texture) {
      return true;
    } else {
      GLCAT.error()
        << "GLSL shader uses buffer texture, which is unsupported by the driver.\n";
      return false;
    }

  default:
    GLCAT.error()
      << "GLSL shader uses unsupported sampler type for texture input.\n";
    return false;
  }
}

/**
 * xyz
 */
CLP(ShaderContext)::
~CLP(ShaderContext)() {
  // Don't call release_resources; we may not have an active context.
}

/**
 * Should deallocate all system resources (such as vertex program handles or
 * Cg contexts).
 */
void CLP(ShaderContext)::
release_resources(CLP(GraphicsStateGuardian) *glgsg) {
  if (_program != 0) {
    glgsg->_glDeleteProgram(_program);
    _program = 0;
  }
  if (_inject_alpha_test) {
    for (int i = 0; i < RenderAttrib::M_always; ++i) {
      if (_linked_programs[i] != 0) {
        glgsg->_glDeleteProgram(_linked_programs[i]);
        _linked_programs[i] = 0;
      }
    }
  }

  for (Module &module : _modules) {
    glgsg->_glDeleteShader(module._handle);
  }

  _modules.clear();

  glgsg->report_my_gl_errors();
}

/**
 * This function gets called whenever the RenderState or TransformState has
 * changed, but the Shader itself has not changed.  It loads new values into
 * the shader's parameters.
 */
void CLP(ShaderContext)::
set_state_and_transform(const RenderState *target_rs,
                        const TransformState *modelview_transform,
                        const TransformState *camera_transform,
                        const TransformState *projection_transform) {

  // Find out which state properties have changed.
  int altered = 0;

  if (_modelview_transform != modelview_transform) {
    _modelview_transform = modelview_transform;
    altered |= Shader::D_transform;
  }
  if (_camera_transform != camera_transform) {
    _camera_transform = camera_transform;
    altered |= Shader::D_view_transform;
  }
  if (_projection_transform != projection_transform) {
    _projection_transform = projection_transform;
    altered |= Shader::D_projection;
  }

  CPT(RenderState) state_rs = _state_rs.lock();
  if (state_rs == nullptr) {
    // Reset all of the state.
    altered |= Shader::D_state;
    _state_rs = target_rs;
    target_rs->get_attrib_def(_color_attrib);
  }
  else if (state_rs == target_rs) {
    // State is the same from last time.
  }
  else if (_inject_alpha_test &&
           target_rs->get_alpha_test_mode() != _alpha_test_mode) {
    // Alpha test mode has changed, bind a different shader and respecify all
    // data.
    bind(_glgsg, target_rs->get_alpha_test_mode());
    altered = _uniform_data_deps | Shader::D_alpha_test;
    _state_rs = target_rs;
  }
  else {
    // The state has changed since last time.
    if (_inject_alpha_test) {
      if (state_rs->get_attrib(AlphaTestAttrib::get_class_slot()) !=
          target_rs->get_attrib(AlphaTestAttrib::get_class_slot())) {
        altered |= Shader::D_alpha_test;
      }
    }
    if (_color_attrib != target_rs->get_attrib(ColorAttrib::get_class_slot())) {
      altered |= Shader::D_color;
      target_rs->get_attrib_def(_color_attrib);
    }
    if (state_rs->get_attrib(ColorScaleAttrib::get_class_slot()) !=
        target_rs->get_attrib(ColorScaleAttrib::get_class_slot())) {
      altered |= Shader::D_colorscale;
    }
    if (state_rs->get_attrib(MaterialAttrib::get_class_slot()) !=
        target_rs->get_attrib(MaterialAttrib::get_class_slot())) {
      altered |= Shader::D_material;
    }
    if (state_rs->get_attrib(FogAttrib::get_class_slot()) !=
        target_rs->get_attrib(FogAttrib::get_class_slot())) {
      altered |= Shader::D_fog;
    }
    if (state_rs->get_attrib(LightAttrib::get_class_slot()) !=
        target_rs->get_attrib(LightAttrib::get_class_slot())) {
      altered |= Shader::D_light;
    }
    if (state_rs->get_attrib(ClipPlaneAttrib::get_class_slot()) !=
        target_rs->get_attrib(ClipPlaneAttrib::get_class_slot())) {
      altered |= Shader::D_clip_planes;
    }
    if (state_rs->get_attrib(TexMatrixAttrib::get_class_slot()) !=
        target_rs->get_attrib(TexMatrixAttrib::get_class_slot())) {
      altered |= Shader::D_tex_matrix;
    }
    if (state_rs->get_attrib(TextureAttrib::get_class_slot()) !=
        target_rs->get_attrib(TextureAttrib::get_class_slot())) {
      altered |= Shader::D_texture;
    }
    if (state_rs->get_attrib(TexGenAttrib::get_class_slot()) !=
        target_rs->get_attrib(TexGenAttrib::get_class_slot())) {
      altered |= Shader::D_tex_gen;
    }
    if (state_rs->get_attrib(RenderModeAttrib::get_class_slot()) !=
        target_rs->get_attrib(RenderModeAttrib::get_class_slot())) {
      altered |= Shader::D_render_mode;
    }
    _state_rs = target_rs;
  }

  if (_shader_attrib != _glgsg->_target_shader) {
    altered |= Shader::D_shader_inputs;
    _shader_attrib = _glgsg->_target_shader;
  }

  // Is this the first time this shader is used this frame?
  int frame_number = ClockObject::get_global_clock()->get_frame_count();
  if (frame_number != _frame_number) {
     altered |= Shader::D_frame;
    _frame_number = frame_number;
  }

  if (altered != 0) {
    issue_parameters(altered);
  }
}

/**
 * This function gets called whenever the RenderState or TransformState has
 * changed, but the Shader itself has not changed.  It loads new values into
 * the shader's parameters.
 */
void CLP(ShaderContext)::
issue_parameters(int altered) {
  PStatTimer timer(_glgsg->_draw_set_state_shader_parameters_pcollector);

  if (GLCAT.is_spam()) {
    GLCAT.spam()
      << "Setting uniforms for " << _shader->get_filename()
      << " (altered 0x" << hex << altered << dec << ")\n";
  }

#ifdef __GNUC__
  unsigned char *scratch = (unsigned char *)__builtin_alloca_with_align(_scratch_space_size, 128);
#else
  unsigned char *scratch = (unsigned char *)alloca(_scratch_space_size);
#endif

  if (altered & _uniform_data_deps) {
    if (altered & _matrix_cache_deps) {
      _glgsg->update_shader_matrix_cache(_shader, &_matrix_cache[0], altered);
    }

    ShaderInputBinding::State state;
    state.gsg = _glgsg;
    state.matrix_cache = &_matrix_cache[0];

    for (const UniformBlock &block : _uniform_blocks) {
      if ((altered & block._dep) == 0) {
        continue;
      }

      for (const UniformBlock::Binding &binding : block._bindings) {
        binding._binding->fetch_data(state, scratch + binding._offset, true);
      }

      const UniformCalls &calls = block._calls[_alpha_test_mode];
      for (const UniformCall &call : calls._matrices) {
        ((PFNGLUNIFORMMATRIX4FVPROC)call._func)(call._location, call._count, GL_FALSE, (const GLfloat *)(scratch + call._offset));
      }

      for (const UniformCall &call : calls._vectors) {
        ((PFNGLUNIFORM4FVPROC)call._func)(call._location, call._count, (const GLfloat *)(scratch + call._offset));
      }
    }
  }

  if (altered & Shader::D_alpha_test) {
    GLint aref_loc = _alpha_test_ref_locations[_alpha_test_mode];
    if (aref_loc >= 0) {
      const AlphaTestAttrib *alpha_test;
      _state_rs->get_attrib_def(alpha_test);
      _glgsg->_glUniform1f(aref_loc, alpha_test->get_reference_alpha());
    }
  }

  _glgsg->report_my_gl_errors();
}

/**
 * Disable all the vertex arrays used by this shader.
 */
void CLP(ShaderContext)::
disable_shader_vertex_arrays() {
  //if (_glsl_program == 0) {
  //  return;
  //}

  for (const Shader::ShaderVarSpec &bind : _shader->_var_spec) {
    GLint p = bind._id._location;

    for (int i = 0; i < bind._elements; ++i) {
      _glgsg->disable_vertex_attrib_array(p + i);
    }
  }

  _glgsg->report_my_gl_errors();
}

/**
 * Disables all vertex arrays used by the previous shader, then enables all
 * the vertex arrays needed by this shader.  Extracts the relevant vertex
 * array data from the gsg.
 */
bool CLP(ShaderContext)::
update_shader_vertex_arrays(ShaderContext *prev, bool force) {
  //if (_glsl_program == 0) {
  //  return true;
  //}

  // Get the active ColorAttrib.  We'll need it to determine how to apply
  // vertex colors.
  const ColorAttrib *color_attrib = _color_attrib;

  const GeomVertexArrayDataHandle *array_reader;

  if (_glgsg->_use_vertex_attrib_binding) {
    // Use experimental new separated formatbinding state.
    const GeomVertexDataPipelineReader *data_reader = _glgsg->_data_reader;

    for (size_t ai = 0; ai < data_reader->get_num_arrays(); ++ai) {
      array_reader = data_reader->get_array_reader(ai);

      // Make sure the vertex buffer is up-to-date.
      CLP(VertexBufferContext) *gvbc = DCAST(CLP(VertexBufferContext),
        array_reader->prepare_now(_glgsg->get_prepared_objects(), _glgsg));
      nassertr(gvbc != (CLP(VertexBufferContext) *)nullptr, false);

      if (!_glgsg->update_vertex_buffer(gvbc, array_reader, force)) {
        return false;
      }

      GLintptr stride = array_reader->get_array_format()->get_stride();

      // Bind the vertex buffer to the binding index.
      if (ai >= _glgsg->_current_vertex_buffers.size()) {
        GLuint zero = 0;
        _glgsg->_current_vertex_buffers.resize(ai + 1, zero);
      }
      if (_glgsg->_current_vertex_buffers[ai] != gvbc->_index) {
        _glgsg->_glBindVertexBuffer(ai, gvbc->_index, 0, stride);
        _glgsg->_current_vertex_buffers[ai] = gvbc->_index;
      }
    }

    // Figure out which attributes to enable or disable.
    BitMask32 enabled_attribs = _enabled_attribs;
    if (_color_attrib_index != -1 &&
        color_attrib->get_color_type() != ColorAttrib::T_vertex) {
      // Vertex colours are disabled.
      enabled_attribs.clear_bit(_color_attrib_index);

#ifdef STDFLOAT_DOUBLE
      _glgsg->_glVertexAttrib4dv(_color_attrib_index, color_attrib->get_color().get_data());
#else
      _glgsg->_glVertexAttrib4fv(_color_attrib_index, color_attrib->get_color().get_data());
#endif
    }

    BitMask32 changed_attribs = enabled_attribs ^ _glgsg->_enabled_vertex_attrib_arrays;

    for (int i = 0; i < 32; ++i) {
      if (changed_attribs.get_bit(i)) {
        if (enabled_attribs.get_bit(i)) {
          _glgsg->_glEnableVertexAttribArray(i);
        } else {
          _glgsg->_glDisableVertexAttribArray(i);
        }
      }
    }
    _glgsg->_enabled_vertex_attrib_arrays = enabled_attribs;

  } else {
    Geom::NumericType numeric_type;
    int start, stride, num_values;
    size_t nvarying = _shader->_var_spec.size();

    GLint max_p = 0;

    for (size_t i = 0; i < nvarying; ++i) {
      const Shader::ShaderVarSpec &bind = _shader->_var_spec[i];
      InternalName *name = bind._name;
      int texslot = bind._append_uv;

      if (texslot >= 0 && texslot < _glgsg->_state_texture->get_num_on_stages()) {
        TextureStage *stage = _glgsg->_state_texture->get_on_stage(texslot);
        InternalName *texname = stage->get_texcoord_name();

        if (name == InternalName::get_texcoord()) {
          name = texname;
        } else if (texname != InternalName::get_texcoord()) {
          name = name->append(texname->get_basename());
        }
      }

      GLint p = bind._id._location;
      max_p = max(max_p, p + bind._elements);

      // Don't apply vertex colors if they are disabled with a ColorAttrib.
      int num_elements, element_stride, divisor;
      bool normalized;
      if ((p != _color_attrib_index || color_attrib->get_color_type() == ColorAttrib::T_vertex) &&
          _glgsg->_data_reader->get_array_info(name, array_reader,
                                               num_values, numeric_type,
                                               normalized, start, stride, divisor,
                                               num_elements, element_stride)) {
        const unsigned char *client_pointer;
        if (!_glgsg->setup_array_data(client_pointer, array_reader, force)) {
          return false;
        }
        client_pointer += start;

        GLenum type = _glgsg->get_numeric_type(numeric_type);
        for (int i = 0; i < num_elements; ++i) {
          _glgsg->enable_vertex_attrib_array(p);

          if (numeric_type == GeomEnums::NT_packed_dabc) {
            // GL_BGRA is a special accepted value available since OpenGL 3.2.
            // It requires us to pass GL_TRUE for normalized.
            _glgsg->_glVertexAttribPointer(p, GL_BGRA, GL_UNSIGNED_BYTE,
                                           GL_TRUE, stride, client_pointer);
          }
          else if (_emulate_float_attribs ||
                   bind._scalar_type == ShaderType::ST_float ||
                   numeric_type == GeomEnums::NT_float32) {
            _glgsg->_glVertexAttribPointer(p, num_values, type,
                                           normalized, stride, client_pointer);
          }
          else if (bind._scalar_type == ShaderType::ST_double) {
            _glgsg->_glVertexAttribLPointer(p, num_values, type,
                                            stride, client_pointer);
          }
          else {
            _glgsg->_glVertexAttribIPointer(p, num_values, type,
                                            stride, client_pointer);
          }

          _glgsg->set_vertex_attrib_divisor(p, divisor);

          ++p;
          client_pointer += element_stride;
        }
      } else {
        for (int i = 0; i < bind._elements; ++i) {
          _glgsg->disable_vertex_attrib_array(p + i);
        }
        if (p == _color_attrib_index) {
          // Vertex colors are disabled or not present.  Apply flat color.
#ifdef STDFLOAT_DOUBLE
          _glgsg->_glVertexAttrib4dv(p, _glgsg->_scene_graph_color.get_data());
#else
          _glgsg->_glVertexAttrib4fv(p, _glgsg->_scene_graph_color.get_data());
#endif
        }
        else if (name == InternalName::get_transform_index() &&
                 _glgsg->_glVertexAttribI4ui != nullptr) {
          _glgsg->_glVertexAttribI4ui(p, 0, 1, 2, 3);
        }
        else if (name == InternalName::get_transform_weight()) {
          // NVIDIA doesn't seem to use to use these defaults by itself
          static const GLfloat weights[4] = {0, 0, 0, 1};
          _glgsg->_glVertexAttrib4fv(p, weights);
        }
        else if (name == InternalName::get_instance_matrix()) {
          const LMatrix4 &ident_mat = LMatrix4::ident_mat();

          for (int i = 0; i < bind._elements; ++i) {
#ifdef STDFLOAT_DOUBLE
            _glgsg->_glVertexAttrib4dv(p, ident_mat.get_data() + i * 4);
#else
            _glgsg->_glVertexAttrib4fv(p, ident_mat.get_data() + i * 4);
#endif
            ++p;
          }
        }
      }
    }

    // Disable attribute arrays we don't use.
    GLint highest_p = _glgsg->_enabled_vertex_attrib_arrays.get_highest_on_bit() + 1;
    for (GLint p = max_p; p < highest_p; ++p) {
      _glgsg->disable_vertex_attrib_array(p);
    }
  }

  if (_uniform_data_deps & Shader::D_vertex_data) {
    issue_parameters(Shader::D_vertex_data);
  }

  // This ought to be moved elsewhere, but it's convenient to do this here for
  // now since it's called before every Geom is drawn.
  issue_memory_barriers();

  _glgsg->report_my_gl_errors();

  return true;
}

/**
 * Disable all the texture bindings used by this shader.
 */
void CLP(ShaderContext)::
disable_shader_texture_bindings() {
  //if (_glsl_program == 0) {
  //  return;
  //}

  DO_PSTATS_STUFF(_glgsg->_texture_state_pcollector.add_level(1));

#ifndef OPENGLES
  if (_glgsg->_supports_multi_bind) {
    _glgsg->_glBindTextures(0, _texture_units.size(), nullptr);
  }
  else if (_glgsg->_supports_dsa) {
    for (size_t i = 0; i < _texture_units.size(); ++i) {
      _glgsg->_glBindTextureUnit(i, 0);
    }
  }
  else
#endif
  {
    for (size_t i = 0; i < _texture_units.size(); ++i) {
      _glgsg->set_active_texture_stage(i);

      GLenum target = _texture_units[i]._target;
      if (target != GL_NONE) {
        glBindTexture(target, 0);
      }
    }
  }

  // Now unbind all the image units.  Not sure if we *have* to do this.
  int num_image_units = (int)_image_units.size();
  if (num_image_units > 0) {
#ifndef OPENGLES
    if (_glgsg->_supports_multi_bind) {
      _glgsg->_glBindImageTextures(0, num_image_units, nullptr);
    } else
#endif
    {
      for (int i = 0; i < num_image_units; ++i) {
        _glgsg->_glBindImageTexture(i, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R8);
      }
    }

    if (gl_enable_memory_barriers) {
      for (int i = 0; i < num_image_units; ++i) {
        ImageUnit &unit = _image_units[i];

        if (unit._gtc != nullptr) {
          unit._gtc->mark_incoherent(unit._written);
          unit._gtc = nullptr;
          unit._written = false;
        }
      }
    }
  }

  _glgsg->report_my_gl_errors();
}

/**
 * Disables all texture bindings used by the previous shader, then enables all
 * the texture bindings needed by this shader.  Extracts the relevant vertex
 * array data from the gsg.  The current implementation is inefficient,
 * because it may unnecessarily disable textures then immediately reenable
 * them.  We may optimize this someday.
 */
void CLP(ShaderContext)::
update_shader_texture_bindings(ShaderContext *prev) {
  // if (prev) { prev->disable_shader_texture_bindings(); }

  //if (_glsl_program == 0) {
  //  return;
  //}

  GLbitfield barriers = 0;

  ShaderInputBinding::State state;
  state.gsg = _glgsg;
  state.matrix_cache = &_matrix_cache[0];

  // First bind all the 'image units'; a bit of an esoteric OpenGL feature
  // right now.
  int num_image_units = (int)_image_units.size();
  if (num_image_units > 0) {
    for (int i = 0; i < num_image_units; ++i) {
      ImageUnit &unit = _image_units[i];
      if (unit._binding == nullptr) {
        // There may be gaps in the binding set.
        continue;
      }

      ShaderType::Access access = ShaderType::Access::READ_WRITE;
      int z = -1;
      int n = 0;
      PT(Texture) tex = unit._binding->fetch_texture_image(state, unit._resource_id, access, z, n);
      access = access & unit._access;

      GLuint gl_tex = 0;
      CLP(TextureContext) *gtc;

      if (tex != nullptr) {
        gtc = DCAST(CLP(TextureContext), tex->prepare_now(_glgsg->_prepared_objects, _glgsg));
        if (gtc != nullptr) {
          unit._gtc = gtc;

          _glgsg->update_texture(gtc, true);

          int view = _glgsg->get_current_tex_view_offset();
          gl_tex = gtc->get_view_index(view);
        }
      }

      if (gl_tex == 0) {
        _glgsg->_glBindImageTexture(i, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R8);

      } else {
        // TODO: automatically convert to sized type instead of plain GL_RGBA
        // If a base type is used, it will crash.
        GLenum internal_format = gtc->_internal_format;
#ifdef OPENGLES
        if (!gtc->_immutable) {
          static bool error_shown = false;
          if (!error_shown) {
            error_shown = true;
            GLCAT.error()
              << "Enable gl-immutable-texture-storage to use image textures in OpenGL ES.\n";
          }
        }
#endif
        if (internal_format == GL_RGBA || internal_format == GL_RGB) {
          GLCAT.error()
            << "Texture " << tex->get_name() << " has an unsized format.  Textures bound "
            << "to a shader as an image need a sized format.\n";

          // This may not actually be right, but may still prevent a crash.
          internal_format = _glgsg->get_internal_image_format(tex, true);
        }

        if (gl_force_image_bindings_writeonly) {
          access = access & ShaderType::Access::WRITE_ONLY;
        }

        GLenum gl_access = GL_READ_WRITE;
        GLint bind_level = n;
        GLint bind_layer = std::max(z, 0);
        GLboolean layered = z < 0;

        switch (access) {
        case ShaderType::Access::NONE:
          gl_tex = 0;
        case ShaderType::Access::READ_ONLY:
          gl_access = GL_READ_ONLY;
          break;

        case ShaderType::Access::WRITE_ONLY:
          gl_access = GL_WRITE_ONLY;
          unit._written = true;
          break;

        case ShaderType::Access::READ_WRITE:
          gl_access = GL_READ_WRITE;
          unit._written = true;
          break;
        }

        if (gtc->needs_barrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT, unit._written)) {
          barriers |= GL_SHADER_IMAGE_ACCESS_BARRIER_BIT;
        }

        _glgsg->_glBindImageTexture(i, gl_tex, bind_level, layered,
                                    bind_layer, gl_access, gtc->_internal_format);

        // Update the size variable, if we have one.
        GLint size_loc = unit._size_loc[_alpha_test_mode];
        if (size_loc != -1) {
          _glgsg->_glUniform4f(size_loc, (GLfloat)gtc->_width, (GLfloat)gtc->_height,
                                         (GLfloat)gtc->_depth, (GLfloat)gtc->_num_levels);
        }
      }
    }
  }

  size_t num_textures = _texture_units.size();
  GLuint *textures = nullptr;
  GLuint *samplers = nullptr;
#ifdef OPENGLES
  static const bool multi_bind = false;
#else
  bool multi_bind = false;
  if (num_textures > 1 &&
      _glgsg->_supports_multi_bind && _glgsg->_supports_sampler_objects) {
    // Prepare to multi-bind the textures and samplers.
    multi_bind = true;
    textures = (GLuint *)alloca(sizeof(GLuint) * num_textures);
    samplers = (GLuint *)alloca(sizeof(GLuint) * num_textures);
  }
#endif

  for (size_t i = 0; i < num_textures; ++i) {
    TextureUnit &unit = _texture_units[i];

    int view = _glgsg->get_current_tex_view_offset();
    SamplerState sampler;

    PT(Texture) tex = unit._binding->fetch_texture(state, unit._resource_id, sampler, view);
    if (tex.is_null()) {
      // Apply a white texture in order to make it easier to use a shader that
      // takes a texture on a model that doesn't have a texture applied.
      if (multi_bind) {
        textures[i] = _glgsg->get_white_texture();
        samplers[i] = 0;
      } else {
        _glgsg->apply_white_texture(i);
      }
      continue;
    }
    else if (Texture::is_integer(tex->get_format())) {
      // Required to satisfy Intel drivers, which will otherwise sample zero.
      sampler.set_minfilter(sampler.uses_mipmaps() ? SamplerState::FT_nearest_mipmap_nearest : SamplerState::FT_nearest);
      sampler.set_magfilter(SamplerState::FT_nearest);
    }

    CLP(TextureContext) *gtc = DCAST(CLP(TextureContext), tex->prepare_now(_glgsg->_prepared_objects, _glgsg));
    if (gtc == nullptr) {
      if (multi_bind) {
        textures[i] = 0;
        samplers[i] = 0;
      }
      continue;
    }

#ifndef OPENGLES
    // If it was recently written to, we will have to issue a memory barrier
    // soon.
    if (gtc->needs_barrier(GL_TEXTURE_FETCH_BARRIER_BIT, false)) {
      barriers |= GL_TEXTURE_FETCH_BARRIER_BIT;
    }
#endif

    // Note that simple RAM images are always 2-D for now, so to avoid errors,
    // we must load the real texture if this is not for a sampler2D.
    bool force = (tex->get_texture_type() != Texture::TT_2d_texture);
#ifndef OPENGLES
    if (multi_bind) {
      // Multi-bind case.
      if (!_glgsg->update_texture(gtc, force)) {
        textures[i] = 0;
      } else {
        gtc->set_active(true);
        textures[i] = gtc->get_view_index(view);
      }

      SamplerContext *sc = sampler.prepare_now(_glgsg->get_prepared_objects(), _glgsg);
      if (sc == nullptr) {
        samplers[i] = 0;
      } else {
        CLP(SamplerContext) *gsc = DCAST(CLP(SamplerContext), sc);
        gsc->enqueue_lru(&_glgsg->_prepared_objects->_sampler_object_lru);
        samplers[i] = gsc->_index;
      }
    } else
#endif  // !OPENGLES
    {
      // Non-multibind case.
      _glgsg->set_active_texture_stage(i);
      if (!_glgsg->update_texture(gtc, force)) {
        continue;
      }
      _glgsg->apply_texture(gtc, view);
      _glgsg->apply_sampler(i, sampler, gtc, view);
    }

    // Update the size variable, if we have one.
    GLint size_loc = unit._size_loc[_alpha_test_mode];
    if (size_loc != -1) {
      _glgsg->_glUniform4f(size_loc, (GLfloat)gtc->_width, (GLfloat)gtc->_height,
                                     (GLfloat)gtc->_depth, (GLfloat)gtc->_num_levels);
    }
  }

#ifndef OPENGLES
  if (multi_bind && num_textures > 0) {
    _glgsg->_glBindTextures(0, num_textures, textures);
    _glgsg->_glBindSamplers(0, num_textures, samplers);
  }
#endif

  if (barriers != 0) {
    // Issue a memory barrier prior to this shader's execution.
    _glgsg->issue_memory_barrier(barriers);
  }

  _glgsg->report_my_gl_errors();
}

/**
 * Updates the shader buffer bindings for this shader.
 */
void CLP(ShaderContext)::
update_shader_buffer_bindings(ShaderContext *prev) {
  // Update the shader storage buffer bindings.
  ShaderInputBinding::State state;
  state.gsg = _glgsg;
  state.matrix_cache = &_matrix_cache[0];

  for (StorageBlock &block : _storage_blocks) {
    PT(ShaderBuffer) buffer = block._binding->fetch_shader_buffer(state, block._resource_id);
    block._gbc = _glgsg->apply_shader_buffer(block._binding_index, buffer);
  }
}

/**
 * Issues memory barriers for shader buffers, should be called before a draw.
 */
void CLP(ShaderContext)::
issue_memory_barriers() {
#ifndef OPENGLES
  bool barrier_needed = false;
  for (StorageBlock &block : _storage_blocks) {
    if (block._gbc != nullptr &&
        block._gbc->_shader_storage_barrier_counter == _glgsg->_shader_storage_barrier_counter) {
      barrier_needed = true;
      break;
    }
  }

  if (barrier_needed) {
    _glgsg->issue_memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT);
  }

  // We assume that all SSBOs will be written to, for now.
  for (StorageBlock &block : _storage_blocks) {
    if (block._gbc != nullptr) {
      block._gbc->_shader_storage_barrier_counter = _glgsg->_shader_storage_barrier_counter;
    }
  }
#endif
}

/**
 * This subroutine prints the infolog for a shader.
 */
void CLP(ShaderContext)::
report_shader_errors(GLuint handle, Shader::Stage stage, bool fatal) {
  char *info_log;
  GLint length = 0;
  GLint num_chars  = 0;

  _glgsg->_glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &length);

  if (length <= 1) {
    return;
  }

  info_log = (char *) alloca(length);
  _glgsg->_glGetShaderInfoLog(handle, length, &num_chars, info_log);
  if (strcmp(info_log, "Success.\n") == 0 ||
      strcmp(info_log, "No errors.\n") == 0) {
    return;
  }

  CPT(ShaderModule) module = _shader->get_module(stage);
  if (!module->is_of_type(ShaderModuleGlsl::get_class_type())) {
    GLCAT.error(false) << info_log;
    return;
  }
  const ShaderModuleGlsl *glsl_module = (const ShaderModuleGlsl *)module.p();

  // Parse the errors so that we can substitute in actual file locations
  // instead of source indices.
  std::istringstream log(info_log);
  string line;
  while (std::getline(log, line)) {
    int fileno, lineno, colno;
    int prefixlen = 0;

    // This first format is used by the majority of compilers.
    if (sscanf(line.c_str(), "ERROR: %d:%d: %n", &fileno, &lineno, &prefixlen) == 2
        && prefixlen > 0) {

      Filename fn = glsl_module->get_filename_from_index(fileno);
      GLCAT.error(false)
        << "ERROR: " << fn << ":" << lineno << ": " << (line.c_str() + prefixlen) << "\n";

    } else if (sscanf(line.c_str(), "WARNING: %d:%d: %n", &fileno, &lineno, &prefixlen) == 2
               && prefixlen > 0) {

      Filename fn = glsl_module->get_filename_from_index(fileno);
      GLCAT.warning(false)
        << "WARNING: " << fn << ":" << lineno << ": " << (line.c_str() + prefixlen) << "\n";

    } else if (sscanf(line.c_str(), "%d(%d) : %n", &fileno, &lineno, &prefixlen) == 2
               && prefixlen > 0) {

      // This is the format NVIDIA uses.
      Filename fn = glsl_module->get_filename_from_index(fileno);
      GLCAT.error(false)
        << fn << "(" << lineno << ") : " << (line.c_str() + prefixlen) << "\n";

    } else if (sscanf(line.c_str(), "%d:%d(%d): %n", &fileno, &lineno, &colno, &prefixlen) == 3
               && prefixlen > 0) {

      // This is the format for Mesa's OpenGL ES 2 implementation.
      Filename fn = glsl_module->get_filename_from_index(fileno);
      GLCAT.error(false)
        << fn << ":" << lineno << "(" << colno << "): " << (line.c_str() + prefixlen) << "\n";

    } else if (!fatal) {
      GLCAT.warning(false) << line << "\n";

    } else {
      GLCAT.error(false) << line << "\n";
    }
  }
}

/**
 * This subroutine prints the infolog for a program.
 */
void CLP(ShaderContext)::
report_program_errors(GLuint program, bool fatal) {
  char *info_log;
  GLint length = 0;
  GLint num_chars  = 0;

  _glgsg->_glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);

  if (length > 1) {
    info_log = (char *) alloca(length);
    _glgsg->_glGetProgramInfoLog(program, length, &num_chars, info_log);

    if (strcmp(info_log, "Success.\n") != 0 &&
        strcmp(info_log, "No errors.\n") != 0 &&
        strcmp(info_log, "Validation successful.\n") != 0) {

#ifdef __APPLE__
      // Filter out these unhelpful warnings that Apple always generates.
      while (true) {
        if (info_log[0] == '\n') {
          ++info_log;
          continue;
        }
        if (info_log[0] == '\0') {
          // We reached the end without finding anything interesting.
          return;
        }
        int linelen = 0;
        if ((sscanf(info_log, "WARNING: Could not find vertex shader attribute %*s to match BindAttributeLocation request.%*[\n]%n", &linelen) == 0 && linelen > 0) ||
            (sscanf(info_log, "WARNING: Could not find fragment shader output %*s to match FragDataBinding request.%*[\n]%n", &linelen) == 0 && linelen > 0)) {
          info_log += linelen;
          continue;
        }
        else {
          break;
        }

        info_log = strchr(info_log, '\n');
        if (info_log == nullptr) {
          // We reached the end without finding anything interesting.
          return;
        }
        ++info_log;
      }
#endif

      if (!fatal) {
        GLCAT.warning()
          << "Shader " << _shader->get_filename() << " produced the "
          << "following warnings:\n" << info_log << "\n";
      } else {
        GLCAT.error(false) << info_log << "\n";
      }
    }
  }
}

/**
 * Compiles the given ShaderModule, returning the handle.
 * The program argument only needs to be passed for fragment modules.
 */
GLuint CLP(ShaderContext)::
create_shader(GLuint program, const ShaderModule *module, size_t mi,
              const Shader::ModuleSpecConstants &consts,
              RenderAttrib::PandaCompareFunc alpha_test_mode) {
  ShaderModule::Stage stage = module->get_stage();

  GLuint handle = 0;
  switch (stage) {
  case ShaderModule::Stage::VERTEX:
    handle = _glgsg->_glCreateShader(GL_VERTEX_SHADER);
    break;
  case ShaderModule::Stage::FRAGMENT:
    handle = _glgsg->_glCreateShader(GL_FRAGMENT_SHADER);
    break;
#ifndef OPENGLES
  case ShaderModule::Stage::GEOMETRY:
    if (_glgsg->get_supports_geometry_shaders()) {
      handle = _glgsg->_glCreateShader(GL_GEOMETRY_SHADER);
    }
    break;
  case ShaderModule::Stage::TESS_CONTROL:
    if (_glgsg->get_supports_tessellation_shaders()) {
      handle = _glgsg->_glCreateShader(GL_TESS_CONTROL_SHADER);
    }
    break;
  case ShaderModule::Stage::TESS_EVALUATION:
    if (_glgsg->get_supports_tessellation_shaders()) {
      handle = _glgsg->_glCreateShader(GL_TESS_EVALUATION_SHADER);
    }
    break;
#endif
  case ShaderModule::Stage::COMPUTE:
    if (_glgsg->get_supports_compute_shaders()) {
      handle = _glgsg->_glCreateShader(GL_COMPUTE_SHADER);
    }
    break;
  default:
    break;
  }
  if (!handle) {
    GLCAT.error()
      << "Could not create a GLSL " << stage << " shader.\n";
    _glgsg->report_my_gl_errors();
    return 0;
  }

  if (_glgsg->_use_object_labels) {
    string name = module->get_source_filename();
    _glgsg->_glObjectLabel(GL_SHADER, handle, name.size(), name.data());
  }

  if (module->is_of_type(ShaderModuleSpirV::get_class_type())) {
    ShaderModuleSpirV *spv = (ShaderModuleSpirV *)module;

    pmap<uint32_t, int> id_to_location;
    pvector<uint32_t> binding_ids;
    for (size_t pi = 0; pi < spv->get_num_parameters(); ++pi) {
      const ShaderModule::Variable &var = spv->get_parameter(pi);
      {
        auto it = _locations.find(var.name);
        if (it != _locations.end()) {
          id_to_location[var.id] = it->second;
        }
      }
      {
        auto it = _bindings.find(var.name);
        if (it != _bindings.end()) {
          if (it->second >= (int)binding_ids.size()) {
            binding_ids.resize(it->second + 1, 0);
          }
          binding_ids[it->second] = var.id;
        }
      }
    }

#ifndef OPENGLES
    if (_glgsg->_supports_spir_v) {
      // Load a SPIR-V binary.
      if (GLCAT.is_debug()) {
        GLCAT.debug()
          << "Compiling SPIR-V " << stage << " shader binary "
          << spv->get_source_filename() << "\n";
      }

      // Make a transformer so we can do some transformations such as assigning
      // locations.
      SpirVTransformer transformer(spv->_instructions);

      if (!id_to_location.empty()) {
        transformer.assign_locations(id_to_location);

        if (_glgsg->_gl_vendor == "NVIDIA Corporation") {
          // Sigh... NVIDIA driver gives an error if the SPIR-V ID doesn't match
          // for variables with overlapping locations if the OpName is stripped.
          // We'll have to just insert OpNames for every parameter.
          // https://forums.developer.nvidia.com/t/gl-arb-gl-spirv-bug-duplicate-location-link-error-if-opname-is-stripped-from-spir-v-shader/128491
          // Bug was found with 446.14 drivers on Windows 10 64-bit.
          transformer.assign_procedural_names("p", id_to_location);
        }
      }
      if (!binding_ids.empty()) {
        transformer.bind_descriptor_set(0, binding_ids);
      }

      if (stage == ShaderModule::Stage::FRAGMENT &&
          alpha_test_mode != RenderAttrib::M_none) {
        // Assign location 0 to the alpha ref value.
        transformer.run(SpirVInjectAlphaTestPass((SpirVInjectAlphaTestPass::Mode)alpha_test_mode, 0));
      }

      ShaderModuleSpirV::InstructionStream stream = transformer.get_result();
      _glgsg->_glShaderBinary(1, &handle, GL_SHADER_BINARY_FORMAT_SPIR_V_ARB,
                              (const char *)stream.get_data(),
                              stream.get_data_size() * sizeof(uint32_t));

      _glgsg->_glSpecializeShader(handle, "main", consts._indices.size(),
                                  (GLuint *)consts._indices.data(),
                                  (GLuint *)consts._values.data());
    }
    else
#endif  // !OPENGLES
    {
      // Compile to GLSL using SPIRV-Cross.
      if (GLCAT.is_debug()) {
        GLCAT.debug()
          << "Transpiling SPIR-V " << stage << " shader "
          << spv->get_source_filename() << "\n";
      }

      spirv_cross::CompilerGLSL::Options options;
      options.version = _glgsg->_glsl_version;
#ifdef OPENGLES
      options.es = true;
#else
      options.es = _glgsg->_glsl_version == 100
                || _glgsg->_glsl_version == 300
                || _glgsg->_glsl_version == 310
                || _glgsg->_glsl_version == 320;
#endif
      options.vertex.support_nonzero_base_instance = false;
      options.enable_420pack_extension = false;

      // We want to use explicit bindings for SSBOs, which requires 420 or
      // the 420pack extension.  Presumably we have it if we support SSBOs.
      if (!binding_ids.empty() && !options.es && options.version < 420) {
        options.enable_420pack_extension = true;
      }

      if (options.version < 130) {
        _emulate_float_attribs = true;
      }

      ShaderModuleSpirV::InstructionStream stream = spv->_instructions;

      // Do we need to emulate certain caps, like texture queries?
      pmap<SpirVTransformPass::AccessChain, uint32_t> size_var_ids;
      uint64_t supported_caps = _glgsg->get_supported_shader_capabilities();
      uint64_t emulate_caps = spv->_emulatable_caps & ~supported_caps;
      if (emulate_caps != 0u) {
        _emulated_caps |= emulate_caps;

        SpirVTransformer transformer(spv->_instructions);
        SpirVEmulateTextureQueriesPass pass(emulate_caps);
        transformer.run(pass);
        size_var_ids = std::move(pass._size_var_ids);
        stream = transformer.get_result();
      }

      if (stage != ShaderModule::Stage::FRAGMENT) {
        alpha_test_mode = RenderAttrib::M_none;
      }

      Compiler compiler(std::move(stream._words), alpha_test_mode);
      compiler.set_common_options(options);

      if (alpha_test_mode != RenderAttrib::M_none &&
          alpha_test_mode != RenderAttrib::M_always &&
          alpha_test_mode != RenderAttrib::M_never) {
        if ((!options.es && options.version < 430) ||
            (options.es && options.version < 310)) {
          compiler.add_header_line("uniform float aref;");
          _remap_locations = true;
        } else {
          compiler.add_header_line("layout(location=0) uniform float aref;");
        }
      }

      // At this time, SPIRV-Cross doesn't always add these automatically.
      uint64_t used_caps = module->get_used_capabilities();
#ifndef OPENGLES
      if (!options.es) {
        if (options.version < 140 && (used_caps & Shader::C_instance_id) != 0) {
          if (_glgsg->has_extension("GL_ARB_draw_instanced")) {
            compiler.require_extension("GL_ARB_draw_instanced");
          } else {
            compiler.require_extension("GL_EXT_gpu_shader4");
          }
        }
        if (options.version < 130 && (used_caps & Shader::C_unified_model) != 0) {
          compiler.require_extension("GL_EXT_gpu_shader4");
        }
        if (options.version < 400 && (used_caps & Shader::C_dynamic_indexing) != 0) {
          compiler.require_extension("GL_ARB_gpu_shader5");
        }
        if (options.version < 430 && (used_caps & Shader::C_storage_buffer) != 0) {
          compiler.require_extension("GL_ARB_shader_storage_buffer_object");
        }
      }
      else
#endif
      {
        if (options.version < 300 && (used_caps & Shader::C_non_square_matrices) != 0) {
          compiler.require_extension("GL_NV_non_square_matrices");
        }
        if (options.version < 320 && (used_caps & Shader::C_dynamic_indexing) != 0) {
          if (_glgsg->has_extension("GL_OES_gpu_shader5")) {
            compiler.require_extension("GL_OES_gpu_shader5");
          } else {
            compiler.require_extension("GL_EXT_gpu_shader5");
          }
        }
      }

      // Assign names based on locations.  This is important to make sure that
      // uniforms shared between shader stages have the same name, or the
      // compiler may start to complain about overlapping locations.
      char buf[1024];
      for (spirv_cross::VariableID id : compiler.get_active_interface_variables()) {
        uint32_t loc = compiler.get_decoration(id, spv::DecorationLocation);
        spv::StorageClass sc = compiler.get_storage_class(id);

        if (sc == spv::StorageClassUniformConstant) {
          auto it = id_to_location.find(id);
          if (it != id_to_location.end()) {
            int location = it->second;
            sprintf(buf, "p%u", location);
            compiler.set_name(id, buf);
            compiler.set_decoration(id, spv::DecorationLocation, location);

            // Older versions of OpenGL (ES) do not support explicit uniform
            // locations, and we need to query the locations later.
            if ((!options.es && options.version < 430) ||
                (options.es && options.version < 310)) {
              _remap_locations = true;
            }
          }
        }
        else if (sc == spv::StorageClassInput) {
          if (stage == ShaderModule::Stage::VERTEX) {
            // Explicit attrib locations were added in GLSL 3.30, but we can
            // override the binding in older versions using the API.
            if (options.version < 330) {
              _bind_attrib_locations |= 1 << loc;
            }
            sprintf(buf, "a%u", loc);
          } else {
            // For all other stages, it's just important that the names match,
            // so we assign the names based on the location and successive
            // numbering of the shaders.
            sprintf(buf, "i%u_%u", (unsigned int)mi, loc);
          }
          compiler.set_name(id, buf);
        }
        else if (sc == spv::StorageClassOutput) {
          if (stage == ShaderModule::Stage::FRAGMENT) {
            // Output of the last stage, same story as above.
            sprintf(buf, "o%u", loc);
            if (options.version < 330) {
              _glgsg->_glBindFragDataLocation(program, loc, buf);
            }
          } else {
            // Match the name of the next stage.
            sprintf(buf, "i%u_%u", (unsigned int)mi + 1u, loc);
          }
          compiler.set_name(id, buf);
        }
      }

      // Assign names to emulated texture/image size variables.
      for (auto &item : size_var_ids) {
        const SpirVTransformPass::AccessChain &chain = item.first;
        auto it = id_to_location.find(chain._var_id);
        if (it != id_to_location.end()) {
          int location = it->second;
          size_t size = sprintf(buf, "p%u", location);
          for (size_t i = 0; i < chain.size(); ++i) {
            size += sprintf(buf + size, "_%d", chain[i]);
          }
          strcpy(buf + size, "_s");
          compiler.set_name(item.second, buf);
        }
      }

      // For all uniform constant structs, we need to ensure we have procedural
      // names like _m0, _m1, _m2, etc.  Furthermore, we need to assign each
      // struct a name that is guaranteed to be the same between stages, since
      // some drivers will complain if the struct name is different for the
      // same uniform between different stages.
      for (auto &item : spv->_uniform_struct_types) {
        std::ostringstream str;
        item.second->output_signature(str);
        compiler.set_name(item.first, str.str());

        for (size_t i = 0; i < item.second->get_num_members(); ++i) {
          sprintf(buf, "m%d", (int)i);
          compiler.set_member_name(item.first, i, buf);
        }
      }

      // Add bindings for the shader storage buffers.
      for (size_t binding = 0; binding < binding_ids.size(); ++binding) {
        uint32_t id = binding_ids[binding];
        if (id > 0) {
          compiler.set_decoration(id, spv::DecorationBinding, binding);
        }
      }

      // Optimize out unused variables.
      compiler.set_enabled_interface_variables(compiler.get_active_interface_variables());

      std::string text = compiler.compile();

      if (GLCAT.is_debug()) {
        GLCAT.debug()
          << "SPIRV-Cross compilation resulted in GLSL shader:\n"
          << text << "\n";
      }

      const char *text_str = text.c_str();
      _glgsg->_glShaderSource(handle, 1, &text_str, nullptr);
    }
  }
  else if (module->is_of_type(ShaderModuleGlsl::get_class_type())) {
    // Legacy preprocessed GLSL.
    ShaderModuleGlsl *glsl_module = (ShaderModuleGlsl *)module;
    if (GLCAT.is_debug()) {
      GLCAT.debug()
        << "Compiling GLSL " << stage << " shader "
        << glsl_module->get_source_filename() << "\n";
    }

    std::string text = glsl_module->get_ir();
    const char *text_str = text.c_str();
    _glgsg->_glShaderSource(handle, 1, &text_str, nullptr);
  }
  else {
    GLCAT.error()
      << "Unsupported shader module type " << module->get_type() << "!\n";
    return 0;
  }

  return handle;
}

/**
 * This subroutine compiles a GLSL shader.  The given locations and SSBO
 * bindings will be assigned.  If that is not possible, it will instead
 * generate names for the uniforms based on the given locations, and
 * _remap_locations will be set to true.
 */
GLuint CLP(ShaderContext)::
compile_and_link(RenderAttrib::PandaCompareFunc alpha_test_mode) {
  GLuint program = _program;
  if (program == 0) {
    program = _glgsg->_glCreateProgram();
    if (UNLIKELY(program == 0)) {
      GLCAT.error()
        << "Failed to create program object\n";
      return 0;
    }
  }

  if (_glgsg->_use_object_labels) {
    const std::string &name = _shader->get_debug_name();
    _glgsg->_glObjectLabel(GL_PROGRAM, program, name.size(), name.data());
  }

  // Do we have a compiled program?  Try to load that.
/*  unsigned int format;
  string binary;
  if (_shader->get_compiled(format, binary)) {
    _glgsg->_glProgramBinary(program, format, binary.data(), binary.size());

    GLint status;
    _glgsg->_glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_TRUE) {
      // Hooray, the precompiled shader worked.
      if (GLCAT.is_debug()) {
        GLCAT.debug() << "Loaded precompiled binary for GLSL shader "
                      << _shader->get_filename() << "\n";
      }
      return true;
    }

    // Bummer, it didn't work..  Oh well, just recompile the shader.
    if (GLCAT.is_debug()) {
      GLCAT.debug() << "Failure loading precompiled binary for GLSL shader "
                    << _shader->get_filename() << "\n";
    }
  }*/

  // We may have to compile a new fragment shader for this alpha test mode.
  if (_inject_alpha_test) {
    for (Module &module : _modules) {
      _glgsg->_glAttachShader(program, module._handle);
    }

    size_t mi = 0;
    for (Shader::LinkedModule &linked_module : _shader->_modules) {
      CPT(ShaderModule) module = linked_module._module.get_read_pointer();
      if (module->get_stage() == Shader::Stage::FRAGMENT) {
        GLuint handle;
        handle = create_shader(program, module, mi, linked_module._consts, alpha_test_mode);
        if (handle == 0) {
          _glgsg->_glDeleteProgram(program);
          return false;
        }

        if (!_glgsg->_supports_spir_v) {
          _glgsg->_glCompileShader(handle);
        }
        _glgsg->_glAttachShader(program, handle);
        _glgsg->_glDeleteShader(handle);
        break;
      }

      ++mi;
    }
  }

  if (_bind_attrib_locations != 0) {
    char buf[32];
    for (unsigned int loc = 0; loc < 32; ++loc) {
      if (_bind_attrib_locations & (1 << loc)) {
        sprintf(buf, "a%u", loc);
        _glgsg->_glBindAttribLocation(program, loc, buf);
      }
    }
  }

  if (_is_legacy) {
    // Under OpenGL's compatibility profile, we have to make sure that we bind
    // something to attribute 0.  Make sure that this is the position array.
    _glgsg->_glBindAttribLocation(program, 0, "p3d_Vertex");
    _glgsg->_glBindAttribLocation(program, 0, "vertex");

    // While we're at it, let's also map these to fixed locations.  These
    // attributes were historically fixed to these locations, so it might help a
    // buggy driver.
    _glgsg->_glBindAttribLocation(program, 2, "p3d_Normal");
    _glgsg->_glBindAttribLocation(program, 3, "p3d_Color");

    if (gl_fixed_vertex_attrib_locations) {
      _glgsg->_glBindAttribLocation(program, 1, "transform_weight");
      _glgsg->_glBindAttribLocation(program, 2, "normal");
      _glgsg->_glBindAttribLocation(program, 3, "color");
      _glgsg->_glBindAttribLocation(program, 7, "transform_index");
      _glgsg->_glBindAttribLocation(program, 8, "p3d_MultiTexCoord0");
      _glgsg->_glBindAttribLocation(program, 8, "texcoord");
    }

    // Also bind the p3d_FragData array to the first index always.
    if (_glgsg->_glBindFragDataLocation != nullptr) {
      _glgsg->_glBindFragDataLocation(program, 0, "p3d_FragData");
    }
  }

  // If we requested to retrieve the shader, we should indicate that before
  // linking.
#ifndef __EMSCRIPTEN__
  bool retrieve_binary = false;
  if (_glgsg->_supports_get_program_binary) {
    retrieve_binary = _shader->get_cache_compiled_shader();

#ifndef NDEBUG
    if (gl_dump_compiled_shaders) {
      retrieve_binary = true;
    }
#endif

    _glgsg->_glProgramParameteri(program, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE);
  }
#endif  // !__EMSCRIPTEN__

  if (GLCAT.is_debug()) {
    GLCAT.debug()
      << "Linking shader " << _shader->get_filename() << "\n";
  }

  _glgsg->_glLinkProgram(program);

  // Query the link status.  This will cause the application to wait for the
  // link to be finished.
  GLint status = GL_FALSE;
  _glgsg->_glGetProgramiv(program, GL_LINK_STATUS, &status);
  if (status != GL_TRUE) {
    // The link failed.  Is it because one of the shaders failed to compile?
    GLCAT.error()
      << "Failed to link shader " << _shader->get_filename() << ":\n";

    bool any_failed = false;
    for (Module &module : _modules) {
      _glgsg->_glGetShaderiv(module._handle, GL_COMPILE_STATUS, &status);

      if (status != GL_TRUE) {
        GLCAT.error(false)
          << "Failed to compile stage " << module._stage << ":\n";
        report_shader_errors(module._handle, module._stage, true);
        any_failed = true;
      } else {
        // Report any warnings.
        report_shader_errors(module._handle, module._stage, false);
      }

      // Delete the shader, we don't need it any more.
      _glgsg->_glDeleteShader(module._handle);
    }
    _modules.clear();

    if (any_failed) {
      // One or more of the shaders failed to compile, which would explain the
      // link failure.  We know enough.
      return 0;
    }

    report_program_errors(program, true);
    return 0;
  }

  // Report any warnings.
  report_program_errors(program, false);

#ifndef __EMSCRIPTEN__
  if (retrieve_binary) {
    GLint length = 0;
    _glgsg->_glGetProgramiv(program, GL_PROGRAM_BINARY_LENGTH, &length);
    length += 2;

    char *binary = (char *)alloca(length);
    GLenum format;
    GLsizei num_bytes = 0;
    _glgsg->_glGetProgramBinary(program, length, &num_bytes, &format, (void*)binary);

    _shader->set_compiled(format, binary, num_bytes);

#ifndef NDEBUG
    // Dump the binary if requested.
    if (gl_dump_compiled_shaders) {
      char filename[64];
      static int gl_dump_count = 0;
      sprintf(filename, "glsl_program%d.dump", gl_dump_count++);

      pofstream s;
      s.open(filename, std::ios::out | std::ios::binary | std::ios::trunc);
      s.write(binary, num_bytes);
      s.close();

      GLCAT.info()
        << "Dumped " << num_bytes << " bytes of program binary with format 0x"
        << hex << format << dec << "  to " << filename << "\n";
    }
#endif  // NDEBUG
  }
#endif  // !__EMSCRIPTEN__

  _glgsg->report_my_gl_errors();
  return program;
}

#endif  // OPENGLES_1
