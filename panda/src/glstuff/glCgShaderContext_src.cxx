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

#include "pStatGPUTimer.h"

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
  _cg_program = 0;
  _glsl_program = 0;
  _has_divisor = false;
  _color_attrib_index = CA_color;
  _transform_table_param = 0;
  _slider_table_param = 0;

  nassertv(s->get_language() == Shader::SL_Cg);

  // Get a Cg context for this GSG.
  CGcontext context = glgsg->_cg_context;
  if (context == 0) {
    // The GSG doesn't have a Cg context yet.  Create one.
    glgsg->_cg_context = context = cgCreateContext();

#if CG_VERSION_NUM >= 3100
    // This just sounds like a good thing to do.
    cgGLSetContextGLSLVersion(context, cgGLDetectGLSLVersion());
    if (glgsg->_shader_caps._active_vprofile == CG_PROFILE_GLSLV) {
      cgGLSetContextOptimalOptions(context, CG_PROFILE_GLSLC);
    }
#endif
  }

  // Ask the shader to compile itself for us and
  // to give us the resulting Cg program objects.
  if (!s->cg_compile_for(_glgsg->_shader_caps, context,
                         _cg_program, _cg_parameter_map)) {
    return;
  }

  _transform_table_param = cgGetNamedParameter(_cg_program, "tbl_transforms");
  if (_transform_table_param) {
    _transform_table_size = cgGetArraySize(_transform_table_param, 0);
  }

  _slider_table_param = cgGetNamedParameter(_cg_program, "tbl_sliders");
  if (_slider_table_param) {
    _slider_table_size = cgGetArraySize(_slider_table_param, 0);
  }

  if (GLCAT.is_debug()) {
    GLCAT.debug()
      << "Loading Cg shader " << s->get_filename() << "\n";
  }

  // Load the program.
  if (_cg_program == 0) {
    const char *str = cgGetErrorString(cgGetError());
    GLCAT.error()
      << "Could not combine Cg program: " << s->get_filename()
      << " (" << str << ")\n";
    release_resources();

  } else {
    cgGLLoadProgram(_cg_program);
    CGerror error = cgGetError();
    if (error != CG_NO_ERROR) {
      const char *str = cgGetErrorString(error);
      GLCAT.error()
        << "Could not load program: " << s->get_filename()
        << " (" << str << ")\n";
      release_resources();
    }
  }

  if (_cg_program != 0) {
    if (cgGetProgramProfile(_cg_program) == CG_PROFILE_GLSLC) {
      _glsl_program = cgGLGetProgramID(_cg_program);

      // Sometimes it fails to link, and Cg fails to propagate the error.
      GLint link_status;
      _glgsg->_glGetProgramiv(_glsl_program, GL_LINK_STATUS, &link_status);
      if (link_status != GL_TRUE) {
        release_resources();
      }
    }
  }

  if (_cg_program == 0) {
    return;
  }

  // We don't use cgGLSetParameterPointer to set the vertex attributes any
  // longer, since it is buggy on non-NVIDIA hardware and doesn't allow explicit
  // control over some parameters.  Instead, we have to figure out ourselves how
  // to map the input varyings to OpenGL vertex attributes.
  //
  // We use positive indices to indicate generic vertex attributes, and negative
  // indices to indicate conventional vertex attributes (ie. glVertexPointer).
  int nvarying = _shader->_var_spec.size();
  for (int i = 0; i < nvarying; ++i) {
    Shader::ShaderVarSpec &bind = _shader->_var_spec[i];
    CGparameter p = _cg_parameter_map[i];
    if (p == 0) {
      bind._id._seqno = CA_unknown;
      continue;
    }

    GLint loc = CA_unknown;
    CGresource res = cgGetParameterResource(p);

    if (cgGetParameterBaseResource(p) == CG_ATTR0) {
      // The Cg toolkit claims that it is bound to a generic vertex attribute.
      if (_glsl_program != 0) {
        // This is where the Cg glslv compiler lies, making the stupid assumption
        // that we're using an NVIDIA card where generic attributes are aliased
        // with conventional vertex attributes.  Instead, it always uses
        // conventional attributes in this case.  Correct this.
        int index = cgGetParameterResourceIndex(p);
        switch (index) {
        case 0:  // gl_Vertex
          loc = CA_vertex;
          break;
        case 2:  // gl_Normal
          loc = CA_normal;
          break;
        case 3:  // gl_Color
          loc = CA_color;
          break;
        case 4:  // gl_SecondaryColor
          loc = CA_secondary_color;
          break;
        case 1:  // glWeightPointerARB?
        case 5:  // gl_FogCoord
        case 6:  // PSIZE?
        case 7:  // BLENDINDICES
          GLCAT.error()
            << "Cg varying " << cgGetParameterName(p) << " is bound to "
               "unrecognized attribute " << index << "\n";
          loc = CA_unknown;
          break;
        default:
          loc = CA_texcoord + (index - 8);
          break;
        }
      } else {
        loc = cgGetParameterResourceIndex(p);

        if (loc != 0 && bind._id._name == "vtx_position") {
          // We really have to bind the vertex position to attribute 0, since
          // OpenGL will hide the model if attribute 0 is not enabled, and we
          // can only ever be sure that vtx_position is bound.
          GLCAT.warning()
            << "CG varying vtx_position is bound to generic attribute " << loc
            << " instead of 0.  Use ATTR0 semantic to prevent this.\n";
        }
      }

    } else if (res == CG_GLSL_ATTRIB || _glsl_program != 0) {
      // With cg-glsl-version 130 and higher, no conventional attributes are
      // used, but it instead uses specially named variables.
      // A bit of guesswork is involved here; Cg seems to mostly use the
      // semantics as attribute names in GLSL, with a few exceptions.
      const char *attribname = NULL;
      switch (res) {
      case CG_POSITION0:
        attribname = "cg_Vertex";
        break;
      case CG_NORMAL0:
        attribname = "NORMAL";
        break;
      case CG_COLOR0:
      case CG_DIFFUSE0:
        attribname = "COLOR";
        break;
      case CG_COLOR1:
      case CG_SPECULAR0:
        attribname = "SPECULAR";
        break;
      default:
        // Everything else appears to be named after the semantic string.
        attribname = cgGetParameterSemantic(p);
      }
      loc = _glgsg->_glGetAttribLocation(_glsl_program, attribname);

      if (bind._id._name == "vtx_color") {
        _color_attrib_index = loc;
      }

      if (loc == -1) {
        const char *resource = cgGetParameterResourceName(p);
        if (!resource) {
          resource = "unknown";
        }
        GLCAT.error()
          << "Could not find Cg varying " << cgGetParameterName(p);
        if (attribname) {
          GLCAT.error(false) << " : " << attribname;
        }
        GLCAT.error(false) << " (" << resource << ") in the compiled GLSL program.\n";

      } else if (loc != 0 && bind._id._name == "vtx_position") {
        // We really have to bind the vertex position to attribute 0, since
        // OpenGL will hide the model if attribute 0 is not enabled, and we
        // can only ever be sure that vtx_position is bound.
        GLCAT.warning()
          << "CG varying vtx_position is bound to generic attribute " << loc
          << "instead of 0.  Use ATTR0 semantic to prevent this.\n";
      }

    } else if (cgGetParameterBaseResource(p) == CG_TEXCOORD0) {
      // A conventional texture coordinate set.
      loc = CA_texcoord + cgGetParameterResourceIndex(p);

    } else {
      // Some other conventional vertex attribute.
      switch (res) {
      case CG_POSITION0:
        loc = CA_vertex;
        break;
      case CG_NORMAL0:
        loc = CA_normal;
        break;
      case CG_COLOR0:
      case CG_DIFFUSE0:
        loc = CA_color;
        break;
      case CG_COLOR1:
      case CG_SPECULAR0:
        loc = CA_secondary_color;
        break;
      default:
        GLCAT.error()
          << "Cg varying " << cgGetParameterName(p);
        if (cgGetParameterSemantic(p)) {
          GLCAT.error(false) << " : " << cgGetParameterSemantic(p);
        }
        GLCAT.error(false) << " has an unrecognized resource";
        if (cgGetParameterResourceName(p)) {
          GLCAT.error(false) << " (" << cgGetParameterResourceName(p) << ")";
        }
        GLCAT.error(false) << ".\n";
        loc = CA_unknown;
      }
    }

#ifndef NDEBUG
    if (GLCAT.is_debug()) {
      GLCAT.debug()
        << "Cg varying " << cgGetParameterName(p);

      const char *semantic = cgGetParameterSemantic(p);
      if (semantic) {
        GLCAT.debug(false) << " : " << semantic;
      }

      if (loc == CA_unknown) {
        GLCAT.debug(false)
          << " is not bound to a vertex attribute\n";

      } else if (loc >= 0) {
        GLCAT.debug(false)
          << " is bound to generic attribute " << loc << "\n";

      } else {
        const char *resource = cgGetParameterResourceName(p);
        if (!resource) {
          resource = "unknown";
        }
        GLCAT.debug(false)
          << " is bound to a conventional attribute (" << resource << ")\n";
      }
    }
    if (loc == CA_unknown) {
      // Suggest fix to developer.
      GLCAT.error() << "Try using a different semantic.\n";
    }
#endif

    // Abuse the seqno field to store the GLSL attribute location.
    bind._id._seqno = loc;
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
  if (_cg_program != 0) {
    cgDestroyProgram(_cg_program);
    _cg_program = 0;
  }
  _cg_parameter_map.clear();
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
  if (_cg_program != 0) {
    if (reissue_parameters) {
      // Pass in k-parameters and transform-parameters
      issue_parameters(Shader::SSD_general);
    }

    // Bind the shaders.
    cgGLEnableProgramProfiles(_cg_program);
    cgGLBindProgram(_cg_program);

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
  if (_cg_program != 0) {
    int num_domains = cgGetNumProgramDomains(_cg_program);
    for (int i = 0; i < num_domains; ++i) {
      CGprofile profile = cgGetProgramDomainProfile(_cg_program, i);
      cgGLUnbindProgram(profile);
      cgGLDisableProfile(profile);
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
  //PStatGPUTimer timer(_glgsg, _glgsg->_draw_set_state_shader_parameters_pcollector);

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
        case Shader::SAC_scalar:
          cgSetParameter1fv(p, (float*)ptr_data->_ptr);
          continue;

        case Shader::SAC_vector:
          switch (_ptr._info._type) {
          case Shader::SAT_vec1:
            cgSetParameter1fv(p, (float*)ptr_data->_ptr);
            continue;
          case Shader::SAT_vec2:
            cgSetParameter2fv(p, (float*)ptr_data->_ptr);
            continue;
          case Shader::SAT_vec3:
            cgSetParameter3fv(p, (float*)ptr_data->_ptr);
            continue;
          case Shader::SAT_vec4:
            cgSetParameter4fv(p, (float*)ptr_data->_ptr);
            continue;
          default:
            nassertd(false) continue;
          }
          continue;

        case Shader::SAC_matrix:
          cgGLSetMatrixParameterfc(p, (float*)ptr_data->_ptr);
          continue;

        case Shader::SAC_array:
          switch (_ptr._info._subclass) {
          case Shader::SAC_scalar:
            cgGLSetParameterArray1f(p, 0, _ptr._dim[0], (float*)ptr_data->_ptr);
            continue;
          case Shader::SAC_vector:
            switch (_ptr._dim[2]) {
            case 1: cgGLSetParameterArray1f(p, 0, _ptr._dim[0], (float*)ptr_data->_ptr); continue;
            case 2: cgGLSetParameterArray2f(p, 0, _ptr._dim[0], (float*)ptr_data->_ptr); continue;
            case 3: cgGLSetParameterArray3f(p, 0, _ptr._dim[0], (float*)ptr_data->_ptr); continue;
            case 4: cgGLSetParameterArray4f(p, 0, _ptr._dim[0], (float*)ptr_data->_ptr); continue;
            default:
              nassertd(_ptr._dim[2] > 0 && _ptr._dim[2] <= 4) continue;
            }
            continue;
          case Shader::SAC_matrix:
            cgGLSetMatrixParameterArrayfc(p, 0, _ptr._dim[0], (float*)ptr_data->_ptr);
            continue;
          default:
            nassertd(false) continue;
          }
        default:
          nassertd(false) continue;
        }

      case Shader::SPT_double:
        switch(_ptr._info._class) {
        case Shader::SAC_scalar:
          cgSetParameter1dv(p, (double*)ptr_data->_ptr);
          continue;

        case Shader::SAC_vector:
          switch (_ptr._info._type) {
          case Shader::SAT_vec1:
            cgSetParameter1dv(p, (double*)ptr_data->_ptr);
            continue;
          case Shader::SAT_vec2:
            cgSetParameter2dv(p, (double*)ptr_data->_ptr);
            continue;
          case Shader::SAT_vec3:
            cgSetParameter3dv(p, (double*)ptr_data->_ptr);
            continue;
          case Shader::SAT_vec4:
            cgSetParameter4dv(p, (double*)ptr_data->_ptr);
            continue;
          default:
            nassertd(false) continue;
          }
          continue;

        case Shader::SAC_matrix:
          cgGLSetMatrixParameterdc(p, (double*)ptr_data->_ptr);
          continue;

        case Shader::SAC_array:
          switch (_ptr._info._subclass) {
          case Shader::SAC_scalar:
            cgGLSetParameterArray1d(p, 0, _ptr._dim[0], (double*)ptr_data->_ptr);
            continue;
          case Shader::SAC_vector:
            switch (_ptr._dim[2]) {
            case 1: cgGLSetParameterArray1d(p, 0, _ptr._dim[0], (double*)ptr_data->_ptr); continue;
            case 2: cgGLSetParameterArray2d(p, 0, _ptr._dim[0], (double*)ptr_data->_ptr); continue;
            case 3: cgGLSetParameterArray3d(p, 0, _ptr._dim[0], (double*)ptr_data->_ptr); continue;
            case 4: cgGLSetParameterArray4d(p, 0, _ptr._dim[0], (double*)ptr_data->_ptr); continue;
            default:
              nassertd(_ptr._dim[2] > 0 && _ptr._dim[2] <= 4) continue;
            }
            continue;
          case Shader::SAC_matrix:
            cgGLSetMatrixParameterArraydc(p, 0, _ptr._dim[0], (double*)ptr_data->_ptr);
            continue;
          default:
            nassertd(false) continue;
          }
        default:
          nassertd(false) continue;
        }
        continue;

      case Shader::SPT_int:
        switch(_ptr._info._class) {
        case Shader::SAC_scalar:
          cgSetParameter1iv(p, (int*)ptr_data->_ptr);
          continue;
        case Shader::SAC_vector:
          switch(_ptr._info._type) {
          case Shader::SAT_vec1: cgSetParameter1iv(p, (int*)ptr_data->_ptr); continue;
          case Shader::SAT_vec2: cgSetParameter2iv(p, (int*)ptr_data->_ptr); continue;
          case Shader::SAT_vec3: cgSetParameter3iv(p, (int*)ptr_data->_ptr); continue;
          case Shader::SAT_vec4: cgSetParameter4iv(p, (int*)ptr_data->_ptr); continue;
          default:
            nassertd(false) continue;
          }
        default:
          nassertd(false) continue;
        }
      default:
        GLCAT.error() << _ptr._id._name << ":" << "unrecognized parameter type\n";
        release_resources();
        return;
      }
    }
  }

  for (int i=0; i<(int)_shader->_mat_spec.size(); i++) {
    if (altered & (_shader->_mat_spec[i]._dep[0] | _shader->_mat_spec[i]._dep[1])) {
      const LMatrix4 *val = _glgsg->fetch_specified_value(_shader->_mat_spec[i], altered);
      if (!val) continue;
      const PN_stdfloat *data = val->get_data();

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
          LMatrix3 upper3 = val->get_upper_3();
          GLfc(cgGLSetMatrixParameter)(p, upper3.get_data());
          continue;
        }
      case Shader::SMP_transpose3x3:
        {
          LMatrix3 upper3 = val->get_upper_3();
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
//     Function: CgGLShaderContext::update_transform_table
//       Access: Public
//  Description: Changes the active transform table, used for hardware
//               skinning.
////////////////////////////////////////////////////////////////////
void CLP(CgShaderContext)::
update_transform_table(const TransformTable *table) {
  LMatrix4f *matrices = (LMatrix4f *)alloca(_transform_table_size * 64);

  int i = 0;
  if (table != NULL) {
    int num_transforms = min(_transform_table_size, (long)table->get_num_transforms());
    for (; i < num_transforms; ++i) {
#ifdef STDFLOAT_DOUBLE
      LMatrix4 matrix;
      table->get_transform(i)->get_matrix(matrix);
      matrices[i] = LCAST(float, matrix);
#else
      table->get_transform(i)->get_matrix(matrices[i]);
#endif
    }
  }
  for (; i < _transform_table_size; ++i) {
    matrices[i] = LMatrix4f::ident_mat();
  }

  cgGLSetMatrixParameterArrayfc(_transform_table_param, 0,
                                _transform_table_size, (float *)matrices);
}

////////////////////////////////////////////////////////////////////
//     Function: CgGLShaderContext::update_slider_table
//       Access: Public
//  Description: Changes the active slider table, used for hardware
//               skinning.
////////////////////////////////////////////////////////////////////
void CLP(CgShaderContext)::
update_slider_table(const SliderTable *table) {
  float *sliders = (float *)alloca(_slider_table_size * 4);
  memset(sliders, 0, _slider_table_size * 4);

  if (table != NULL) {
    int num_sliders = min(_slider_table_size, (long)table->get_num_sliders());
    for (int i = 0; i < num_sliders; ++i) {
      sliders[i] = table->get_slider(i)->get_slider();
    }
  }

  cgGLSetParameterArray4f(_slider_table_param, 0, _slider_table_size, sliders);
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

  for (int i = 0; i < (int)_shader->_var_spec.size(); ++i) {
    GLint p = _shader->_var_spec[i]._id._seqno;

    if (p >= 0) {
      _glgsg->_glDisableVertexAttribArray(p);
      if (_has_divisor) {
        _glgsg->_glVertexAttribDivisor(p, 0);
      }
    } else {
#ifdef SUPPORT_FIXED_FUNCTION
      switch (p) {
      case CA_unknown:
        break;
      case CA_vertex:
        glDisableClientState(GL_VERTEX_ARRAY);
        break;
      case CA_normal:
        glDisableClientState(GL_NORMAL_ARRAY);
        break;
      case CA_color:
        glDisableClientState(GL_COLOR_ARRAY);
        break;
      case CA_secondary_color:
        glDisableClientState(GL_SECONDARY_COLOR_ARRAY);
        break;
      default:
        _glgsg->_glClientActiveTexture(GL_TEXTURE0 + (p - CA_texcoord));
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        break;
      }
#endif  // SUPPORT_FIXED_FUNCTION
    }
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
      GLint p = bind._id._seqno;

      // Don't apply vertex colors if they are disabled with a ColorAttrib.
      int num_elements, element_stride, divisor;
      bool normalized;
      if ((p != _color_attrib_index || _glgsg->_vertex_colors_enabled) &&
          _glgsg->_data_reader->get_array_info(name, array_reader,
                                               num_values, numeric_type,
                                               normalized, start, stride, divisor,
                                               num_elements, element_stride)) {
        const unsigned char *client_pointer;
        if (!_glgsg->setup_array_data(client_pointer, array_reader, force)) {
          return false;
        }
        client_pointer += start;

        // We don't use cgGLSetParameterPointer because it is very buggy and
        // limited in the options we can set.start
        GLenum type = _glgsg->get_numeric_type(numeric_type);
        if (p >= 0) {
          _glgsg->_glEnableVertexAttribArray(p);

          if (bind._integer) {
            _glgsg->_glVertexAttribIPointer(p, num_values, type,
                                            stride, client_pointer);
          } else if (numeric_type == GeomEnums::NT_packed_dabc) {
            // GL_BGRA is a special accepted value available since OpenGL 3.2.
            // It requires us to pass GL_TRUE for normalized.
            _glgsg->_glVertexAttribPointer(p, GL_BGRA, GL_UNSIGNED_BYTE,
                                           GL_TRUE, stride, client_pointer);
          } else {
            _glgsg->_glVertexAttribPointer(p, num_values, type,
                                           normalized, stride, client_pointer);
          }

          if (_glgsg->_supports_vertex_attrib_divisor) {
            _glgsg->_glVertexAttribDivisor(p, divisor);
            _has_divisor = true;
          }

        } else {
          // It's a conventional vertex attribute.  Ugh.
#ifdef SUPPORT_FIXED_FUNCTION
          switch (p) {
          case CA_unknown:
            break;

          case CA_vertex:
            glVertexPointer(num_values, type, stride, client_pointer);
            glEnableClientState(GL_VERTEX_ARRAY);
            break;

          case CA_normal:
            glNormalPointer(type, stride, client_pointer);
            glEnableClientState(GL_NORMAL_ARRAY);
            break;

          case CA_color:
            if (numeric_type == GeomEnums::NT_packed_dabc) {
              glColorPointer(GL_BGRA, GL_UNSIGNED_BYTE, stride, client_pointer);
            } else {
              glColorPointer(num_values, type, stride, client_pointer);
            }
            glEnableClientState(GL_COLOR_ARRAY);
            break;

          case CA_secondary_color:
            _glgsg->_glSecondaryColorPointer(num_values, type,
                                             stride, client_pointer);
            glEnableClientState(GL_SECONDARY_COLOR_ARRAY);
            break;

          default:
            _glgsg->_glClientActiveTexture(GL_TEXTURE0 + (p - CA_texcoord));
            glTexCoordPointer(num_values, type, stride, client_pointer);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            break;
          }
#endif  // SUPPORT_FIXED_FUNCTION
        }
      } else {
        // There is no vertex column with this name; disable the attribute array.
#ifdef SUPPORT_FIXED_FUNCTION
        if (p == 0) {
          //NOTE: if we disable attribute 0 in compatibility profile, the object
          // will disappear.  In GLSL we fix this by forcing the vertex column
          // to be at 0, but we don't have control over that with Cg.  So, we
          // work around this by just binding something silly to 0.
          // This breaks flat colors, but it's better than invisible objects?
          _glgsg->_glEnableVertexAttribArray(0);
          if (bind._integer) {
            _glgsg->_glVertexAttribIPointer(0, 4, GL_INT, 0, 0);
          } else {
            _glgsg->_glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
          }

        } else
#endif  // SUPPORT_FIXED_FUNCTION
        if (p >= 0) {
          _glgsg->_glDisableVertexAttribArray(p);

          if (p == _color_attrib_index) {
#ifdef STDFLOAT_DOUBLE
            _glgsg->_glVertexAttrib4dv(p, _glgsg->_scene_graph_color.get_data());
#else
            _glgsg->_glVertexAttrib4fv(p, _glgsg->_scene_graph_color.get_data());
#endif
          }
        } else {
#ifdef SUPPORT_FIXED_FUNCTION
          switch (p) {
          case CA_unknown:
            break;
          case CA_vertex:
            glDisableClientState(GL_VERTEX_ARRAY);
            break;
          case CA_normal:
            glDisableClientState(GL_NORMAL_ARRAY);
            break;
          case CA_color:
            glDisableClientState(GL_COLOR_ARRAY);
#ifdef STDFLOAT_DOUBLE
            glColor4dv(_glgsg->_scene_graph_color.get_data());
#else
            glColor4fv(_glgsg->_scene_graph_color.get_data());
#endif
            break;
          case CA_secondary_color:
            glDisableClientState(GL_SECONDARY_COLOR_ARRAY);
            break;
          default:
            _glgsg->_glClientActiveTexture(GL_TEXTURE0 + (p - CA_texcoord));
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            break;
          }
#endif  // SUPPORT_FIXED_FUNCTION
        }
      }
    }
  }

  if (_transform_table_param) {
    const TransformTable *table = _glgsg->_data_reader->get_transform_table();
    update_transform_table(table);
  }

  if (_slider_table_param) {
    const SliderTable *table = _glgsg->_data_reader->get_slider_table();
    update_slider_table(table);
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

  for (int i = 0; i < (int)_shader->_tex_spec.size(); ++i) {
    CGparameter p = _cg_parameter_map[_shader->_tex_spec[i]._id._seqno];
    if (p == 0) continue;

    int texunit = cgGetParameterResourceIndex(p);
    _glgsg->_glActiveTexture(GL_TEXTURE0 + texunit);

    glBindTexture(GL_TEXTURE_1D, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    if (_glgsg->_supports_3d_texture) {
      glBindTexture(GL_TEXTURE_3D, 0);
    }
    if (_glgsg->_supports_2d_texture_array) {
      glBindTexture(GL_TEXTURE_2D_ARRAY_EXT, 0);
    }
    if (_glgsg->_supports_cube_map) {
      glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }
    // This is probably faster - but maybe not as safe?
    //cgGLDisableTextureParameter(p);
  }

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
    Shader::ShaderTexSpec &spec = _shader->_tex_spec[i];
    const InternalName *id = spec._name;

    CGparameter p = _cg_parameter_map[spec._id._seqno];
    if (p == 0) {
      continue;
    }
    int texunit = cgGetParameterResourceIndex(p);

    Texture *tex = NULL;
    int view = _glgsg->get_current_tex_view_offset();
    SamplerState sampler;

    if (id != NULL) {
      tex = _glgsg->_target_shader->get_shader_input_texture(id, &sampler);

    } else {
      if (spec._stage >= texattrib->get_num_on_stages()) {
        // Apply a white texture in order to make it easier to use a shader
        // that takes a texture on a model that doesn't have a texture applied.
        _glgsg->_glActiveTexture(GL_TEXTURE0 + texunit);
        _glgsg->apply_white_texture();
        continue;
      }
      TextureStage *stage = texattrib->get_on_stage(spec._stage);
      tex = texattrib->get_on_texture(stage);
      sampler = texattrib->get_on_sampler(stage);
      view += stage->get_tex_view_offset();
    }

    if (spec._suffix != 0) {
      // The suffix feature is inefficient. It is a temporary hack.
      if (tex == 0) {
        continue;
      }
      tex = tex->load_related(spec._suffix);
    }
    if ((tex == 0) || (tex->get_texture_type() != spec._desired_type)) {
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
    _glgsg->apply_sampler(texunit, sampler, tc);
  }

  cg_report_errors();
  _glgsg->report_my_gl_errors();
}

#endif  // !OPENGLES
