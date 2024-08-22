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

#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef HAVE_CG
#include <Cg/cgD3D9.h>
#endif

#define DEBUG_SHADER 0

TypeHandle DXShaderContext9::_type_handle;

/**
 * xyz
 */
DXShaderContext9::
DXShaderContext9(Shader *s, GSG *gsg) : ShaderContext(s) {
  _vertex_element_array = nullptr;
  _vertex_declaration = nullptr;

  _num_bound_streams = 0;

  _name = s->get_filename();

#ifdef HAVE_CG
  CGcontext context = DCAST(DXGraphicsStateGuardian9, gsg)->_cg_context;

  if (s->get_language() == Shader::SL_Cg) {
    // Ask the shader to compile itself for us and to give us the resulting Cg
    // program objects.
    if (!s->cg_compile_for(gsg->_shader_caps, context,
                           _cg_program, _cg_parameter_map)) {
      return;
    }

    // Load the program.
    DWORD assembly_flags = 0;
#if DEBUG_SHADER
    assembly_flags |= D3DXSHADER_DEBUG;
#endif

    HRESULT hr;
    bool success = true;
    hr = cgD3D9LoadProgram(_cg_program, FALSE, assembly_flags);
    if (FAILED(hr)) {
      dxgsg9_cat.error()
        << "cgD3D9LoadProgram failed " << D3DERRORSTRING(hr);

      CGerror error = cgGetError();
      if (error != CG_NO_ERROR) {
        dxgsg9_cat.error() << "  CG ERROR: " << cgGetErrorString(error) << "\n";
      }
      release_resources();
    }
  }
#endif

  _mat_part_cache = new LVecBase4f[s->cp_get_mat_cache_size()];
  _mat_scratch_space = new LVecBase4f[_shader->cp_get_mat_scratch_size()];
}

/**
 * xyz
 */
DXShaderContext9::
~DXShaderContext9() {
  release_resources();

  if (_vertex_declaration != nullptr) {
    _vertex_declaration->Release();
    _vertex_declaration = nullptr;
  }

  if (_vertex_element_array != nullptr) {
    delete _vertex_element_array;
    _vertex_element_array = nullptr;
  }

  delete[] _mat_part_cache;
  delete[] _mat_scratch_space;
}

/**
 * Should deallocate all system resources (such as vertex program handles or
 * Cg contexts).
 */
void DXShaderContext9::
release_resources() {
#ifdef HAVE_CG
  if (_cg_program) {
    cgDestroyProgram(_cg_program);
    _cg_program = 0;
    _cg_parameter_map.clear();
  }
#endif

  // I think we need to call SetStreamSource for _num_bound_streams --
  // basically the logic from disable_shader_vertex_arrays -- but to do that
  // we need to introduce logic like the GL code has to manage _last_gsg, so
  // we can get at the device.  Sigh.
}

/**
 * This function is to be called to enable a new shader.  It also initializes
 * all of the shader's input parameters.
 */
bool DXShaderContext9::
bind(GSG *gsg) {
  bool bind_state = false;

#ifdef HAVE_CG
  if (_cg_program) {
    // clear the last cached FVF to make sure the next SetFVF call goes
    // through

    gsg->_last_fvf = 0;

    // Pass in k-parameters and transform-parameters
    issue_parameters(gsg, Shader::SSD_general);

    HRESULT hr;

    // Bind the shaders.
    bind_state = true;
    hr = cgD3D9BindProgram(_cg_program);
    if (FAILED(hr)) {
      dxgsg9_cat.error() << "cgD3D9BindProgram failed " << D3DERRORSTRING(hr);

      CGerror error = cgGetError();
      if (error != CG_NO_ERROR) {
        dxgsg9_cat.error() << "  CG ERROR: " << cgGetErrorString(error) << "\n";
      }

      bind_state = false;
    }
  }
#endif

  return bind_state;
}

/**
 * This function disables a currently-bound shader.
 */
void DXShaderContext9::
unbind(GSG *gsg) {
#ifdef HAVE_CG
  if (_cg_program) {
    HRESULT hr;
    hr = cgD3D9UnbindProgram(_cg_program);
    if (FAILED(hr)) {
      dxgsg9_cat.error()
        << "cgD3D9UnbindProgram failed " << D3DERRORSTRING(hr);
    }
  }
#endif
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
#ifdef HAVE_CG
  if (_cg_program) {
    if (altered & _shader->_mat_deps) {
      if (altered & _shader->_mat_cache_deps) {
        gsg->update_shader_matrix_cache(_shader, _mat_part_cache, altered);
      }

      LMatrix4f scratch;

      for (Shader::ShaderMatSpec &spec : _shader->_mat_spec) {
        if ((altered & spec._dep) == 0) {
          continue;
        }

        CGparameter p = _cg_parameter_map[spec._id._seqno];
        if (p == nullptr) {
          continue;
        }

        const LVecBase4f *val = gsg->fetch_specified_value(spec, _mat_part_cache, _mat_scratch_space);
        if (val) {
          const float *data = (const float *)val + spec._offset;
          LVecBase4f v;
          LMatrix4f temp_matrix;
          LMatrix3f temp_matrix3;

          switch (spec._piece) {
          case Shader::SMP_mat4_whole:
            // TRANSPOSE REQUIRED
            temp_matrix.transpose_from(*(const LMatrix4f *)data);
            data = temp_matrix.get_data();
            break;

          case Shader::SMP_mat4_column:
            v.set(data[0], data[4], data[8], data[12]);
            data = v.get_data();
            break;

          case Shader::SMP_mat4_upper3x3:
            // TRANSPOSE REQUIRED
            temp_matrix3.set(data[0], data[4], data[8], data[1], data[5], data[9], data[2], data[6], data[10]);
            data = temp_matrix3.get_data();
            break;

          case Shader::SMP_mat4_transpose3x3:
            // NO TRANSPOSE REQUIRED
            temp_matrix3.set(data[0], data[1], data[2], data[4], data[5], data[6], data[8], data[9], data[10]);
            data = temp_matrix3.get_data();
            break;

          default:
            break;
          }

          HRESULT hr = cgD3D9SetUniform(p, data);
          if (FAILED(hr)) {
            std::string name = "unnamed";

            if (spec._arg[0]) {
              name = spec._arg[0]->get_basename();
            }

            dxgsg9_cat.error()
              << "NAME  " << name << "\n" << "MAT TYPE  " << spec._piece
              << " cgD3D9SetUniform failed " << D3DERRORSTRING(hr);

            CGerror error = cgGetError();
            if (error != CG_NO_ERROR) {
              dxgsg9_cat.error() << "  CG ERROR: " << cgGetErrorString(error) << "\n";
            }
          }
        }
      }
    }
  }
#endif
}

/**
 * Disable all the vertex arrays used by this shader.
 */
void DXShaderContext9::
disable_shader_vertex_arrays(GSG *gsg) {
  LPDIRECT3DDEVICE9 device = gsg->_screen->_d3d_device;

  for (int array_index = 0; array_index < _num_bound_streams; ++array_index) {
    device->SetStreamSource(array_index, nullptr, 0, 0);
  }
  _num_bound_streams = 0;
}

/**
 * Disables all vertex arrays used by the previous shader, then enables all
 * the vertex arrays needed by this shader.  Extracts the relevant vertex
 * array data from the gsg.  The current implementation is inefficient,
 * because it may unnecessarily disable arrays then immediately reenable them.
 * We may optimize this someday.
 */
bool DXShaderContext9::
update_shader_vertex_arrays(DXShaderContext9 *prev, GSG *gsg, bool force) {
  if (prev) prev->disable_shader_vertex_arrays(gsg);
#ifdef HAVE_CG
  if (!_cg_program) {
    return true;
  }

#ifdef SUPPORT_IMMEDIATE_MODE
/*
    if (gsg->_use_sender) {
      dxgsg9_cat.error() << "immediate mode shaders not implemented yet\n";
    } else
*/
#endif // SUPPORT_IMMEDIATE_MODE
  {
    int nvarying = _shader->_var_spec.size();
    LPDIRECT3DDEVICE9 device = gsg->_screen->_d3d_device;
    HRESULT hr;

    // Discard and recreate the VertexElementArray.  This thrashes pretty
    // bad....
    if (_vertex_element_array != nullptr) {
      delete _vertex_element_array;
    }
    _vertex_element_array = new VertexElementArray(nvarying + 2);
    VertexElementArray* vertex_element_array = _vertex_element_array;

    // Experimentally determined that DX doesn't like us crossing the streams!
    // It seems to be okay with out-of-order offsets in both source and
    // destination, but it wants all stream X entries grouped together, then
    // all stream Y entries, etc.  To accomplish this out outer loop processes
    // arrays ("streams"), and we repeatedly iterate the parameters to pull
    // out only those for a single stream.

    bool apply_white_color = false;

    int number_of_arrays = gsg->_data_reader->get_num_arrays();
    for (int array_index = 0; array_index < number_of_arrays; ++array_index) {
      const GeomVertexArrayDataHandle* array_reader =
        gsg->_data_reader->get_array_reader(array_index);
      if (array_reader == nullptr) {
        dxgsg9_cat.error() << "Unable to get reader for array " << array_index << "\n";
        continue;
      }

      for (int var_index = 0; var_index < nvarying; ++var_index) {
        CGparameter p = _cg_parameter_map[_shader->_var_spec[var_index]._id._seqno];
        if (p == nullptr) {
          dxgsg9_cat.info() <<
            "No parameter in map for parameter " << var_index <<
            " (probably optimized away)\n";
          continue;
        }

        InternalName *name = _shader->_var_spec[var_index]._name;

        // This is copied from the GL version of this function, and I've yet
        // to 100% convince myself that it works properly....
        int texslot = _shader->_var_spec[var_index]._append_uv;
        if (texslot >= 0 && texslot < gsg->_state_texture->get_num_on_stages()) {
          TextureStage *stage = gsg->_state_texture->get_on_stage(texslot);
          InternalName *texname = stage->get_texcoord_name();
          if (name == InternalName::get_texcoord()) {
            name = texname;
          } else if (texname != InternalName::get_texcoord()) {
            name = name->append(texname->get_basename());
          }
        }

        if (name == InternalName::get_color() && !gsg->_vertex_colors_enabled) {
          apply_white_color = true;
          continue;
        }

        const GeomVertexArrayDataHandle *param_array_reader;
        Geom::NumericType numeric_type;
        int num_values, start, stride;
        if (!gsg->_data_reader->get_array_info(name, param_array_reader,
                                               num_values, numeric_type,
                                               start, stride)) {
          // This is apparently not an error (actually I think it is, just not
          // a fatal one). The GL implementation fails silently in this case,
          // but the net result is that we end up not supplying input for a
          // shader parameter, which can cause Bad Things to happen so I'd
          // like to at least get a hint as to what's gone wrong.
          dxgsg9_cat.info() << "Geometry contains no data for shader parameter " << *name << "\n";
          if (name == InternalName::get_color()) {
            apply_white_color = true;
          }
          continue;
        }

        // If not associated with the array we're working on, move on.
        if (param_array_reader != array_reader) {
          continue;
        }

        const char *semantic = cgGetParameterSemantic(p);
        if (semantic == nullptr) {
          dxgsg9_cat.error() << "Unable to retrieve semantic for parameter " << var_index << "\n";
          continue;
        }

        if (strncmp(semantic, "POSITION", strlen("POSITION")) == 0) {
          if (numeric_type == Geom::NT_float32) {
            switch (num_values) {
            case 3:
              vertex_element_array->add_position_xyz_vertex_element(array_index, start);
              break;
            case 4:
              vertex_element_array->add_position_xyzw_vertex_element(array_index, start);
              break;
            default:
              dxgsg9_cat.error() << "VE ERROR: invalid number of vertex coordinate elements " << num_values << "\n";
              break;
            }
          } else {
            dxgsg9_cat.error() << "VE ERROR: invalid vertex type " << numeric_type << "\n";
          }
        } else if (strncmp(semantic, "TEXCOORD", strlen("TEXCOORD")) == 0) {
          int slot = atoi(semantic + strlen("TEXCOORD"));
          if (numeric_type == Geom::NT_float32) {
            switch (num_values) {
            case 1:
              vertex_element_array->add_u_vertex_element(array_index, start, slot);
              break;
            case 2:
              vertex_element_array->add_uv_vertex_element(array_index, start, slot);
              break;
            case 3:
              vertex_element_array->add_uvw_vertex_element(array_index, start, slot);
              break;
            case 4:
              vertex_element_array->add_xyzw_vertex_element(array_index, start, slot);
              break;
            default:
              dxgsg9_cat.error() << "VE ERROR: invalid number of vertex texture coordinate elements " << num_values <<  "\n";
              break;
            }
          } else {
            dxgsg9_cat.error() << "VE ERROR: invalid texture coordinate type " << numeric_type << "\n";
          }
        } else if (strncmp(semantic, "COLOR", strlen("COLOR")) == 0) {
          if (numeric_type == Geom::NT_packed_dcba ||
              numeric_type == Geom::NT_packed_dabc ||
              numeric_type == Geom::NT_uint8) {
            switch (num_values) {
            case 4:
              vertex_element_array->add_diffuse_color_vertex_element(array_index, start);
              break;
            default:
              dxgsg9_cat.error() << "VE ERROR: invalid color coordinates " << num_values << "\n";
              break;
            }
          } else {
            dxgsg9_cat.error() << "VE ERROR: invalid color type " << numeric_type << "\n";
          }
        } else if (strncmp(semantic, "NORMAL", strlen("NORMAL")) == 0) {
          if (numeric_type == Geom::NT_float32) {
            switch (num_values) {
            case 3:
              vertex_element_array->add_normal_vertex_element(array_index, start);
              break;
            default:
              dxgsg9_cat.error() << "VE ERROR: invalid number of normal coordinate elements " << num_values << "\n";
              break;
            }
          } else {
            dxgsg9_cat.error() << "VE ERROR: invalid normal type " << numeric_type << "\n";
          }
        } else if (strncmp(semantic, "BINORMAL", strlen("BINORMAL")) == 0) {
          if (numeric_type == Geom::NT_float32) {
            switch (num_values) {
            case 3:
              vertex_element_array->add_binormal_vertex_element(array_index, start);
              break;
            default:
              dxgsg9_cat.error() << "VE ERROR: invalid number of binormal coordinate elements " << num_values << "\n";
              break;
            }
          } else {
            dxgsg9_cat.error() << "VE ERROR: invalid binormal type " << numeric_type << "\n";
          }
        } else if (strncmp(semantic, "TANGENT", strlen("TANGENT")) == 0) {
          if (numeric_type == Geom::NT_float32) {
            switch (num_values) {
            case 3:
              vertex_element_array->add_tangent_vertex_element(array_index, start);
              break;
            default:
              dxgsg9_cat.error() << "VE ERROR: invalid number of tangent coordinate elements " << num_values << "\n";
              break;
            }
          } else {
            dxgsg9_cat.error() << "VE ERROR: invalid tangent type " << numeric_type << "\n";
          }
        } else {
          dxgsg9_cat.error() << "Unsupported semantic " << semantic << " for parameter " << var_index << "\n";
        }
      }

      // Get the vertex buffer for this array.
      DXVertexBufferContext9 *dvbc;
      if (!gsg->setup_array_data(dvbc, array_reader, force)) {
        dxgsg9_cat.error() << "Unable to setup vertex buffer for array " << array_index << "\n";
        continue;
      }

      // Bind this array as the data source for the corresponding stream.
      const GeomVertexArrayFormat *array_format = array_reader->get_array_format();
      hr = device->SetStreamSource(array_index, dvbc->_vbuffer, 0, array_format->get_stride());
      if (FAILED(hr)) {
        dxgsg9_cat.error() << "SetStreamSource failed" << D3DERRORSTRING(hr);
      }
    }

    _num_bound_streams = number_of_arrays;

    if (apply_white_color) {
      // The shader needs a vertex color, but vertex colors are disabled.
      // Bind a vertex buffer containing only one white colour.
      int array_index = number_of_arrays;
      LPDIRECT3DVERTEXBUFFER9 vbuffer = gsg->get_white_vbuffer();
      hr = device->SetStreamSource(array_index, vbuffer, 0, 0);
      if (FAILED(hr)) {
        dxgsg9_cat.error() << "SetStreamSource failed" << D3DERRORSTRING(hr);
      }
      vertex_element_array->add_diffuse_color_vertex_element(array_index, 0);
      ++_num_bound_streams;
    }

    if (_vertex_element_array != nullptr &&
        _vertex_element_array->add_end_vertex_element()) {
      if (dxgsg9_cat.is_debug()) {
        // Note that the currently generated vertex declaration works but
        // never validates.  My theory is that this is due to the shader
        // programs always using float4 whereas the vertex declaration
        // correctly sets the number of inputs (float2, float3, etc.).
        if (cgD3D9ValidateVertexDeclaration(_cg_program,
                                            _vertex_element_array->_vertex_element_array) == CG_TRUE) {
          dxgsg9_cat.debug() << "cgD3D9ValidateVertexDeclaration succeeded\n";
        } else {
          dxgsg9_cat.debug() << "cgD3D9ValidateVertexDeclaration failed\n";
        }
      }

      // Discard the old VertexDeclaration.  This thrashes pretty bad....
      if (_vertex_declaration != nullptr) {
        _vertex_declaration->Release();
        _vertex_declaration = nullptr;
      }

      hr = device->CreateVertexDeclaration(_vertex_element_array->_vertex_element_array,
                                           &_vertex_declaration);
      if (FAILED(hr)) {
        dxgsg9_cat.error() << "CreateVertexDeclaration failed" << D3DERRORSTRING(hr);
      } else {
        hr = device->SetVertexDeclaration(_vertex_declaration);
        if (FAILED(hr)) {
          dxgsg9_cat.error() << "SetVertexDeclaration failed" << D3DERRORSTRING(hr);
        }
      }
    } else {
      dxgsg9_cat.error() << "VertexElementArray creation failed\n";
    }
  }
#endif // HAVE_CG

  return true;
}

/**
 * Disable all the texture bindings used by this shader.
 */
void DXShaderContext9::
disable_shader_texture_bindings(GSG *gsg) {
#ifdef HAVE_CG
  if (_cg_program) {
    for (size_t i = 0; i < _shader->_tex_spec.size(); ++i) {
      CGparameter p = _cg_parameter_map[_shader->_tex_spec[i]._id._seqno];
      if (p == nullptr) {
        continue;
      }
      int texunit = cgGetParameterResourceIndex(p);

      HRESULT hr;

      hr = gsg->_d3d_device->SetTexture(texunit, nullptr);
      if (FAILED(hr)) {
        dxgsg9_cat.error()
          << "SetTexture(" << texunit << ", NULL) failed "
          << D3DERRORSTRING(hr);
      }
    }
  }
#endif
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

#ifdef HAVE_CG
  if (_cg_program) {
    for (size_t i = 0; i < _shader->_tex_spec.size(); ++i) {
      Shader::ShaderTexSpec &spec = _shader->_tex_spec[i];
      CGparameter p = _cg_parameter_map[spec._id._seqno];
      if (p == nullptr) {
        continue;
      }

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

      if (tex->get_texture_type() != spec._desired_type) {
        continue;
      }

      TextureContext *tc = tex->prepare_now(gsg->_prepared_objects, gsg);
      if (tc == nullptr) {
        continue;
      }

      int texunit = cgGetParameterResourceIndex(p);
      gsg->apply_texture(texunit, tc, view, sampler);
    }
  }
#endif
}

// DEBUG CODE TO TEST ASM CODE GENERATED BY Cg
void assemble_shader_test(char *file_path) {
  int flags;
  D3DXMACRO *defines;
  LPD3DXINCLUDE include;
  LPD3DXBUFFER shader;
  LPD3DXBUFFER error_messages;

  flags = 0;
  defines = 0;
  include = 0;
  shader = 0;
  error_messages = 0;

  D3DXAssembleShaderFromFile(file_path, defines, include, flags, &shader, &error_messages);
  if (error_messages) {
    char *error_message;

    error_message = (char *)error_messages->GetBufferPointer();
    if (error_message) {
      dxgsg9_cat.error() << error_message;
    }

    error_messages->Release();
  }
}
