// Filename: glShaderContext_src.cxx
// Created by: jyelon (01Sep05)
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

#ifdef HAVE_CG
#include "Cg/cgGL.h"
#endif
#include "pStatTimer.h"

#define DEBUG_GL_SHADER 0

TypeHandle CLP(ShaderContext)::_type_handle;

#ifndef GL_GEOMETRY_SHADER
#define GL_GEOMETRY_SHADER 0x8DD9
#endif

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::Constructor
//       Access: Public
//  Description: xyz
////////////////////////////////////////////////////////////////////
CLP(ShaderContext)::
CLP(ShaderContext)(Shader *s, GSG *gsg) : ShaderContext(s) {
  _last_gsg = gsg;
  _glsl_program = 0;
  _glsl_vshader = 0;
  _glsl_fshader = 0;
  _glsl_gshader = 0;
#ifdef HAVE_CG
  _cg_context = 0;
  if (s->get_language() == Shader::SL_Cg) {
    
    // Ask the shader to compile itself for us and 
    // to give us the resulting Cg program objects.

    if (!s->cg_compile_for(gsg->_shader_caps,
                           _cg_context,
                           _cg_vprogram,
                           _cg_fprogram, 
                           _cg_gprogram,
                           _cg_parameter_map)) {
      return;
    }
    
    // Load the program.
    
    cgGLLoadProgram(_cg_vprogram);
    CGerror verror = cgGetError();
    if (verror != CG_NO_ERROR) {
      const char *str = (const char *)GLP(GetString)(GL_PROGRAM_ERROR_STRING_ARB);
      GLCAT.error() << "Could not load Cg vertex program:" << s->get_filename() << " (" << 
        cgGetProfileString(cgGetProgramProfile(_cg_vprogram)) << " " << str << ")\n";
      release_resources(gsg);
    }
    cgGLLoadProgram(_cg_fprogram);
    CGerror ferror = cgGetError();
    if (ferror != CG_NO_ERROR) {
      const char *str = (const char *)GLP(GetString)(GL_PROGRAM_ERROR_STRING_ARB);
      GLCAT.error() << "Could not load Cg fragment program:" << s->get_filename() << " (" << 
        cgGetProfileString(cgGetProgramProfile(_cg_fprogram)) << " " << str << ")\n";
      release_resources(gsg);
    }
    if (glGetError() != GL_NO_ERROR) {
      GLCAT.error() << "GL error in ShaderContext constructor\n";
    }
    if (_cg_gprogram != 0) {
      cgGLLoadProgram(_cg_gprogram);
      if (GLCAT.is_debug()) {
        GLCAT.debug()
          << "Loaded geom prog: " << _cg_gprogram << "\n";
      }

      CGerror gerror = cgGetError();
      if (gerror != CG_NO_ERROR) {
        const char *str = (const char *)GLP(GetString)(GL_PROGRAM_ERROR_STRING_ARB);
        GLCAT.error() << "Could not load Cg geometry program:" << s->get_filename() << " (" << 
          cgGetProfileString(cgGetProgramProfile(_cg_gprogram)) << " " << str << ")\n";
        release_resources(gsg);
      }
      if (glGetError() != GL_NO_ERROR) {
        GLCAT.error() << "GL error in ShaderContext constructor\n";
      }
    }
  }
#endif

  if (s->get_language() == Shader::SL_GLSL) {
    // We compile and analyze the shader here, instead of in shader.cxx, to avoid gobj getting a dependency on GL stuff.
    if (_glsl_program == 0) {
      if (!glsl_compile_shader(gsg)) {
        release_resources(gsg);
        s->_error_flag = true;
        return;
      }
    }
    // Analyze the uniforms and put them in _glsl_parameter_map
    if (s->_glsl_parameter_map.size() == 0) {
      int seqno = 0, texunitno = 0;
      int num_uniforms, uniform_maxlength;
      gsg->_glGetProgramiv(_glsl_program, GL_ACTIVE_UNIFORMS, &num_uniforms);
      gsg->_glGetProgramiv(_glsl_program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &uniform_maxlength);
      for (int i = 0; i < num_uniforms; ++i) {
        int param_size;
        GLenum param_type;
        char param_name[uniform_maxlength];
        gsg->_glGetActiveUniform(_glsl_program, i, uniform_maxlength, NULL, &param_size, &param_type, param_name);
        GLint p = gsg->_glGetUniformLocation(_glsl_program, param_name);
        if (p > -1) {
          Shader::ShaderArgId arg_id;
          arg_id._name  = param_name;
          arg_id._seqno = seqno++;
          s->_glsl_parameter_map.push_back(p);
          PT(InternalName) inputname = InternalName::make(param_name);
          if (inputname->get_name().substr(0, 11) == "p3d_Texture") {
            Shader::ShaderTexSpec bind;
            bind._id = arg_id;
            bind._name = 0;
            bind._desired_type = Texture::TT_2d_texture;
            bind._stage = atoi(inputname->get_name().substr(12).c_str());
            s->_tex_spec.push_back(bind);
            continue;
          }
          switch (param_type) {
            case GL_SAMPLER_1D_SHADOW:
            case GL_SAMPLER_1D: {
              Shader::ShaderTexSpec bind;
              bind._id = arg_id;
              bind._name = inputname;
              bind._desired_type = Texture::TT_1d_texture;
              bind._stage = texunitno++;
              s->_tex_spec.push_back(bind);
              continue; }
            case GL_SAMPLER_2D_SHADOW:
            case GL_SAMPLER_2D: {
              Shader::ShaderTexSpec bind;
              bind._id = arg_id;
              bind._name = inputname;
              bind._desired_type = Texture::TT_2d_texture;
              bind._stage = texunitno++;
              s->_tex_spec.push_back(bind);
              continue; }
            case GL_SAMPLER_3D: {
              Shader::ShaderTexSpec bind;
              bind._id = arg_id;
              bind._name = inputname;
              bind._desired_type = Texture::TT_3d_texture;
              bind._stage = texunitno++;
              s->_tex_spec.push_back(bind);
              continue; }
            case GL_SAMPLER_CUBE: {
              Shader::ShaderTexSpec bind;
              bind._id = arg_id;
              bind._name = inputname;
              bind._desired_type = Texture::TT_cube_map;
              bind._stage = texunitno++;
              s->_tex_spec.push_back(bind);
              continue; }
            case GL_FLOAT_MAT2:
            case GL_FLOAT_MAT3:
            case GL_FLOAT_MAT2x3:
            case GL_FLOAT_MAT2x4:
            case GL_FLOAT_MAT3x2:
            case GL_FLOAT_MAT3x4:
            case GL_FLOAT_MAT4x2:
            case GL_FLOAT_MAT4x3:
              GLCAT.warning() << "GLSL shader requested an unrecognized matrix type\n";
              continue;
            case GL_FLOAT_MAT4: {
              Shader::ShaderMatSpec bind;
              bind._id = arg_id;
              bind._piece = Shader::SMP_whole;
              bind._func = Shader::SMF_first;
              bind._part[0] = Shader::SMO_mat_constant_x;
              bind._arg[0] = inputname;
              bind._part[1] = Shader::SMO_identity;
              bind._arg[1] = NULL;
              s->_mat_spec.push_back(bind);
              continue; }
            case GL_BOOL:
            case GL_BOOL_VEC2:
            case GL_BOOL_VEC3:
            case GL_BOOL_VEC4:
            case GL_FLOAT:
            case GL_FLOAT_VEC2:
            case GL_FLOAT_VEC3:
            case GL_FLOAT_VEC4: {
              Shader::ShaderMatSpec bind;
              bind._id = arg_id;
              switch (param_type) {
              case GL_BOOL:
              case GL_FLOAT:      bind._piece = Shader::SMP_row3x1; break;
              case GL_BOOL_VEC2:
              case GL_FLOAT_VEC2: bind._piece = Shader::SMP_row3x2; break;
              case GL_BOOL_VEC3:
              case GL_FLOAT_VEC3: bind._piece = Shader::SMP_row3x3; break;
              case GL_BOOL_VEC4:
              case GL_FLOAT_VEC4: bind._piece = Shader::SMP_row3  ; break;
              }
              bind._func = Shader::SMF_first;
              bind._part[0] = Shader::SMO_vec_constant_x;
              bind._arg[0] = inputname;
              bind._part[1] = Shader::SMO_identity;
              bind._arg[1] = NULL;
              s->_mat_spec.push_back(bind);
              continue; }
            case GL_INT:
            case GL_INT_VEC2:
            case GL_INT_VEC3:
            case GL_INT_VEC4:
              GLCAT.warning() << "Panda does not support passing integers to shaders (yet)!\n";
              continue;
            default:
              GLCAT.warning() << "Ignoring unrecognized GLSL parameter type!\n";
          }
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::Destructor
//       Access: Public
//  Description: xyz
////////////////////////////////////////////////////////////////////
CLP(ShaderContext)::
~CLP(ShaderContext)() {
  release_resources(_last_gsg);
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::release_resources
//       Access: Public
//  Description: Should deallocate all system resources (such as
//               vertex program handles or Cg contexts).
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
release_resources(const GSG *gsg) {
#ifdef HAVE_CG
  if (_cg_context) {
    cgDestroyContext(_cg_context);
    _cg_context  = 0;
    _cg_vprogram = 0;
    _cg_fprogram = 0;
    _cg_gprogram = 0;
    _cg_parameter_map.clear();
  }
  if (glGetError() != GL_NO_ERROR) {
    GLCAT.error() << "GL error in ShaderContext destructor\n";
  }
#endif

  if (!gsg) {
    return;
  }
  if (_glsl_program != 0) {
    if (!_glsl_vshader != 0) {
      gsg->_glDetachShader(_glsl_program, _glsl_vshader);
      gsg->_glDeleteShader(_glsl_vshader);
      _glsl_vshader = 0;
    }
    if (!_glsl_fshader != 0) {
      gsg->_glDetachShader(_glsl_program, _glsl_fshader);
      gsg->_glDeleteShader(_glsl_fshader);
      _glsl_fshader = 0;
    }
    if (!_glsl_gshader != 0) {
      gsg->_glDetachShader(_glsl_program, _glsl_gshader);
      gsg->_glDeleteShader(_glsl_gshader);
      _glsl_gshader = 0;
    }
    gsg->_glDeleteProgram(_glsl_program);
    _glsl_program = 0;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::bind
//       Access: Public
//  Description: This function is to be called to enable a new
//               shader.  It also initializes all of the shader's
//               input parameters.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
bind(GSG *gsg) {
  _last_gsg = gsg;
  // Pass in k-parameters and transform-parameters
  issue_parameters(gsg, Shader::SSD_general);

#ifdef HAVE_CG
  if (_cg_context != 0) {
    // Bind the shaders.
    cgGLEnableProfile(cgGetProgramProfile(_cg_vprogram));
    cgGLBindProgram(_cg_vprogram);
    cgGLEnableProfile(cgGetProgramProfile(_cg_fprogram));
    cgGLBindProgram(_cg_fprogram);
    if (_cg_gprogram != 0) {
      cgGLEnableProfile(cgGetProgramProfile(_cg_gprogram));
      cgGLBindProgram(_cg_gprogram);
    }

    cg_report_errors();
  }
#endif
  
  if (_shader->get_language() == Shader::SL_GLSL && !_shader->get_error_flag()) {
    gsg->_glUseProgram(_glsl_program);
  }
  if (glGetError() != GL_NO_ERROR) {
    GLCAT.error() << "GL error in ShaderContext::bind\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::unbind
//       Access: Public
//  Description: This function disables a currently-bound shader.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
unbind(GSG *gsg) {
  _last_gsg = gsg;

#ifdef HAVE_CG
  if (_cg_context != 0) {
    cgGLDisableProfile(cgGetProgramProfile(_cg_vprogram));
    cgGLDisableProfile(cgGetProgramProfile(_cg_fprogram));
    if (_cg_gprogram != 0) {
      cgGLDisableProfile(cgGetProgramProfile(_cg_gprogram));
    }

    cg_report_errors();
  }
#endif
  
  if (_shader->get_language() == Shader::SL_GLSL) {
    gsg->_glUseProgram(0);
  }
  if (glGetError() != GL_NO_ERROR) {
    GLCAT.error() << "GL error in ShaderContext::unbind\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::issue_parameters
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
void CLP(ShaderContext)::
issue_parameters(GSG *gsg, int altered) {
  _last_gsg = gsg;

  PStatTimer timer(gsg->_draw_set_state_shader_parameters_pcollector);

  if (!valid()) {
    return;
  }

  for (int i=0; i<(int)_shader->_mat_spec.size(); i++) {
    if (altered & (_shader->_mat_spec[i]._dep[0] | _shader->_mat_spec[i]._dep[1])) {
      const LMatrix4f *val = gsg->fetch_specified_value(_shader->_mat_spec[i], altered);
      if (!val) continue;
      const float *data = val->get_data();
      if (_shader->get_language() == Shader::SL_GLSL) {
        GLint p = _shader->_glsl_parameter_map[_shader->_mat_spec[i]._id._seqno];
        switch (_shader->_mat_spec[i]._piece) {
          case Shader::SMP_whole: gsg->_glUniformMatrix4fv(p, 1, false, data); continue;
          case Shader::SMP_transpose: gsg->_glUniformMatrix4fv(p, 1, true, data); continue;
          case Shader::SMP_col0: gsg->_glUniform4f(p, data[0], data[4], data[ 8], data[12]); continue;
          case Shader::SMP_col1: gsg->_glUniform4f(p, data[1], data[5], data[ 9], data[13]); continue;
          case Shader::SMP_col2: gsg->_glUniform4f(p, data[2], data[6], data[10], data[14]); continue;
          case Shader::SMP_col3: gsg->_glUniform4f(p, data[3], data[7], data[11], data[15]); continue;
          case Shader::SMP_row0: gsg->_glUniform4fv(p, 1, data+ 0); continue;
          case Shader::SMP_row1: gsg->_glUniform4fv(p, 1, data+ 4); continue;
          case Shader::SMP_row2: gsg->_glUniform4fv(p, 1, data+ 8); continue;
          case Shader::SMP_row3: gsg->_glUniform4fv(p, 1, data+12); continue;
          case Shader::SMP_row3x1: gsg->_glUniform1fv(p, 1, data+12); continue;
          case Shader::SMP_row3x2: gsg->_glUniform2fv(p, 1, data+12); continue;
          case Shader::SMP_row3x3: gsg->_glUniform3fv(p, 1, data+12); continue;
        }
      }
#ifdef HAVE_CG
      else if (_shader->get_language() == Shader::SL_Cg) {
        CGparameter p = _cg_parameter_map[_shader->_mat_spec[i]._id._seqno];
        switch (_shader->_mat_spec[i]._piece) {
          case Shader::SMP_whole: cgGLSetMatrixParameterfc(p, data); continue;
          case Shader::SMP_transpose: cgGLSetMatrixParameterfr(p, data); continue;
          case Shader::SMP_col0: cgGLSetParameter4f(p, data[0], data[4], data[ 8], data[12]); continue;
          case Shader::SMP_col1: cgGLSetParameter4f(p, data[1], data[5], data[ 9], data[13]); continue;
          case Shader::SMP_col2: cgGLSetParameter4f(p, data[2], data[6], data[10], data[14]); continue;
          case Shader::SMP_col3: cgGLSetParameter4f(p, data[3], data[7], data[11], data[15]); continue;
          case Shader::SMP_row0: cgGLSetParameter4fv(p, data+ 0); continue;
          case Shader::SMP_row1: cgGLSetParameter4fv(p, data+ 4); continue;
          case Shader::SMP_row2: cgGLSetParameter4fv(p, data+ 8); continue;
          case Shader::SMP_row3: cgGLSetParameter4fv(p, data+12); continue;
          case Shader::SMP_row3x1: cgGLSetParameter1fv(p, data+12); continue;
          case Shader::SMP_row3x2: cgGLSetParameter2fv(p, data+12); continue;
          case Shader::SMP_row3x3: cgGLSetParameter3fv(p, data+12); continue;
        }
      }
#endif
    }
  }
#ifdef HAVE_CG
  cg_report_errors();
#endif

  if (glGetError() != GL_NO_ERROR) {
    GLCAT.error() << "GL error in ShaderContext::issue_parameters\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::disable_shader_vertex_arrays
//       Access: Public
//  Description: Disable all the vertex arrays used by this shader.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
disable_shader_vertex_arrays(GSG *gsg) {
  _last_gsg = gsg;

#ifdef HAVE_CG
  if (_cg_context == 0) {
    return;
  }

  for (int i=0; i<(int)_shader->_var_spec.size(); i++) {
    CGparameter p = _cg_parameter_map[_shader->_var_spec[i]._id._seqno];
    if (p == 0) continue;
    cgGLDisableClientState(p);
  }
  cg_report_errors();
  if (glGetError() != GL_NO_ERROR) {
    GLCAT.error() << "GL error in ShaderContext::disable_shader_vertex_arrays\n";
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::update_shader_vertex_arrays
//       Access: Public
//  Description: Disables all vertex arrays used by the previous
//               shader, then enables all the vertex arrays needed
//               by this shader.  Extracts the relevant vertex array
//               data from the gsg.
//               The current implementation is inefficient, because
//               it may unnecessarily disable arrays then immediately
//               reenable them.  We may optimize this someday.
////////////////////////////////////////////////////////////////////
bool CLP(ShaderContext)::
update_shader_vertex_arrays(CLP(ShaderContext) *prev, GSG *gsg,
                            bool force) {
  _last_gsg = gsg;
  if (prev) prev->disable_shader_vertex_arrays(gsg);
#ifdef HAVE_CG
  if (!valid()) {
    return true;
  }
  cg_report_errors();

#ifdef SUPPORT_IMMEDIATE_MODE
  if (gsg->_use_sender) {
    GLCAT.error() << "immediate mode shaders not implemented yet\n";
  } else
#endif // SUPPORT_IMMEDIATE_MODE
  {
    const GeomVertexArrayDataHandle *array_reader;
    Geom::NumericType numeric_type;
    int start, stride, num_values;
    int nvarying = _shader->_var_spec.size();
    for (int i=0; i<nvarying; i++) {
      CGparameter p = _cg_parameter_map[_shader->_var_spec[i]._id._seqno];
      if (p == 0) continue;
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
      if (gsg->_data_reader->get_array_info(name,
                                            array_reader, num_values, numeric_type,
                                            start, stride)) {
        const unsigned char *client_pointer;
        if (!gsg->setup_array_data(client_pointer, array_reader, force)) {
          return false;
        }
        cgGLSetParameterPointer(p,
                                num_values, gsg->get_numeric_type(numeric_type),
                                stride, client_pointer + start);
        cgGLEnableClientState(p);
      } else {
        cgGLDisableClientState(p);
      }
    }
  }

  cg_report_errors();
  if (glGetError() != GL_NO_ERROR) {
    GLCAT.error() << "GL error in ShaderContext::update_shader_vertex_arrays\n";
  }
#endif // HAVE_CG
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::disable_shader_texture_bindings
//       Access: Public
//  Description: Disable all the texture bindings used by this shader.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
disable_shader_texture_bindings(GSG *gsg) {
  _last_gsg = gsg;
  if (!valid()) {
    return;
  }

  for (int i=0; i<(int)_shader->_tex_spec.size(); i++) {
    if (_shader->get_language() == Shader::SL_GLSL) {
      if (_shader->_tex_spec[i]._name == 0) {
        gsg->_glActiveTexture(GL_TEXTURE0 + _shader->_tex_spec[i]._stage);
      } else {
        gsg->_glActiveTexture(GL_TEXTURE0 + _shader->_tex_spec[i]._stage + _stage_offset);
      }
#ifdef HAVE_CG
    } else if (_shader->get_language() == Shader::SL_Cg) {
      CGparameter p = _cg_parameter_map[_shader->_tex_spec[i]._id._seqno];
      if (p == 0) continue;
      int texunit = cgGetParameterResourceIndex(p);
      gsg->_glActiveTexture(GL_TEXTURE0 + texunit);
#endif
    } else {
      return;
    }
#ifndef OPENGLES
    GLP(Disable)(GL_TEXTURE_1D);
#endif
    GLP(Disable)(GL_TEXTURE_2D);
#ifndef OPENGLES_1
    if (gsg->_supports_3d_texture) {
      GLP(Disable)(GL_TEXTURE_3D);
    }
#endif
    if (gsg->_supports_cube_map) {
      GLP(Disable)(GL_TEXTURE_CUBE_MAP);
    }
    // This is probably faster - but maybe not as safe?
    // cgGLDisableTextureParameter(p);
  }
  _stage_offset = 0;

#ifdef HAVE_CG
  cg_report_errors();
#endif

  if (glGetError() != GL_NO_ERROR) {
    GLCAT.error() << "GL error in ShaderContext::disable_shader_texture_bindings\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::update_shader_texture_bindings
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
update_shader_texture_bindings(CLP(ShaderContext) *prev, GSG *gsg) {
  _last_gsg = gsg;
  if (prev) {
    prev->disable_shader_texture_bindings(gsg);
  }

  if (!valid()) {
    return;
  }

  // We get the TextureAttrib directly from the _target_rs, not the
  // filtered TextureAttrib in _target_texture.
  const TextureAttrib *texattrib = DCAST(TextureAttrib, gsg->_target_rs->get_attrib_def(TextureAttrib::get_class_slot()));
  nassertv(texattrib != (TextureAttrib *)NULL);
  _stage_offset = texattrib->get_num_on_stages();

  for (int i = 0; i < (int)_shader->_tex_spec.size(); ++i) {
    InternalName *id = _shader->_tex_spec[i]._name;
    int texunit;

    if (_shader->get_language() == Shader::SL_GLSL) {
      texunit = _shader->_tex_spec[i]._stage;
      if (id != 0) {
        texunit += _stage_offset;
      }
    }
#ifdef HAVE_CG
    if (_shader->get_language() == Shader::SL_Cg) {
      CGparameter p = _cg_parameter_map[_shader->_tex_spec[i]._id._seqno];
      if (p == 0) {
        continue;
      }
      texunit = cgGetParameterResourceIndex(p);
    }
#endif

    Texture *tex = 0;
    if (id != 0) {
      const ShaderInput *input = gsg->_target_shader->get_shader_input(id);
      tex = input->get_texture();
    } else {
      if (_shader->_tex_spec[i]._stage >= texattrib->get_num_on_stages()) {
        continue;
      }
      TextureStage *stage = texattrib->get_on_stage(_shader->_tex_spec[i]._stage);
      tex = texattrib->get_on_texture(stage);
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
    if (tc == (TextureContext*)NULL) {
      continue;
    }

    gsg->_glActiveTexture(GL_TEXTURE0 + texunit);

    GLenum target = gsg->get_texture_target(tex->get_texture_type());
    if (target == GL_NONE) {
      // Unsupported texture mode.
      continue;
    }
    GLP(Enable)(target);
    gsg->apply_texture(tc);

    if (_shader->get_language() == Shader::SL_GLSL) {
      GLint p = _shader->_glsl_parameter_map[_shader->_tex_spec[i]._id._seqno];
      gsg->_glUniform1i(p, texunit);
    }

    if (!gsg->update_texture(tc, false)) {
      GLP(Disable)(target);
      continue;
    }
  }

#ifdef HAVE_CG
  cg_report_errors();
#endif

  if (glGetError() != GL_NO_ERROR) {
    GLCAT.error() << "GL error in ShaderContext::update_shader_texture_bindings\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::cg_report_errors
//       Access: Public
//  Description: Report any Cg errors that were not previously caught.
////////////////////////////////////////////////////////////////////
#ifdef HAVE_CG
void CLP(ShaderContext)::
cg_report_errors() {
  CGerror err = cgGetError();
  if (err != CG_NO_ERROR) {
    GLCAT.error() << _shader->get_filename() << " " << cgGetErrorString(err) << "\n";
  }
}
#endif

////////////////////////////////////////////////////////////////////
//     Function: Shader::glsl_report_shader_errors
//       Access: Private
//  Description: This subroutine prints the infolog for a shader.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
glsl_report_shader_errors(GSG *gsg, unsigned int shader) {
  char *info_log;
  int length = 0;
  int num_chars  = 0;

	gsg->_glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

  if (length > 0) {
    info_log = (char *) malloc(length);
    gsg->_glGetShaderInfoLog(shader, length, &num_chars, info_log);
    GLCAT.error(false) << info_log;
    free(info_log);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::glsl_report_program_errors
//       Access: Private
//  Description: This subroutine prints the infolog for a program.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
glsl_report_program_errors(GSG *gsg, unsigned int program) {
  char *info_log;
  int length = 0;
  int num_chars  = 0;

	gsg->_glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);

  if (length > 0) {
    info_log = (char *) malloc(length);
    gsg->_glGetProgramInfoLog(program, length, &num_chars, info_log);
    GLCAT.error(false) << info_log;
    free(info_log);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::glsl_compile_entry_point
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
unsigned int CLP(ShaderContext)::
glsl_compile_entry_point(GSG *gsg, const char *entry, Shader::ShaderType type) {
  unsigned int handle;
  switch (type) {
    case Shader::ST_VERTEX:
      handle = gsg->_glCreateShader(GL_VERTEX_SHADER);
      break;
    case Shader::ST_FRAGMENT:
      handle = gsg->_glCreateShader(GL_FRAGMENT_SHADER);
      break;
    case Shader::ST_GEOMETRY:
      handle = gsg->_glCreateShader(GL_GEOMETRY_SHADER);
      break;
    default:
      return 0;
  }
  if (!handle) {
    return 0;
  }
  // We define our own main() in which we call the right function.
  ostringstream str;
  str << "void main() { " << entry << "(); };";
  const char* sources[2] = {_shader->_text.c_str(), str.str().c_str()};
  gsg->_glShaderSource(handle, 2, sources, NULL);
  gsg->_glCompileShader(handle);
  int status;
  gsg->_glGetShaderiv(handle, GL_COMPILE_STATUS, &status);
  if (status != GL_TRUE) {
    GLCAT.error() << "An error occurred while compiling " << entry << "!\n";
    glsl_report_shader_errors(gsg, handle);
    gsg->_glDeleteShader(handle);
    return 0;
  }
  return handle;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::glsl_compile_shader
//       Access: Private
//  Description: This subroutine compiles a GLSL shader.
////////////////////////////////////////////////////////////////////
bool CLP(ShaderContext)::
glsl_compile_shader(GSG *gsg) {
  // Terribly hacky. I hope this will go away when we
  // add support for separated shader programs later.

  _glsl_program = gsg->_glCreateProgram();
  if (!_glsl_program) return false;

  if (_shader->_text.find("vshader") != -1) {
    _glsl_vshader = glsl_compile_entry_point(gsg, "vshader", Shader::ST_VERTEX);
    if (!_glsl_vshader) return false;
    gsg->_glAttachShader(_glsl_program, _glsl_vshader);
  } else {
    GLCAT.warning() << "Could not locate function 'vshader' in shader text!\n";
  }

  if (_shader->_text.find("fshader") != -1) {
    _glsl_fshader = glsl_compile_entry_point(gsg, "fshader", Shader::ST_FRAGMENT);
    if (!_glsl_fshader) return false;
    gsg->_glAttachShader(_glsl_program, _glsl_fshader);
  } else {
    GLCAT.warning() << "Could not locate function 'fshader' in shader text!\n";
  }

  if (_shader->_text.find("gshader") != -1) {
    _glsl_gshader = glsl_compile_entry_point(gsg, "gshader", Shader::ST_GEOMETRY);
    if (!_glsl_gshader) return false;
    gsg->_glAttachShader(_glsl_program, _glsl_gshader);
  }
  
  gsg->_glLinkProgram(_glsl_program);

  int status;
  gsg->_glGetProgramiv(_glsl_program, GL_LINK_STATUS, &status);
  if (status != GL_TRUE) {
    GLCAT.error() << "An error occurred while linking shader program!\n";
    glsl_report_program_errors(gsg, _glsl_program);
    return false;
  }
  
  // There might be warnings. Only report them for one shader program.
  if (_glsl_vshader != 0) {
    glsl_report_shader_errors(gsg, _glsl_vshader);
  } else if (_glsl_fshader != 0) {
    glsl_report_shader_errors(gsg, _glsl_fshader);
  } else if (_glsl_gshader != 0) {
    glsl_report_shader_errors(gsg, _glsl_gshader);
  }

  if (glGetError() != GL_NO_ERROR) {
    GLCAT.error() << "Failed to compile shader\n";
    return false;
  }
  return true;
}


