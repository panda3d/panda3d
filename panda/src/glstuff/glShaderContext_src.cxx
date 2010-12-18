// Filename: glShaderContext_src.cxx
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

#ifndef OPENGLES_1

#if defined(HAVE_CG) && !defined(OPENGLES)
#include "Cg/cgGL.h"
#endif
#include "pStatTimer.h"

#define DEBUG_GL_SHADER 0

TypeHandle CLP(ShaderContext)::_type_handle;

#ifndef GL_GEOMETRY_SHADER_EXT
#define GL_GEOMETRY_SHADER_EXT 0x8DD9
#endif
#ifndef GL_GEOMETRY_VERTICES_OUT_EXT
#define GL_GEOMETRY_VERTICES_OUT_EXT 0x8DDA
#endif
#ifndef GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT
#define GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT 0x8DE0
#endif

#if defined(HAVE_CG) && !defined(OPENGLES)
#ifndef NDEBUG
#define cg_report_errors() { \
  CGerror err = cgGetError(); \
  if (err != CG_NO_ERROR) { \
    GLCAT.error() << __FILE__ ", line " << __LINE__ << ": " << cgGetErrorString(err) << "\n"; \
  } }
#else
#define cg_report_errors()
#endif
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
#if defined(HAVE_CG) && !defined(OPENGLES)
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
    
    if (_cg_vprogram != 0) {
      cgGLLoadProgram(_cg_vprogram);
      CGerror verror = cgGetError();
      if (verror != CG_NO_ERROR) {
        const char *str = (const char *)GLP(GetString)(GL_PROGRAM_ERROR_STRING_ARB);
        GLCAT.error() << "Could not load Cg vertex program:" << s->get_filename(Shader::ST_vertex) << " (" << 
          cgGetProfileString(cgGetProgramProfile(_cg_vprogram)) << " " << str << ")\n";
        release_resources(gsg);
      }
    }

    if (_cg_fprogram != 0) {
      cgGLLoadProgram(_cg_fprogram);
      CGerror ferror = cgGetError();
      if (ferror != CG_NO_ERROR) {
        const char *str = (const char *)GLP(GetString)(GL_PROGRAM_ERROR_STRING_ARB);
        GLCAT.error() << "Could not load Cg fragment program:" << s->get_filename(Shader::ST_fragment) << " (" << 
          cgGetProfileString(cgGetProgramProfile(_cg_fprogram)) << " " << str << ")\n";
        release_resources(gsg);
      }
    }

    if (_cg_gprogram != 0) {
      cgGLLoadProgram(_cg_gprogram);
      CGerror gerror = cgGetError();
      if (gerror != CG_NO_ERROR) {
        const char *str = (const char *)GLP(GetString)(GL_PROGRAM_ERROR_STRING_ARB);
        GLCAT.error() << "Could not load Cg geometry program:" << s->get_filename(Shader::ST_geometry) << " (" << 
          cgGetProfileString(cgGetProgramProfile(_cg_gprogram)) << " " << str << ")\n";
        release_resources(gsg);
      }
    }
    gsg->report_my_gl_errors();
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
      string noprefix;
      GLint param_count, param_maxlength, param_size;
      GLenum param_type;
      gsg->_glGetProgramiv(_glsl_program, GL_ACTIVE_UNIFORMS, &param_count);
      gsg->_glGetProgramiv(_glsl_program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &param_maxlength);
      char* param_name = new char[param_maxlength];
      for (int i = 0; i < param_count; ++i) {
        gsg->_glGetActiveUniform(_glsl_program, i, param_maxlength, NULL, &param_size, &param_type, param_name);
        GLint p = gsg->_glGetUniformLocation(_glsl_program, param_name);
        if (p > -1) {
          Shader::ShaderArgId arg_id;
          arg_id._name  = param_name;
          arg_id._seqno = seqno++;
          s->_glsl_parameter_map.push_back(p);
          PT(InternalName) inputname = InternalName::make(param_name);
          if (inputname->get_name().substr(0, 4) == "p3d_" || inputname->get_name().substr(0, 3) == "gl_") {
            if (inputname->get_name().substr(0, 3) == "gl_") noprefix = inputname->get_name().substr(3);
            else noprefix = inputname->get_name().substr(4);
            
            if (noprefix == "ModelViewProjectionMatrix") {
              Shader::ShaderMatSpec bind;
              bind._id = arg_id;
              bind._piece = Shader::SMP_whole;
              bind._func = Shader::SMF_compose;
              bind._part[0] = Shader::SMO_model_to_view;
              bind._arg[0] = NULL;
              bind._part[1] = Shader::SMO_view_to_apiclip;
              bind._arg[1] = NULL;
              s->_mat_spec.push_back(bind);
              continue;
            }
            if (noprefix == "ModelViewMatrix") {
              Shader::ShaderMatSpec bind;
              bind._id = arg_id;
              bind._piece = Shader::SMP_whole;
              bind._func = Shader::SMF_first;
              bind._part[0] = Shader::SMO_model_to_view;
              bind._arg[0] = NULL;
              s->_mat_spec.push_back(bind);
              continue;
            }
            if (noprefix == "ProjectionMatrix") {
              Shader::ShaderMatSpec bind;
              bind._id = arg_id;
              bind._piece = Shader::SMP_whole;
              bind._func = Shader::SMF_first;
              bind._part[0] = Shader::SMO_view_to_apiclip;
              bind._arg[0] = NULL;
              s->_mat_spec.push_back(bind);
              continue;
            }
            if (noprefix.substr(0, 7) == "Texture") {
              Shader::ShaderTexSpec bind;
              bind._id = arg_id;
              bind._name = 0;
              bind._desired_type = Texture::TT_2d_texture;
              bind._stage = atoi(noprefix.substr(7).c_str());
              s->_tex_spec.push_back(bind);
              continue;
            }
            GLCAT.error() << "Unrecognized uniform name '" << param_name << "'!\n";
            continue;
          }
          switch (param_type) {
#ifndef OPENGLES
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
#endif
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
#ifndef OPENGLES
            case GL_FLOAT_MAT2x3:
            case GL_FLOAT_MAT2x4:
            case GL_FLOAT_MAT3x2:
            case GL_FLOAT_MAT3x4:
            case GL_FLOAT_MAT4x2:
            case GL_FLOAT_MAT4x3:
#endif
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
      delete[] param_name;
      
      int attrib_idx = 0;
      // Now we've processed the uniforms, we'll process the attribs.
      gsg->_glGetProgramiv(_glsl_program, GL_ACTIVE_ATTRIBUTES, &param_count);
      gsg->_glGetProgramiv(_glsl_program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &param_maxlength);
      param_name = new char[param_maxlength];
      for (int i = 0; i < param_count; ++i) {
        gsg->_glGetActiveAttrib(_glsl_program, i, param_maxlength, NULL, &param_size, &param_type, param_name);
        PT(InternalName) inputname = InternalName::make(param_name);
        if (inputname->get_name().substr(0, 4) == "p3d_") {
          noprefix = inputname->get_name().substr(4);
          Shader::ShaderVarSpec bind;
          Shader::ShaderArgId arg_id;
          arg_id._name  = param_name;
          arg_id._seqno = -1;
          bind._append_uv = -1;
          if (noprefix == "Vertex") {
            bind._name = InternalName::get_vertex();
            s->_var_spec.push_back(bind);
            gsg->_glBindAttribLocation(_glsl_program, i, param_name);
            continue;
          }
          if (noprefix == "Normal") {
            bind._name = InternalName::get_normal();
            s->_var_spec.push_back(bind);
            gsg->_glBindAttribLocation(_glsl_program, i, param_name);
            continue;
          }
          if (noprefix == "Color") {
            bind._name = InternalName::get_color();
            s->_var_spec.push_back(bind);
            gsg->_glBindAttribLocation(_glsl_program, i, param_name);
            continue;
          }
          if (noprefix.substr(0, 13)  == "MultiTexCoord") {
            bind._name = InternalName::get_texcoord();
            bind._append_uv = atoi(inputname->get_name().substr(13).c_str());
            s->_var_spec.push_back(bind);
            gsg->_glBindAttribLocation(_glsl_program, i, param_name);
            continue;
          }
          GLCAT.error() << "Unrecognized vertex attrib '" << param_name << "'!\n";
          continue;
        }
      }
      delete[] param_name;
    }
    
    // Finally, re-link the program, or otherwise the glBindAttribLocation
    // calls won't have any effect.
    gsg->_glLinkProgram(_glsl_program);

    GLint status;
    gsg->_glGetProgramiv(_glsl_program, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
      GLCAT.error() << "An error occurred while relinking shader program!\n";
      glsl_report_program_errors(gsg, _glsl_program);
      s->_error_flag = true;
    }
  }
  
  gsg->report_my_gl_errors();
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
release_resources(GSG *gsg) {
#if defined(HAVE_CG) && !defined(OPENGLES)
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
  if (gsg) {
    gsg->report_my_gl_errors();
  } else if (glGetError() != GL_NO_ERROR) {
    GLCAT.error() << "GL error in ShaderContext destructor\n";
  }
#endif

  if (!gsg) {
    return;
  }
  if (_glsl_program != 0) {
    if (_glsl_vshader != 0) {
      gsg->_glDetachShader(_glsl_program, _glsl_vshader);
    }
    if (_glsl_fshader != 0) {
      gsg->_glDetachShader(_glsl_program, _glsl_fshader);
    }
    if (_glsl_gshader != 0) {
      gsg->_glDetachShader(_glsl_program, _glsl_gshader);
    }
    gsg->_glDeleteProgram(_glsl_program);
    _glsl_program = 0;
  }
  if (_glsl_vshader != 0) {
    gsg->_glDeleteShader(_glsl_vshader);
    _glsl_vshader = 0;
  }
  if (_glsl_fshader != 0) {
    gsg->_glDeleteShader(_glsl_fshader);
    _glsl_fshader = 0;
  }
  if (_glsl_gshader != 0) {
    gsg->_glDeleteShader(_glsl_gshader);
    _glsl_gshader = 0;
  }
  
  gsg->report_my_gl_errors();
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::bind
//       Access: Public
//  Description: This function is to be called to enable a new
//               shader.  It also initializes all of the shader's
//               input parameters.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
bind(GSG *gsg, bool reissue_parameters) {
  _last_gsg = gsg;

  // GLSL shaders need to be bound before passing parameters.
  if (_shader->get_language() == Shader::SL_GLSL && !_shader->get_error_flag()) {
    gsg->_glUseProgram(_glsl_program);
  }

  if (reissue_parameters) {
    // Pass in k-parameters and transform-parameters
    issue_parameters(gsg, Shader::SSD_general);
  }

#if defined(HAVE_CG) && !defined(OPENGLES)
  if (_cg_context != 0) {
    // Bind the shaders.
    if (_cg_vprogram != 0) {
      cgGLEnableProfile(cgGetProgramProfile(_cg_vprogram));
      cgGLBindProgram(_cg_vprogram);
    }
    if (_cg_fprogram != 0) {
      cgGLEnableProfile(cgGetProgramProfile(_cg_fprogram));
      cgGLBindProgram(_cg_fprogram);
    }
    if (_cg_gprogram != 0) {
      cgGLEnableProfile(cgGetProgramProfile(_cg_gprogram));
      cgGLBindProgram(_cg_gprogram);
    }

    cg_report_errors();
  }
#endif

  gsg->report_my_gl_errors();
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::unbind
//       Access: Public
//  Description: This function disables a currently-bound shader.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
unbind(GSG *gsg) {
  _last_gsg = gsg;

#if defined(HAVE_CG) && !defined(OPENGLES)
  if (_cg_context != 0) {
    if (_cg_vprogram != 0) {
      cgGLDisableProfile(cgGetProgramProfile(_cg_vprogram));
    }
    if (_cg_fprogram != 0) {
      cgGLDisableProfile(cgGetProgramProfile(_cg_fprogram));
    }
    if (_cg_gprogram != 0) {
      cgGLDisableProfile(cgGetProgramProfile(_cg_gprogram));
    }

    cg_report_errors();
  }
#endif
  
  if (_shader->get_language() == Shader::SL_GLSL) {
    gsg->_glUseProgram(0);
  }
  gsg->report_my_gl_errors();
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

  // Iterate through _ptr parameters
  for (int i=0; i<(int)_shader->_ptr_spec.size(); i++) {
    if(altered & (_shader->_ptr_spec[i]._dep[0] | _shader->_ptr_spec[i]._dep[1])){
      if (_shader->get_language() == Shader::SL_GLSL){ 
        GLCAT.error() << _shader->_ptr_spec[i]._id._name << ": parameter type supported only in Cg\n";
        release_resources(gsg);
        return;
      }
#if defined(HAVE_CG) && !defined(OPENGLES)
      else if (_shader->get_language() == Shader::SL_Cg) {
        const Shader::ShaderPtrSpec& _ptr = _shader->_ptr_spec[i];
        Shader::ShaderPtrData* _ptr_data = 
          const_cast< Shader::ShaderPtrData*>(gsg->fetch_ptr_parameter(_ptr));
        
        if (_ptr_data == NULL){ //the input is not contained in ShaderPtrData
          release_resources(gsg);
          return;
        }
        //check if the data must be shipped to the GPU
        /*if (!_ptr_data->_updated)
          continue;
        _ptr_data->_updated = false;*/

        //Check if the size of the shader input and _ptr_data match
        int input_size = _ptr._dim[0] * _ptr._dim[1] * _ptr._dim[2];

        // dimension is negative only if the parameter had the (deprecated)k_ prefix.
        if ((input_size != _ptr_data->_size) && (_ptr._dim[0] > 0)) { 
          GLCAT.error() << _ptr._id._name << ": incorrect number of elements, expected " 
            <<  input_size <<" got " <<  _ptr_data->_size << "\n";
          release_resources(gsg);
          return;
        }
        CGparameter p = _cg_parameter_map[_ptr._id._seqno];
        
        switch(_ptr_data->_type) {
          case Shader::SPT_float:
            switch(_ptr._info._class) {
              case Shader::SAC_scalar: cgSetParameter1fv(p,(float*)_ptr_data->_ptr); continue;
              case Shader::SAC_vector:
                switch(_ptr._info._type) {
                  case Shader::SAT_vec1: cgSetParameter1fv(p,(float*)_ptr_data->_ptr); continue;
                  case Shader::SAT_vec2: cgSetParameter2fv(p,(float*)_ptr_data->_ptr); continue;
                  case Shader::SAT_vec3: cgSetParameter3fv(p,(float*)_ptr_data->_ptr); continue;
                  case Shader::SAT_vec4: cgSetParameter4fv(p,(float*)_ptr_data->_ptr); continue;
                }
              case Shader::SAC_matrix: cgGLSetMatrixParameterfc(p,(float*)_ptr_data->_ptr); continue;
              case Shader::SAC_array: {
                switch(_ptr._info._subclass) {
                  case Shader::SAC_scalar: 
                    cgGLSetParameterArray1f(p,0,_ptr._dim[0],(float*)_ptr_data->_ptr); continue;
                  case Shader::SAC_vector:
                    switch(_ptr._dim[2]) {
                      case 1: cgGLSetParameterArray1f(p,0,_ptr._dim[0],(float*)_ptr_data->_ptr); continue;
                      case 2: cgGLSetParameterArray2f(p,0,_ptr._dim[0],(float*)_ptr_data->_ptr); continue;
                      case 3: cgGLSetParameterArray3f(p,0,_ptr._dim[0],(float*)_ptr_data->_ptr); continue;
                      case 4: cgGLSetParameterArray4f(p,0,_ptr._dim[0],(float*)_ptr_data->_ptr); continue;
                    }
                  case Shader::SAC_matrix:
                    cgGLSetMatrixParameterArrayfc(p,0,_ptr._dim[0],(float*)_ptr_data->_ptr); continue;
                }
              } 
            }
          case Shader::SPT_double:
            switch(_ptr._info._class) {
              case Shader::SAC_scalar: cgSetParameter1dv(p,(double*)_ptr_data->_ptr); continue;
              case Shader::SAC_vector:
                switch(_ptr._info._type) {
                  case Shader::SAT_vec1: cgSetParameter1dv(p,(double*)_ptr_data->_ptr); continue;
                  case Shader::SAT_vec2: cgSetParameter2dv(p,(double*)_ptr_data->_ptr); continue;
                  case Shader::SAT_vec3: cgSetParameter3dv(p,(double*)_ptr_data->_ptr); continue;
                  case Shader::SAT_vec4: cgSetParameter4dv(p,(double*)_ptr_data->_ptr); continue;
                }
              case Shader::SAC_matrix: cgGLSetMatrixParameterdc(p,(double*)_ptr_data->_ptr); continue;
              case Shader::SAC_array: {
                switch(_ptr._info._subclass) {
                  case Shader::SAC_scalar: 
                    cgGLSetParameterArray1d(p,0,_ptr._dim[0],(double*)_ptr_data->_ptr); continue;
                  case Shader::SAC_vector:
                    switch(_ptr._dim[2]) {
                      case 1: cgGLSetParameterArray1d(p,0,_ptr._dim[0],(double*)_ptr_data->_ptr); continue;
                      case 2: cgGLSetParameterArray2d(p,0,_ptr._dim[0],(double*)_ptr_data->_ptr); continue;
                      case 3: cgGLSetParameterArray3d(p,0,_ptr._dim[0],(double*)_ptr_data->_ptr); continue;
                      case 4: cgGLSetParameterArray4d(p,0,_ptr._dim[0],(double*)_ptr_data->_ptr); continue;
                    }
                  case Shader::SAC_matrix:
                    cgGLSetMatrixParameterArraydc(p,0,_ptr._dim[0],(double*)_ptr_data->_ptr); continue;
                }
              } 
            }
          default: GLCAT.error() << _ptr._id._name << ":" << "unrecognized parameter type\n"; 
                   release_resources(gsg); 
                   return;
        }
      }
#endif
    }
  }

  //FIXME: this could be much faster if we used deferred parameter setting.

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
#if defined(HAVE_CG) && !defined(OPENGLES)
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
#if defined(HAVE_CG) && !defined(OPENGLES)
  cg_report_errors();
#endif

  gsg->report_my_gl_errors();
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::disable_shader_vertex_arrays
//       Access: Public
//  Description: Disable all the vertex arrays used by this shader.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
disable_shader_vertex_arrays(GSG *gsg) {
  _last_gsg = gsg;
  if (!valid()) {
    return;
  }

  if (_shader->get_language() == Shader::SL_GLSL) {
    for (int i=0; i<(int)_shader->_var_spec.size(); i++) {
      gsg->_glDisableVertexAttribArray(i);
    }
  }
#if defined(HAVE_CG) && !defined(OPENGLES)
  else if (_shader->get_language() == Shader::SL_Cg) {
    for (int i=0; i<(int)_shader->_var_spec.size(); i++) {
      CGparameter p = _cg_parameter_map[_shader->_var_spec[i]._id._seqno];
      if (p == 0) continue;
      cgGLDisableClientState(p);
    }
    cg_report_errors();
  }
#endif

  gsg->report_my_gl_errors();
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
  if (!valid()) {
    return true;
  }
#if defined(HAVE_CG) && !defined(OPENGLES)
  cg_report_errors();
#endif

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
#if defined(HAVE_CG) && !defined(OPENGLES)
      if (_shader->get_language() == Shader::SL_Cg) {
        if (_cg_parameter_map[_shader->_var_spec[i]._id._seqno] == 0) {
          continue;
        }
      }
#endif
      
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
        if (_shader->get_language() == Shader::SL_GLSL) {
#ifndef OPENGLES_2
          glEnableClientState(GL_VERTEX_ARRAY);
#endif
          gsg->_glEnableVertexAttribArray(i);
          gsg->_glVertexAttribPointer(i, num_values, gsg->get_numeric_type(numeric_type),
                                GL_FALSE, stride, client_pointer + start);
#ifndef OPENGLES_2
          glDisableClientState(GL_VERTEX_ARRAY);
#endif
#if defined(HAVE_CG) && !defined(OPENGLES)
        } else if (_shader->get_language() == Shader::SL_Cg) {
          CGparameter p = _cg_parameter_map[_shader->_var_spec[i]._id._seqno];
          cgGLSetParameterPointer(p,
                                  num_values, gsg->get_numeric_type(numeric_type),
                                  stride, client_pointer + start);
          cgGLEnableClientState(p);
#endif
        }
      }
#if defined(HAVE_CG) && !defined(OPENGLES)
      else if (_shader->get_language() == Shader::SL_Cg) {
        CGparameter p = _cg_parameter_map[_shader->_var_spec[i]._id._seqno];
        cgGLDisableClientState(p);
      }
#endif
    }
  }

#if defined(HAVE_CG) && !defined(OPENGLES)
  cg_report_errors();
#endif
  gsg->report_my_gl_errors();
  
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

#ifndef OPENGLES_2
  for (int i=0; i<(int)_shader->_tex_spec.size(); i++) {
    if (_shader->get_language() == Shader::SL_GLSL) {
      if (_shader->_tex_spec[i]._name == 0) {
        gsg->_glActiveTexture(GL_TEXTURE0 + _shader->_tex_spec[i]._stage);
      } else {
        gsg->_glActiveTexture(GL_TEXTURE0 + _shader->_tex_spec[i]._stage + _stage_offset);
      }
#if defined(HAVE_CG) && !defined(OPENGLES)
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
    GLP(BindTexture)(GL_TEXTURE_1D, 0);
#endif  // OPENGLES
    GLP(BindTexture)(GL_TEXTURE_2D, 0);
#ifndef OPENGLES_1
    if (gsg->_supports_3d_texture) {
      GLP(BindTexture)(GL_TEXTURE_3D, 0);
    }
#endif  // OPENGLES_1
#ifndef OPENGLES
    if (gsg->_supports_2d_texture_array) {
      GLP(BindTexture)(GL_TEXTURE_2D_ARRAY_EXT, 0);
    }
#endif
    if (gsg->_supports_cube_map) {
      GLP(BindTexture)(GL_TEXTURE_CUBE_MAP, 0);
    }
    // This is probably faster - but maybe not as safe?
    // cgGLDisableTextureParameter(p);
  }
#endif  // OPENGLES_2
  _stage_offset = 0;

#if defined(HAVE_CG) && !defined(OPENGLES)
  cg_report_errors();
#endif

  gsg->report_my_gl_errors();
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
#if defined(HAVE_CG) && !defined(OPENGLES)
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
    gsg->apply_texture(tc);

    if (_shader->get_language() == Shader::SL_GLSL) {
      GLint p = _shader->_glsl_parameter_map[_shader->_tex_spec[i]._id._seqno];
      gsg->_glUniform1i(p, texunit);
    }

    if (!gsg->update_texture(tc, false)) {
      continue;
    }
  }

#if defined(HAVE_CG) && !defined(OPENGLES)
  cg_report_errors();
#endif

  gsg->report_my_gl_errors();
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::glsl_report_shader_errors
//       Access: Private
//  Description: This subroutine prints the infolog for a shader.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
glsl_report_shader_errors(GSG *gsg, unsigned int shader) {
  char *info_log;
  GLint length = 0;
  GLint num_chars  = 0;

  gsg->_glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

  if (length > 0) {
    info_log = (char *) malloc(length);
    gsg->_glGetShaderInfoLog(shader, length, &num_chars, info_log);
    if (strcmp(info_log, "Success.\n") != 0) {
      GLCAT.error(false) << info_log << "\n";
    }
  }
  delete[] info_log;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::glsl_report_program_errors
//       Access: Private
//  Description: This subroutine prints the infolog for a program.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
glsl_report_program_errors(GSG *gsg, unsigned int program) {
  char *info_log;
  GLint length = 0;
  GLint num_chars  = 0;

  gsg->_glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);

  if (length > 0) {
    info_log = (char *) malloc(length);
    gsg->_glGetProgramInfoLog(program, length, &num_chars, info_log);
    if (strcmp(info_log, "Success.\n") != 0) {
      GLCAT.error(false) << info_log << "\n";
    }
  }
  delete[] info_log;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::glsl_compile_entry_point
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
unsigned int CLP(ShaderContext)::
glsl_compile_entry_point(GSG *gsg, Shader::ShaderType type) {
  unsigned int handle;
  switch (type) {
    case Shader::ST_vertex:
      handle = gsg->_glCreateShader(GL_VERTEX_SHADER);
      break;
    case Shader::ST_fragment:
      handle = gsg->_glCreateShader(GL_FRAGMENT_SHADER);
      break;
    case Shader::ST_geometry:
      handle = gsg->_glCreateShader(GL_GEOMETRY_SHADER_EXT);
      break;
    default:
      return 0;
  }
  if (!handle) {
    return 0;
  }
  string text_str = _shader->get_text(type);
  const char* text = text_str.c_str();
  gsg->_glShaderSource(handle, 1, &text, NULL);
  gsg->_glCompileShader(handle);
  GLint status;
  gsg->_glGetShaderiv(handle, GL_COMPILE_STATUS, &status);
  if (status != GL_TRUE) {
    GLCAT.error() << "An error occurred while compiling shader!\n";
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
  _glsl_program = gsg->_glCreateProgram();
  if (!_glsl_program) return false;

  if (!_shader->get_text(Shader::ST_vertex).empty()) {
    _glsl_vshader = glsl_compile_entry_point(gsg, Shader::ST_vertex);
    if (!_glsl_vshader) return false;
    gsg->_glAttachShader(_glsl_program, _glsl_vshader);
  }

  if (!_shader->get_text(Shader::ST_fragment).empty()) {
    _glsl_fshader = glsl_compile_entry_point(gsg, Shader::ST_fragment);
    if (!_glsl_fshader) return false;
    gsg->_glAttachShader(_glsl_program, _glsl_fshader);
  }

  if (!_shader->get_text(Shader::ST_geometry).empty()) {
    _glsl_gshader = glsl_compile_entry_point(gsg, Shader::ST_geometry);
    if (!_glsl_gshader) return false;
    gsg->_glAttachShader(_glsl_program, _glsl_gshader);
    
#ifdef OPENGLES
    nassertr(false, false); // OpenGL ES has no geometry shaders.
#else
    // Set the vertex output limit to the maximum
    nassertr(gsg->_glProgramParameteri != NULL, false);
    GLint max_vertices;
    glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT, &max_vertices);
    gsg->_glProgramParameteri(_glsl_program, GL_GEOMETRY_VERTICES_OUT_EXT, max_vertices); 
#endif
  }
  
  // There might be warnings. Only report them for one shader program.
  if (_glsl_vshader != 0) {
    glsl_report_shader_errors(gsg, _glsl_vshader);
  } else if (_glsl_fshader != 0) {
    glsl_report_shader_errors(gsg, _glsl_fshader);
  } else if (_glsl_gshader != 0) {
    glsl_report_shader_errors(gsg, _glsl_gshader);
  }

  gsg->_glLinkProgram(_glsl_program);

  GLint status;
  gsg->_glGetProgramiv(_glsl_program, GL_LINK_STATUS, &status);
  if (status != GL_TRUE) {
    GLCAT.error() << "An error occurred while linking shader program!\n";
    glsl_report_program_errors(gsg, _glsl_program);
    return false;
  }

  gsg->report_my_gl_errors();
  return true;
}

#endif  // OPENGLES_1

