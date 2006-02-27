// Filename: glShaderContext_src.cxx
// Created by: jyelon (01Sep05)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#define DEBUG_GL_SHADER 0

TypeHandle CLP(ShaderContext)::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::Constructor
//       Access: Public
//  Description: xyz
////////////////////////////////////////////////////////////////////
CLP(ShaderContext)::
CLP(ShaderContext)(ShaderExpansion *s, GSG *gsg) : ShaderContext(s) {
  string header;
  s->parse_init();
  s->parse_line(header, true, true);

  _state = false;

#ifdef HAVE_CGGL
  _cg_context = (CGcontext)0;
  _cg_profile[SHADER_type_vert] = CG_PROFILE_UNKNOWN;
  _cg_profile[SHADER_type_frag] = CG_PROFILE_UNKNOWN;
  _cg_program[SHADER_type_vert] = (CGprogram)0;
  _cg_program[SHADER_type_frag] = (CGprogram)0;

  if (header == "//Cg") {
    // Create the Cg context.
    _cg_context = cgCreateContext();
    if (_cg_context == 0) {
      release_resources();
      GLCAT.error() << "Cg not supported by this video card.\n";
      return;
    }

    // Parse any directives in the source.
    string directive;
    while (!s->parse_eof()) {
      s->parse_line(directive, true, true);
      vector_string pieces;
      tokenize(directive, pieces, " \t");
      if ((pieces.size()==4)&&(pieces[0]=="//Cg")&&(pieces[1]=="profile")) {
        suggest_cg_profile(pieces[2], pieces[3]);
      }
    }

    // Select a profile if no preferred profile specified in the source.
    if (_cg_profile[SHADER_type_vert] == CG_PROFILE_UNKNOWN) {
      _cg_profile[SHADER_type_vert] = cgGLGetLatestProfile(CG_GL_VERTEX);
    }
    if (_cg_profile[SHADER_type_frag] == CG_PROFILE_UNKNOWN) {
      _cg_profile[SHADER_type_frag] = cgGLGetLatestProfile(CG_GL_FRAGMENT);
    }

    // If we still haven't chosen a profile, give up.
    if ((_cg_profile[SHADER_type_vert] == CG_PROFILE_UNKNOWN)||
        (_cg_profile[SHADER_type_frag] == CG_PROFILE_UNKNOWN)) {
      release_resources();
      GLCAT.error() << "Cg not supported by this video card.\n";
      return;
    }

    // Compile the program.
    try_cg_compile(s, gsg);
    return;
  }
#endif

  GLCAT.error() << s->get_name() << ": unrecognized shader language " << header << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::suggest_cg_profile
//       Access: Private
//  Description: xyz
////////////////////////////////////////////////////////////////////
#ifdef HAVE_CGGL
void CLP(ShaderContext)::
suggest_cg_profile(const string &vpro, const string &fpro)
{
  // If a good profile has already been suggested, ignore suggestion.
  if ((_cg_profile[SHADER_type_vert] != CG_PROFILE_UNKNOWN)||
      (_cg_profile[SHADER_type_frag] != CG_PROFILE_UNKNOWN)) {
    return;
  }

  // Parse the suggestion. If not parseable, print error and ignore.
  _cg_profile[SHADER_type_vert] = parse_cg_profile(vpro, true);
  _cg_profile[SHADER_type_frag] = parse_cg_profile(fpro, false);
  if ((_cg_profile[SHADER_type_vert] == CG_PROFILE_UNKNOWN)||
      (_cg_profile[SHADER_type_frag] == CG_PROFILE_UNKNOWN)) {
    GLCAT.error() << "Cg: unrecognized profile name: " << vpro << " " << fpro << "\n";
    _cg_profile[SHADER_type_vert] = CG_PROFILE_UNKNOWN;
    _cg_profile[SHADER_type_frag] = CG_PROFILE_UNKNOWN;
    return;
  }

  // If the suggestion is parseable, but not supported, ignore silently.
  if ((!cgGLIsProfileSupported(_cg_profile[SHADER_type_vert]))||
      (!cgGLIsProfileSupported(_cg_profile[SHADER_type_frag]))) {
    _cg_profile[SHADER_type_vert] = CG_PROFILE_UNKNOWN;
    _cg_profile[SHADER_type_frag] = CG_PROFILE_UNKNOWN;
    return;
  }
}
#endif

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::parse_cg_profile
//       Access: Private
//  Description: xyz
////////////////////////////////////////////////////////////////////
#ifdef HAVE_CGGL
CGprofile CLP(ShaderContext)::
parse_cg_profile(const string &id, bool vertex)
{
  int nvprofiles = 4;
  int nfprofiles = 4;
  CGprofile vprofiles[] = { CG_PROFILE_ARBVP1, CG_PROFILE_VP20, CG_PROFILE_VP30, CG_PROFILE_VP40 };
  CGprofile fprofiles[] = { CG_PROFILE_ARBFP1, CG_PROFILE_FP20, CG_PROFILE_FP30, CG_PROFILE_FP40 };
  if (vertex) {
    for (int i=0; i<nvprofiles; i++) {
      if (id == cgGetProfileString(vprofiles[i])) {
        return vprofiles[i];
      }
    }
  } else {
    for (int i=0; i<nfprofiles; i++) {
      if (id == cgGetProfileString(fprofiles[i])) {
        return fprofiles[i];
      }
    }
  }
  return CG_PROFILE_UNKNOWN;
}
#endif  // HAVE_CGGL

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::try_cg_compile
//       Access: Private
//  Description: xyz
////////////////////////////////////////////////////////////////////
#ifdef HAVE_CGGL
bool CLP(ShaderContext)::
try_cg_compile(ShaderExpansion *s, GSG *gsg)
{
  cgGetError();
  _cg_program[0] =
    cgCreateProgram(_cg_context, CG_SOURCE, s->_text.c_str(),
                    _cg_profile[0], "vshader", (const char**)NULL);
  report_cg_compile_errors(s->get_name(), _cg_context, GLCAT.get_safe_ptr());

  cgGetError();
  _cg_program[1] =
    cgCreateProgram(_cg_context, CG_SOURCE, s->_text.c_str(),
                    _cg_profile[1], "fshader", (const char**)NULL);
  report_cg_compile_errors(s->get_name(), _cg_context, GLCAT.get_safe_ptr());

  if ((_cg_program[SHADER_type_vert]==0)||(_cg_program[SHADER_type_frag]==0)) {
    release_resources();
    return false;
  }

#if DEBUG_GL_SHADER
  // DEBUG: output the generated program
  {
    const char *program;
    program = cgGetProgramString (_cg_program[1], CG_COMPILED_PROGRAM);

    GLCAT.debug() << program << "\n";
  }
#endif

  // The following code is present to work around a bug in the Cg compiler.
  // It does not generate correct code for shadow map lookups when using arbfp1.
  // This is a particularly onerous limitation, given that arbfp1 is the only
  // Cg target that works on radeons.  I suspect this is an intentional
  // omission on nvidia's part.  The following code fetches the output listing,
  // detects the error, repairs the code, and resumbits the repaired code to Cg.
  if ((_cg_profile[1] == CG_PROFILE_ARBFP1) && (gsg->_supports_shadow_filter)) {
    bool shadowunit[32];
    bool anyshadow = false;
    memset(shadowunit, 0, sizeof(shadowunit));
    vector_string lines;
    tokenize(cgGetProgramString(_cg_program[1], CG_COMPILED_PROGRAM), lines, "\n");
    // figure out which texture units contain shadow maps.
    for (int lineno=0; lineno<(int)lines.size(); lineno++) {
      if (lines[lineno].compare(0,21,"#var sampler2DSHADOW ")) {
        continue;
      }
      vector_string fields;
      tokenize(lines[lineno], fields, ":");
      if (fields.size()!=5) {
        continue;
      }
      vector_string words;
      tokenize(trim(fields[2]), words, " ");
      if (words.size()!=2) {
        continue;
      }
      int unit = atoi(words[1].c_str());
      if ((unit < 0)||(unit >= 32)) {
        continue;
      }
      anyshadow = true;
      shadowunit[unit] = true;
    }
    // modify all TEX statements that use the relevant texture units.
    if (anyshadow) {
      for (int lineno=0; lineno<(int)lines.size(); lineno++) {
        if (lines[lineno].compare(0,4,"TEX ")) {
          continue;
        }
        vector_string fields;
        tokenize(lines[lineno], fields, ",");
        if ((fields.size()!=4)||(trim(fields[3]) != "2D;")) {
          continue;
        }
        vector_string texunitf;
        tokenize(trim(fields[2]), texunitf, "[]");
        if ((texunitf.size()!=3)||(texunitf[0] != "texture")||(texunitf[2]!="")) {
          continue;
        }
        int unit = atoi(texunitf[1].c_str());
        if ((unit < 0) || (unit >= 32) || (shadowunit[unit]==false)) {
          continue;
        }
        lines[lineno] = fields[0]+","+fields[1]+","+fields[2]+", SHADOW2D;";
      }
      string result = "!!ARBfp1.0\nOPTION ARB_fragment_program_shadow;\n";
      for (int lineno=1; lineno<(int)lines.size(); lineno++) {
        result += (lines[lineno] + "\n");
      }
      cgDestroyProgram(_cg_program[1]);
      _cg_program[1] =
        cgCreateProgram(_cg_context, CG_OBJECT, result.c_str(),
                        _cg_profile[1], "fshader", (const char**)NULL);
      report_cg_compile_errors(s->get_name(), _cg_context, GLCAT.get_safe_ptr());
      if (_cg_program[SHADER_type_frag]==0) {
        release_resources();
        return false;
      }
    }
  }

  bool success = true;
  CGparameter parameter;
  for (int progindex=0; progindex<2; progindex++) {
    int nvtx = _var_spec.size();
    for (parameter = cgGetFirstLeafParameter(_cg_program[progindex],CG_PROGRAM);
         parameter != 0;
         parameter = cgGetNextLeafParameter(parameter)) {
      success &= compile_cg_parameter(parameter, GLCAT.get_safe_ptr());
    }
    if ((progindex == SHADER_type_frag) && (nvtx != _var_spec.size())) {
      GLCAT.error() << "Cannot use vtx parameters in an fshader\n";
      success = false;
    }
  }
  if (!success) {
    release_resources();
    return false;
  }

  cgGLLoadProgram(_cg_program[SHADER_type_vert]);
  cgGLLoadProgram(_cg_program[SHADER_type_frag]);

  _state = true;

  return true;
}
#endif

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
#ifdef HAVE_CGGL
  if (_cg_context) {
    cgDestroyContext(_cg_context);
    _cg_context = (CGcontext)0;
    _cg_profile[SHADER_type_vert] = (CGprofile)0;
    _cg_profile[SHADER_type_frag] = (CGprofile)0;
    _cg_program[SHADER_type_vert] = (CGprogram)0;
    _cg_program[SHADER_type_frag] = (CGprogram)0;
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
#ifdef HAVE_CGGL
  if (_cg_context != 0) {

    // Pass in k-parameters and transform-parameters
    issue_parameters(gsg, true);

    // Bind the shaders.
    cgGLEnableProfile(_cg_profile[SHADER_type_vert]);
    cgGLBindProgram(_cg_program[SHADER_type_vert]);
    cgGLEnableProfile(_cg_profile[SHADER_type_frag]);
    cgGLBindProgram(_cg_program[SHADER_type_frag]);
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::unbind
//       Access: Public
//  Description: This function disables a currently-bound shader.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
unbind()
{
#ifdef HAVE_CGGL
  if (_cg_context != 0) {
    cgGLDisableProfile(_cg_profile[SHADER_type_vert]);
    cgGLDisableProfile(_cg_profile[SHADER_type_frag]);
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::issue_parameters
//       Access: Public
//  Description: This function gets called whenever the RenderState
//               or TransformState has changed, but the ShaderExpansion
//               itself has not changed.  It loads new values into the
//               shader's parameters.  The flag "all" is false if the
//               only thing that has changed is the modelview matrix.
//               In this case, only the transform-dependent parameters
//               are reloaded.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
issue_parameters(GSG *gsg, bool all)
{
#ifdef HAVE_CGGL
  if (_cg_context == 0) {
    return;
  }
  
  for (int i=0; i<(int)_mat_spec.size(); i++) {
    if (all || _mat_spec[i]._trans_dependent) {
      LMatrix4f result;
      CGparameter p = (CGparameter)(_mat_spec[i]._parameter);
      if (gsg->fetch_specified_value(_mat_spec[i], result)) {
        const float *data = result.get_data();
        switch (_mat_spec[i]._piece) {
        case SMP_whole: cgGLSetMatrixParameterfc(p, data); break;
        case SMP_transpose: cgGLSetMatrixParameterfr(p, data); break;
        case SMP_row0: cgGLSetParameter4fv(p, data+ 0); break;
        case SMP_row1: cgGLSetParameter4fv(p, data+ 4); break;
        case SMP_row2: cgGLSetParameter4fv(p, data+ 8); break;
        case SMP_row3: cgGLSetParameter4fv(p, data+12); break;
        case SMP_col0: cgGLSetParameter4f(p, data[0], data[4], data[ 8], data[12]); break;
        case SMP_col1: cgGLSetParameter4f(p, data[1], data[5], data[ 9], data[13]); break;
        case SMP_col2: cgGLSetParameter4f(p, data[2], data[6], data[10], data[14]); break;
        case SMP_col3: cgGLSetParameter4f(p, data[3], data[7], data[11], data[15]); break;
        }
      }
    }
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::disable_shader_vertex_arrays
//       Access: Public
//  Description: Disable all the vertex arrays used by this shader.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
disable_shader_vertex_arrays(GSG *gsg)
{
#ifdef HAVE_CGGL
  if (_cg_context == 0) {
    return;
  }
  
  for (int i=0; i<(int)_var_spec.size(); i++) {
    CGparameter p = (CGparameter)(_var_spec[i]._parameter);
    cgGLDisableClientState(p);
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
void CLP(ShaderContext)::
update_shader_vertex_arrays(CLP(ShaderContext) *prev, GSG *gsg)
{
  if (prev) prev->disable_shader_vertex_arrays(gsg);
#ifdef HAVE_CGGL
  if (_cg_context == 0) {
    return;
  }
  
#ifdef SUPPORT_IMMEDIATE_MODE
  if (gsg->_use_sender) {
    GLCAT.error() << "immediate mode shaders not implemented yet\n";
  } else
#endif // SUPPORT_IMMEDIATE_MODE
  {
    const GeomVertexArrayData *array_data;
    Geom::NumericType numeric_type;
    int start, stride, num_values;
    int nvarying = _var_spec.size();
    for (int i=0; i<nvarying; i++) {
      CGparameter p = (CGparameter)(_var_spec[i]._parameter);
      InternalName *name = _var_spec[i]._name;
      int texslot = _var_spec[i]._append_uv;
      if (texslot >= 0) {
        const Geom::ActiveTextureStages &active_stages =
          gsg->_state._texture->get_on_stages();
        if (texslot < (int)active_stages.size()) {
          TextureStage *stage = active_stages[texslot];
          InternalName *texname = stage->get_texcoord_name();
          if (name == InternalName::get_texcoord()) {
            name = texname;
          } else if (texname != InternalName::get_texcoord()) {
            name = name->append(texname->get_basename());
          }
        }
      }
      if (gsg->_vertex_data->get_array_info(name,
                                            array_data, num_values, numeric_type,
                                            start, stride)) {
        const unsigned char *client_pointer = gsg->setup_array_data(array_data);
        cgGLSetParameterPointer(p,
                                num_values, gsg->get_numeric_type(numeric_type),
                                stride, client_pointer + start);
        cgGLEnableClientState(p);
      } else {
        cgGLDisableClientState(p);
      }
    }
  }
#endif // HAVE_CGGL
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::disable_shader_texture_bindings
//       Access: Public
//  Description: Disable all the texture bindings used by this shader.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
disable_shader_texture_bindings(GSG *gsg)
{
#ifdef HAVE_CGGL
  if (_cg_context == 0) {
    return;
  }

  for (int i=0; i<(int)_tex_spec.size(); i++) {
    CGparameter p = (CGparameter)(_tex_spec[i]._parameter);
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
update_shader_texture_bindings(CLP(ShaderContext) *prev, GSG *gsg)
{
  if (prev) prev->disable_shader_texture_bindings(gsg);
#ifdef HAVE_CGGL
  if (_cg_context == 0) {
    return;
  }

  for (int i=0; i<(int)_tex_spec.size(); i++) {
    CGparameter p = (CGparameter)(_tex_spec[i]._parameter);
    Texture *tex = 0;
    InternalName *id = _tex_spec[i]._name;
    if (id != 0) {
      const ShaderInput *input = gsg->_target._shader->get_shader_input(id);
      tex = input->get_texture();
    } else {
      if (_tex_spec[i]._stage >= gsg->_target._texture->get_num_on_stages()) {
        continue;
      }
      TextureStage *stage = gsg->_target._texture->get_on_stage(_tex_spec[i]._stage);
      tex = gsg->_target._texture->get_on_texture(stage);
    }
    if (_tex_spec[i]._suffix != 0) {
      // The suffix feature is inefficient. It is a temporary hack.
      if (tex == 0) {
        continue;
      }
      tex = tex->load_related(_tex_spec[i]._suffix);
    }
    if ((tex == 0) || (tex->get_texture_type() != _tex_spec[i]._desired_type)) {
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
  }
#endif
}

