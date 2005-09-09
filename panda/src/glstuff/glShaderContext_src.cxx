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

TypeHandle CLP(ShaderContext)::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::Constructor
//       Access: Public
//  Description: xyz
////////////////////////////////////////////////////////////////////
CLP(ShaderContext)::
CLP(ShaderContext)(Shader *s) : ShaderContext(s) {
  string header;
  s->parse_init();
  s->parse_line(header, true, true);
  
#ifdef HAVE_CGGL
  _cg_context = (CGcontext)0;
  _cg_profile[VERT_SHADER] = (CGprofile)0;
  _cg_profile[FRAG_SHADER] = (CGprofile)0;
  _cg_program[VERT_SHADER] = (CGprogram)0;
  _cg_program[FRAG_SHADER] = (CGprogram)0;

  if (header == "Cg") {

    CGerror err;
    _cg_context = cgCreateContext();
    if (_cg_context == 0) {
      release_resources();
      cerr << "Cg not supported by this video card.\n";
      return;
    }
    
    _cg_profile[VERT_SHADER] = cgGLGetLatestProfile(CG_GL_VERTEX);
    _cg_profile[FRAG_SHADER] = cgGLGetLatestProfile(CG_GL_FRAGMENT);
    if ((_cg_profile[VERT_SHADER] == CG_PROFILE_UNKNOWN)||
        (_cg_profile[FRAG_SHADER] == CG_PROFILE_UNKNOWN)) {
      release_resources();
      cerr << "Cg not supported by this video card II\n";
      return;
    }
    cgGetError();
    
    string commentary, stext[2];
    s->parse_upto(commentary, "---*---", false);
    _cg_linebase[0] = s->parse_lineno();
    s->parse_upto(stext[0], "---*---", false);
    _cg_linebase[1] = s->parse_lineno();
    s->parse_upto(stext[1], "---*---", false);
    
    for (int progindex=0; progindex<2; progindex++) {
      _cg_program[progindex] =
        cgCreateProgram(_cg_context, CG_SOURCE, stext[progindex].c_str(),
                        _cg_profile[progindex], "main", (const char**)NULL);
      err = cgGetError();
      if (err != CG_NO_ERROR) {
        if (err == CG_COMPILER_ERROR) {
          string listing = cgGetLastListing(_cg_context);
          vector_string errlines;
          tokenize(listing, errlines, "\n");
          for (int i=0; i<(int)errlines.size(); i++) {
            string line = trim(errlines[i]);
            if (line != "") {
              cerr << s->_file << " " << (_cg_linebase[progindex]-1) << "+" << errlines[i] << "\n";
            }
          }
        } else {
          cerr << s->_file << ": " << cgGetErrorString(err) << "\n";
        }
      }
    }

    if ((_cg_program[VERT_SHADER]==0)||(_cg_program[FRAG_SHADER]==0)) {
      release_resources();
      return;
    }

    bool success = true;
    CGparameter parameter;
    for (int progindex=0; progindex<2; progindex++) {
      for (parameter = cgGetFirstLeafParameter(_cg_program[progindex],CG_PROGRAM);
           parameter != 0;
           parameter = cgGetNextLeafParameter(parameter)) {
        success &= compile_cg_parameter(parameter);
      }
    }
    if (!success) {
      release_resources();
      return;
    }
        
    cgGLLoadProgram(_cg_program[VERT_SHADER]);
    cgGLLoadProgram(_cg_program[FRAG_SHADER]);
    
    cerr << s->_file << ": compiled ok.\n";
    return;
  }
#endif
  
  cerr << s->_file << ": unrecognized shader language " << header << "\n";
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
#ifdef HAVE_CGGL
  if (_cg_context) {
    cgDestroyContext(_cg_context);
    _cg_context = (CGcontext)0;
    _cg_profile[VERT_SHADER] = (CGprofile)0;
    _cg_profile[FRAG_SHADER] = (CGprofile)0;
    _cg_program[VERT_SHADER] = (CGprogram)0;
    _cg_program[FRAG_SHADER] = (CGprogram)0;
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::bind
//       Access: Public
//  Description: xyz
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
bind(ShaderMode *m, GraphicsStateGuardianBase *gsg) {
#ifdef HAVE_CGGL
  if (_cg_context != 0) {
    LVector4d tvec;

    // Assign the uniform Auto-Matrices
    for (int i=0; i<(int)_cg_autobind.size(); i++) {
      cgGLSetStateMatrixParameter(_cg_autobind[i].parameter,
                                  _cg_autobind[i].matrix,
                                  _cg_autobind[i].orientation);
    }
  
    // Bind the shaders.
    cgGLEnableProfile(_cg_profile[VERT_SHADER]);
    cgGLBindProgram(_cg_program[VERT_SHADER]);
    cgGLEnableProfile(_cg_profile[FRAG_SHADER]);
    cgGLBindProgram(_cg_program[FRAG_SHADER]);

    // Pass in uniform sampler2d parameters.
    for (int i=0; i<(int)_cg_tbind2d.size(); i++) {
      int index = _cg_tbind2d[i].argindex;
      if (index >= (int)m->_args.size()) continue;
      if (m->_args[index]._type != ShaderModeArg::SAT_TEXTURE) continue;
      Texture *tex = m->_args[index]._tvalue;
      TextureContext *tc = tex->prepare_now(gsg->get_prepared_objects(),gsg);
      CLP(TextureContext) *gtc = DCAST(CLP(TextureContext), tc);
      cgGLSetTextureParameter(_cg_tbind2d[i].parameter, gtc->_index);
      cgGLEnableTextureParameter(_cg_tbind2d[i].parameter);
    }
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::unbind
//       Access: Public
//  Description: xyz
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
unbind() {
#ifdef HAVE_CGGL
  if (_cg_context != 0) {
    // Disable texture parameters.
    for (int i=0; i<(int)_cg_tbind2d.size(); i++)
      cgGLDisableTextureParameter(_cg_tbind2d[i].parameter);
    
    cgGLDisableProfile(_cg_profile[VERT_SHADER]);
    cgGLDisableProfile(_cg_profile[FRAG_SHADER]);
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::rebind
//       Access: Public
//  Description: xyz
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
rebind(ShaderMode *oldmode, ShaderMode *newmode) {
}

#ifdef HAVE_CGGL
////////////////////////////////////////////////////////////////////
//     Function: Shader::errchk_cg_parameter_words
//       Access: Public, Static
//  Description: Make sure the provided Cg parameter contains 
//               the specified number of words.  If not, print
//               error message and return false.
////////////////////////////////////////////////////////////////////
bool CLP(ShaderContext)::
errchk_cg_parameter_words(CGparameter p, int len)
{
  vector_string words;
  tokenize(cgGetParameterName(p), words, "_");
  if (words.size() != len) {
    errchk_cg_output(p, "parameter name has wrong number of words");
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::errchk_cg_parameter_direction
//       Access: Public, Static
//  Description: Make sure the provided Cg parameter has the
//               correct direction.  If not, print
//               error message and return false.
////////////////////////////////////////////////////////////////////
bool CLP(ShaderContext)::
errchk_cg_parameter_direction(CGparameter p, CGenum dir)
{
  if (cgGetParameterDirection(p) != dir) {
    if (dir == CG_IN)    errchk_cg_output(p, "parameter should be declared 'in'");
    if (dir == CG_OUT)   errchk_cg_output(p, "parameter should be declared 'out'");
    if (dir == CG_INOUT) errchk_cg_output(p, "parameter should be declared 'inout'");
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::errchk_cg_parameter_variance
//       Access: Public, Static
//  Description: Make sure the provided Cg parameter has the
//               correct variance.  If not, print
//               error message and return false.
////////////////////////////////////////////////////////////////////
bool CLP(ShaderContext)::
errchk_cg_parameter_variance(CGparameter p, CGenum var)
{
  if (cgGetParameterVariability(p) != var) {
    if (var == CG_UNIFORM) 
      errchk_cg_output(p, "parameter should be declared 'uniform'");
    if (var == CG_VARYING) 
      errchk_cg_output(p, "parameter should be declared 'varying'");
    if (var == CG_CONSTANT)
      errchk_cg_output(p, "parameter should be declared 'const'");
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::errchk_cg_parameter_prog
//       Access: Public, Static
//  Description: Make sure the provided Cg parameter is a part
//               of the correct program.  If not, print
//               error message and return false.
////////////////////////////////////////////////////////////////////
bool CLP(ShaderContext)::
errchk_cg_parameter_prog(CGparameter p, CGprogram prog, const string &msg)
{
  if (cgGetParameterProgram(p) != prog) {
    string fmsg = "parameter can only be used in a ";
    errchk_cg_output(p, fmsg+msg+" program");
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::errchk_cg_parameter_semantic
//       Access: Public, Static
//  Description: Make sure the provided Cg parameter has the
//               correct semantic string.  If not, print
//               error message and return false.
////////////////////////////////////////////////////////////////////
bool CLP(ShaderContext)::
errchk_cg_parameter_semantic(CGparameter p, const string &semantic)
{
  if (semantic != cgGetParameterSemantic(p)) {
    if (semantic == "") {
      errchk_cg_output(p, "parameter should have no semantic string");
    } else {
      string msg = "parameter should have the semantic string ";
      errchk_cg_output(p, msg + semantic);
    }
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::errchk_cg_parameter_type
//       Access: Public, Static
//  Description: Make sure the provided Cg parameter has the
//               correct type.  If not, print
//               error message and return false.
////////////////////////////////////////////////////////////////////
bool CLP(ShaderContext)::
errchk_cg_parameter_type(CGparameter p, CGtype dt)
{
  if (cgGetParameterType(p) != dt) {
    string msg = "parameter should be of type ";
    errchk_cg_output(p, msg + cgGetTypeString(dt));
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::errchk_cg_parameter_float
//       Access: Public, Static
//  Description: Make sure the provided Cg parameter has
//               a floating point type.  If not, print
//               error message and return false.
////////////////////////////////////////////////////////////////////
bool CLP(ShaderContext)::
errchk_cg_parameter_float(CGparameter p)
{
  CGtype t = cgGetParameterType(p);
  if ((t != CG_FLOAT1)&&(t != CG_FLOAT2)&&(t != CG_FLOAT3)&&(t != CG_FLOAT4)) {
    errchk_cg_output(p, "parameter should have a float type");
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::errchk_cg_parameter_sampler
//       Access: Public, Static
//  Description: Make sure the provided Cg parameter has
//               a texture type.  If not, print
//               error message and return false.
////////////////////////////////////////////////////////////////////
bool CLP(ShaderContext)::
errchk_cg_parameter_sampler(CGparameter p)
{
  CGtype t = cgGetParameterType(p);
  if (t != CG_SAMPLER2D) {
    errchk_cg_output(p, "parameter should have a 'sampler' type");
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::errchk_cg_parameter_direction
//       Access: Public, Static
//  Description: Print an error message including a description
//               of the specified Cg parameter.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
errchk_cg_output(CGparameter p, const string &msg)
{
  string vstr;
  CGenum v = cgGetParameterVariability(p);
  if (v == CG_UNIFORM)  vstr = "uniform ";
  if (v == CG_VARYING)  vstr = "varying ";
  if (v == CG_CONSTANT) vstr = "const ";

  string dstr;
  CGenum d = cgGetParameterDirection(p);
  if (d == CG_IN)    dstr = "in ";
  if (d == CG_OUT)   dstr = "out ";
  if (d == CG_INOUT) dstr = "inout ";
  
  const char *ts = cgGetTypeString(cgGetParameterType(p));
  const char *ss = cgGetParameterSemantic(p);
  
  string err;
  string fn = _shader->_file;
  if (ss) {
    err = fn + ": " + msg + " (" + vstr + dstr + ts + " " + cgGetParameterName(p) + ":" + ss + ")\n";
  } else {
    err = fn + ": " + msg + " (" + vstr + dstr + ts + " " + cgGetParameterName(p) + ")\n";
  }
  cerr << err << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::compile_cg_parameter
//       Access: Public
//  Description: Analyzes a Cg parameter and decides how to
//               bind the parameter to some part of panda's
//               internal state.  Updates one of the cg bind
//               arrays to cause the binding to occur.
////////////////////////////////////////////////////////////////////
bool CLP(ShaderContext)::
compile_cg_parameter(CGparameter p)
{
  string pname = cgGetParameterName(p);
  if (pname.size() == 0) return true;
  if (pname[0] == '$') return true;
  vector_string pieces;
  tokenize(pname, pieces, "_");
  
  if (pieces[0] == "vtx") {
    if ((!errchk_cg_parameter_words(p,2)) ||
        (!errchk_cg_parameter_direction(p, CG_IN)) ||
        (!errchk_cg_parameter_variance(p, CG_VARYING)) ||
        (!errchk_cg_parameter_prog(p, _cg_program[VERT_SHADER], "vertex")))
      return false;
    if (pieces[1] == "position") {
      if (!errchk_cg_parameter_semantic(p,"POSITION")) return false;
      if (!errchk_cg_parameter_float(p)) return false;
      return true; // Cg handles this automatically.
    }
    if (pieces[1] == "normal") {
      if (!errchk_cg_parameter_semantic(p,"NORMAL")) return false;
      if (!errchk_cg_parameter_float(p)) return false;
      return true; // Cg handles this automatically.
    }
    if (pieces[1] == "color") {
      if (!errchk_cg_parameter_semantic(p,"COLOR")) return false;
      if (!errchk_cg_parameter_float(p)) return false;
      return true; // Cg handles this automatically.
    }
    if (pieces[1].substr(0,8) == "texcoord") {
      string ss = upcase(pieces[1]);
      if (!errchk_cg_parameter_semantic(p,ss)) return false;
      if (!errchk_cg_parameter_float(p)) return false;
      return true; // Cg handles this automatically.
    }
    // Handle an arbitrary column taken from the vertex array.
    // IMPLEMENT ME.
    errchk_cg_output(p, "Arbitrary vertex fields not implemented yet");
    return false;
  }
  
  if ((pieces[0] == "trans")||
      (pieces[0] == "tpose")||
      (pieces[0] == "row0")||
      (pieces[0] == "row1")||
      (pieces[0] == "row2")||
      (pieces[0] == "row3")||
      (pieces[0] == "col0")||
      (pieces[0] == "col1")||
      (pieces[0] == "col2")||
      (pieces[0] == "col3")||
      (pieces[0] == "xvec")||
      (pieces[0] == "yvec")||
      (pieces[0] == "zvec")||
      (pieces[0] == "pos")) {
    if ((!errchk_cg_parameter_words(p,4)) ||
        (!errchk_cg_parameter_direction(p, CG_IN)) ||
        (!errchk_cg_parameter_variance(p, CG_UNIFORM)) ||
        (!errchk_cg_parameter_semantic(p, "")))
      return false;
    if (pieces[2] != "rel") {
      errchk_cg_output(p, "syntax error");
      return false;
    }
    ShaderTransBind bind;
    bind.parameter = p;

    if      (pieces[0]=="trans") bind.trans_piece = TRANS_NORMAL;
    else if (pieces[0]=="tpose") bind.trans_piece = TRANS_TPOSE;
    else if (pieces[0]=="row0") bind.trans_piece = TRANS_ROW0;
    else if (pieces[0]=="row1") bind.trans_piece = TRANS_ROW1;
    else if (pieces[0]=="row2") bind.trans_piece = TRANS_ROW2;
    else if (pieces[0]=="row3") bind.trans_piece = TRANS_ROW3;
    else if (pieces[0]=="col0") bind.trans_piece = TRANS_COL0;
    else if (pieces[0]=="col1") bind.trans_piece = TRANS_COL1;
    else if (pieces[0]=="col2") bind.trans_piece = TRANS_COL2;
    else if (pieces[0]=="col3") bind.trans_piece = TRANS_COL3;
    else if (pieces[0]=="xvec") bind.trans_piece = TRANS_ROW0;
    else if (pieces[0]=="yvec") bind.trans_piece = TRANS_ROW1;
    else if (pieces[0]=="zvec") bind.trans_piece = TRANS_ROW2;
    else if (pieces[0]=="pos")  bind.trans_piece = TRANS_ROW3;

    if      (pieces[1] == "world")  bind.src_argindex = ARGINDEX_WORLD;
    else if (pieces[1] == "camera") bind.src_argindex = ARGINDEX_CAMERA;
    else if (pieces[1] == "model")  bind.src_argindex = ARGINDEX_MODEL;
    else bind.src_argindex = _shader->arg_index(pieces[1]);
    if      (pieces[3] == "world")  bind.rel_argindex = ARGINDEX_WORLD;
    else if (pieces[3] == "camera") bind.rel_argindex = ARGINDEX_CAMERA;
    else if (pieces[3] == "model")  bind.rel_argindex = ARGINDEX_MODEL;
    else bind.rel_argindex = _shader->arg_index(pieces[3]);

    if ((bind.trans_piece == TRANS_NORMAL)||(bind.trans_piece == TRANS_TPOSE)) {
      if (!errchk_cg_parameter_type(p, CG_FLOAT4x4)) return false;
    } else {
      if (!errchk_cg_parameter_type(p, CG_FLOAT4)) return false;
    }

    _cg_trans_bind.push_back(bind);
    if ((bind.src_argindex == ARGINDEX_MODEL) ||
        (bind.rel_argindex == ARGINDEX_MODEL)) {
      _cg_trans_rebind.push_back(bind);
    }
    return true;
  }

  if ((pieces[0]=="mstrans")||
      (pieces[0]=="cstrans")||
      (pieces[0]=="wstrans")||
      (pieces[0]=="mspos")||
      (pieces[0]=="cspos")||
      (pieces[0]=="wspos")) {
    if ((!errchk_cg_parameter_words(p,2)) ||
        (!errchk_cg_parameter_direction(p, CG_IN)) ||
        (!errchk_cg_parameter_variance(p, CG_UNIFORM)) ||
        (!errchk_cg_parameter_semantic(p, "")))
      return false;
    ShaderTransBind bind;
    bind.parameter = p;
    
    if      (pieces[0]=="wstrans") { bind.rel_argindex = ARGINDEX_WORLD;  bind.trans_piece = TRANS_NORMAL; }
    else if (pieces[0]=="cstrans") { bind.rel_argindex = ARGINDEX_CAMERA; bind.trans_piece = TRANS_NORMAL; }
    else if (pieces[0]=="mstrans") { bind.rel_argindex = ARGINDEX_MODEL;  bind.trans_piece = TRANS_NORMAL; }
    else if (pieces[0]=="wspos")   { bind.rel_argindex = ARGINDEX_WORLD;  bind.trans_piece = TRANS_ROW3; }
    else if (pieces[0]=="cspos")   { bind.rel_argindex = ARGINDEX_CAMERA; bind.trans_piece = TRANS_ROW3; }
    else if (pieces[0]=="mspos")   { bind.rel_argindex = ARGINDEX_MODEL;  bind.trans_piece = TRANS_ROW3; }
    
    if      (pieces[1] == "world")  bind.src_argindex = ARGINDEX_WORLD;
    else if (pieces[1] == "camera") bind.src_argindex = ARGINDEX_CAMERA;
    else if (pieces[1] == "model")  bind.src_argindex = ARGINDEX_MODEL;
    else bind.src_argindex = _shader->arg_index(pieces[1]);
    
    if ((bind.trans_piece == TRANS_NORMAL)||(bind.trans_piece == TRANS_TPOSE)) {
      if (!errchk_cg_parameter_type(p, CG_FLOAT4x4)) return false;
    } else {
      if (!errchk_cg_parameter_type(p, CG_FLOAT4)) return false;
    }
    _cg_trans_bind.push_back(bind);
    if ((bind.src_argindex == ARGINDEX_MODEL) ||
        (bind.rel_argindex == ARGINDEX_MODEL)) {
      _cg_trans_rebind.push_back(bind);
    }
    return true;
  }

  if ((pieces[0]=="mat")||
      (pieces[0]=="inv")||
      (pieces[0]=="tps")||
      (pieces[0]=="itp")) {
    if ((!errchk_cg_parameter_words(p,2)) ||
        (!errchk_cg_parameter_direction(p, CG_IN)) ||
        (!errchk_cg_parameter_variance(p, CG_UNIFORM)) ||
        (!errchk_cg_parameter_type(p, CG_FLOAT4x4)))
      return false;
    ShaderAutoBind bind;
    bind.parameter = p;
    if      (pieces[1] == "modelview")  bind.matrix = CG_GL_MODELVIEW_MATRIX;
    else if (pieces[1] == "projection") bind.matrix = CG_GL_PROJECTION_MATRIX;
    else if (pieces[1] == "texmatrix")  bind.matrix = CG_GL_TEXTURE_MATRIX;
    else if (pieces[1] == "modelproj")  bind.matrix = CG_GL_MODELVIEW_PROJECTION_MATRIX;
    else {
      errchk_cg_output(p, "unrecognized matrix name");
      return false;
    }
    if      (pieces[0]=="mat") bind.orientation = CG_GL_MATRIX_IDENTITY;
    else if (pieces[0]=="inv") bind.orientation = CG_GL_MATRIX_INVERSE;
    else if (pieces[0]=="tps") bind.orientation = CG_GL_MATRIX_TRANSPOSE;
    else if (pieces[0]=="itp") bind.orientation = CG_GL_MATRIX_INVERSE_TRANSPOSE;
    _cg_autobind.push_back(bind);
    return true;
  }

  if (pieces[0] == "tex") {
    if ((!errchk_cg_parameter_words(p,2)) ||
        (!errchk_cg_parameter_direction(p, CG_IN)) ||
        (!errchk_cg_parameter_variance(p, CG_UNIFORM)) ||
        (!errchk_cg_parameter_sampler(p)))
      return false;
    if (cgGetParameterSemantic(p)[0]!=0) {
      string texunit = "TEX"+upcase(pieces[1]);
      if (!errchk_cg_parameter_semantic(p, texunit)) return false;
    }
    return true; // Cg handles this automatically.
  }

  if (pieces[0] == "k") {
    if ((!errchk_cg_parameter_words(p,2)) ||
        (!errchk_cg_parameter_direction(p, CG_IN)) ||
        (!errchk_cg_parameter_variance(p, CG_UNIFORM)))
      return false;
    ShaderArgBind bind;
    bind.parameter = p;
    bind.argindex = _shader->arg_index(pieces[1]);
    switch (cgGetParameterType(p)) {
    case CG_FLOAT1:    _cg_vbind1.push_back(bind); break;
    case CG_FLOAT2:    _cg_vbind2.push_back(bind); break;
    case CG_FLOAT3:    _cg_vbind3.push_back(bind); break;
    case CG_FLOAT4:    _cg_vbind4.push_back(bind); break;
    case CG_SAMPLER2D: _cg_tbind2d.push_back(bind); break;
    case CG_SAMPLER3D: _cg_tbind3d.push_back(bind); break;
    case CG_FLOAT4x4:  _cg_npbind.push_back(bind); break;
    default:
      errchk_cg_output(p, "Invalid type for a k-parameter");
      return false;
    }
    return true;
  }

  if (pieces[0] == "l") {
    // IMPLEMENT THE ERROR CHECKING
    return true; // Cg handles this automatically.
  }

  if (pieces[0] == "o") {
    // IMPLEMENT THE ERROR CHECKING
    return true; // Cg handles this automatically.
  }

  errchk_cg_output(p, "unrecognized parameter name");
  return false;
}
#endif
