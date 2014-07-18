// Filename: glCgShaderContext_src.cxx
// Created by: jyelon (01Sep05)
// Updated by: fperazzi, PandaSE (29Apr10) (updated CLP with note that some
//   parameter types only supported under Cg)
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

#if defined(HAVE_CG) && !defined(OPENGLES)

#include "Cg/cgGL.h"

#include "pStatTimer.h"

TypeHandle CLP(CgShaderContext)::_type_handle;

#ifndef NDEBUG
#define cg_report_errors() { \
  CGerror err = cgGetError(); \
  if (err != CG_NO_ERROR) { \
    GLCAT.error() << __FILE__ ", line " << __LINE__ << ": " << cgGetErrorString(err) << "\n"; \
  } }
#else
#define cg_report_errors()
#endif

////////////////////////////////////////////////////////////////////
//     Function: GLCgShaderContext::Constructor
//       Access: Public
//  Description: xyz
////////////////////////////////////////////////////////////////////
CLP(CgShaderContext)::
CLP(CgShaderContext)(CLP(GraphicsStateGuardian) *glgsg, Shader *s) : ShaderContext(s) {
  _glgsg = glgsg;
  _cg_context = 0;
  _cg_vprogram = 0;
  _cg_fprogram = 0;
  _cg_gprogram = 0;
  _cg_vprofile = CG_PROFILE_UNKNOWN;
  _cg_fprofile = CG_PROFILE_UNKNOWN;
  _cg_gprofile = CG_PROFILE_UNKNOWN;

  nassertv(s->get_language() == Shader::SL_Cg);

  // Ask the shader to compile itself for us and 
  // to give us the resulting Cg program objects.
  if (!s->cg_compile_for(_glgsg->_shader_caps,
                         _cg_context,
                         _cg_vprogram,
                         _cg_fprogram,
                         _cg_gprogram,
                         _cg_parameter_map)) {
    return;
  }

  // Load the program.
  if (_cg_vprogram != 0) {
    _cg_vprofile = cgGetProgramProfile(_cg_vprogram);
    cgGLLoadProgram(_cg_vprogram);
    CGerror verror = cgGetError();
    if (verror != CG_NO_ERROR) {
      const char *str = cgGetErrorString(verror);
      GLCAT.error()
        << "Could not load Cg vertex program: " << s->get_filename(Shader::ST_vertex)
        << " (" << cgGetProfileString(_cg_vprofile) << " " << str << ")\n";
      release_resources();
    }
  }

  if (_cg_fprogram != 0) {
    _cg_fprofile = cgGetProgramProfile(_cg_fprogram);
    cgGLLoadProgram(_cg_fprogram);
    CGerror ferror = cgGetError();
    if (ferror != CG_NO_ERROR) {
      const char *str = cgGetErrorString(ferror);
      GLCAT.error()
        << "Could not load Cg fragment program: " << s->get_filename(Shader::ST_fragment)
        << " (" << cgGetProfileString(_cg_fprofile) << " " << str << ")\n";
      release_resources();
    }
  }

  if (_cg_gprogram != 0) {
    _cg_gprofile = cgGetProgramProfile(_cg_gprogram);
    cgGLLoadProgram(_cg_gprogram);
    CGerror gerror = cgGetError();
    if (gerror != CG_NO_ERROR) {
      const char *str = cgGetErrorString(gerror);
      GLCAT.error()
        << "Could not load Cg geometry program: " << s->get_filename(Shader::ST_geometry)
        << " (" << cgGetProfileString(_cg_gprofile) << " " << str << ")\n";
      release_resources();
    }
  }

  _glgsg->report_my_gl_errors();
}

////////////////////////////////////////////////////////////////////
//     Function: GLCgShaderContext::Destructor
//       Access: Public
//  Description: xyz
////////////////////////////////////////////////////////////////////
CLP(CgShaderContext)::
~CLP(CgShaderContext)() {
  release_resources();
}

////////////////////////////////////////////////////////////////////
//     Function: GLCgShaderContext::release_resources
//       Access: Public
//  Description: Should deallocate all system resources (such as
//               vertex program handles or Cg contexts).
////////////////////////////////////////////////////////////////////
void CLP(CgShaderContext)::
release_resources() {
  if (_cg_context) {
    cgDestroyContext(_cg_context);
    _cg_context  = 0;
    // Do *NOT* destroy the programs here! It causes problems.
//  if (_cg_vprogram != 0) cgDestroyProgram(_cg_vprogram);
//  if (_cg_fprogram != 0) cgDestroyProgram(_cg_fprogram);
//  if (_cg_gprogram != 0) cgDestroyProgram(_cg_gprogram);
    _cg_vprogram = 0;
    _cg_fprogram = 0;
    _cg_gprogram = 0;
    _cg_parameter_map.clear();
  }
  if (_glgsg) {
    _glgsg->report_my_gl_errors();
  } else if (glGetError() != GL_NO_ERROR) {
    GLCAT.error() << "GL error in ShaderContext destructor\n";
  }

  if (!_glgsg) {
    return;
  }
  _glgsg->report_my_gl_errors();
}

////////////////////////////////////////////////////////////////////
//     Function: GLCgShaderContext::bind
//       Access: Public
//  Description: This function is to be called to enable a new
//               shader.  It also initializes all of the shader's
//               input parameters.
////////////////////////////////////////////////////////////////////
void CLP(CgShaderContext)::
bind(bool reissue_parameters) {
  if (reissue_parameters) {
    // Pass in k-parameters and transform-parameters
    issue_parameters(Shader::SSD_general);
  }

  if (_cg_context != 0) {
    // Bind the shaders.
    if (_cg_vprogram != 0) {
      cgGLEnableProfile(_cg_vprofile);
      cgGLBindProgram(_cg_vprogram);
    }
    if (_cg_fprogram != 0) {
      cgGLEnableProfile(_cg_fprofile);
      cgGLBindProgram(_cg_fprogram);
    }
    if (_cg_gprogram != 0) {
      cgGLEnableProfile(_cg_gprofile);
      cgGLBindProgram(_cg_gprogram);
    }

    cg_report_errors();
    _glgsg->report_my_gl_errors();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLCgShaderContext::unbind
//       Access: Public
//  Description: This function disables a currently-bound shader.
////////////////////////////////////////////////////////////////////
void CLP(CgShaderContext)::
unbind() {
  if (_cg_context != 0) {
    if (_cg_vprogram != 0) {
      cgGLUnbindProgram(_cg_vprofile);
      cgGLDisableProfile(_cg_vprofile);
    }
    if (_cg_fprogram != 0) {
      cgGLUnbindProgram(_cg_fprofile);
      cgGLDisableProfile(_cg_fprofile);
    }
    if (_cg_gprogram != 0) {
      cgGLUnbindProgram(_cg_gprofile);
      cgGLDisableProfile(_cg_gprofile);
    }

    cg_report_errors();
    _glgsg->report_my_gl_errors();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLCgShaderContext::issue_parameters
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
void CLP(CgShaderContext)::
issue_parameters(int altered) {
  PStatTimer timer(_glgsg->_draw_set_state_shader_parameters_pcollector);

  if (!valid()) {
    return;
  }

  // Iterate through _ptr parameters
  for (int i=0; i<(int)_shader->_ptr_spec.size(); i++) {
    if (altered & (_shader->_ptr_spec[i]._dep[0] | _shader->_ptr_spec[i]._dep[1])) {
      const Shader::ShaderPtrSpec& _ptr = _shader->_ptr_spec[i];
      Shader::ShaderPtrData* ptr_data = 
        const_cast< Shader::ShaderPtrData*>(_glgsg->fetch_ptr_parameter(_ptr));
      
      if (ptr_data == NULL){ //the input is not contained in ShaderPtrData
        release_resources();
        return;
      }
      //check if the data must be shipped to the GPU
      /*if (!ptr_data->_updated)
        continue;
        ptr_data->_updated = false;*/

      //Check if the size of the shader input and ptr_data match
      int input_size = _ptr._dim[0] * _ptr._dim[1] * _ptr._dim[2];

      // dimension is negative only if the parameter had the (deprecated)k_ prefix.
      if ((input_size > ptr_data->_size) && (_ptr._dim[0] > 0)) { 
        GLCAT.error() << _ptr._id._name << ": incorrect number of elements, expected " 
                      <<  input_size <<" got " <<  ptr_data->_size << "\n";
        release_resources();
        return;
      }
      CGparameter p = _cg_parameter_map[_ptr._id._seqno];
      
      switch (ptr_data->_type) {
      case Shader::SPT_float:
        switch(_ptr._info._class) {
        case Shader::SAC_scalar: cgSetParameter1fv(p,(float*)ptr_data->_ptr); continue;
        case Shader::SAC_vector:
          switch(_ptr._info._type) {
          case Shader::SAT_vec1: cgSetParameter1fv(p,(float*)ptr_data->_ptr); continue;
          case Shader::SAT_vec2: cgSetParameter2fv(p,(float*)ptr_data->_ptr); continue;
          case Shader::SAT_vec3: cgSetParameter3fv(p,(float*)ptr_data->_ptr); continue;
          case Shader::SAT_vec4: cgSetParameter4fv(p,(float*)ptr_data->_ptr); continue;
          }
        case Shader::SAC_matrix: cgGLSetMatrixParameterfc(p,(float*)ptr_data->_ptr); continue;
        case Shader::SAC_array: {
          switch(_ptr._info._subclass) {
          case Shader::SAC_scalar: 
            cgGLSetParameterArray1f(p,0,_ptr._dim[0],(float*)ptr_data->_ptr); continue;
          case Shader::SAC_vector:
            switch(_ptr._dim[2]) {
            case 1: cgGLSetParameterArray1f(p,0,_ptr._dim[0],(float*)ptr_data->_ptr); continue;
            case 2: cgGLSetParameterArray2f(p,0,_ptr._dim[0],(float*)ptr_data->_ptr); continue;
            case 3: cgGLSetParameterArray3f(p,0,_ptr._dim[0],(float*)ptr_data->_ptr); continue;
            case 4: cgGLSetParameterArray4f(p,0,_ptr._dim[0],(float*)ptr_data->_ptr); continue;
            }
          case Shader::SAC_matrix:
            cgGLSetMatrixParameterArrayfc(p,0,_ptr._dim[0],(float*)ptr_data->_ptr); continue;
          }
        }
        }
      case Shader::SPT_double:
        switch(_ptr._info._class) {
        case Shader::SAC_scalar: cgSetParameter1dv(p,(double*)ptr_data->_ptr); continue;
        case Shader::SAC_vector:
          switch(_ptr._info._type) {
          case Shader::SAT_vec1: cgSetParameter1dv(p,(double*)ptr_data->_ptr); continue;
          case Shader::SAT_vec2: cgSetParameter2dv(p,(double*)ptr_data->_ptr); continue;
          case Shader::SAT_vec3: cgSetParameter3dv(p,(double*)ptr_data->_ptr); continue;
          case Shader::SAT_vec4: cgSetParameter4dv(p,(double*)ptr_data->_ptr); continue;
          }
        case Shader::SAC_matrix: cgGLSetMatrixParameterdc(p,(double*)ptr_data->_ptr); continue;
        case Shader::SAC_array: {
          switch(_ptr._info._subclass) {
          case Shader::SAC_scalar: 
            cgGLSetParameterArray1d(p,0,_ptr._dim[0],(double*)ptr_data->_ptr); continue;
          case Shader::SAC_vector:
            switch(_ptr._dim[2]) {
            case 1: cgGLSetParameterArray1d(p,0,_ptr._dim[0],(double*)ptr_data->_ptr); continue;
            case 2: cgGLSetParameterArray2d(p,0,_ptr._dim[0],(double*)ptr_data->_ptr); continue;
            case 3: cgGLSetParameterArray3d(p,0,_ptr._dim[0],(double*)ptr_data->_ptr); continue;
            case 4: cgGLSetParameterArray4d(p,0,_ptr._dim[0],(double*)ptr_data->_ptr); continue;
            }
          case Shader::SAC_matrix:
            cgGLSetMatrixParameterArraydc(p,0,_ptr._dim[0],(double*)ptr_data->_ptr); continue;
          }
        }
        }
      case Shader::SPT_int:
        switch(_ptr._info._class) {
        case Shader::SAC_scalar: cgSetParameter1iv(p,(int*)ptr_data->_ptr); continue;
        case Shader::SAC_vector:
          switch(_ptr._info._type) {
          case Shader::SAT_vec1: cgSetParameter1iv(p,(int*)ptr_data->_ptr); continue;
          case Shader::SAT_vec2: cgSetParameter2iv(p,(int*)ptr_data->_ptr); continue;
          case Shader::SAT_vec3: cgSetParameter3iv(p,(int*)ptr_data->_ptr); continue;
          case Shader::SAT_vec4: cgSetParameter4iv(p,(int*)ptr_data->_ptr); continue;
          }
        }
      default: GLCAT.error() << _ptr._id._name << ":" << "unrecognized parameter type\n"; 
        release_resources(); 
        return;
      }
    }
  }

  for (int i=0; i<(int)_shader->_mat_spec.size(); i++) {
    if (altered & (_shader->_mat_spec[i]._dep[0] | _shader->_mat_spec[i]._dep[1])) {
      const LMatrix4 *val = _glgsg->fetch_specified_value(_shader->_mat_spec[i], altered);
      if (!val) continue;
#ifndef STDFLOAT_DOUBLE
      // In this case, the data is already single-precision.
      const PN_float32 *data = val->get_data();
#else
      // In this case, we have to convert it.
      LMatrix4f valf = LCAST(PN_float32, *val);
      const PN_float32 *data = valf.get_data();
#endif

      CGparameter p = _cg_parameter_map[_shader->_mat_spec[i]._id._seqno];
      switch (_shader->_mat_spec[i]._piece) {
      case Shader::SMP_whole: GLfc(cgGLSetMatrixParameter)(p, data); continue;
      case Shader::SMP_transpose: GLfr(cgGLSetMatrixParameter)(p, data); continue;
      case Shader::SMP_col0: GLf(cgGLSetParameter4)(p, data[0], data[4], data[ 8], data[12]); continue;
      case Shader::SMP_col1: GLf(cgGLSetParameter4)(p, data[1], data[5], data[ 9], data[13]); continue;
      case Shader::SMP_col2: GLf(cgGLSetParameter4)(p, data[2], data[6], data[10], data[14]); continue;
      case Shader::SMP_col3: GLf(cgGLSetParameter4)(p, data[3], data[7], data[11], data[15]); continue;
      case Shader::SMP_row0: GLfv(cgGLSetParameter4)(p, data+ 0); continue;
      case Shader::SMP_row1: GLfv(cgGLSetParameter4)(p, data+ 4); continue;
      case Shader::SMP_row2: GLfv(cgGLSetParameter4)(p, data+ 8); continue;
      case Shader::SMP_row3: GLfv(cgGLSetParameter4)(p, data+12); continue;
      case Shader::SMP_row3x1: GLfv(cgGLSetParameter1)(p, data+12); continue;
      case Shader::SMP_row3x2: GLfv(cgGLSetParameter2)(p, data+12); continue;
      case Shader::SMP_row3x3: GLfv(cgGLSetParameter3)(p, data+12); continue;
      case Shader::SMP_upper3x3:
        {
          LMatrix3f upper3 = val->get_upper_3();
          GLfc(cgGLSetMatrixParameter)(p, upper3.get_data());
          continue;
        }
      case Shader::SMP_transpose3x3:
        {
          LMatrix3f upper3 = val->get_upper_3();
          GLfr(cgGLSetMatrixParameter)(p, upper3.get_data());
          continue;
        }
      case Shader::SMP_cell15:
        GLf(cgGLSetParameter1)(p, data[15]);
        continue;
      }
    }
  }

  _glgsg->report_my_gl_errors();
}

////////////////////////////////////////////////////////////////////
//     Function: GLCgShaderContext::disable_shader_vertex_arrays
//       Access: Public
//  Description: Disable all the vertex arrays used by this shader.
////////////////////////////////////////////////////////////////////
void CLP(CgShaderContext)::
disable_shader_vertex_arrays() {
  if (!valid()) {
    return;
  }

  for (int i=0; i<(int)_shader->_var_spec.size(); i++) {
    CGparameter p = _cg_parameter_map[_shader->_var_spec[i]._id._seqno];
    if (p == 0) continue;
    cgGLDisableClientState(p);
  }

  cg_report_errors();
  _glgsg->report_my_gl_errors();
}

////////////////////////////////////////////////////////////////////
//     Function: GLCgShaderContext::update_shader_vertex_arrays
//       Access: Public
//  Description: Disables all vertex arrays used by the previous
//               shader, then enables all the vertex arrays needed
//               by this shader.  Extracts the relevant vertex array
//               data from the gsg.
//               The current implementation is inefficient, because
//               it may unnecessarily disable arrays then immediately
//               reenable them.  We may optimize this someday.
////////////////////////////////////////////////////////////////////
bool CLP(CgShaderContext)::
update_shader_vertex_arrays(ShaderContext *prev, bool force) {
  if (prev) {
    prev->disable_shader_vertex_arrays();
  }

  if (!valid()) {
    return true;
  }

  cg_report_errors();

#ifdef SUPPORT_IMMEDIATE_MODE
  if (_glgsg->_use_sender) {
    GLCAT.error() << "immediate mode shaders not implemented yet\n";
  } else
#endif // SUPPORT_IMMEDIATE_MODE
  {
    const GeomVertexArrayDataHandle *array_reader;
    Geom::NumericType numeric_type;
    int start, stride, num_values;
    int nvarying = _shader->_var_spec.size();
    for (int i = 0; i < nvarying; ++i) {
      if (_cg_parameter_map[_shader->_var_spec[i]._id._seqno] == 0) {
        continue;
      }

      InternalName *name = _shader->_var_spec[i]._name;
      int texslot = _shader->_var_spec[i]._append_uv;
      if (texslot >= 0 && texslot < _glgsg->_state_texture->get_num_on_stages()) {
        TextureStage *stage = _glgsg->_state_texture->get_on_stage(texslot);
        InternalName *texname = stage->get_texcoord_name();

        if (name == InternalName::get_texcoord()) {
          name = texname;
        } else if (texname != InternalName::get_texcoord()) {
          name = name->append(texname->get_basename());
        }
      }
      if (_glgsg->_data_reader->get_array_info(name,
                                               array_reader, num_values, numeric_type,
                                               start, stride)) {
        const unsigned char *client_pointer;
        if (!_glgsg->setup_array_data(client_pointer, array_reader, force)) {
          return false;
        }

        CGparameter p = _cg_parameter_map[_shader->_var_spec[i]._id._seqno];

        if (numeric_type == GeomEnums::NT_packed_dabc) {
          cgGLSetParameterPointer(p, GL_BGRA, GL_UNSIGNED_BYTE,
                                  stride, client_pointer + start);
        } else {
          if (name == InternalName::get_normal() && num_values == 4) {
            // In some cases, the normals are aligned to 4 values.
            // This would cause an error on some rivers, so we tell it
            // to use the first three values only.
            num_values = 3;
          }
          cgGLSetParameterPointer(p,
                                  num_values, _glgsg->get_numeric_type(numeric_type),
                                  stride, client_pointer + start);
        }
        cgGLEnableClientState(p);
      } else {
        CGparameter p = _cg_parameter_map[_shader->_var_spec[i]._id._seqno];
        cgGLDisableClientState(p);
      }
    }
  }

  cg_report_errors();
  _glgsg->report_my_gl_errors();

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GLCgShaderContext::disable_shader_texture_bindings
//       Access: Public
//  Description: Disable all the texture bindings used by this shader.
////////////////////////////////////////////////////////////////////
void CLP(CgShaderContext)::
disable_shader_texture_bindings() {
  if (!valid()) {
    return;
  }

#ifndef OPENGLES_2
  for (int i=0; i<(int)_shader->_tex_spec.size(); i++) {
    CGparameter p = _cg_parameter_map[_shader->_tex_spec[i]._id._seqno];
    if (p == 0) continue;
    int texunit = cgGetParameterResourceIndex(p);
    _glgsg->_glActiveTexture(GL_TEXTURE0 + texunit);

#ifndef OPENGLES
    glBindTexture(GL_TEXTURE_1D, 0);
#endif  // OPENGLES
    glBindTexture(GL_TEXTURE_2D, 0);
#ifndef OPENGLES_1
    if (_glgsg->_supports_3d_texture) {
      glBindTexture(GL_TEXTURE_3D, 0);
    }
#endif  // OPENGLES_1
#ifndef OPENGLES
    if (_glgsg->_supports_2d_texture_array) {
      glBindTexture(GL_TEXTURE_2D_ARRAY_EXT, 0);
    }
#endif
    if (_glgsg->_supports_cube_map) {
      glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }
    // This is probably faster - but maybe not as safe?
    // cgGLDisableTextureParameter(p);
  }
#endif  // OPENGLES_2

  cg_report_errors();
  _glgsg->report_my_gl_errors();
}

////////////////////////////////////////////////////////////////////
//     Function: GLCgShaderContext::update_shader_texture_bindings
//       Access: Public
//  Description: Disables all texture bindings used by the previous
//               shader, then enables all the texture bindings needed
//               by this shader.  Extracts the relevant vertex array
//               data from the gsg.
//               The current implementation is inefficient, because
//               it may unnecessarily disable textures then immediately
//               reenable them.  We may optimize this someday.
////////////////////////////////////////////////////////////////////
void CLP(CgShaderContext)::
update_shader_texture_bindings(ShaderContext *prev) {
  if (prev) {
    prev->disable_shader_texture_bindings();
  }

  if (!valid()) {
    return;
  }

  // We get the TextureAttrib directly from the _target_rs, not the
  // filtered TextureAttrib in _target_texture.
  const TextureAttrib *texattrib = DCAST(TextureAttrib, _glgsg->_target_rs->get_attrib_def(TextureAttrib::get_class_slot()));
  nassertv(texattrib != (TextureAttrib *)NULL);

  for (int i = 0; i < (int)_shader->_tex_spec.size(); ++i) {
    InternalName *id = _shader->_tex_spec[i]._name;

    CGparameter p = _cg_parameter_map[_shader->_tex_spec[i]._id._seqno];
    if (p == 0) {
      continue;
    }
    int texunit = cgGetParameterResourceIndex(p);

    Texture *tex = 0;
    int view = _glgsg->get_current_tex_view_offset();
    if (id != 0) {
      const ShaderInput *input = _glgsg->_target_shader->get_shader_input(id);
      tex = input->get_texture();
    } else {
      if (_shader->_tex_spec[i]._stage >= texattrib->get_num_on_stages()) {
        continue;
      }
      TextureStage *stage = texattrib->get_on_stage(_shader->_tex_spec[i]._stage);
      tex = texattrib->get_on_texture(stage);
      view += stage->get_tex_view_offset();
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

    _glgsg->_glActiveTexture(GL_TEXTURE0 + texunit);

    TextureContext *tc = tex->prepare_now(view, _glgsg->_prepared_objects, _glgsg);
    if (tc == (TextureContext*)NULL) {
      continue;
    }

    GLenum target = _glgsg->get_texture_target(tex->get_texture_type());
    if (target == GL_NONE) {
      // Unsupported texture mode.
      continue;
    }

    if (!_glgsg->update_texture(tc, false)) {
      continue;
    }

    _glgsg->apply_texture(tc);
  }

  cg_report_errors();
  _glgsg->report_my_gl_errors();
}

#endif  // OPENGLES_1
