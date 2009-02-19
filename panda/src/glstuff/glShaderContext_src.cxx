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


////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::Constructor
//       Access: Public
//  Description: xyz
////////////////////////////////////////////////////////////////////
CLP(ShaderContext)::
CLP(ShaderContext)(Shader *s, GSG *gsg) : ShaderContext(s) {
#ifdef HAVE_CG
  if (s->get_header() == "//Cg") {
    
    // Ask the shader to compile itself for us and 
    // to give us the resulting Cg program objects.

    if (!s->cg_compile_for(gsg->_shader_caps,
                           _cg_context,
                           _cg_vprogram,
                           _cg_fprogram, 
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
      release_resources();
    }
    cgGLLoadProgram(_cg_fprogram);
    CGerror ferror = cgGetError();
    if (ferror != CG_NO_ERROR) {
      const char *str = (const char *)GLP(GetString)(GL_PROGRAM_ERROR_STRING_ARB);
      GLCAT.error() << "Could not load Cg fragment program:" << s->get_filename() << " (" << 
        cgGetProfileString(cgGetProgramProfile(_cg_fprogram)) << " " << str << ")\n";
      release_resources();
    }
    if (glGetError() != GL_NO_ERROR) {
      GLCAT.error() << "GL error in ShaderContext constructor\n";
    }
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::Destructor
//       Access: Public
//  Description: xyz
////////////////////////////////////////////////////////////////////
CLP(ShaderContext)::
~CLP(ShaderContext)() {
  release_resources();
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::release_resources
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
    _cg_parameter_map.clear();
  }
  if (glGetError() != GL_NO_ERROR) {
    GLCAT.error() << "GL error in ShaderContext destructor\n";
  }
#endif
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
#ifdef HAVE_CG
  if (_cg_context != 0) {

    // Pass in k-parameters and transform-parameters
    issue_parameters(gsg, Shader::SSD_general);

    // Bind the shaders.
    cgGLEnableProfile(cgGetProgramProfile(_cg_vprogram));
    cgGLBindProgram(_cg_vprogram);
    cgGLEnableProfile(cgGetProgramProfile(_cg_fprogram));
    cgGLBindProgram(_cg_fprogram);
    cg_report_errors();
    if (glGetError() != GL_NO_ERROR) {
      GLCAT.error() << "GL error in ShaderContext::bind\n";
    }
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::unbind
//       Access: Public
//  Description: This function disables a currently-bound shader.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
unbind() {
#ifdef HAVE_CG
  if (_cg_context != 0) {
    cgGLDisableProfile(cgGetProgramProfile(_cg_vprogram));
    cgGLDisableProfile(cgGetProgramProfile(_cg_fprogram));
    cg_report_errors();
    if (glGetError() != GL_NO_ERROR) {
      GLCAT.error() << "GL error in ShaderContext::unbind\n";
    }
  }
#endif
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
#ifdef HAVE_CG
  PStatTimer timer(gsg->_draw_set_state_shader_parameters_pcollector);
  if (_cg_context == 0) {
    return;
  }

  for (int i=0; i<(int)_shader->_mat_spec.size(); i++) {
    if (altered & (_shader->_mat_spec[i]._dep[0] | _shader->_mat_spec[i]._dep[1])) {
      CGparameter p = _cg_parameter_map[_shader->_mat_spec[i]._id._seqno];
      const LMatrix4f *val = gsg->fetch_specified_value(_shader->_mat_spec[i], altered);
      if (val) {
        const float *data = val->get_data();
        switch (_shader->_mat_spec[i]._piece) {
        case Shader::SMP_whole: cgGLSetMatrixParameterfc(p, data); break;
        case Shader::SMP_transpose: cgGLSetMatrixParameterfr(p, data); break;
        case Shader::SMP_row0: cgGLSetParameter4fv(p, data+ 0); break;
        case Shader::SMP_row1: cgGLSetParameter4fv(p, data+ 4); break;
        case Shader::SMP_row2: cgGLSetParameter4fv(p, data+ 8); break;
        case Shader::SMP_row3: cgGLSetParameter4fv(p, data+12); break;
        case Shader::SMP_col0: cgGLSetParameter4f(p, data[0], data[4], data[ 8], data[12]); break;
        case Shader::SMP_col1: cgGLSetParameter4f(p, data[1], data[5], data[ 9], data[13]); break;
        case Shader::SMP_col2: cgGLSetParameter4f(p, data[2], data[6], data[10], data[14]); break;
        case Shader::SMP_col3: cgGLSetParameter4f(p, data[3], data[7], data[11], data[15]); break;
        }
      }
    }
  }
  cg_report_errors();
  if (glGetError() != GL_NO_ERROR) {
    GLCAT.error() << "GL error in ShaderContext::issue_parameters\n";
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::disable_shader_vertex_arrays
//       Access: Public
//  Description: Disable all the vertex arrays used by this shader.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
disable_shader_vertex_arrays(GSG *gsg) {
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
  if (prev) prev->disable_shader_vertex_arrays(gsg);
#ifdef HAVE_CG
  if (_cg_context == 0) {
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
#ifdef HAVE_CG
  if (_cg_context == 0) {
    return;
  }

  for (int i=0; i<(int)_shader->_tex_spec.size(); i++) {
    CGparameter p = _cg_parameter_map[_shader->_tex_spec[i]._id._seqno];
    if (p == 0) continue;
    int texunit = cgGetParameterResourceIndex(p);
    gsg->_glActiveTexture(GL_TEXTURE0 + texunit);
    GLP(Disable)(GL_TEXTURE_1D);
    GLP(Disable)(GL_TEXTURE_2D);
    if (gsg->_supports_3d_texture) {
      GLP(Disable)(GL_TEXTURE_3D);
    }
    if (gsg->_supports_cube_map) {
      GLP(Disable)(GL_TEXTURE_CUBE_MAP);
    }
    // This is probably faster - but maybe not as safe?
    // cgGLDisableTextureParameter(p);
  }
  cg_report_errors();
  if (glGetError() != GL_NO_ERROR) {
    GLCAT.error() << "GL error in ShaderContext::disable_shader_texture_bindings\n";
  }
#endif
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
  if (prev) {
    prev->disable_shader_texture_bindings(gsg);
  }

#ifdef HAVE_CG
  if (_cg_context == 0) {
    return;
  }

  // We get the TextureAttrib directly from the _target_rs, not the
  // filtered TextureAttrib in _target_texture.
  const TextureAttrib *texattrib = DCAST(TextureAttrib, gsg->_target_rs->get_attrib_def(TextureAttrib::get_class_slot()));
  nassertv(texattrib != (TextureAttrib *)NULL);

  for (int i = 0; i < (int)_shader->_tex_spec.size(); ++i) {
    CGparameter p = _cg_parameter_map[_shader->_tex_spec[i]._id._seqno];
    if (p == 0) {
      continue;
    }

    Texture *tex = 0;
    InternalName *id = _shader->_tex_spec[i]._name;
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

    int texunit = cgGetParameterResourceIndex(p);
    gsg->_glActiveTexture(GL_TEXTURE0 + texunit);

    GLenum target = gsg->get_texture_target(tex->get_texture_type());
    if (target == GL_NONE) {
      // Unsupported texture mode.
      continue;
    }
    GLP(Enable)(target);

    gsg->apply_texture(tc);
    if (!gsg->update_texture(tc, false)) {
      GLP(Disable)(target);
      continue;
    }
  }
  cg_report_errors();
  if (glGetError() != GL_NO_ERROR) {
    GLCAT.error() << "GL error in ShaderContext::update_shader_texture_bindings\n";
  }
#endif
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
