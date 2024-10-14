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

#define SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS
#include <spirv_cross/spirv_glsl.hpp>

using std::dec;
using std::hex;
using std::max;
using std::min;
using std::string;

TypeHandle CLP(ShaderContext)::_type_handle;

/**
 * xyz
 */
CLP(ShaderContext)::
CLP(ShaderContext)(CLP(GraphicsStateGuardian) *glgsg, Shader *s) : ShaderContext(s) {
  _glgsg = glgsg;
  _glsl_program = 0;
  _uses_standard_vertex_arrays = false;
  _enabled_attribs.clear();
  _color_attrib_index = -1;
  _validated = !gl_validate_shaders;

  // Ignoring any locations that may have already been set, we assign a new
  // unique location to each parameter.
  LocationMap locations;
  GLint next_location = 0;
  for (const Shader::Parameter &param : _shader->_parameters) {
    locations[param._name] = next_location;
    next_location += param._type->get_num_parameter_locations();
  }

  // We compile and analyze the shader here, instead of in shader.cxx, to
  // avoid gobj getting a dependency on GL stuff.
  bool needs_query_locations = false;
  if (!compile_and_link(locations, needs_query_locations)) {
    release_resources();
    s->_error_flag = true;
    return;
  }

  // Bind the program, so that we can call glUniform1i for the textures.
  _glgsg->_glUseProgram(_glsl_program);

  // If this is a legacy GLSL shader, we don't have the parameter definitions
  // yet, so we need to perform reflection on the shader.
  SparseArray active_locations;
  if (_is_legacy) {
    reflect_program();
    needs_query_locations = true;
  }
  else if (!needs_query_locations) {
    // We still need to query which uniform locations are actually in use,
    // because the GL driver may have optimized some out.
    GLint num_active_uniforms = 0;
    glgsg->_glGetProgramInterfaceiv(_glsl_program, GL_UNIFORM, GL_ACTIVE_RESOURCES, &num_active_uniforms);

    for (GLint i = 0; i < num_active_uniforms; ++i) {
      GLenum props[2] = {GL_LOCATION, GL_ARRAY_SIZE};
      GLint values[2];
      glgsg->_glGetProgramResourceiv(_glsl_program, GL_UNIFORM, i, 2, props, 2, nullptr, values);
      GLint location = values[0];
      if (location >= 0) {
        GLint array_size = values[1];
        active_locations.set_range(location, array_size);
      }
    }
  }

  _matrix_cache = pvector<LMatrix4>(s->_matrix_cache_desc.size(), LMatrix4::ident_mat());
  _matrix_cache_deps = s->_matrix_cache_deps;

  _scratch_space_size = 16;
  for (const Shader::Parameter &param : s->_parameters) {
    if (param._binding == nullptr) {
      continue;
    }

    UniformBlock block;
    block._dep = param._binding->get_state_dep();
    block._bindings.push_back({param._binding, 0});

    int chosen_location = locations[param._name];
    int actual_location = needs_query_locations ? -1 : chosen_location;

    // Though the code is written to take advantage of UBOs, we're not using
    // UBOs yet, so we instead have the parameters copied to a scratch space.
    // Now make a list of the individual uniform calls we will have to do.
    int resource_index = 0;
    std::string name = param._name->get_name();
    char sym_buffer[16];
    sprintf(sym_buffer, "p%d", chosen_location);
    r_collect_uniforms(param, block, param._type, name.c_str(), sym_buffer,
                       actual_location, active_locations, resource_index);

    if (!block._matrices.empty() || !block._vectors.empty()) {
      _uniform_data_deps |= block._dep;
      _uniform_blocks.push_back(std::move(block));

      // Pad space to 16-byte boundary
      uint32_t size = param._type->get_size_bytes();
      size = (size + 15) & ~15;
      _scratch_space_size = std::max(_scratch_space_size, (size_t)size);
    }
  }

  if (_image_units.size() > (size_t)_glgsg->_max_image_units) {
    _image_units.resize((size_t)_glgsg->_max_image_units);
  }

  // Do we have a p3d_Color attribute?
  for (auto it = s->_var_spec.begin(); it != s->_var_spec.end(); ++it) {
    Shader::ShaderVarSpec &spec = *it;
    if (spec._name == InternalName::get_color()) {
      _color_attrib_index = spec._id._location;
      break;
    }
  }

  _glgsg->report_my_gl_errors();

  // Restore the active shader.
  if (_glgsg->_current_shader_context == nullptr) {
    _glgsg->_glUseProgram(0);
  } else {
    _glgsg->_current_shader_context->bind();
  }
}

/**
 * For UBO emulation, expand the individual uniforms within aggregate types and
 * make a list of individual glUniform calls, also querying their location from
 * the driver if necessary.
 * Also finds all resources and adds them to the respective arrays.
 */
void CLP(ShaderContext)::
r_collect_uniforms(const Shader::Parameter &param, UniformBlock &block,
                   const ShaderType *type, const char *name, const char *sym,
                   int location, const SparseArray &active_locations,
                   int &resource_index, size_t offset) {

  ShaderType::ScalarType scalar_type;
  uint32_t num_elements;
  uint32_t num_rows;
  uint32_t num_cols;
  if (type->as_scalar_type(scalar_type, num_elements, num_rows, num_cols)) {
    if (location < 0) {
      location = _glgsg->_glGetUniformLocation(_glsl_program, _is_legacy ? name : sym);
      if (location < 0) {
        return;
      }
    } else if (!active_locations.get_bit(location)) {
      return;
    }
    if (GLCAT.is_debug()) {
      GLCAT.debug()
        << "Active uniform " << name << " with type " << *type
        << " is bound to location " << location << "\n";
    }

    UniformBlock::Call call;
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
      block._matrices.push_back(std::move(call));
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
      block._vectors.push_back(std::move(call));
    }
    return;
  }
  if (const ShaderType::Array *array_type = type->as_array()) {
    // Recurse.
    char *name_buffer = (char *)alloca(strlen(name) + 14);
    char *sym_buffer = (char *)alloca(strlen(sym) + 14);
    const ShaderType *element_type = array_type->get_element_type();
    int num_locations = element_type->get_num_parameter_locations();
    size_t stride = (size_t)array_type->get_stride_bytes();

    for (uint32_t i = 0; i < array_type->get_num_elements(); ++i) {
      sprintf(name_buffer, "%s[%u]", name, i);
      sprintf(sym_buffer, "%s[%u]", sym, i);
      r_collect_uniforms(param, block, element_type, name_buffer, sym_buffer, location, active_locations, resource_index, offset);
      if (location >= 0) {
        location += num_locations;
      }
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

      // SPIRV-Cross names struct members _m0, _m1, etc. in declaration order.
      sprintf(sym_buffer, "%s._m%u", sym, i);
      r_collect_uniforms(param, block, member.type, qualname.c_str(), sym_buffer, location, active_locations, resource_index, offset + member.offset);

      if (location >= 0) {
        location += member.type->get_num_parameter_locations();
      }
    }
    return;
  }

  if (location < 0) {
    location = _glgsg->_glGetUniformLocation(_glsl_program, _is_legacy ? name : sym);
    if (location < 0) {
      return;
    }
  } else if (!active_locations.get_bit(location)) {
    return;
  }
  if (GLCAT.is_debug()) {
    GLCAT.debug()
      << "Active uniform " << name << " with type " << *type
      << " is bound to location " << location << "\n";
  }

  if (const ::ShaderType::SampledImage *sampler = type->as_sampled_image()) {
    TextureUnit unit;
    unit._binding = param._binding;
    unit._resource_id = param._binding->get_resource_id(resource_index++, type);
    unit._target = _glgsg->get_texture_target(sampler->get_texture_type());

    _glgsg->_glUniform1i(location, (GLint)_texture_units.size());
    _texture_units.push_back(std::move(unit));
  }
  else if (const ::ShaderType::Image *image = type->as_image()) {
    ImageUnit unit;
    unit._binding = param._binding;
    unit._resource_id = param._binding->get_resource_id(resource_index++, type);
    unit._access = image->get_access();
    unit._written = false;

    _glgsg->_glUniform1i(location, (GLint)_image_units.size());
    _image_units.push_back(std::move(unit));
  }
  else if (type->as_resource()) {
    resource_index++;
  }
}

/**
 * Analyzes the uniforms, attributes, etc. of a shader that was not already
 * reflected.
 */
void CLP(ShaderContext)::
reflect_program() {
  // Process the vertex attributes first.
  GLint param_count = 0;
  GLint name_buflen = 0;
  _glgsg->_glGetProgramiv(_glsl_program, GL_ACTIVE_ATTRIBUTES, &param_count);
  _glgsg->_glGetProgramiv(_glsl_program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &name_buflen);
  name_buflen = max(64, name_buflen);
  char *name_buffer = (char *)alloca(name_buflen);

  _shader->_var_spec.clear();
  for (int i = 0; i < param_count; ++i) {
    reflect_attribute(i, name_buffer, name_buflen);
  }

  /*if (gl_fixed_vertex_attrib_locations) {
    // Relink the shader for glBindAttribLocation to take effect.
    _glgsg->_glLinkProgram(_glsl_program);
  }*/

  // Create a buffer the size of the longest uniform name.  Note that Intel HD
  // drivers report values that are too low.
  name_buflen = 0;
  _glgsg->_glGetProgramiv(_glsl_program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &name_buflen);
  name_buflen = max(64, name_buflen);
  name_buffer = (char *)alloca(name_buflen);

  // Get the used uniform blocks.
  if (_glgsg->_supports_uniform_buffers) {
    GLint block_count = 0, block_maxlength = 0;
    _glgsg->_glGetProgramiv(_glsl_program, GL_ACTIVE_UNIFORM_BLOCKS, &block_count);

    // Intel HD drivers report GL_INVALID_ENUM here.  They reportedly fixed
    // it, but I don't know in which driver version the fix is.
    if (_glgsg->_gl_vendor != "Intel") {
      _glgsg->_glGetProgramiv(_glsl_program, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, &block_maxlength);
      block_maxlength = max(64, block_maxlength);
    } else {
      block_maxlength = 1024;
    }

    char *block_name_cstr = (char *)alloca(block_maxlength);

    for (int i = 0; i < block_count; ++i) {
      block_name_cstr[0] = 0;
      _glgsg->_glGetActiveUniformBlockName(_glsl_program, i, block_maxlength, nullptr, block_name_cstr);

      reflect_uniform_block(i, block_name_cstr, name_buffer, name_buflen);
    }
  }

#ifndef OPENGLES
  // Get the used shader storage blocks.
  if (_glgsg->_supports_shader_buffers) {
    GLint block_count = 0, block_maxlength = 0;

    _glgsg->_glGetProgramInterfaceiv(_glsl_program, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &block_count);
    _glgsg->_glGetProgramInterfaceiv(_glsl_program, GL_SHADER_STORAGE_BLOCK, GL_MAX_NAME_LENGTH, &block_maxlength);

    block_maxlength = max(64, block_maxlength);
    char *block_name_cstr = (char *)alloca(block_maxlength);

    BitArray bindings;

    for (int i = 0; i < block_count; ++i) {
      block_name_cstr[0] = 0;
      _glgsg->_glGetProgramResourceName(_glsl_program, GL_SHADER_STORAGE_BLOCK, i, block_maxlength, nullptr, block_name_cstr);

      const GLenum props[] = {GL_BUFFER_BINDING, GL_BUFFER_DATA_SIZE};
      GLint values[2];
      _glgsg->_glGetProgramResourceiv(_glsl_program, GL_SHADER_STORAGE_BLOCK, i, 2, props, 2, nullptr, values);

      if (bindings.get_bit(values[0])) {
        // Binding index already in use, assign a different one.
        values[0] = bindings.get_lowest_off_bit();
        _glgsg->_glShaderStorageBlockBinding(_glsl_program, i, values[0]);
      }
      bindings.set_bit(values[0]);

      if (GLCAT.is_debug()) {
        GLCAT.debug()
          << "Active shader storage block " << block_name_cstr
          << " with size " << values[1] << " is bound to binding "
          << values[0] << "\n";
      }

      StorageBlock block;
      block._name = InternalName::make(block_name_cstr);
      block._binding_index = values[0];
      block._min_size = (GLuint)values[1];
      _storage_blocks.push_back(block);
    }
  }
#endif

  // Analyze the uniforms.  All the structs and arrays thereof are unrolled,
  // so we need to spend some effort to work out the original structures.
  param_count = 0;
  _glgsg->_glGetProgramiv(_glsl_program, GL_ACTIVE_UNIFORMS, &param_count);

  _shader->_parameters.clear();

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
    _glgsg->_glGetActiveUniform(_glsl_program, i, name_buflen, nullptr, &param_size, &param_type, name_buffer);
    GLint loc = _glgsg->_glGetUniformLocation(_glsl_program, name_buffer);

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

    uniforms.insert({loc, {std::string(name_buffer), param_size, param_type}});
  }

  struct StackItem {
    std::string name;
    GLint loc;
    int num_elements; // 0 means not an array
    ShaderType::Struct type;
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

    size_t i = 0;
    bool skip = false;
    while (i < struct_stack.size() && i < parts.size() - 1 &&
           struct_stack[i].name == parts[i]) {
      if (sizes[i] > struct_stack[i].num_elements) {
        struct_stack[i].num_elements = sizes[i];
      }
      if (sizes[i] > 1) {
        // Ignore all but the first struct element for determining the type.
        skip = true;
      }
      ++i;
    }
    if (skip) {
      continue;
    }

    while (i < struct_stack.size()) {
      StackItem item = std::move(struct_stack.back());
      struct_stack.pop_back();
      const ShaderType *type = ShaderType::register_type(std::move(item.type));
      if (item.num_elements > 0) {
        type = ShaderType::register_type(ShaderType::Array(type, item.num_elements));
      }

      if (struct_stack.empty()) {
        // Add struct as top-level
        _shader->add_parameter(InternalName::make(item.name), type, item.loc);
      }
      else if (struct_stack.back().num_elements <= 1) {
        // Add as nested struct member
        while (item.loc > struct_stack.back().loc + struct_stack.back().type.get_num_parameter_locations()) {
          // Add a dummy member
          struct_stack.back().type.add_member(ShaderType::void_type, "");
        }
        struct_stack.back().type.add_member(type, item.name);
      }
    }
    while (struct_stack.size() < parts.size() - 1) {
      struct_stack.push_back({parts[struct_stack.size()], loc, sizes[struct_stack.size()], {}});
    }

    const ShaderType *type = get_param_type(param.type);
    if (sizes.back() > 0 || param.size > 1) {
      type = ShaderType::register_type(ShaderType::Array(type, param.size));
    }

    if (struct_stack.empty()) {
      // Add as top-level
      assert(parts.size() == 1);
      _shader->add_parameter(InternalName::make(parts[0]), type, loc);
    }
    else if (struct_stack.back().num_elements <= 1) {
      // Add as struct member
      assert(parts.size() > 1);
      while (loc > struct_stack.back().loc + struct_stack.back().type.get_num_parameter_locations()) {
        // Add a dummy member
        struct_stack.back().type.add_member(ShaderType::void_type, "");
      }
      struct_stack.back().type.add_member(type, parts.back());
    }
  }

  while (!struct_stack.empty()) {
    StackItem item = std::move(struct_stack.back());
    struct_stack.pop_back();
    const ShaderType *type = ShaderType::register_type(std::move(item.type));
    if (item.num_elements > 0) {
      type = ShaderType::register_type(ShaderType::Array(type, item.num_elements));
    }

    if (struct_stack.empty()) {
      // Add struct as top-level
      _shader->add_parameter(InternalName::make(item.name), type, item.loc);
    }
    else if (struct_stack.back().num_elements <= 1) {
      // Add as nested struct member
      while (item.loc > struct_stack.back().loc + struct_stack.back().type.get_num_parameter_locations()) {
        // Add a dummy member
        struct_stack.back().type.add_member(ShaderType::void_type, "");
      }
      struct_stack.back().type.add_member(type, item.name);
    }
  }
}

/**
 * Analyzes the vertex attribute and stores the information it needs to
 * remember.
 */
void CLP(ShaderContext)::
reflect_attribute(int i, char *name_buffer, GLsizei name_buflen) {
  GLint param_size;
  GLenum param_type;

  // Get the name, size, and type of this attribute.
  name_buffer[0] = 0;
  _glgsg->_glGetActiveAttrib(_glsl_program, i, name_buflen, nullptr,
                             &param_size, &param_type, name_buffer);

  // Get the attrib location.
  GLint p = _glgsg->_glGetAttribLocation(_glsl_program, name_buffer);

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
reflect_uniform_block(int i, const char *name, char *name_buffer, GLsizei name_buflen) {
 // GLint offset = 0;

  GLint data_size = 0;
  GLint param_count = 0;
  _glgsg->_glGetActiveUniformBlockiv(_glsl_program, i, GL_UNIFORM_BLOCK_DATA_SIZE, &data_size);
  _glgsg->_glGetActiveUniformBlockiv(_glsl_program, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &param_count);

  if (param_count <= 0) {
    return;
  }

  // We use a GeomVertexArrayFormat to describe the uniform buffer layout.
  // GeomVertexArrayFormat block_format; block_format.set_pad_to(data_size);

  // Get an array containing the indices of all the uniforms in this block.
  GLuint *indices = (GLuint *)alloca(param_count * sizeof(GLint));
  _glgsg->_glGetActiveUniformBlockiv(_glsl_program, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, (GLint *)indices);

  // Acquire information about the way the uniforms in this block are packed.
  GLint *offsets = (GLint *)alloca(param_count * sizeof(GLint));
  GLint *mstrides = (GLint *)alloca(param_count * sizeof(GLint));
  GLint *astrides = (GLint *)alloca(param_count * sizeof(GLint));
  _glgsg->_glGetActiveUniformsiv(_glsl_program, param_count, indices, GL_UNIFORM_OFFSET, offsets);
  _glgsg->_glGetActiveUniformsiv(_glsl_program, param_count, indices, GL_UNIFORM_MATRIX_STRIDE, mstrides);
  _glgsg->_glGetActiveUniformsiv(_glsl_program, param_count, indices, GL_UNIFORM_ARRAY_STRIDE, astrides);

  for (int ui = 0; ui < param_count; ++ui) {
    name_buffer[0] = 0;
    GLint param_size;
    GLenum param_type;
    _glgsg->_glGetActiveUniform(_glsl_program, indices[ui], name_buflen, nullptr, &param_size, &param_type, name_buffer);

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
    return ShaderType::register_type(ShaderType::Image(Texture::TT_1d_texture, ShaderType::ST_float, ShaderType::Access::read_write));

  case GL_INT_IMAGE_1D:
    return ShaderType::register_type(ShaderType::Image(Texture::TT_1d_texture, ShaderType::ST_int, ShaderType::Access::read_write));

  case GL_UNSIGNED_INT_IMAGE_1D:
    return ShaderType::register_type(ShaderType::Image(Texture::TT_1d_texture, ShaderType::ST_uint, ShaderType::Access::read_write));

  case GL_IMAGE_1D_ARRAY:
    return ShaderType::register_type(ShaderType::Image(Texture::TT_1d_texture_array, ShaderType::ST_float, ShaderType::Access::read_write));

  case GL_INT_IMAGE_1D_ARRAY:
    return ShaderType::register_type(ShaderType::Image(Texture::TT_1d_texture_array, ShaderType::ST_int, ShaderType::Access::read_write));

  case GL_UNSIGNED_INT_IMAGE_1D_ARRAY:
    return ShaderType::register_type(ShaderType::Image(Texture::TT_1d_texture_array, ShaderType::ST_uint, ShaderType::Access::read_write));
#endif

  case GL_IMAGE_2D:
    return ShaderType::register_type(ShaderType::Image(Texture::TT_2d_texture, ShaderType::ST_float, ShaderType::Access::read_write));

  case GL_INT_IMAGE_2D:
    return ShaderType::register_type(ShaderType::Image(Texture::TT_2d_texture, ShaderType::ST_int, ShaderType::Access::read_write));

  case GL_UNSIGNED_INT_IMAGE_2D:
    return ShaderType::register_type(ShaderType::Image(Texture::TT_2d_texture, ShaderType::ST_uint, ShaderType::Access::read_write));

  case GL_IMAGE_2D_ARRAY:
    return ShaderType::register_type(ShaderType::Image(Texture::TT_2d_texture_array, ShaderType::ST_float, ShaderType::Access::read_write));

  case GL_INT_IMAGE_2D_ARRAY:
    return ShaderType::register_type(ShaderType::Image(Texture::TT_2d_texture_array, ShaderType::ST_int, ShaderType::Access::read_write));

  case GL_UNSIGNED_INT_IMAGE_2D_ARRAY:
    return ShaderType::register_type(ShaderType::Image(Texture::TT_2d_texture_array, ShaderType::ST_uint, ShaderType::Access::read_write));

  case GL_IMAGE_3D:
    return ShaderType::register_type(ShaderType::Image(Texture::TT_3d_texture, ShaderType::ST_float, ShaderType::Access::read_write));

  case GL_INT_IMAGE_3D:
    return ShaderType::register_type(ShaderType::Image(Texture::TT_3d_texture, ShaderType::ST_int, ShaderType::Access::read_write));

  case GL_UNSIGNED_INT_IMAGE_3D:
    return ShaderType::register_type(ShaderType::Image(Texture::TT_3d_texture, ShaderType::ST_uint, ShaderType::Access::read_write));

  case GL_IMAGE_CUBE:
    return ShaderType::register_type(ShaderType::Image(Texture::TT_cube_map, ShaderType::ST_float, ShaderType::Access::read_write));

  case GL_INT_IMAGE_CUBE:
    return ShaderType::register_type(ShaderType::Image(Texture::TT_cube_map, ShaderType::ST_int, ShaderType::Access::read_write));

  case GL_UNSIGNED_INT_IMAGE_CUBE:
    return ShaderType::register_type(ShaderType::Image(Texture::TT_cube_map, ShaderType::ST_uint, ShaderType::Access::read_write));

  case GL_IMAGE_CUBE_MAP_ARRAY:
    return ShaderType::register_type(ShaderType::Image(Texture::TT_cube_map_array, ShaderType::ST_float, ShaderType::Access::read_write));

  case GL_INT_IMAGE_CUBE_MAP_ARRAY:
    return ShaderType::register_type(ShaderType::Image(Texture::TT_cube_map_array, ShaderType::ST_int, ShaderType::Access::read_write));

  case GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY:
    return ShaderType::register_type(ShaderType::Image(Texture::TT_cube_map_array, ShaderType::ST_uint, ShaderType::Access::read_write));

  case GL_IMAGE_BUFFER:
    return ShaderType::register_type(ShaderType::Image(Texture::TT_buffer_texture, ShaderType::ST_float, ShaderType::Access::read_write));

  case GL_INT_IMAGE_BUFFER:
    return ShaderType::register_type(ShaderType::Image(Texture::TT_buffer_texture, ShaderType::ST_int, ShaderType::Access::read_write));

  case GL_UNSIGNED_INT_IMAGE_BUFFER:
    return ShaderType::register_type(ShaderType::Image(Texture::TT_buffer_texture, ShaderType::ST_uint, ShaderType::Access::read_write));
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
release_resources() {
  if (!_glgsg) {
    return;
  }
  if (_glsl_program != 0) {
    for (Module &module : _modules) {
      _glgsg->_glDetachShader(_glsl_program, module._handle);
    }
    _glgsg->_glDeleteProgram(_glsl_program);
    _glsl_program = 0;
  }

  for (Module &module : _modules) {
    _glgsg->_glDeleteShader(module._handle);
  }

  _modules.clear();

  _glgsg->report_my_gl_errors();
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
  return (_glsl_program != 0);
}

/**
 * This function is to be called to enable a new shader.  It also initializes
 * all of the shader's input parameters.
 */
void CLP(ShaderContext)::
bind() {
  if (!_validated) {
    _glgsg->_glValidateProgram(_glsl_program);
    report_program_errors(_glsl_program, false);
    _validated = true;
  }

  if (!_shader->get_error_flag()) {
    _glgsg->_glUseProgram(_glsl_program);
  }

  if (GLCAT.is_spam()) {
    GLCAT.spam() << "glUseProgram(" << _glsl_program << "): "
                 << _shader->get_filename() << "\n";
  }

  _glgsg->report_my_gl_errors();
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
  else if (state_rs != target_rs) {
    // The state has changed since last time.
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
        binding._binding->fetch_data(state, scratch + binding._offset, false);
      }

      for (const UniformBlock::Call &call : block._matrices) {
        ((PFNGLUNIFORMMATRIX4FVPROC)call._func)(call._location, call._count, GL_FALSE, (const GLfloat *)(scratch + call._offset));
      }

      for (const UniformBlock::Call &call : block._vectors) {
        ((PFNGLUNIFORM4FVPROC)call._func)(call._location, call._count, (const GLfloat *)(scratch + call._offset));
      }
    }
  }

  _glgsg->report_my_gl_errors();
}

/**
 * Disable all the vertex arrays used by this shader.
 */
void CLP(ShaderContext)::
disable_shader_vertex_arrays() {
  if (_glsl_program == 0) {
    return;
  }

  for (size_t i = 0; i < _shader->_var_spec.size(); ++i) {
    const Shader::ShaderVarSpec &bind = _shader->_var_spec[i];
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
  if (_glsl_program == 0) {
    return true;
  }

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

  _glgsg->report_my_gl_errors();

  return true;
}

/**
 * Disable all the texture bindings used by this shader.
 */
void CLP(ShaderContext)::
disable_shader_texture_bindings() {
  if (_glsl_program == 0) {
    return;
  }

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

  if (_glsl_program == 0) {
    return;
  }

#ifndef OPENGLES
  GLbitfield barriers = 0;
#endif

  ShaderInputBinding::State state;
  state.gsg = _glgsg;
  state.matrix_cache = &_matrix_cache[0];

  // First bind all the 'image units'; a bit of an esoteric OpenGL feature
  // right now.
  int num_image_units = (int)_image_units.size();
  if (num_image_units > 0) {
    for (int i = 0; i < num_image_units; ++i) {
      ImageUnit &unit = _image_units[i];

      ShaderType::Access access = ShaderType::Access::read_write;
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

#ifndef OPENGLES
          if (gtc->needs_barrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT)) {
            barriers |= GL_SHADER_IMAGE_ACCESS_BARRIER_BIT;
          }
#endif
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
          access = access & ShaderType::Access::write_only;
        }

        GLenum gl_access = GL_READ_WRITE;
        GLint bind_level = n;
        GLint bind_layer = std::max(z, 0);
        GLboolean layered = z < 0;

        switch (access) {
        case ShaderType::Access::none:
          gl_tex = 0;
        case ShaderType::Access::read_only:
          gl_access = GL_READ_ONLY;
          break;

        case ShaderType::Access::write_only:
          gl_access = GL_WRITE_ONLY;
          unit._written = true;
          break;

        case ShaderType::Access::read_write:
          gl_access = GL_READ_WRITE;
          unit._written = true;
          break;
        }

        _glgsg->_glBindImageTexture(i, gl_tex, bind_level, layered, bind_layer,
                                    gl_access, gtc->_internal_format);
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
    if (gtc->needs_barrier(GL_TEXTURE_FETCH_BARRIER_BIT)) {
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
  }

#ifndef OPENGLES
  if (multi_bind && num_textures > 0) {
    _glgsg->_glBindTextures(0, num_textures, textures);
    _glgsg->_glBindSamplers(0, num_textures, samplers);
  }

  if (barriers != 0) {
    // Issue a memory barrier prior to this shader's execution.
    _glgsg->issue_memory_barrier(barriers);
  }
#endif

  _glgsg->report_my_gl_errors();
}

/**
 * Updates the shader buffer bindings for this shader.
 */
void CLP(ShaderContext)::
update_shader_buffer_bindings(ShaderContext *prev) {
#ifndef OPENGLES
  // Update the shader storage buffer bindings.
  const ShaderAttrib *attrib = _glgsg->_target_shader;

  for (size_t i = 0; i < _storage_blocks.size(); ++i) {
    StorageBlock &block = _storage_blocks[i];

    ShaderBuffer *buffer = attrib->get_shader_input_buffer(block._name);
#ifndef NDEBUG
    if (buffer->get_data_size_bytes() < block._min_size) {
      GLCAT.error()
        << "cannot bind " << *buffer << " to shader because it is too small"
           " (expected at least " << block._min_size << " bytes)\n";
    }
#endif
    _glgsg->apply_shader_buffer(block._binding_index, buffer);
  }
#endif
}

/**
 * This subroutine prints the infolog for a shader.
 */
void CLP(ShaderContext)::
report_shader_errors(const Module &module, bool fatal) {
  char *info_log;
  GLint length = 0;
  GLint num_chars  = 0;

  _glgsg->_glGetShaderiv(module._handle, GL_INFO_LOG_LENGTH, &length);

  if (length <= 1) {
    return;
  }

  info_log = (char *) alloca(length);
  _glgsg->_glGetShaderInfoLog(module._handle, length, &num_chars, info_log);
  if (strcmp(info_log, "Success.\n") == 0 ||
      strcmp(info_log, "No errors.\n") == 0) {
    return;
  }

  if (!module._module->is_of_type(ShaderModuleGlsl::get_class_type())) {
    GLCAT.error(false) << info_log;
    return;
  }
  const ShaderModuleGlsl *glsl_module = (const ShaderModuleGlsl *)module._module;

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
 * Compiles the given ShaderModuleGlsl and attaches it to the program.
 */
bool CLP(ShaderContext)::
attach_shader(const ShaderModule *module, Shader::ModuleSpecConstants &consts,
              const LocationMap &locations, bool &needs_query_locations) {
  ShaderModule::Stage stage = module->get_stage();

  GLuint handle = 0;
  switch (stage) {
  case ShaderModule::Stage::vertex:
    handle = _glgsg->_glCreateShader(GL_VERTEX_SHADER);
    break;
  case ShaderModule::Stage::fragment:
    handle = _glgsg->_glCreateShader(GL_FRAGMENT_SHADER);
    break;
#ifndef OPENGLES
  case ShaderModule::Stage::geometry:
    if (_glgsg->get_supports_geometry_shaders()) {
      handle = _glgsg->_glCreateShader(GL_GEOMETRY_SHADER);
    }
    break;
  case ShaderModule::Stage::tess_control:
    if (_glgsg->get_supports_tessellation_shaders()) {
      handle = _glgsg->_glCreateShader(GL_TESS_CONTROL_SHADER);
    }
    break;
  case ShaderModule::Stage::tess_evaluation:
    if (_glgsg->get_supports_tessellation_shaders()) {
      handle = _glgsg->_glCreateShader(GL_TESS_EVALUATION_SHADER);
    }
    break;
#endif
  case ShaderModule::Stage::compute:
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
    return false;
  }

  if (_glgsg->_use_object_labels) {
    string name = module->get_source_filename();
    _glgsg->_glObjectLabel(GL_SHADER, handle, name.size(), name.data());
  }

  bool needs_compile = false;
  if (module->is_of_type(ShaderModuleSpirV::get_class_type())) {
    ShaderModuleSpirV *spv = (ShaderModuleSpirV *)module;

    pmap<uint32_t, int> id_to_location;
    for (size_t pi = 0; pi < spv->get_num_parameters(); ++pi) {
      const ShaderModule::Variable &var = spv->get_parameter(pi);
      auto it = locations.find(var.name);
      if (it != locations.end()) {
        id_to_location[var.id] = it->second;
      }
    }

#ifndef OPENGLES
    if (_glgsg->_supports_spir_v) {
      // Load a SPIR-V binary.
      if (GLCAT.is_debug()) {
        GLCAT.debug()
          << "Attaching SPIR-V " << stage << " shader binary "
          << module->get_source_filename() << "\n";
      }

      // Make a transformer so we can do some transformations such as assigning
      // locations.
      SpirVTransformer transformer(spv->_instructions);

      if (!id_to_location.empty()) {
        transformer.assign_locations(id_to_location);
      }

      ShaderModuleSpirV::InstructionStream stream = transformer.get_result();

      if (_glgsg->_gl_vendor == "NVIDIA Corporation" && !id_to_location.empty()) {
        // Sigh... NVIDIA driver gives an error if the SPIR-V ID doesn't match
        // for variables with overlapping locations if the OpName is stripped.
        // We'll have to just insert OpNames for every parameter.
        // https://forums.developer.nvidia.com/t/gl-arb-gl-spirv-bug-duplicate-location-link-error-if-opname-is-stripped-from-spir-v-shader/128491
        // Bug was found with 446.14 drivers on Windows 10 64-bit.
        ShaderModuleSpirV::InstructionIterator it = stream.begin_annotations();
        for (ShaderModuleSpirV::Instruction op : spv->_instructions) {
          if (op.opcode == spv::OpVariable &&
              (spv::StorageClass)op.args[2] == spv::StorageClassUniformConstant) {
            uint32_t var_id = op.args[1];
            auto lit = id_to_location.find(var_id);
            if (lit != id_to_location.end()) {
              uint32_t args[4] = {var_id, 0, 0, 0};
              int len = sprintf((char *)(args + 1), "p%d", lit->second);
              nassertr(len > 0 && len < 12, false);
              it = stream.insert(it, spv::OpName, args, len / 4 + 2);
              ++it;
            }
          }
        }
      }

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
          << module->get_source_filename() << "\n";
      }
      spirv_cross::CompilerGLSL compiler(std::vector<uint32_t>(spv->get_data(), spv->get_data() + spv->get_data_size()));
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
      compiler.set_common_options(options);

      if (options.version < 130) {
        _emulate_float_attribs = true;
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
      for (spirv_cross::VariableID id : compiler.get_active_interface_variables()) {
        uint32_t loc = compiler.get_decoration(id, spv::DecorationLocation);
        spv::StorageClass sc = compiler.get_storage_class(id);

        char buf[1024];
        if (sc == spv::StorageClassUniformConstant) {
          CPT(InternalName) name;
          for (size_t i = 0; i < spv->get_num_parameters(); ++i) {
            const ShaderModule::Variable &var = spv->get_parameter(i);
            if (var.id == id) {
              auto it = locations.find(var.name);
              if (it != locations.end()) {
                sprintf(buf, "p%u", it->second);
                compiler.set_name(id, buf);
                compiler.set_decoration(id, spv::DecorationLocation, it->second);
              }
              break;
            }
          }

          // Older versions of OpenGL (ES) do not support explicit uniform
          // locations, and we need to query the locations later.
          if ((!options.es && options.version < 430) ||
              (options.es && options.version < 310)) {
            needs_query_locations = true;
          }
        }
        else if (sc == spv::StorageClassInput) {
          if (stage == ShaderModule::Stage::vertex) {
            // Explicit attrib locations were added in GLSL 3.30, but we can
            // override the binding in older versions using the API.
            sprintf(buf, "a%u", loc);
            if (options.version < 330) {
              _glgsg->_glBindAttribLocation(_glsl_program, loc, buf);
            }
          } else {
            // For all other stages, it's just important that the names match,
            // so we assign the names based on the location and successive
            // numbering of the shaders.
            sprintf(buf, "i%u_%u", (unsigned)_modules.size(), loc);
          }
          compiler.set_name(id, buf);
        }
        else if (sc == spv::StorageClassOutput) {
          if (stage == ShaderModule::Stage::fragment) {
            // Output of the last stage, same story as above.
            sprintf(buf, "o%u", loc);
            if (options.version < 330) {
              _glgsg->_glBindFragDataLocation(_glsl_program, loc, buf);
            }
          } else {
            // Match the name of the next stage.
            sprintf(buf, "i%u_%u", (unsigned)_modules.size() + 1u, loc);
          }
          compiler.set_name(id, buf);
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
      needs_compile = true;
    }
  }
  else if (module->is_of_type(ShaderModuleGlsl::get_class_type())) {
    // Legacy preprocessed GLSL.
    if (GLCAT.is_debug()) {
      GLCAT.debug()
        << "Compiling GLSL " << stage << " shader "
        << module->get_source_filename() << "\n";
    }

    ShaderModuleGlsl *glsl_module = (ShaderModuleGlsl *)module;
    std::string text = glsl_module->get_ir();
    const char *text_str = text.c_str();
    _glgsg->_glShaderSource(handle, 1, &text_str, nullptr);

    needs_compile = true;
    _is_legacy = true;
  } else {
    GLCAT.error()
      << "Unsupported shader module type " << module->get_type() << "!\n";
    return false;
  }

  // Don't check compile status yet, which would force the compile to complete
  // synchronously.
  _glgsg->_glAttachShader(_glsl_program, handle);

  Module moddef = {module, handle, needs_compile};
  _modules.push_back(std::move(moddef));

  return true;
}

/**
 * This subroutine compiles a GLSL shader.
 */
bool CLP(ShaderContext)::
compile_and_link(const LocationMap &locations, bool &needs_query_locations) {
  _modules.clear();
  _glsl_program = _glgsg->_glCreateProgram();
  if (!_glsl_program) {
    return false;
  }

  if (_glgsg->_use_object_labels) {
    const std::string &name = _shader->get_debug_name();
    _glgsg->_glObjectLabel(GL_PROGRAM, _glsl_program, name.size(), name.data());
  }

  // Do we have a compiled program?  Try to load that.
  unsigned int format;
  string binary;
  if (_shader->get_compiled(format, binary)) {
    _glgsg->_glProgramBinary(_glsl_program, format, binary.data(), binary.size());

    GLint status;
    _glgsg->_glGetProgramiv(_glsl_program, GL_LINK_STATUS, &status);
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
  }

  bool valid = true;
  for (Shader::LinkedModule &linked_module : _shader->_modules) {
    valid &= attach_shader(linked_module._module.get_read_pointer(), linked_module._consts,
                           locations, needs_query_locations);
  }

  if (!valid) {
    return false;
  }

  // Now compile the individual shaders.  NVIDIA drivers seem to cope better
  // when we compile them all in one go.
  for (Module &module : _modules) {
    if (module._needs_compile) {
      _glgsg->_glCompileShader(module._handle);
      module._needs_compile = false;
    }
  }

  // There might be warnings, so report those.  GLSLShaders::const_iterator
  // it; for (it = _modules.begin(); it != _modules.end(); ++it) {
  // report_shader_errors(*it); }

  // Under OpenGL's compatibility profile, we have to make sure that we bind
  // something to attribute 0.  Make sure that this is the position array.
  _glgsg->_glBindAttribLocation(_glsl_program, 0, "p3d_Vertex");
  _glgsg->_glBindAttribLocation(_glsl_program, 0, "vertex");

  // While we're at it, let's also map these to fixed locations.  These
  // attributes were historically fixed to these locations, so it might help a
  // buggy driver.
  _glgsg->_glBindAttribLocation(_glsl_program, 2, "p3d_Normal");
  _glgsg->_glBindAttribLocation(_glsl_program, 3, "p3d_Color");

  if (gl_fixed_vertex_attrib_locations) {
    _glgsg->_glBindAttribLocation(_glsl_program, 1, "transform_weight");
    _glgsg->_glBindAttribLocation(_glsl_program, 2, "normal");
    _glgsg->_glBindAttribLocation(_glsl_program, 3, "color");
    _glgsg->_glBindAttribLocation(_glsl_program, 7, "transform_index");
    _glgsg->_glBindAttribLocation(_glsl_program, 8, "p3d_MultiTexCoord0");
    _glgsg->_glBindAttribLocation(_glsl_program, 8, "texcoord");
  }

  // Also bind the p3d_FragData array to the first index always.
  if (_glgsg->_glBindFragDataLocation != nullptr) {
    _glgsg->_glBindFragDataLocation(_glsl_program, 0, "p3d_FragData");
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

    _glgsg->_glProgramParameteri(_glsl_program, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE);
  }
#endif  // !__EMSCRIPTEN__

  if (GLCAT.is_debug()) {
    GLCAT.debug()
      << "Linking shader " << _shader->get_filename() << "\n";
  }

  _glgsg->_glLinkProgram(_glsl_program);

  // Query the link status.  This will cause the application to wait for the
  // link to be finished.
  GLint status = GL_FALSE;
  _glgsg->_glGetProgramiv(_glsl_program, GL_LINK_STATUS, &status);
  if (status != GL_TRUE) {
    // The link failed.  Is it because one of the shaders failed to compile?
    bool any_failed = false;
    for (Module &module : _modules) {
      _glgsg->_glGetShaderiv(module._handle, GL_COMPILE_STATUS, &status);

      if (status != GL_TRUE) {
        GLCAT.error()
          << "An error occurred while compiling shader module "
          << module._module->get_source_filename() << ":\n";
        report_shader_errors(module, true);
        any_failed = true;
      } else {
        // Report any warnings.
        report_shader_errors(module, false);
      }

      // Delete the shader, we don't need it any more.
      _glgsg->_glDeleteShader(module._handle);
    }
    _modules.clear();

    if (any_failed) {
      // One or more of the shaders failed to compile, which would explain the
      // link failure.  We know enough.
      return false;
    }

    GLCAT.error() << "An error occurred while linking shader "
                  << _shader->get_filename() << "\n";
    report_program_errors(_glsl_program, true);
    return false;
  }

  // Report any warnings.
  report_program_errors(_glsl_program, false);

#ifndef __EMSCRIPTEN__
  if (retrieve_binary) {
    GLint length = 0;
    _glgsg->_glGetProgramiv(_glsl_program, GL_PROGRAM_BINARY_LENGTH, &length);
    length += 2;

    char *binary = (char *)alloca(length);
    GLenum format;
    GLsizei num_bytes = 0;
    _glgsg->_glGetProgramBinary(_glsl_program, length, &num_bytes, &format, (void*)binary);

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
  return valid;
}

#endif  // OPENGLES_1
