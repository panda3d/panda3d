/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dxShaderContext9.cxx
 * @author jyelon
 * @date 2005-09-01
 */

#include "dxGraphicsStateGuardian9.h"
#include "dxShaderContext9.h"
#include "dxVertexBufferContext9.h"
#include "shaderModuleSpirV.h"

#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <spirv_cross/spirv_hlsl.hpp>

#define DEBUG_SHADER 0

TypeHandle DXShaderContext9::_type_handle;

/**
 * xyz
 */
DXShaderContext9::
DXShaderContext9(Shader *s, GSG *gsg) : ShaderContext(s) {
  DWORD *vs_data;
  CPT(ShaderModule) vertex_module = s->get_module(Shader::Stage::vertex);
  if (compile_module(vertex_module, vs_data)) {
    HRESULT result = gsg->_d3d_device->CreateVertexShader(vs_data, &_vertex_shader);
    if (FAILED(result)) {
      dxgsg9_cat.error()
        << "Failed to create vertex shader: " << D3DERRORSTRING(result) << "\n";
    }
  }

  DWORD *ps_data;
  CPT(ShaderModule) fragment_module = s->get_module(Shader::Stage::fragment);
  if (compile_module(fragment_module, ps_data)) {
    HRESULT result = gsg->_d3d_device->CreatePixelShader(ps_data, &_pixel_shader);
    if (FAILED(result)) {
      dxgsg9_cat.error()
        << "Failed to create pixel shader: " << D3DERRORSTRING(result) << "\n";
    }
  }
}

/**
 * xyz
 */
DXShaderContext9::
~DXShaderContext9() {
  release_resources();
}

/**
 * Compiles the given shader module.
 */
bool DXShaderContext9::
compile_module(const ShaderModule *module, DWORD *&data) {
  const ShaderModuleSpirV *spv;
  DCAST_INTO_R(spv, module, false);

  if (dxgsg9_cat.is_debug()) {
    dxgsg9_cat.debug()
      << "Transpiling SPIR-V " << spv->get_stage() << " shader "
      << spv->get_source_filename() << "\n";
  }

  spirv_cross::CompilerHLSL compiler(std::vector<uint32_t>(spv->get_data(), spv->get_data() + spv->get_data_size()));
  spirv_cross::CompilerHLSL::Options options;
  options.shader_model = 30;
  options.flatten_matrix_vertex_input_semantics = true;
  options.point_size_compat = false;
  options.point_coord_compat = true;
  compiler.set_hlsl_options(options);

  // Bind certain known attributes to specific semantics.
  int texcoord_index = 0;
  for (const Shader::ShaderVarSpec &spec : _shader->_var_spec) {
    uint32_t idx = (uint32_t)spec._id._location;
    if (spec._name == InternalName::get_vertex()) {
      compiler.add_vertex_attribute_remap({idx, "POSITION"});
    }
    else if (spec._name == InternalName::get_transform_weight()) {
      compiler.add_vertex_attribute_remap({idx, "BLENDWEIGHT"});
    }
    else if (spec._name == InternalName::get_transform_index()) {
      compiler.add_vertex_attribute_remap({idx, "BLENDINDICES"});
    }
    else if (spec._name == InternalName::get_normal()) {
      compiler.add_vertex_attribute_remap({idx, "NORMAL"});
    }
    else if (spec._name == InternalName::get_tangent()) {
      compiler.add_vertex_attribute_remap({idx, "TANGENT"});
    }
    else if (spec._name == InternalName::get_binormal()) {
      compiler.add_vertex_attribute_remap({idx, "BINORMAL"});
    }
    else if (spec._name == InternalName::get_color()) {
      compiler.add_vertex_attribute_remap({idx, "COLOR"});
    }
    else if (spec._name == InternalName::get_size()) {
      compiler.add_vertex_attribute_remap({idx, "PSIZE"});
    }
    else {
      // The rest gets mapped to TEXCOORD + location.
      for (size_t i = 0; i < spec._elements; ++i) {
        char buffer[16];
        sprintf(buffer, "TEXCOORD%d", (int)texcoord_index);
        compiler.add_vertex_attribute_remap({idx, buffer});
        ++texcoord_index;
        ++idx;
      }
    }
  }

  // Create a mapping from id to parameter index.  This makes reflection
  // a little easier later on.
  pmap<uint32_t, unsigned int> params_by_id;
  for (size_t i = 0; i < module->get_num_parameters(); ++i) {
    const ShaderModule::Variable &var = module->get_parameter(i);

    for (size_t j = 0; j < _shader->_parameters.size(); ++j) {
      if (_shader->_parameters[j]._name == var.name) {
        params_by_id[var.id] = (unsigned int)j;
        break;
      }
    }
  }

  // Tell spirv-cross to rename the constants to "p#", where # is the index of
  // the original parameter.  This makes it easier to map the compiled
  // constants back to the original parameters later on.
  for (spirv_cross::VariableID id : compiler.get_active_interface_variables()) {
    spv::StorageClass sc = compiler.get_storage_class(id);

    char buf[24];
    if (sc == spv::StorageClassUniformConstant) {
      nassertd(params_by_id.count(id)) continue;

      unsigned int index = params_by_id[id];
      sprintf(buf, "p%u", index);
      compiler.set_name(id, buf);
    }
  }

  // Optimize out unused variables.
  compiler.set_enabled_interface_variables(compiler.get_active_interface_variables());

  std::string code = compiler.compile();

  if (dxgsg9_cat.is_debug()) {
    dxgsg9_cat.debug()
      << "SPIRV-Cross compilation resulted in HLSL shader:\n"
      << code << "\n";
  }

  const char *profile;
  switch (spv->get_stage()) {
  case Shader::Stage::vertex:
    profile = "vs_3_0";
    break;

  case Shader::Stage::fragment:
    profile = "ps_3_0";
    break;

  default:
    return false;
  }

  ID3DXBuffer *shader;
  ID3DXBuffer *errors;
  HRESULT result = D3DXCompileShader(code.c_str(), code.size(), nullptr, nullptr, "main", profile, D3DXSHADER_PACKMATRIX_ROWMAJOR, &shader, &errors, nullptr);
  if (SUCCEEDED(result)) {
    nassertr(shader != nullptr, false);
    data = (DWORD *)shader->GetBufferPointer();

#ifndef NDEBUG
    if (dxgsg9_cat.is_debug()) {
      std::ostream &out = dxgsg9_cat.debug()
        << "Disassembly of compiled " << profile << " module:\n";
      LPD3DXBUFFER dis;
      if (SUCCEEDED(D3DXDisassembleShader(data, FALSE, NULL, &dis))) {
        out << (char *)dis->GetBufferPointer();
        dis->Release();
      }
    }
#endif

    if (errors != nullptr) {
      errors->Release();
    }

    return query_constants(module, data);
  }
  else {
    dxgsg9_cat.error()
      << "Failed to compile " << module->get_stage() << " shader ("
      << DXGetErrorString(result) << "):\n";

    if (errors != nullptr) {
      std::string messages((const char *)errors->GetBufferPointer(), errors->GetBufferSize());
      dxgsg9_cat.error(false) << messages << "\n";
      errors->Release();
    }
    return false;
  }
}

/**
 * Walks the constant table.
 */
bool DXShaderContext9::
query_constants(const ShaderModule *module, DWORD *data) {
  if (memcmp(data + 2, "CTAB", 4) != 0) {
    dxgsg9_cat.error()
      << "Could not find constant table!\n";
    return false;
  }

  BYTE *table_data = (BYTE *)(data + 3);
  D3DXSHADER_CONSTANTTABLE *table = (D3DXSHADER_CONSTANTTABLE *)table_data;
  D3DXSHADER_CONSTANTINFO *constants = (D3DXSHADER_CONSTANTINFO *)(table_data + table->ConstantInfo);

  if (dxgsg9_cat.is_debug()) {
    if (table->Constants != 0) {
      dxgsg9_cat.debug()
        << "Constant table for compiled " << *module << ":\n";
    } else {
      dxgsg9_cat.debug()
        << "Empty constant table for compiled " << *module << "\n";
    }
  }

  Shader::Stage stage = module->get_stage();

  for (DWORD ci = 0; ci < table->Constants; ++ci) {
    D3DXSHADER_CONSTANTINFO &constant = constants[ci];
    D3DXSHADER_TYPEINFO *type = (D3DXSHADER_TYPEINFO *)(table_data + constant.TypeInfo);

    // We renamed the constants to p# earlier on, so extract the original
    // parameter index.
    const char *name = (const char *)(table_data + constant.Name);
    if (name[0] != 'p') {
      if (stage == Shader::Stage::vertex && strcmp(name, "gl_HalfPixel") == 0) {
        // This is a special input generated by spirv-cross.
        _half_pixel_register = constant.RegisterIndex;
        continue;
      }
      dxgsg9_cat.warning()
        << "Ignoring unknown " << stage << " shader constant " << name << "\n";
      continue;
    }
    int index = atoi(name + 1);
    const Shader::Parameter &param = _shader->_parameters[index];
    const ShaderType *element_type = param._type;
    size_t num_elements = 1;

    // If there is no binding yet for this parameter, add it.
    size_t offset = (size_t)-1;
    if (param._binding != nullptr) {
      for (const Binding &binding : _data_bindings) {
        if (param._binding == binding._binding) {
          offset = binding._offset;
        }
      }
      if (offset == (size_t)-1) {
        offset = _scratch_space_size;

        Binding binding;
        binding._binding = param._binding;
        binding._offset = offset;
        binding._dep = param._binding->get_state_dep();
        _constant_deps |= binding._dep;
        _data_bindings.push_back(std::move(binding));

        // Pad space to 16-byte boundary
        uint32_t size = param._type->get_size_bytes(true);
        size = (size + 15) & ~15;
        _scratch_space_size += size;
      }
    }

    if (const ShaderType::Array *array_type = param._type->as_array()) {
      element_type = array_type->get_element_type();
      num_elements = array_type->get_num_elements();
    }

    int reg_set = constant.RegisterSet;
    int reg_idx = constant.RegisterIndex;
    int reg_end = reg_idx + constant.RegisterCount;
    if (!r_query_constants(stage, param, param._type, offset, 0, table_data, *type, reg_set, reg_idx, reg_end)) {
      return false;
    }

#ifndef NDEBUG
    if (dxgsg9_cat.is_debug()) {
      const char sets[] = {'b', 'i', 'c', 's'};
      if (type->Class == D3DXPC_STRUCT) {
        dxgsg9_cat.debug()
          << "  struct " << name << "[" << type->Elements << "] (" << *param._name
          << ") at register " << sets[constant.RegisterSet]
          << constant.RegisterIndex;
      } else {
        const char *types[] = {"void", "bool", "int", "float", "string", "texture", "texture1D", "texture2D", "texture3D", "textureCUBE", "sampler", "sampler1D", "sampler2D", "sampler3D", "samplerCUBE"};
        dxgsg9_cat.debug()
          << "  " << ((type->Type <= D3DXPT_SAMPLERCUBE) ? types[type->Type] : "unknown")
          << " " << name << "[" << type->Elements << "] (" << *param._name
          << ") at register " << sets[constant.RegisterSet]
          << constant.RegisterIndex;
      }
      if (constant.RegisterCount > 1) {
        dxgsg9_cat.debug(false)
          << ".." << (constant.RegisterIndex + constant.RegisterCount - 1);
      }
      dxgsg9_cat.debug(false) << std::endl;
    }
#endif
  }

  return true;
}

/**
 * Recursive method used by query_constants.
 */
bool DXShaderContext9::
r_query_constants(Shader::Stage stage, const Shader::Parameter &param,
                  const ShaderType *type, size_t offset, int resource_index,
                  BYTE *table_data, D3DXSHADER_TYPEINFO &typeinfo,
                  int reg_set, int &reg_idx, int reg_end) {
  if (typeinfo.Class == D3DXPC_STRUCT) {
    int stride = 0;
    const ShaderType *element_type = type;
    if (const ShaderType::Array *array_type = type->as_array()) {
      element_type = array_type->get_element_type();
      stride = array_type->get_stride_bytes();
    }

    const ShaderType::Struct *struct_type = element_type->as_struct();
    nassertr(struct_type != nullptr, false);

    D3DXSHADER_STRUCTMEMBERINFO *members = (D3DXSHADER_STRUCTMEMBERINFO *)(table_data + typeinfo.StructMemberInfo);

    for (WORD ei = 0; ei < typeinfo.Elements && reg_idx < reg_end; ++ei) {
      DWORD mi = 0;
      for (; mi < typeinfo.StructMembers && reg_idx < reg_end; ++mi) {
        D3DXSHADER_TYPEINFO *typeinfo = (D3DXSHADER_TYPEINFO *)(table_data + members[mi].TypeInfo);

        const ShaderType::Struct::Member &member = struct_type->get_member(mi);
        if (!r_query_constants(stage, param, member.type, offset + member.offset, resource_index, table_data, *typeinfo, reg_set, reg_idx, reg_end)) {
          return false;
        }

        resource_index += member.type->get_num_resources();
      }

      if (reg_idx < reg_end) {
        // If there are members left over in the struct, be sure to increment
        // the resource_index anyway, for the next array element.
        while (mi < struct_type->get_num_members()) {
          const ShaderType::Struct::Member &member = struct_type->get_member(mi++);
          resource_index += member.type->get_num_resources();
        }
      }

      offset += stride;
    }
  }
  else if (reg_set == D3DXRS_SAMPLER) {
    const ShaderType *element_type;
    uint32_t num_elements;
    type->unwrap_array(element_type, num_elements);

    for (UINT ei = 0; ei < typeinfo.Elements && reg_idx < reg_end; ++ei) {
      TextureRegister reg;
      reg.unit = reg_idx;
      reg.binding = param._binding;
      reg.resource_id = param._binding->get_resource_id(resource_index++, element_type);
      _textures.push_back(std::move(reg));
      ++reg_idx;
    }
  }
  else {
    // Non-aggregate type.  Note that arrays of arrays are not supported.
    //nassertr(!element_type->is_aggregate_type(), false);

    // Note that RegisterCount may be lower than Rows * Elements if the
    // optimizer decided that eg. the last row of a matrix is not used!

    ConstantRegister reg;
    reg.set = (D3DXREGISTER_SET)reg_set;
    reg.reg = reg_idx;
    reg.count = std::min((UINT)typeinfo.Elements * typeinfo.Rows, (UINT)(reg_end - reg_idx));
    reg.dep = param._binding ? param._binding->get_state_dep() : 0;
    reg.offset = offset;

    // Regularly, ints and bools actually get mapped to a float constant
    // register, so we need to do an extra conversion step.
    reg.convert = (reg.set == D3DXRS_FLOAT4 && typeinfo.Type != D3DXPT_FLOAT);

    if (stage == Shader::Stage::vertex) {
      _vertex_constants.push_back(std::move(reg));
    }
    if (stage == Shader::Stage::fragment) {
      _pixel_constants.push_back(std::move(reg));
    }

    reg_idx += typeinfo.Elements * typeinfo.Rows;
  }

  return true;
}

/**
 * Should deallocate all system resources (such as vertex program handles or
 * Cg contexts).
 */
void DXShaderContext9::
release_resources() {
  if (_vertex_shader != nullptr) {
    _vertex_shader->Release();
    _vertex_shader = nullptr;
  }
  if (_pixel_shader != nullptr) {
    _pixel_shader->Release();
    _pixel_shader = nullptr;
  }

  for (const auto &it : _vertex_declarations) {
    it.second.first->Release();
  }
  _vertex_declarations.clear();
}

/**
 * This function is to be called to enable a new shader.  It also initializes
 * all of the shader's input parameters.
 */
bool DXShaderContext9::
bind(GSG *gsg) {
  if (!valid(gsg)) {
    return false;
  }

  // clear the last cached FVF to make sure the next SetFVF call goes
  // through
  gsg->_last_fvf = 0;

  // Bind the shaders.
  HRESULT result;
  result = gsg->_d3d_device->SetVertexShader(_vertex_shader);
  if (FAILED(result)) {
    dxgsg9_cat.error() << "SetVertexShader failed " << D3DERRORSTRING(result);
    return false;
  }

  result = gsg->_d3d_device->SetPixelShader(_pixel_shader);
  if (FAILED(result)) {
    dxgsg9_cat.error() << "SetPixelShader failed " << D3DERRORSTRING(result);
    gsg->_d3d_device->SetVertexShader(nullptr);
    return false;
  }

  //TODO: what should we set this to?
  if (_half_pixel_register >= 0) {
    const float data[4] = {0, 0, 0, 0};
    gsg->_d3d_device->SetVertexShaderConstantF(_half_pixel_register, data, 1);
  }

  return true;
}

/**
 * This function disables a currently-bound shader.
 */
void DXShaderContext9::
unbind(GSG *gsg) {
  gsg->_d3d_device->SetVertexShader(nullptr);
  gsg->_d3d_device->SetPixelShader(nullptr);
}

/**
 * This function gets called whenever the RenderState or TransformState has
 * changed, but the Shader itself has not changed.  It loads new values into
 * the shader's parameters.
 *
 * If "altered" is false, that means you promise that the parameters for this
 * shader context have already been issued once, and that since the last time
 * the parameters were issued, no part of the render state has changed except
 * the external and internal transforms.
 */
void DXShaderContext9::
issue_parameters(GSG *gsg, int altered) {
  if (!valid(gsg)) {
    return;
  }

  nassertv(gsg->_target_shader != nullptr);

  LPDIRECT3DDEVICE9 device = gsg->_d3d_device;

  if (altered & _constant_deps) {
    unsigned char *scratch = (unsigned char *)alloca(_scratch_space_size);

    ShaderInputBinding::State state;
    state.gsg = gsg;
    state.matrix_cache = &gsg->_matrix_cache[0];

    for (const Binding &binding : _data_bindings) {
      if (altered & binding._dep) {
        binding._binding->fetch_data(state, scratch + binding._offset, true);
      }
    }

    for (const ConstantRegister &reg : _vertex_constants) {
      if ((altered & reg.dep) == 0) {
        continue;
      }

      const void *data = scratch + reg.offset;

      switch (reg.set) {
      case D3DXRS_FLOAT4:
        if (reg.convert) {
          for (UINT i = 0; i < reg.count; ++i) {
            LVecBase4i from = ((LVecBase4i *)data)[i];
            ((LVecBase4f *)data)[i] = LCAST(float, from);
          }
        }
        device->SetVertexShaderConstantF(reg.reg, (const float *)data, reg.count);
        break;

      case D3DXRS_INT4:
        device->SetVertexShaderConstantI(reg.reg, (const int *)data, reg.count);
        break;

      case D3DXRS_BOOL:
        device->SetVertexShaderConstantB(reg.reg, (const BOOL *)data, reg.count);
        break;
      }
    }

    for (const ConstantRegister &reg : _pixel_constants) {
      if ((altered & reg.dep) == 0) {
        continue;
      }

      const void *data = scratch + reg.offset;

      switch (reg.set) {
      case D3DXRS_FLOAT4:
        if (reg.convert) {
          for (UINT i = 0; i < reg.count; ++i) {
            LVecBase4i from = ((LVecBase4i *)data)[i];
            ((LVecBase4f *)data)[i] = LCAST(float, from);
          }
        }
        device->SetPixelShaderConstantF(reg.reg, (const float *)data, reg.count);
        break;

      case D3DXRS_INT4:
        device->SetPixelShaderConstantI(reg.reg, (const int *)data, reg.count);
        break;

      case D3DXRS_BOOL:
        device->SetPixelShaderConstantB(reg.reg, (const BOOL *)data, reg.count);
        break;
      }
    }
  }
}

/**
 * Changes the active transform and slider table, used for hardware skinning.
 */
void DXShaderContext9::
update_tables(GSG *gsg, const GeomVertexDataPipelineReader *data_reader) {
  issue_parameters(gsg, Shader::D_vertex_data);
}

/**
 * Disable all the texture bindings used by this shader.
 */
void DXShaderContext9::
disable_shader_texture_bindings(GSG *gsg) {
  for (const TextureRegister &reg : _textures) {
    HRESULT hr = gsg->_d3d_device->SetTexture(reg.unit, nullptr);
    if (FAILED(hr)) {
      dxgsg9_cat.error()
        << "SetTexture(" << reg.unit << ", NULL) failed "
        << D3DERRORSTRING(hr);
    }
  }
}

/**
 * Disables all texture bindings used by the previous shader, then enables all
 * the texture bindings needed by this shader.  Extracts the relevant vertex
 * array data from the gsg.  The current implementation is inefficient,
 * because it may unnecessarily disable textures then immediately reenable
 * them.  We may optimize this someday.
 */
void DXShaderContext9::
update_shader_texture_bindings(DXShaderContext9 *prev, GSG *gsg) {
  if (prev) {
    prev->disable_shader_texture_bindings(gsg);
  }
  if (!valid(gsg)) {
    return;
  }

  ShaderInputBinding::State state;
  state.gsg = gsg;
  state.matrix_cache = &gsg->_matrix_cache[0];

  for (const TextureRegister &reg : _textures) {
    int view = gsg->get_current_tex_view_offset();
    SamplerState sampler;

    PT(Texture) tex = reg.binding->fetch_texture(state, reg.resource_id, sampler, view);
    if (tex.is_null()) {
      continue;
    }

    TextureContext *tc = tex->prepare_now(gsg->_prepared_objects, gsg);
    if (tc == nullptr) {
      continue;
    }

    gsg->apply_texture(reg.unit, tc, view, sampler);
  }
}

/**
 * Returns a vertex declaration object suitable for rendering geometry with the
 * given GeomVertexFormat.
 */
LPDIRECT3DVERTEXDECLARATION9 DXShaderContext9::
get_vertex_declaration(GSG *gsg, const GeomVertexFormat *format, BitMask32 &used_streams) {
  // Look up the GeomVertexFormat in the cache.
  auto it = _vertex_declarations.find(format);
  if (it != _vertex_declarations.end()) {
    // We've previously rendered geometry with this GeomVertexFormat, so we
    // already have a vertex declaration.
    used_streams = it->second.second;
    return it->second.first;
  }

  used_streams = 0;
  int texcoord_index = 0;
  size_t const num_arrays = format->get_num_arrays();
  std::vector<D3DVERTEXELEMENT9> elements;

  for (const Shader::ShaderVarSpec &spec : _shader->_var_spec) {
    D3DDECLUSAGE usage;
    if (spec._name == InternalName::get_vertex()) {
      usage = D3DDECLUSAGE_POSITION;
    }
    else if (spec._name == InternalName::get_transform_weight()) {
      usage = D3DDECLUSAGE_BLENDWEIGHT;
    }
    else if (spec._name == InternalName::get_transform_index()) {
      usage = D3DDECLUSAGE_BLENDINDICES;
    }
    else if (spec._name == InternalName::get_normal()) {
      usage = D3DDECLUSAGE_NORMAL;
    }
    else if (spec._name == InternalName::get_tangent()) {
      usage = D3DDECLUSAGE_TANGENT;
    }
    else if (spec._name == InternalName::get_binormal()) {
      usage = D3DDECLUSAGE_BINORMAL;
    }
    else if (spec._name == InternalName::get_color()) {
      usage = D3DDECLUSAGE_COLOR;
    }
    else if (spec._name == InternalName::get_size()) {
      usage = D3DDECLUSAGE_PSIZE;
    }
    else {
      usage = D3DDECLUSAGE_TEXCOORD;
    }

    int array_index;
    const GeomVertexColumn *column;
    if (!format->get_array_info(spec._name, array_index, column)) {
      // Certain missing ones need to be gotten from the "constant vbuffer",
      // rather than receive the default value of (0, 0, 0, 1).
      if (spec._name == InternalName::get_color()) {
        elements.push_back({
          (WORD)num_arrays,
          (WORD)0,
          (BYTE)D3DDECLTYPE_D3DCOLOR,
          (BYTE)D3DDECLMETHOD_DEFAULT,
          (BYTE)D3DDECLUSAGE_COLOR,
          (BYTE)0,
        });
        used_streams.set_bit(num_arrays);
      }
      else if (spec._name == InternalName::get_transform_index()) {
        elements.push_back({
          (WORD)num_arrays,
          (WORD)4,
          (BYTE)D3DDECLTYPE_UBYTE4,
          (BYTE)D3DDECLMETHOD_DEFAULT,
          (BYTE)D3DDECLUSAGE_BLENDINDICES,
          (BYTE)0,
        });
        used_streams.set_bit(num_arrays);
      }
      else if (spec._name == InternalName::get_instance_matrix()) {
        // Binding the last row isn't necessary; the default is (0, 0, 0, 1)
        for (size_t ei = 0; ei < 3 && ei < spec._elements; ++ei) {
          elements.push_back({
            (WORD)num_arrays,
            (WORD)(8 + 4 * ei),
            (BYTE)D3DDECLTYPE_UBYTE4N,
            (BYTE)D3DDECLMETHOD_DEFAULT,
            (BYTE)D3DDECLUSAGE_TEXCOORD,
            (BYTE)(texcoord_index + ei),
          });
        }
        used_streams.set_bit(num_arrays);
        texcoord_index += spec._elements;
      }
      else if (usage == D3DDECLUSAGE_TEXCOORD) {
        texcoord_index += spec._elements;
      }
      continue;
    }
    used_streams.set_bit(array_index);

    if (column->get_contents() == GeomEnums::C_clip_point &&
        column->get_name() == InternalName::get_vertex()) {
      usage = D3DDECLUSAGE_POSITIONT;
    }
    size_t offset = column->get_start();
    bool normalized = (column->get_contents() == GeomEnums::C_color);
    int num_components = column->get_num_components();

    D3DDECLTYPE type;
    switch (column->get_numeric_type()) {
    case GeomEnums::NT_uint8:
      type = normalized ? D3DDECLTYPE_UBYTE4N : D3DDECLTYPE_UBYTE4;
      break;
    case GeomEnums::NT_uint16:
      type = (num_components > 2) ? D3DDECLTYPE_USHORT4N : D3DDECLTYPE_USHORT2N;
      break;
    case GeomEnums::NT_packed_dcba:
      type = normalized ? D3DDECLTYPE_UBYTE4N : D3DDECLTYPE_UBYTE4;
      break;
    case GeomEnums::NT_packed_dabc:
      type = normalized ? D3DDECLTYPE_D3DCOLOR : D3DDECLTYPE_UBYTE4;
      break;
#ifndef STDFLOAT_DOUBLE
    case GeomEnums::NT_stdfloat:
#endif
    case GeomEnums::NT_float32:
      type = (D3DDECLTYPE)(D3DDECLTYPE_FLOAT1 + num_components - 1);
      break;
    case GeomEnums::NT_int16:
      type = (num_components > 2)
        ? (normalized ? D3DDECLTYPE_SHORT4N : D3DDECLTYPE_SHORT4)
        : (normalized ? D3DDECLTYPE_SHORT2N : D3DDECLTYPE_SHORT2);
      break;
    default:
      dxgsg9_cat.error()
        << "Unsupported numeric type " << column->get_numeric_type()
        << " for vertex column " << *spec._name << "\n";
      continue;
    }

    for (size_t ei = 0; ei < spec._elements; ++ei) {
      elements.push_back({
        (WORD)array_index,
        (WORD)offset,
        (BYTE)type,
        (BYTE)D3DDECLMETHOD_DEFAULT,
        (BYTE)usage,
        (BYTE)((usage == D3DDECLUSAGE_TEXCOORD) ? texcoord_index++ : 0),
      });
      offset += column->get_element_stride();
    }
  }

  // Sort the elements, as D3D seems to require them to be in order.
  struct less_than {
    bool operator () (const D3DVERTEXELEMENT9 &a, const D3DVERTEXELEMENT9 &b) {
      if (a.Stream != b.Stream) {
        return a.Stream < b.Stream;
      }
      return a.Offset < b.Offset;
    }
  };
  std::sort(elements.begin(), elements.end(), less_than());

  // CreateVertexDeclaration is fickle so it helps to have some good debugging
  // info here.
  if (dxgsg9_cat.is_debug()) {
    const char *types[] = {"FLOAT1", "FLOAT2", "FLOAT3", "FLOAT4", "D3DCOLOR", "UBYTE4", "SHORT2", "SHORT4", "UBYTE4N", "SHORT2N", "SHORT4N", "USHORT2N", "USHORT4N", "UDEC3", "DEC3N", "FLOAT16_2", "FLOAT16_4", "UNUSED"};
    const char *usages[] = {"POSITION", "BLENDWEIGHT", "BLENDINDICES", "NORMAL", "PSIZE", "TEXCOORD", "TANGENT", "BINORMAL", "TESSFACTOR", "POSITIONT", "COLOR", "FOG", "DEPTH", "SAMPLE"};
    dxgsg9_cat.debug()
      << "Creating vertex declaration for format " << *format << ":\n";

    for (const D3DVERTEXELEMENT9 &element : elements) {
      dxgsg9_cat.debug()
        << "  {" << element.Stream << ", " << element.Offset << ", "
        << "D3DDECLTYPE_" << types[element.Type] << ", "
        << "D3DDECLMETHOD_DEFAULT, "
        << "D3DDECLUSAGE_" << usages[element.Usage] << ", "
        << (int)element.UsageIndex << "}\n";
    }
  }

  elements.push_back(D3DDECL_END());

  LPDIRECT3DVERTEXDECLARATION9 decl;
  HRESULT result = gsg->_d3d_device->CreateVertexDeclaration(elements.data(), &decl);
  if (FAILED(result)) {
    dxgsg9_cat.error()
      << "CreateVertexDeclaration failed" << D3DERRORSTRING(result);
    return nullptr;
  }

  _vertex_declarations[format] = std::make_pair(decl, used_streams);
  return decl;
}
