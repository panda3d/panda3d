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

  _mat_part_cache = new LMatrix4[s->cp_get_mat_cache_size()];
}

/**
 * xyz
 */
DXShaderContext9::
~DXShaderContext9() {
  release_resources();

  delete[] _mat_part_cache;
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

  // Create a mapping from locations to parameter index.  This makes
  // reflection a little easier later on.
  pmap<int, unsigned int> params_by_location;
  for (size_t i = 0; i < module->get_num_parameters(); ++i) {
    const ShaderModule::Variable &var = module->get_parameter(i);
    if (var.has_location()) {
      params_by_location[var.get_location()] = (unsigned int)i;
    }
  }

  // Tell spirv-cross to rename the constants to "p#", where # is the index of
  // the original parameter.  This makes it easier to map the compiled
  // constants back to the original parameters later on.
  for (spirv_cross::VariableID id : compiler.get_active_interface_variables()) {
    uint32_t loc = compiler.get_decoration(id, spv::DecorationLocation);
    spv::StorageClass sc = compiler.get_storage_class(id);

    char buf[24];
    if (sc == spv::StorageClassUniformConstant) {
      nassertd(params_by_location.count(loc)) continue;

      unsigned int index = params_by_location[loc];
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

  BYTE *offset = (BYTE *)(data + 3);
  D3DXSHADER_CONSTANTTABLE *table = (D3DXSHADER_CONSTANTTABLE *)offset;
  D3DXSHADER_CONSTANTINFO *constants = (D3DXSHADER_CONSTANTINFO *)(offset + table->ConstantInfo);

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
    D3DXSHADER_TYPEINFO *type = (D3DXSHADER_TYPEINFO *)(offset + constant.TypeInfo);

    // We renamed the constants to p# earlier on, so extract the original
    // parameter index.
    const char *name = (const char *)(offset + constant.Name);
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
    const ShaderModule::Variable &var = module->get_parameter(index);
    nassertd(var.has_location()) continue;
    int loc = var.get_location();

    int loc_end = loc + var.type->get_num_interface_locations();
    if ((size_t)loc_end > _register_map.size()) {
      _register_map.resize((size_t)loc_end);
    }

    const ShaderType *element_type = var.type;
    size_t num_elements = 1;

    if (const ShaderType::Array *array_type = var.type->as_array()) {
      element_type = array_type->get_element_type();
      num_elements = array_type->get_num_elements();
    }

    int reg_set = constant.RegisterSet;
    int reg_idx = constant.RegisterIndex;
    int reg_end = reg_idx + constant.RegisterCount;
    if (!r_query_constants(stage, offset, *type, loc, reg_set, reg_idx, reg_end)) {
      return false;
    }

#ifndef NDEBUG
    if (dxgsg9_cat.is_debug()) {
      const char sets[] = {'b', 'i', 'c', 's'};
      if (type->Class == D3DXPC_STRUCT) {
        dxgsg9_cat.debug()
          << "  struct " << name << "[" << type->Elements << "] (" << *var.name
          << "@" << loc << ") at register " << sets[constant.RegisterSet]
          << constant.RegisterIndex;
      } else {
        const char *types[] = {"void", "bool", "int", "float", "string", "texture", "texture1D", "texture2D", "texture3D", "textureCUBE", "sampler", "sampler1D", "sampler2D", "sampler3D", "samplerCUBE"};
        dxgsg9_cat.debug()
          << "  " << ((type->Type <= D3DXPT_SAMPLERCUBE) ? types[type->Type] : "unknown")
          << " " << name << "[" << type->Elements << "] (" << *var.name
          << "@" << loc << ") at register " << sets[constant.RegisterSet]
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
r_query_constants(Shader::Stage stage, BYTE *offset, D3DXSHADER_TYPEINFO &typeinfo,
                  int &loc, int reg_set, int &reg_idx, int reg_end) {
  if (typeinfo.Class == D3DXPC_STRUCT) {
    //const ShaderType::Struct *struct_type = element_type->as_struct();
    //nassertr(struct_type != nullptr, false);
    D3DXSHADER_STRUCTMEMBERINFO *members = (D3DXSHADER_STRUCTMEMBERINFO *)(offset + typeinfo.StructMemberInfo);

    for (WORD ei = 0; ei < typeinfo.Elements && reg_idx < reg_end; ++ei) {
      for (DWORD mi = 0; mi < typeinfo.StructMembers && reg_idx < reg_end; ++mi) {
        D3DXSHADER_TYPEINFO *typeinfo = (D3DXSHADER_TYPEINFO *)(offset + members[mi].TypeInfo);

        if (!r_query_constants(stage, offset, *typeinfo, loc, reg_set, reg_idx, reg_end)) {
          return false;
        }
      }
    }
  } else {
    // Non-aggregate type.  Note that arrays of arrays are not supported.
    //nassertr(!element_type->is_aggregate_type(), false);

    // Note that RegisterCount may be lower than Rows * Elements if the
    // optimizer decided that eg. the last row of a matrix is not used!

    nassertr((size_t)loc < _register_map.size(), false);

    ConstantRegister &reg = _register_map[(size_t)loc];
    reg.set = (D3DXREGISTER_SET)reg_set;
    reg.count = std::max(reg.count, (UINT)(reg_end - reg_idx));
    switch (stage) {
    case ShaderModule::Stage::vertex:
      reg.vreg = reg_idx;
      break;
    case ShaderModule::Stage::fragment:
      reg.freg = reg_idx;
      break;
    default:
      reg.count = 0;
      break;
    }

    loc += typeinfo.Elements;
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
  if (_vertex_shader != nullptr) {
    _vertex_shader->Release();
    _vertex_shader = nullptr;
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

  // Pass in k-parameters and transform-parameters.
  // Since the shader is always unbound at the end of a frame, this is a good
  // place to check for frame parameter as well.
  int altered = Shader::SSD_general;
  int frame_number = ClockObject::get_global_clock()->get_frame_count();
  if (frame_number != _frame_number) {
     altered |= Shader::SSD_frame;
    _frame_number = frame_number;
  }
  issue_parameters(gsg, altered);

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

  LPDIRECT3DDEVICE9 device = gsg->_d3d_device;

  // We have no way to track modifications to PTAs, so we assume that they are
  // modified every frame and when we switch ShaderAttribs.
  if (altered & (Shader::SSD_shaderinputs | Shader::SSD_frame)) {
    // Iterate through _ptr parameters
    for (const Shader::ShaderPtrSpec &spec : _shader->_ptr_spec) {
      Shader::ShaderPtrData ptr_data;
      if (!gsg->fetch_ptr_parameter(spec, ptr_data)) { //the input is not contained in ShaderPtrData
        release_resources();
        return;
      }
      if (spec._id._location < 0 || (size_t)spec._id._location >= _register_map.size()) {
        continue;
      }

      ConstantRegister &reg = _register_map[spec._id._location];
      if (reg.count == 0) {
        continue;
      }

      // Calculate how many elements to transfer; no more than it expects,
      // but certainly no more than we have.
      size_t num_cols = spec._dim[2];
      if (num_cols == 0) {
        continue;
      }
      size_t num_rows = std::min((size_t)spec._dim[0] * (size_t)spec._dim[1],
                                 (size_t)(ptr_data._size / num_cols));

      switch (reg.set) {
      case D3DXRS_BOOL:
        {
          BOOL *data = (BOOL *)alloca(sizeof(BOOL) * 4 * num_rows);
          memset(data, 0, sizeof(BOOL) * 4 * num_rows);
          switch (ptr_data._type) {
          case ShaderType::ST_int:
          case ShaderType::ST_uint:
          case ShaderType::ST_float:
            // All have the same 0-representation.
            if (num_cols == 1) {
              for (size_t i = 0; i < num_rows; ++i) {
                data[i * 4] = ((int *)ptr_data._ptr)[i] != 0;
              }
            } else if (num_cols == 2) {
              for (size_t i = 0; i < num_rows; ++i) {
                data[i * 4 + 0] = ((int *)ptr_data._ptr)[i * 2] != 0;
                data[i * 4 + 1] = ((int *)ptr_data._ptr)[i * 2 + 1] != 0;
              }
            } else if (num_cols == 3) {
              for (size_t i = 0; i < num_rows; ++i) {
                data[i * 4 + 0] = ((int *)ptr_data._ptr)[i * 3] != 0;
                data[i * 4 + 1] = ((int *)ptr_data._ptr)[i * 3 + 1] != 0;
                data[i * 4 + 2] = ((int *)ptr_data._ptr)[i * 3 + 2] != 0;
              }
            } else {
              for (size_t i = 0; i < num_rows * 4; ++i) {
                data[i] = ((int *)ptr_data._ptr)[i] != 0;
              }
            }
            break;

          case ShaderType::ST_double:
            if (num_cols == 1) {
              for (size_t i = 0; i < num_rows; ++i) {
                data[i * 4] = ((double *)ptr_data._ptr)[i] != 0;
              }
            } else if (num_cols == 2) {
              for (size_t i = 0; i < num_rows; ++i) {
                data[i * 4 + 0] = ((double *)ptr_data._ptr)[i * 2] != 0;
                data[i * 4 + 1] = ((double *)ptr_data._ptr)[i * 2 + 1] != 0;
              }
            } else if (num_cols == 3) {
              for (size_t i = 0; i < num_rows; ++i) {
                data[i * 4 + 0] = ((double *)ptr_data._ptr)[i * 3] != 0;
                data[i * 4 + 1] = ((double *)ptr_data._ptr)[i * 3 + 1] != 0;
                data[i * 4 + 2] = ((double *)ptr_data._ptr)[i * 3 + 2] != 0;
              }
            } else {
              for (size_t i = 0; i < num_rows * 4; ++i) {
                data[i] = ((double *)ptr_data._ptr)[i] != 0;
              }
            }
            break;
          }
          if (reg.vreg >= 0) {
            device->SetVertexShaderConstantB(reg.vreg, data, num_rows);
          }
          if (reg.freg >= 0) {
            device->SetPixelShaderConstantB(reg.freg, data, num_rows);
          }
        }
        break;

      case D3DXRS_INT4:
        if (ptr_data._type != ShaderType::ST_int &&
            ptr_data._type != ShaderType::ST_uint) {
          dxgsg9_cat.error()
            << "Cannot pass floating-point data to integer shader input '" << spec._id._name << "'\n";

          // Deactivate it to make sure the user doesn't get flooded with this
          // error.
          reg.count = -1;
        }
        else if (num_cols == 4) {
          // Straight passthrough, hooray!
          void *data = ptr_data._ptr;
          if (reg.vreg >= 0) {
            device->SetVertexShaderConstantI(reg.vreg, (int *)data, num_rows);
          }
          if (reg.freg >= 0) {
            device->SetPixelShaderConstantI(reg.freg, (int *)data, num_rows);
          }
        }
        else {
          // Need to pad out the rows.
          LVecBase4i *data = (LVecBase4i *)alloca(sizeof(LVecBase4i) * num_rows);
          memset(data, 0, sizeof(LVecBase4i) * num_rows);
          if (num_cols == 1) {
            for (size_t i = 0; i < num_rows; ++i) {
              data[i].set(((int *)ptr_data._ptr)[i], 0, 0, 0);
            }
          } else if (num_cols == 2) {
            for (size_t i = 0; i < num_rows; ++i) {
              data[i].set(((int *)ptr_data._ptr)[i * 2],
                          ((int *)ptr_data._ptr)[i * 2 + 1],
                          0, 0);
            }
          } else if (num_cols == 3) {
            for (size_t i = 0; i < num_rows; ++i) {
              data[i].set(((int *)ptr_data._ptr)[i * 3],
                          ((int *)ptr_data._ptr)[i * 3 + 1],
                          ((int *)ptr_data._ptr)[i * 3 + 2],
                          0);
            }
          }
          if (reg.vreg >= 0) {
            device->SetVertexShaderConstantI(reg.vreg, (int *)data, num_rows);
          }
          if (reg.freg >= 0) {
            device->SetPixelShaderConstantI(reg.freg, (int *)data, num_rows);
          }
        }
        break;

      case D3DXRS_FLOAT4:
        if (ptr_data._type == ShaderType::ST_float && num_cols == 4) {
          // Straight passthrough, hooray!
          void *data = ptr_data._ptr;
          if (reg.vreg >= 0) {
            device->SetVertexShaderConstantF(reg.vreg, (float *)data, num_rows);
          }
          if (reg.freg >= 0) {
            device->SetPixelShaderConstantF(reg.freg, (float *)data, num_rows);
          }
        }
        else {
          // Need to pad out the rows.
          LVecBase4f *data = (LVecBase4f *)alloca(sizeof(LVecBase4f) * num_rows);
          memset(data, 0, sizeof(LVecBase4f) * num_rows);
          switch (ptr_data._type) {
          case ShaderType::ST_int:
            if (num_cols == 1) {
              for (size_t i = 0; i < num_rows; ++i) {
                data[i].set((float)((int *)ptr_data._ptr)[i], 0, 0, 0);
              }
            } else if (num_cols == 2) {
              for (size_t i = 0; i < num_rows; ++i) {
                data[i].set((float)((int *)ptr_data._ptr)[i * 2],
                            (float)((int *)ptr_data._ptr)[i * 2 + 1],
                            0, 0);
              }
            } else if (num_cols == 3) {
              for (size_t i = 0; i < num_rows; ++i) {
                data[i].set((float)((int *)ptr_data._ptr)[i * 3],
                            (float)((int *)ptr_data._ptr)[i * 3 + 1],
                            (float)((int *)ptr_data._ptr)[i * 3 + 2],
                            0);
              }
            } else {
              for (size_t i = 0; i < num_rows * 4; ++i) {
                ((float *)data)[i] = (float)((int *)ptr_data._ptr)[i];
              }
            }
            break;

          case ShaderType::ST_uint:
            if (num_cols == 1) {
              for (size_t i = 0; i < num_rows; ++i) {
                data[i].set((float)((unsigned int *)ptr_data._ptr)[i], 0, 0, 0);
              }
            } else if (num_cols == 2) {
              for (size_t i = 0; i < num_rows; ++i) {
                data[i].set((float)((unsigned int *)ptr_data._ptr)[i * 2],
                            (float)((unsigned int *)ptr_data._ptr)[i * 2 + 1],
                            0, 0);
              }
            } else if (num_cols == 3) {
              for (size_t i = 0; i < num_rows; ++i) {
                data[i].set((float)((unsigned int *)ptr_data._ptr)[i * 3],
                            (float)((unsigned int *)ptr_data._ptr)[i * 3 + 1],
                            (float)((unsigned int *)ptr_data._ptr)[i * 3 + 2],
                            0);
              }
            } else {
              for (size_t i = 0; i < num_rows * 4; ++i) {
                ((float *)data)[i] = (float)((unsigned int *)ptr_data._ptr)[i];
              }
            }
            break;

          case ShaderType::ST_float:
            if (num_cols == 1) {
              for (size_t i = 0; i < num_rows; ++i) {
                data[i].set(((float *)ptr_data._ptr)[i], 0, 0, 0);
              }
            } else if (num_cols == 2) {
              for (size_t i = 0; i < num_rows; ++i) {
                data[i].set(((float *)ptr_data._ptr)[i * 2],
                            ((float *)ptr_data._ptr)[i * 2 + 1],
                            0, 0);
              }
            } else if (num_cols == 3) {
              for (size_t i = 0; i < num_rows; ++i) {
                data[i].set(((float *)ptr_data._ptr)[i * 3],
                            ((float *)ptr_data._ptr)[i * 3 + 1],
                            ((float *)ptr_data._ptr)[i * 3 + 2],
                            0);
              }
            } else {
              for (size_t i = 0; i < num_rows * 4; ++i) {
                ((float *)data)[i] = ((float *)ptr_data._ptr)[i];
              }
            }
            break;

          case ShaderType::ST_double:
            if (num_cols == 1) {
              for (size_t i = 0; i < num_rows; ++i) {
                data[i].set((float)((double *)ptr_data._ptr)[i], 0, 0, 0);
              }
            } else if (num_cols == 2) {
              for (size_t i = 0; i < num_rows; ++i) {
                data[i].set((float)((double *)ptr_data._ptr)[i * 2],
                            (float)((double *)ptr_data._ptr)[i * 2 + 1],
                            0, 0);
              }
            } else if (num_cols == 3) {
              for (size_t i = 0; i < num_rows; ++i) {
                data[i].set((float)((double *)ptr_data._ptr)[i * 3],
                            (float)((double *)ptr_data._ptr)[i * 3 + 1],
                            (float)((double *)ptr_data._ptr)[i * 3 + 2],
                            0);
              }
            } else {
              for (size_t i = 0; i < num_rows * 4; ++i) {
                ((float *)data)[i] = (float)((double *)ptr_data._ptr)[i];
              }
            }
            break;
          }

          if (reg.vreg >= 0) {
            device->SetVertexShaderConstantF(reg.vreg, (float *)data, num_rows);
          }
          if (reg.freg >= 0) {
            device->SetPixelShaderConstantF(reg.freg, (float *)data, num_rows);
          }
        }
        break;

      default:
        continue;
      }
    }
  }

  if (altered & _shader->_mat_deps) {
    gsg->update_shader_matrix_cache(_shader, _mat_part_cache, altered);

    for (Shader::ShaderMatSpec &spec : _shader->_mat_spec) {
      if ((altered & spec._dep) == 0) {
        continue;
      }
      if (spec._id._location < 0 || (size_t)spec._id._location >= _register_map.size()) {
        continue;
      }

      ConstantRegister &reg = _register_map[spec._id._location];
      if (reg.count == 0) {
        continue;
      }
      nassertd(reg.set == D3DXRS_FLOAT4) continue;

      const LMatrix4 *val = gsg->fetch_specified_value(spec, _mat_part_cache, altered);
      if (!val) continue;

#ifndef STDFLOAT_DOUBLE
      // In this case, the data is already single-precision.
      const PN_float32 *data = val->get_data();
#else
      // In this case, we have to convert it.
      LMatrix4f valf = LCAST(PN_float32, *val);
      const PN_float32 *data = valf.get_data();
#endif
      PN_float32 scratch[16];

      switch (spec._piece) {
      case Shader::SMP_whole:
      case Shader::SMP_upper3x4:
      case Shader::SMP_upper4x3:
        break;
      case Shader::SMP_transpose:
      case Shader::SMP_transpose3x4:
      case Shader::SMP_transpose4x3:
        scratch[0] = data[0];
        scratch[1] = data[4];
        scratch[2] = data[8];
        scratch[3] = data[12];
        scratch[4] = data[1];
        scratch[5] = data[5];
        scratch[6] = data[9];
        scratch[7] = data[13];
        scratch[8] = data[2];
        scratch[9] = data[6];
        scratch[10] = data[10];
        scratch[11] = data[14];
        scratch[12] = data[3];
        scratch[13] = data[7];
        scratch[14] = data[11];
        scratch[15] = data[15];
        data = scratch;
        break;
      case Shader::SMP_row0:
        break;
      case Shader::SMP_row1:
        data = data + 4;
        break;
      case Shader::SMP_row2:
        data = data + 8;
        break;
      case Shader::SMP_row3:
        data = data + 12;
        break;
      case Shader::SMP_col0:
        scratch[0] = data[0];
        scratch[1] = data[4];
        scratch[2] = data[8];
        scratch[3] = data[12];
        data = scratch;
        break;
      case Shader::SMP_col1:
        scratch[0] = data[1];
        scratch[1] = data[5];
        scratch[2] = data[9];
        scratch[3] = data[13];
        data = scratch;
        break;
      case Shader::SMP_col2:
        scratch[0] = data[2];
        scratch[1] = data[6];
        scratch[2] = data[10];
        scratch[3] = data[14];
        data = scratch;
        break;
      case Shader::SMP_col3:
        scratch[0] = data[3];
        scratch[1] = data[7];
        scratch[2] = data[11];
        scratch[3] = data[15];
        data = scratch;
        break;
      case Shader::SMP_row3x1:
      case Shader::SMP_row3x2:
      case Shader::SMP_row3x3:
        data = data + 12;
        break;
      case Shader::SMP_upper3x3:
        scratch[0] = data[0];
        scratch[1] = data[1];
        scratch[2] = data[2];
        scratch[3] = data[4];
        scratch[4] = data[5];
        scratch[5] = data[6];
        scratch[6] = data[8];
        scratch[7] = data[9];
        scratch[8] = data[10];
        data = scratch;
        break;
      case Shader::SMP_transpose3x3:
        scratch[0] = data[0];
        scratch[1] = data[4];
        scratch[2] = data[8];
        scratch[3] = data[1];
        scratch[4] = data[5];
        scratch[5] = data[9];
        scratch[6] = data[2];
        scratch[7] = data[6];
        scratch[8] = data[10];
        data = scratch;
        break;
      case Shader::SMP_cell15:
        // Need to copy to scratch, otherwise D3D will read out of bounds.
        scratch[0] = data[15];
        scratch[1] = 0;
        scratch[2] = 0;
        scratch[3] = 0;
        data = scratch;
        break;
      case Shader::SMP_cell14:
        scratch[0] = data[14];
        scratch[1] = 0;
        scratch[2] = 0;
        scratch[3] = 0;
        data = scratch;
        break;
      case Shader::SMP_cell13:
        scratch[0] = data[13];
        scratch[1] = 0;
        scratch[2] = 0;
        scratch[3] = 0;
        data = scratch;
        break;
      }

      if (reg.vreg >= 0) {
        device->SetVertexShaderConstantF(reg.vreg, data, reg.count);
      }
      if (reg.freg >= 0) {
        device->SetPixelShaderConstantF(reg.freg, data, reg.count);
      }
    }
  }

  if (altered & Shader::SSD_frame) {
    //TODO: what should we set this to?
    if (_half_pixel_register >= 0) {
      const float data[4] = {0, 0, 0, 0};
      gsg->_d3d_device->SetVertexShaderConstantF(_half_pixel_register, data, 1);
    }
  }
}

/**
 * Changes the active transform and slider table, used for hardware skinning.
 */
void DXShaderContext9::
update_tables(GSG *gsg, const GeomVertexDataPipelineReader *data_reader) {
  int loc = _shader->_transform_table_loc;
  if (loc >= 0) {
    ConstantRegister &reg = _register_map[(size_t)loc];

    float *data;
    const TransformTable *table = data_reader->get_transform_table();
    if (!_shader->_transform_table_reduced) {
      // reg.count is the number of registers, which is 4 per matrix.  However,
      // due to optimization, the last row of the last matrix may be cut off.
      size_t num_matrices = (reg.count + 3) / 4;
      data = (float *)alloca(num_matrices * sizeof(LMatrix4f));
      LMatrix4f *matrices = (LMatrix4f *)data;

      size_t i = 0;
      if (table != nullptr) {
        bool transpose = (_shader->get_language() == Shader::SL_Cg);
        size_t num_transforms = std::min(num_matrices, table->get_num_transforms());
        for (; i < num_transforms; ++i) {
#ifdef STDFLOAT_DOUBLE
          LMatrix4 matrix;
          table->get_transform(i)->get_matrix(matrix);
          if (transpose) {
            matrix.transpose_in_place();
          }
          matrices[i] = LCAST(float, matrix);
#else
          table->get_transform(i)->get_matrix(matrices[i]);
          if (transpose) {
            matrices[i].transpose_in_place();
          }
#endif
        }
      }
      for (; i < num_matrices; ++i) {
        matrices[i] = LMatrix4f::ident_mat();
      }
    }
    else {
      // Reduced 3x4 matrix, used by shader generator
      size_t num_matrices = (reg.count + 2) / 3;
      data = (float *)alloca(num_matrices * sizeof(LVecBase4f) * 3);
      LVecBase4f *vectors = (LVecBase4f *)data;

      size_t i = 0;
      if (table != nullptr) {
        size_t num_transforms = std::min(num_matrices, table->get_num_transforms());
        for (; i < num_transforms; ++i) {
          LMatrix4f matrix;
#ifdef STDFLOAT_DOUBLE
          LMatrix4d matrixd;
          table->get_transform(i)->get_matrix(matrixd);
          matrix = LCAST(float, matrixd);
#else
          table->get_transform(i)->get_matrix(matrix);
#endif
          vectors[i * 3 + 0] = matrix.get_col(0);
          vectors[i * 3 + 1] = matrix.get_col(1);
          vectors[i * 3 + 2] = matrix.get_col(2);
        }
      }
      for (; i < num_matrices; ++i) {
        vectors[i * 3 + 0].set(1, 0, 0, 0);
        vectors[i * 3 + 1].set(0, 1, 0, 0);
        vectors[i * 3 + 2].set(0, 0, 1, 0);
      }
    }

    if (reg.vreg >= 0) {
      gsg->_d3d_device->SetVertexShaderConstantF(reg.vreg, data, reg.count);
    }
    if (reg.freg >= 0) {
      gsg->_d3d_device->SetPixelShaderConstantF(reg.freg, data, reg.count);
    }
  }

  loc = _shader->_slider_table_loc;
  if (loc >= 0) {
    ConstantRegister &reg = _register_map[(size_t)loc];

    LVecBase4f *sliders = (LVecBase4f *)alloca(reg.count * sizeof(LVecBase4f));
    memset(sliders, 0, reg.count * sizeof(LVecBase4f));

    const SliderTable *table = data_reader->get_slider_table();
    if (table != nullptr) {
      size_t num_sliders = std::min((size_t)reg.count, table->get_num_sliders());
      for (size_t i = 0; i < num_sliders; ++i) {
        sliders[i] = table->get_slider(i)->get_slider();
      }
    }

    if (reg.vreg >= 0) {
      gsg->_d3d_device->SetVertexShaderConstantF(reg.vreg, (float *)sliders, reg.count);
    }
    if (reg.freg >= 0) {
      gsg->_d3d_device->SetPixelShaderConstantF(reg.freg, (float *)sliders, reg.count);
    }
  }
}

/**
 * Disable all the texture bindings used by this shader.
 */
void DXShaderContext9::
disable_shader_texture_bindings(GSG *gsg) {
  for (Shader::ShaderTexSpec &spec : _shader->_tex_spec) {
    ConstantRegister &reg = _register_map[spec._id._location];
    if (reg.count == 0) {
      continue;
    }

    int texunit = reg.freg;
    if (texunit == -1) {
      texunit = reg.vreg;
      if (texunit == -1) {
        continue;
      }
    }

    HRESULT hr = gsg->_d3d_device->SetTexture(texunit, nullptr);
    if (FAILED(hr)) {
      dxgsg9_cat.error()
        << "SetTexture(" << texunit << ", NULL) failed "
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

  for (Shader::ShaderTexSpec &spec : _shader->_tex_spec) {
    if (spec._id._location < 0 || (size_t)spec._id._location >= _register_map.size()) {
      continue;
    }

    ConstantRegister &reg = _register_map[spec._id._location];
    if (reg.count == 0) {
      continue;
    }
    nassertd(reg.set == D3DXRS_SAMPLER) continue;

    int view = gsg->get_current_tex_view_offset();
    SamplerState sampler;

    PT(Texture) tex = gsg->fetch_specified_texture(spec, sampler, view);
    if (tex.is_null()) {
      continue;
    }

    if (spec._suffix != nullptr) {
      // The suffix feature is inefficient.  It is a temporary hack.
      tex = tex->load_related(spec._suffix);
    }

    Texture::TextureType tex_type = tex->get_texture_type();
    if (tex_type != spec._desired_type) {
      // Permit binding 2D texture to a 1D target, if it is one pixel high.
      if (tex_type != Texture::TT_2d_texture ||
          spec._desired_type != Texture::TT_1d_texture ||
          tex->get_y_size() != 1) {
        continue;
      }
    }

    int texunit = reg.freg;
    if (texunit == -1) {
      texunit = reg.vreg;
      if (texunit == -1) {
        continue;
      }
    }

    TextureContext *tc = tex->prepare_now(view, gsg->_prepared_objects, gsg);
    if (tc == nullptr) {
      continue;
    }

    gsg->apply_texture(texunit, tc, sampler);
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
