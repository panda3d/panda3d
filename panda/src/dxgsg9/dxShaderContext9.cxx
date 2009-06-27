// Filename: dxShaderContext9.cxx
// Created by: jyelon (01Sep05), conversion aignacio (Jan-Mar06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "dxGraphicsStateGuardian9.h"
#include "dxShaderContext9.h"

#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_CG
#include "Cg/cgD3D9.h"
#endif

#define DEBUG_SHADER 0

TypeHandle CLP(ShaderContext)::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: DXShaderContext9::Constructor
//       Access: Public
//  Description: xyz
////////////////////////////////////////////////////////////////////
CLP(ShaderContext)::
CLP(ShaderContext)(Shader *s, GSG *gsg) : ShaderContext(s) {

  _vertex_size = 0;
  _vertex_element_array = 0;
  _name = s->get_filename ( );

#ifdef HAVE_CG
  if (s->get_language() == Shader::SL_Cg) {
    
    // Ask the shader to compile itself for us and 
    // to give us the resulting Cg program objects.

    if (!s->cg_compile_for(gsg->_shader_caps,
                           _cg_context,
                           _cg_vprogram,
                           _cg_fprogram, 
                           _cg_gprogram,        // CG2 CHANGE
                           _cg_parameter_map)) {
      return;
    }
        
    // Load the program.

    BOOL paramater_shadowing;
    DWORD assembly_flags;
    
    paramater_shadowing = FALSE;
    assembly_flags = 0;
    
#if DEBUG_SHADER
    assembly_flags |= D3DXSHADER_DEBUG;
#endif

    HRESULT hr;
    bool success = true;
    hr = cgD3D9LoadProgram(_cg_vprogram, paramater_shadowing, assembly_flags);
    if (FAILED (hr)) {
      dxgsg9_cat.error()
        << "vertex shader cgD3D9LoadProgram failed "
        << D3DERRORSTRING(hr);
      
      CGerror error = cgGetError();
      if (error != CG_NO_ERROR) {
        dxgsg9_cat.error() << "  CG ERROR: " << cgGetErrorString(error) << "\n";
      }
      success = false;
    }
    
    hr = cgD3D9LoadProgram(_cg_fprogram, paramater_shadowing, assembly_flags);
    if (FAILED (hr)) {
      dxgsg9_cat.error()
        << "pixel shader cgD3D9LoadProgram failed "
        << D3DERRORSTRING(hr);
      
      CGerror error = cgGetError();
      if (error != CG_NO_ERROR) {
        dxgsg9_cat.error() << "  CG ERROR: " << cgGetErrorString(error) << "\n";
      }
      success = false;
    }    

    // BEGIN CG2 CHANGE
    if (_cg_gprogram != 0)
    {
        hr = cgD3D9LoadProgram(_cg_gprogram, paramater_shadowing, assembly_flags);
        if (FAILED (hr)) {
          dxgsg9_cat.error()
            << "geometry shader cgD3D9LoadProgram failed "
            << D3DERRORSTRING(hr);

          CGerror error = cgGetError();
          if (error != CG_NO_ERROR) {
            dxgsg9_cat.error() << "  CG ERROR: " << cgGetErrorString(error) << "\n";
          }
          success = false;
        }
    }
    // END CG2 CHANGE

    if (!success) {
      release_resources();
    }
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: DXShaderContext9::Destructor
//       Access: Public
//  Description: xyz
////////////////////////////////////////////////////////////////////
CLP(ShaderContext)::
~CLP(ShaderContext)() {
  release_resources();

  if (_vertex_element_array) {
    delete _vertex_element_array;
    _vertex_element_array = 0;
  }
}

// int save_file (int size, void *data, char *file_path)
// {
//   int state;
//   int file_handle;
// 
//   state = false;
//   file_handle = _open (file_path, _O_CREAT | _O_RDWR | _O_TRUNC, _S_IREAD | _S_IWRITE);
//   if (file_handle != -1) {
//     if (_write (file_handle, data, size) == size) {
//       state = true;
//     }
//     _close (file_handle);
//   }
// 
//   return state;
// }
// 
//   if (dxgsg9_cat.is_debug()) {
//     // DEBUG: output the generated program
//     const char *vertex_program;
//     const char *pixel_program;
// 
//     vertex_program = cgGetProgramString (_cg_program[0], CG_COMPILED_PROGRAM);
//     pixel_program = cgGetProgramString (_cg_program[1], CG_COMPILED_PROGRAM);
// 
//     dxgsg9_cat.debug() << vertex_program << "\n";
//     dxgsg9_cat.debug() << pixel_program << "\n";
// 
//     // save the generated program to a file
//     int size;
//     char file_path [512];
// 
//     char drive[_MAX_DRIVE];
//     char dir[_MAX_DIR];
//     char fname[_MAX_FNAME];
//     char ext[_MAX_EXT];
// 
//     _splitpath (_name.c_str ( ), drive, dir, fname, ext);
// 
//     size = strlen (vertex_program);
//     sprintf (file_path, "%s.vasm", fname);
//     save_file (size, (void *) vertex_program, file_path);
// 
//     size = strlen (pixel_program);
//     sprintf (file_path, "%s.pasm", fname);
//     save_file (size, (void *) pixel_program, file_path);
//   }

////////////////////////////////////////////////////////////////////
//     Function: DXShaderContext9::release_resources
//       Access: Public
//  Description: Should deallocate all system resources (such as
//               vertex program handles or Cg contexts).
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
release_resources() {
#ifdef HAVE_CG
  if (_cg_context) {
    cgDestroyContext(_cg_context);
    _cg_context = 0;
    _cg_vprogram = 0;
    _cg_fprogram = 0;
    _cg_gprogram = 0;   // CG2 CHANGE
    _cg_parameter_map.clear();
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: DXShaderContext9::bind
//       Access: Public
//  Description: This function is to be called to enable a new
//               shader.  It also initializes all of the shader's
//               input parameters.
////////////////////////////////////////////////////////////////////
bool CLP(ShaderContext)::
bind(GSG *gsg) {

  bool bind_state;

  bind_state = false;
#ifdef HAVE_CG
  if (_cg_context) {
    // clear the last cached FVF to make sure the next SetFVF call goes through
                                                           
    gsg -> _last_fvf = 0;

    // Pass in k-parameters and transform-parameters
    issue_parameters(gsg, Shader::SSD_general);
    
    HRESULT hr;
    
    // Bind the shaders.
    bind_state = true;
    hr = cgD3D9BindProgram(_cg_vprogram);
    if (FAILED (hr)) {
      dxgsg9_cat.error() << "cgD3D9BindProgram vertex shader failed " << D3DERRORSTRING(hr);
      
      CGerror error = cgGetError();
      if (error != CG_NO_ERROR) {
        dxgsg9_cat.error() << "  CG ERROR: " << cgGetErrorString(error) << "\n";
      }
      
      bind_state = false;
    }
    hr = cgD3D9BindProgram(_cg_fprogram);
    if (FAILED (hr)) {
      dxgsg9_cat.error() << "cgD3D9BindProgram pixel shader failed " << D3DERRORSTRING(hr);
      
      CGerror error = cgGetError();
      if (error != CG_NO_ERROR) {
        dxgsg9_cat.error() << "  CG ERROR: " << cgGetErrorString(error) << "\n";
      }
      
      bind_state = false;
    }

    // BEGIN CG2 CHANGE
    if (_cg_gprogram != 0)
    {
        hr = cgD3D9BindProgram(_cg_gprogram);
        if (FAILED (hr)) {
          dxgsg9_cat.error() << "cgD3D9BindProgram geometry shader failed " << D3DERRORSTRING(hr);

          CGerror error = cgGetError();
          if (error != CG_NO_ERROR) {
            dxgsg9_cat.error() << "  CG ERROR: " << cgGetErrorString(error) << "\n";
          }

          bind_state = false;
        }
    }
    // END CG2 CHANGE
  }
#endif

  return bind_state;
}

////////////////////////////////////////////////////////////////////
//     Function: DXShaderContext9::unbind
//       Access: Public
//  Description: This function disables a currently-bound shader.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
unbind(GSG *gsg) {

#ifdef HAVE_CG
  if (_cg_context) {
    HRESULT hr;

    hr = gsg -> _d3d_device -> SetVertexShader (NULL);
    if (FAILED (hr)) {
      dxgsg9_cat.error()
        << "SetVertexShader (NULL) failed " << D3DERRORSTRING(hr);
    }
    hr = gsg -> _d3d_device -> SetPixelShader (NULL);
    if (FAILED (hr)) {
      dxgsg9_cat.error()
        << "SetPixelShader (NULL) failed " << D3DERRORSTRING(hr);
    }
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: DXShaderContext9::issue_parameters
//       Access: Public
//  Description: This function gets called whenever the RenderState
//               or TransformState has changed, but the Shader
//               itself has not changed.  It loads new values into the
//               shader's parameters.
//
//               If "altered" is false, that means you promise that
//               the parameters for this shader context have already
//               been issued once, and that since the last time the
//               parameters were issued, no part of the render
//               state has changed except the external and internal
//               transforms.
////////////////////////////////////////////////////////////////////

#if DEBUG_SHADER
float *global_data = 0;
ShaderContext::ShaderMatSpec *global_shader_mat_spec = 0;
InternalName *global_internal_name_0 = 0;
InternalName *global_internal_name_1 = 0;
#endif

void CLP(ShaderContext)::
issue_parameters(GSG *gsg, int altered)
{
#ifdef HAVE_CG
  if (_cg_context) {
    for (int i=0; i<(int)_shader->_mat_spec.size(); i++) {
      if (altered & (_shader->_mat_spec[i]._dep[0] | _shader->_mat_spec[i]._dep[1])) {
        CGparameter p = _cg_parameter_map[_shader->_mat_spec[i]._id._seqno];
        if (p == NULL) {
          continue;
        }        
        const LMatrix4f *val = gsg->fetch_specified_value(_shader->_mat_spec[i], altered);
        if (val) {
          HRESULT hr;
          float v [4];
          LMatrix4f temp_matrix;

          hr = D3D_OK;

          const float *data;

          data = val -> get_data ( );

          #if DEBUG_SHADER
          // DEBUG
          global_data = (float *) data;
          global_shader_mat_spec = &_shader->_mat_spec[i];
          global_internal_name_0 = global_shader_mat_spec -> _arg [0];
          global_internal_name_1 = global_shader_mat_spec -> _arg [1];
          #endif

          switch (_shader->_mat_spec[i]._piece) {
          case Shader::SMP_whole:
            // TRANSPOSE REQUIRED
            temp_matrix.transpose_from (*val);
            data = temp_matrix.get_data();

            hr = cgD3D9SetUniform (p, data);
            break;

          case Shader::SMP_transpose:
            // NO TRANSPOSE REQUIRED
            hr = cgD3D9SetUniform (p, data);
            break;

          case Shader::SMP_row0:
            hr = cgD3D9SetUniform (p, data + 0);
            break;
          case Shader::SMP_row1:
            hr = cgD3D9SetUniform (p, data + 4);
            break;
          case Shader::SMP_row2:
            hr = cgD3D9SetUniform (p, data + 8);
            break;
          case Shader::SMP_row3x1:
          case Shader::SMP_row3x2:
          case Shader::SMP_row3x3:
          case Shader::SMP_row3:
            hr = cgD3D9SetUniform (p, data + 12);
            break;

          case Shader::SMP_col0:
            v[0] = data[0]; v[1] = data[4]; v[2] = data[8]; v[3] = data[12];
            hr = cgD3D9SetUniform (p, v);
            break;
          case Shader::SMP_col1:
            v[0] = data[1]; v[1] = data[5]; v[2] = data[9]; v[3] = data[13];
            hr = cgD3D9SetUniform (p, v);
            break;
          case Shader::SMP_col2:
            v[0] = data[2]; v[1] = data[6]; v[2] = data[10]; v[3] = data[14];
            hr = cgD3D9SetUniform (p, v);
            break;
          case Shader::SMP_col3:
            v[0] = data[3]; v[1] = data[7]; v[2] = data[11]; v[3] = data[15];
            hr = cgD3D9SetUniform (p, v);
            break;

          default:
            dxgsg9_cat.error()
              << "issue_parameters ( ) SMP parameter type not implemented " << _shader->_mat_spec[i]._piece << "\n";
            break;
          }

          if (FAILED (hr)) {

            string name = "unnamed";

            if (_shader->_mat_spec[i]._arg [0]) {
              name = _shader->_mat_spec[i]._arg [0] -> get_basename ( );
            }

            dxgsg9_cat.error()
              << "NAME  " << name << "\n"
              << "MAT TYPE  "
              << _shader->_mat_spec[i]._piece
              << " cgD3D9SetUniform failed "
              << D3DERRORSTRING(hr);

            CGerror error = cgGetError ();
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

////////////////////////////////////////////////////////////////////
//     Function: DXShaderContext9::disable_shader_vertex_arrays
//       Access: Public
//  Description: Disable all the vertex arrays used by this shader.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
disable_shader_vertex_arrays(GSG *gsg)
{
#ifdef HAVE_CG
  if (_cg_context) {
    // DO NOTHING, CURRENTLY USING ONLY ONE STREAM SOURCE
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: DXShaderContext9::update_shader_vertex_arrays
//       Access: Public
//  Description: Disables all vertex arrays used by the previous
//               shader, then enables all the vertex arrays needed
//               by this shader.  Extracts the relevant vertex array
//               data from the gsg.
//               The current implementation is inefficient, because
//               it may unnecessarily disable arrays then immediately
//               reenable them.  We may optimize this someday.
////////////////////////////////////////////////////////////////////

// DEBUG
#if DEBUG_SHADER
VertexElementArray *global_vertex_element_array = 0;
#endif

void CLP(ShaderContext)::
update_shader_vertex_arrays(CLP(ShaderContext) *prev, GSG *gsg)
{
  if (prev) prev->disable_shader_vertex_arrays(gsg);
#ifdef HAVE_CG
  if (_cg_context) {

  #ifdef SUPPORT_IMMEDIATE_MODE
/*
    if (gsg->_use_sender) {
      dxgsg9_cat.error() << "immediate mode shaders not implemented yet\n";
    } else
*/
  #endif // SUPPORT_IMMEDIATE_MODE
    {
      if (_vertex_element_array == 0) {
        bool error;
        const GeomVertexArrayDataHandle *array_reader;
        Geom::NumericType numeric_type;
        int start, stride, num_values;
        int nvarying = _shader->_var_spec.size();

        int stream_index;
        VertexElementArray *vertex_element_array;

        error = false;
        // SHADER ISSUE: STREAM INDEX ALWAYS 0 FOR VERTEX BUFFER?
        stream_index = 0;
        vertex_element_array = new VertexElementArray (nvarying + 2);

        #if DEBUG_SHADER
        // DEBUG
        global_vertex_element_array = vertex_element_array;
        #endif

        for (int i=0; i<nvarying; i++) {
          CGparameter p = _cg_parameter_map[_shader->_var_spec[i]._id._seqno];
          if (p == NULL) {
            continue;
          }        
          InternalName *name = _shader->_var_spec[i]._name;
          int texslot = _shader->_var_spec[i]._append_uv;
          if (texslot >= 0 && texslot < gsg->_state_texture->get_num_on_stages()) {
            TextureStage *stage = gsg->_state_texture->get_on_stage(texslot);
            InternalName *texname = stage->get_texcoord_name();
            if (name == InternalName::get_texcoord()) {
              name = texname;
            } else if (texname != InternalName::get_texcoord()) {
              name = name->append(texname->get_basename());
            }
          }
          if (gsg->_data_reader->get_array_info(name, array_reader, num_values, numeric_type, start, stride)) {

            if (false) {

            } else if (name -> get_top ( ) == InternalName::get_vertex ( )) {

              if (numeric_type == Geom::NT_float32) {
                switch (num_values) {
                  case 3:
                    vertex_element_array -> add_position_xyz_vertex_element (stream_index);
                    break;
                  case 4:
                    vertex_element_array -> add_position_xyzw_vertex_element (stream_index);
                    break;
                  default:
                    dxgsg9_cat.error ( ) << "VE ERROR: invalid number of vertex coordinate elements " << num_values << "\n";
                    break;
                }
              } else {
                dxgsg9_cat.error ( ) << "VE ERROR: invalid vertex type " << numeric_type << "\n";
              }

            } else if (name -> get_top ( ) == InternalName::get_texcoord ( )) {

              if (numeric_type == Geom::NT_float32) {
                switch (num_values)
                {
                  case 1:
                    vertex_element_array -> add_u_vertex_element (stream_index);
                    break;
                  case 2:
                    vertex_element_array -> add_uv_vertex_element (stream_index);
                    break;
                  case 3:
                    vertex_element_array -> add_uvw_vertex_element (stream_index);
                    break;
                  default:
                    dxgsg9_cat.error ( ) << "VE ERROR: invalid number of vertex texture coordinate elements " << num_values <<  "\n";
                    break;
                }
              } else {
                dxgsg9_cat.error ( ) << "VE ERROR: invalid texture coordinate type " << numeric_type << "\n";
              }

            } else if (name -> get_top ( ) == InternalName::get_normal ( )) {

              if (numeric_type == Geom::NT_float32) {
                switch (num_values)
                {
                  case 3:
                    vertex_element_array -> add_normal_vertex_element (stream_index);
                    break;
                  default:
                    dxgsg9_cat.error ( ) << "VE ERROR: invalid number of normal coordinate elements " << num_values << "\n";
                    break;
                }
              } else {
                dxgsg9_cat.error ( ) << "VE ERROR: invalid normal type " << numeric_type << "\n";
              }

            } else if (name -> get_top ( ) == InternalName::get_binormal ( )) {

              if (numeric_type == Geom::NT_float32) {
                switch (num_values)
                {
                  case 3:
                    vertex_element_array -> add_binormal_vertex_element (stream_index);
                    break;
                  default:
                    dxgsg9_cat.error ( ) << "VE ERROR: invalid number of binormal coordinate elements " << num_values << "\n";
                    break;
                }
              } else {
                dxgsg9_cat.error ( ) << "VE ERROR: invalid binormal type " << numeric_type << "\n";
              }

            } else if (name -> get_top ( ) == InternalName::get_tangent ( )) {

              if (numeric_type == Geom::NT_float32) {
                switch (num_values)
                {
                  case 3:
                    vertex_element_array -> add_tangent_vertex_element (stream_index);
                    break;
                  default:
                    dxgsg9_cat.error ( ) << "VE ERROR: invalid number of tangent coordinate elements " << num_values << "\n";
                    break;
                }
              } else {
                dxgsg9_cat.error ( ) << "VE ERROR: invalid tangent type " << numeric_type << "\n";
              }

            } else if (name -> get_top ( ) == InternalName::get_color ( )) {

              if (numeric_type == Geom::NT_packed_dcba ||
                  numeric_type == Geom::NT_packed_dabc ||
                  numeric_type == Geom::NT_uint8) {
                switch (num_values)
                {
                  case 4:
                    vertex_element_array -> add_diffuse_color_vertex_element (stream_index);
                    break;
                  default:
                    dxgsg9_cat.error ( ) << "VE ERROR: invalid color coordinates " << num_values << "\n";
                    break;
                }
              } else {
                dxgsg9_cat.error ( ) << "VE ERROR: invalid color type " << numeric_type << "\n";
              }

            } else {
              dxgsg9_cat.error ( ) << "VE ERROR: unsupported vertex element " << name -> get_name ( ) << "\n";
            }

          } else {
            dxgsg9_cat.error ( )
              << "get_array_info ( ) failed for shader "
              << _name
              << "\n"
              << "  vertex element name = "
              << name -> get_name ( )
              << "\n";
            error = true;
          }
        }

        if (error) {
          delete vertex_element_array;
        }
        else {
          int state;

          state = vertex_element_array -> add_end_vertex_element ( );
          if (state) {
            if (_cg_context) {
              if (cgD3D9ValidateVertexDeclaration (_cg_vprogram,
                    vertex_element_array -> vertex_element_array) == CG_TRUE) {
                dxgsg9_cat.debug() << "|||||cgD3D9ValidateVertexDeclaration succeeded\n";
              }
              else {
              }
            }
            else {

            }

            _vertex_size = vertex_element_array -> offset;
            _vertex_element_array = vertex_element_array;
          }
          else {
            dxgsg9_cat.error ( ) << "VertexElementArray creation failed\n";
            delete vertex_element_array;
          }
        }
      }
    }
  }
#endif // HAVE_CG
}

////////////////////////////////////////////////////////////////////
//     Function: DXShaderContext9::disable_shader_texture_bindings
//       Access: Public
//  Description: Disable all the texture bindings used by this shader.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
disable_shader_texture_bindings(GSG *gsg)
{
#ifdef HAVE_CG
  if (_cg_context) {
    for (int i=0; i<(int)_shader->_tex_spec.size(); i++) {
      CGparameter p = _cg_parameter_map[_shader->_tex_spec[i]._id._seqno];
      if (p == NULL) {
        continue;
      }        
      int texunit = cgGetParameterResourceIndex(p);

      HRESULT hr;

      hr = gsg -> _d3d_device -> SetTexture (texunit, NULL);
      if (FAILED (hr)) {
        dxgsg9_cat.error()
          << "SetTexture ("
          << texunit
          << ", NULL) failed "
          << D3DERRORSTRING(hr);
      }
    }
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: DXShaderContext9::update_shader_texture_bindings
//       Access: Public
//  Description: Disables all texture bindings used by the previous
//               shader, then enables all the texture bindings needed
//               by this shader.  Extracts the relevant vertex array
//               data from the gsg.
//               The current implementation is inefficient, because
//               it may unnecessarily disable textures then immediately
//               reenable them.  We may optimize this someday.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
update_shader_texture_bindings(CLP(ShaderContext) *prev, GSG *gsg)
{
  if (prev) prev->disable_shader_texture_bindings(gsg);

#ifdef HAVE_CG
  if (_cg_context) {

    for (int i=0; i<(int)_shader->_tex_spec.size(); i++) {
      CGparameter p = _cg_parameter_map[_shader->_tex_spec[i]._id._seqno];
      if (p == NULL) {
        continue;
      }        
      Texture *tex = 0;
      InternalName *id = _shader->_tex_spec[i]._name;
      if (id != 0) {
        const ShaderInput *input = gsg->_target_shader->get_shader_input(id);
        tex = input->get_texture();
      } else {
        if (_shader->_tex_spec[i]._stage >= gsg->_target_texture->get_num_on_stages()) {
          continue;
        }
        TextureStage *stage = gsg->_target_texture->get_on_stage(_shader->_tex_spec[i]._stage);
        tex = gsg->_target_texture->get_on_texture(stage);
      }
      if (_shader->_tex_spec[i]._suffix != 0) {
        // The suffix feature is inefficient. It is a temporary hack.
        if (tex == 0) {
          continue;
        }
        tex = tex->load_related(_shader->_tex_spec[i]._suffix);
      }
      if ((tex == 0) || (tex->get_texture_type() != _shader->_tex_spec[i]._desired_type)) {
        continue;
      }
      TextureContext *tc = tex->prepare_now(gsg->_prepared_objects, gsg);
      //      TextureContext *tc = tex->prepare_now(gsg->get_prepared_objects(), gsg);
      if (tc == (TextureContext*)NULL) {
        continue;
      }
      
      int texunit = cgGetParameterResourceIndex(p);
      
      gsg->apply_texture(texunit, tc);
    }
  }
#endif
}

// DEBUG CODE TO TEST ASM CODE GENERATED BY Cg
void assemble_shader_test(char *file_path)
{
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

  D3DXAssembleShaderFromFile (file_path, defines, include, flags, &shader, &error_messages);
  if (error_messages)
  {
    char *error_message;

    error_message = (char *) (error_messages -> GetBufferPointer ( ));
    if (error_message)
    {
      dxgsg9_cat.error() << error_message;
    }

    error_messages -> Release ( );
  }
}
