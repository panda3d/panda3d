// Filename: shaderContext.cxx
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

TypeHandle ShaderContext::_type_handle;

#ifdef HAVE_CG
#include "Cg/cg.h"

////////////////////////////////////////////////////////////////////
//     Function: Shader::report_cg_compile_errors
//       Access: Public, Static
//  Description: Used only after a Cg compile command, to print
//               out any error messages that may have occurred
//               during the Cg shader compilation.  The 'file'
//               is the name of the file containing the Cg code.
////////////////////////////////////////////////////////////////////
void ShaderContext::
report_cg_compile_errors(const string &file, CGcontext ctx,
                         NotifyCategory *cat)
{
  CGerror err = cgGetError();
  if (err != CG_NO_ERROR) {
    if (err == CG_COMPILER_ERROR) {
      string listing = cgGetLastListing(ctx);
      vector_string errlines;
      tokenize(listing, errlines, "\n");
      for (int i=0; i<(int)errlines.size(); i++) {
        string line = trim(errlines[i]);
        if (line != "") {
          cat->error() << file << " " << errlines[i] << "\n";
        }
      }
    } else {
      cat->error() << file << ": " << cgGetErrorString(err) << "\n";
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::report_cg_parameter_error
//       Access: Public
//  Description: Generate an error message including a description
//               of the specified Cg parameter.
////////////////////////////////////////////////////////////////////
void ShaderContext::
report_cg_parameter_error(CGparameter p, const string &msg)
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

  string err;
  string fn = _shader_expansion->get_name();
  _cg_report_cat->error() << fn << ": " << msg << " (" <<
    vstr << dstr << ts << " " << cgGetParameterName(p) << ")\n";
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::errchk_cg_parameter_words
//       Access: Public, Static
//  Description: Make sure the provided Cg parameter contains
//               the specified number of words.  If not, print
//               error message and return false.
////////////////////////////////////////////////////////////////////
bool ShaderContext::
errchk_cg_parameter_words(CGparameter p, int len)
{
  vector_string words;
  tokenize(cgGetParameterName(p), words, "_");
  if (words.size() != len) {
    report_cg_parameter_error(p, "parameter name has wrong number of words");
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::errchk_cg_parameter_in
//       Access: Public, Static
//  Description: Make sure the provided Cg parameter has the
//               CG_IN direction.  If not, print
//               error message and return false.
////////////////////////////////////////////////////////////////////
bool ShaderContext::
errchk_cg_parameter_in(CGparameter p)
{
  if (cgGetParameterDirection(p) != CG_IN) {
    report_cg_parameter_error(p, "parameter should be declared 'in'");
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::errchk_cg_parameter_varying
//       Access: Public, Static
//  Description: Make sure the provided Cg parameter has the
//               correct variance.  If not, print
//               error message and return false.
////////////////////////////////////////////////////////////////////
bool ShaderContext::
errchk_cg_parameter_varying(CGparameter p)
{
  if (cgGetParameterVariability(p) != CG_VARYING) {
    report_cg_parameter_error(p, "parameter should be declared 'varying'");
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::errchk_cg_parameter_uniform
//       Access: Public, Static
//  Description: Make sure the provided Cg parameter has the
//               correct variance.  If not, print
//               error message and return false.
////////////////////////////////////////////////////////////////////
bool ShaderContext::
errchk_cg_parameter_uniform(CGparameter p)
{
  if (cgGetParameterVariability(p) != CG_UNIFORM) {
    report_cg_parameter_error(p, "parameter should be declared 'uniform'");
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
bool ShaderContext::
errchk_cg_parameter_float(CGparameter p, int lo, int hi)
{
  CGtype t = cgGetParameterType(p);
  int nfloat = 0;
  switch (t) {
  case CG_FLOAT1: nfloat = 1; break;
  case CG_FLOAT2: nfloat = 2; break;
  case CG_FLOAT3: nfloat = 3; break;
  case CG_FLOAT4: nfloat = 4; break;
  case CG_FLOAT4x4: nfloat = 16; break;
  }
  if ((nfloat < lo)||(nfloat > hi)) {
    report_cg_parameter_error(p, "wrong float-type for parameter");
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
bool ShaderContext::
errchk_cg_parameter_sampler(CGparameter p)
{
  CGtype t = cgGetParameterType(p);
  if ((t!=CG_SAMPLER1D)&&
      (t!=CG_SAMPLER2D)&&
      (t!=CG_SAMPLER3D)&&
      (t!=CG_SAMPLERCUBE)) {
    report_cg_parameter_error(p, "parameter should have a 'sampler' type");
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::parse_cg_trans_clause
//       Access: Public
//  Description: Parses a single clause of a "trans" parameter.
////////////////////////////////////////////////////////////////////
bool ShaderContext::
parse_cg_trans_clause(CGparameter p, ShaderMatSpec &spec, const vector_string &pieces,
                      int &next, ShaderMatOp ofop, ShaderMatOp op) {
  if (pieces[next+1]=="of") {
    if (pieces[next+2]=="") {
      report_cg_parameter_error(p, "'of' should be followed by a name");
      return false;
    }
    if (ofop != SMO_noop) {
      spec._opcodes.push_back(ofop);
      spec._args.push_back(InternalName::make(pieces[next+2]));
    }
    next += 3;
    return true;
  } else {
    if (op != SMO_noop) {
      spec._opcodes.push_back(op);
    }
    next += 1;
    return true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::compile_cg_parameter
//       Access: Public
//  Description: Analyzes a Cg parameter and decides how to
//               bind the parameter to some part of panda's
//               internal state.  Updates one of the cg bind
//               arrays to cause the binding to occur.
//
//               If there is an error, this routine will append
//               an error message onto the error messages.
////////////////////////////////////////////////////////////////////
bool ShaderContext::
compile_cg_parameter(CGparameter p, NotifyCategory *cat)
{
  _cg_report_cat = cat;
  string pname = cgGetParameterName(p);
  if (pname.size() == 0) return true;
  if (pname[0] == '$') return true;
  vector_string pieces;
  tokenize(pname, pieces, "_");

  if (pieces.size() < 2) {
    report_cg_parameter_error(p, "invalid parameter name");
    return false;
  }

  // Implement vtx parameters - the varying kind.
  
  if (pieces[0] == "vtx") {
    if ((!errchk_cg_parameter_in(p)) ||
        (!errchk_cg_parameter_varying(p)) ||
        (!errchk_cg_parameter_float(p, 1, 4))) {
      return false;
    }
    ShaderVarSpec bind;
    bind._parameter = (void*)p;
    if (pieces.size() == 2) {
      if (pieces[1]=="position") {
        bind._name = InternalName::get_vertex();
        bind._append_uv = -1;
        _var_spec.push_back(bind);
        return true;
      }
      if (pieces[1].substr(0,8)=="texcoord") {
        bind._name = InternalName::get_texcoord();
        bind._append_uv = atoi(pieces[1].c_str()+8);
        _var_spec.push_back(bind);
        return true;
      }
      if (pieces[1].substr(0,7)=="tangent") {
        bind._name = InternalName::get_tangent();
        bind._append_uv = atoi(pieces[1].c_str()+7);
        _var_spec.push_back(bind);
        return true;
      }
      if (pieces[1].substr(0,8)=="binormal") {
        bind._name = InternalName::get_binormal();
        bind._append_uv = atoi(pieces[1].c_str()+8);
        _var_spec.push_back(bind);
        return true;
      }
    }
    bind._name = InternalName::get_root();
    bind._append_uv = -1;
    for (int i=1; i<(int)(pieces.size()-0); i++)
      bind._name = bind._name->append(pieces[i]);
    _var_spec.push_back(bind);
    return true;
  }

  // Implement some macros. Macros work by altering the
  // contents of the 'pieces' array, and then falling through.

  if (pieces[0] == "mstrans") {
    pieces[0] = "trans";
    pieces.push_back("to");
    pieces.push_back("model");
  }
  if (pieces[0] == "wstrans") {
    pieces[0] = "trans";
    pieces.push_back("to");
    pieces.push_back("world");
  }
  if (pieces[0] == "vstrans") {
    pieces[0] = "trans";
    pieces.push_back("to");
    pieces.push_back("view");
  }
  if (pieces[0] == "cstrans") {
    pieces[0] = "trans";
    pieces.push_back("to");
    pieces.push_back("clip");
  }
  if (pieces[0] == "mspos") {
    pieces[0] = "row3";
    pieces.push_back("to");
    pieces.push_back("model");
  }
  if (pieces[0] == "wspos") {
    pieces[0] = "row3";
    pieces.push_back("to");
    pieces.push_back("world");
  }
  if (pieces[0] == "vspos") {
    pieces[0] = "row3";
    pieces.push_back("to");
    pieces.push_back("view");
  }
  if (pieces[0] == "cspos") {
    pieces[0] = "row3";
    pieces.push_back("to");
    pieces.push_back("clip");
  }

  // Implement the modelview macros.

  if ((pieces[0] == "mat")||(pieces[0] == "inv")||
      (pieces[0] == "tps")||(pieces[0] == "itp")) {
    if (!errchk_cg_parameter_words(p, 2)) {
      return false;
    }
    string trans = pieces[0];
    string matrix = pieces[1];
    pieces.clear();
    if (matrix == "modelview") {
      tokenize("trans_model_to_apiview", pieces, "_");
    } else if (matrix == "projection") {
      tokenize("trans_apiview_to_apiclip", pieces, "_");
    } else if (matrix == "modelproj") {
      tokenize("trans_model_to_apiclip", pieces, "_");
    } else {
      report_cg_parameter_error(p,"unrecognized matrix name");
      return false;
    }
    if (trans=="mat") {
      pieces[0] = "trans";
    } else if (trans=="inv") {
      string t = pieces[1];
      pieces[1] = pieces[3];
      pieces[3] = t;
    } else if (trans=="tps") {
      pieces[0] = "tpose";
    } else if (trans=="itp") {
      string t = pieces[1];
      pieces[1] = pieces[3];
      pieces[3] = t;
      pieces[0] = "tpose";
    }
  }

  // Implement the transform-matrix generator.

  if ((pieces[0]=="trans")||
      (pieces[0]=="tpose")||
      (pieces[0]=="row0")||
      (pieces[0]=="row1")||
      (pieces[0]=="row2")||
      (pieces[0]=="row3")||
      (pieces[0]=="col0")||
      (pieces[0]=="col1")||
      (pieces[0]=="col2")||
      (pieces[0]=="col3")) {
    
    if ((!errchk_cg_parameter_in(p)) ||
        (!errchk_cg_parameter_uniform(p)))
      return false;
    
    ShaderMatSpec bind;
    bind._parameter = (void*)p;
    bind._trans_dependent = false;

    int next = 1;
    pieces.push_back("");

    // Decide whether this is a matrix or vector.
    if      (pieces[0]=="trans")   bind._piece = SMP_whole;
    else if (pieces[0]=="tpose")   bind._piece = SMP_whole;
    else if (pieces[0]=="row0")    bind._piece = SMP_row0;
    else if (pieces[0]=="row1")    bind._piece = SMP_row1;
    else if (pieces[0]=="row2")    bind._piece = SMP_row2;
    else if (pieces[0]=="row3")    bind._piece = SMP_row3;
    else if (pieces[0]=="col0")    bind._piece = SMP_col0;
    else if (pieces[0]=="col1")    bind._piece = SMP_col1;
    else if (pieces[0]=="col2")    bind._piece = SMP_col2;
    else if (pieces[0]=="col3")    bind._piece = SMP_col3;
    if (bind._piece == SMP_whole) {
      if (!errchk_cg_parameter_float(p, 16, 16)) return false;
    } else {
      if (!errchk_cg_parameter_float(p, 4, 4)) return false;
    }
    
    // Parse the first half of the clause.
    bool ok = true;
    if ((pieces[next]=="")||(pieces[next]=="of")||(pieces[next]=="to")){
      report_cg_parameter_error(p, "argument missing");
      return false;
    } else if (pieces[next] == "world") {
      bind._opcodes.push_back(SMO_world_to_view);
      next += 1;
    } else if (pieces[next] == "model") {
      ok &= parse_cg_trans_clause(p, bind, pieces, next, SMO_view_x_to_view, SMO_model_to_view);
    } else if (pieces[next] == "clip") {
      ok &= parse_cg_trans_clause(p, bind, pieces, next, SMO_clip_x_to_view, SMO_clip_to_view);
    } else if (pieces[next] == "view") {
      ok &= parse_cg_trans_clause(p, bind, pieces, next, SMO_view_x_to_view, SMO_identity);
    } else if (pieces[next] == "apiview") {
      ok &= parse_cg_trans_clause(p, bind, pieces, next, SMO_apiview_x_to_view, SMO_apiview_to_view);
    } else if (pieces[next] == "apiclip") {
      ok &= parse_cg_trans_clause(p, bind, pieces, next, SMO_apiclip_x_to_view, SMO_apiclip_to_view);
    } else {
      bind._opcodes.push_back(SMO_view_x_to_view);
      bind._args.push_back(InternalName::make(pieces[next]));
      next += 1;
    }

    // Check for errors in the first clause.
    if (!ok) {
      return false;
    }

    // Check for transform-dependence.
    if (bind._opcodes.back() == SMO_model_to_view) {
      bind._trans_dependent = true;
    }

    // Check for syntactic well-formed-ness.
    if (pieces[next] != "to") {
      report_cg_parameter_error(p, "keyword 'to' expected");
      return false;
    } else {
      next += 1;
    }
    
    // Parse the second half of the clause.
    if ((pieces[next]=="")||(pieces[next]=="of")||(pieces[next]=="to")){
      report_cg_parameter_error(p, "argument missing");
      return false;
    } else if (pieces[next] == "world") {
      bind._opcodes.push_back(SMO_view_to_world_C);
      next += 1;
    } else if (pieces[next] == "model") {
      ok &= parse_cg_trans_clause(p, bind, pieces, next, SMO_view_to_view_x_C, SMO_view_to_model_C);
    } else if (pieces[next] == "clip") {
      ok &= parse_cg_trans_clause(p, bind, pieces, next, SMO_view_to_clip_x_C, SMO_view_to_clip_C);
    } else if (pieces[next] == "view") {
      ok &= parse_cg_trans_clause(p, bind, pieces, next, SMO_view_to_view_x_C, SMO_noop);
    } else if (pieces[next] == "apiview") {
      ok &= parse_cg_trans_clause(p, bind, pieces, next, SMO_view_to_apiview_x_C, SMO_view_to_apiview_C);
    } else if (pieces[next] == "apiclip") {
      ok &= parse_cg_trans_clause(p, bind, pieces, next, SMO_view_to_apiclip_x_C, SMO_view_to_apiclip_C);
    } else {
      bind._opcodes.push_back(SMO_view_to_view_x_C);
      bind._args.push_back(InternalName::make(pieces[next]));
      next += 1;
    }
    
    // Check for errors in the second clause.
    if (!ok) {
      return false;
    }

    // Check for transform-dependence.
    if (bind._opcodes.back() == SMO_view_to_model_C) {
      bind._trans_dependent = true;
    }
    
    // Check for syntactic well-formed-ness.
    if (pieces[next] != "") {
      report_cg_parameter_error(p, "end of line expected");
      return false;
    }
    
    _mat_spec.push_back(bind);
    return true;
  }

  // Keywords to access unusual parameters.
  
  if (pieces[0] == "sys") {
    if ((!errchk_cg_parameter_words(p,2)) ||
        (!errchk_cg_parameter_in(p)) ||
        (!errchk_cg_parameter_uniform(p))) {
      return false;
    }
    ShaderMatSpec bind;
    bind._parameter = (void*)p;
    bind._trans_dependent = false;
    bind._piece = SMP_row3;
    if (pieces[1] == "pixelsize") {
      if (!errchk_cg_parameter_float(p, 2, 2)) {
        return false;
      }
      bind._opcodes.push_back(SMO_pixel_size);
    } else if (pieces[1] == "windowsize") {
      if (!errchk_cg_parameter_float(p, 2, 2)) {
        return false;
      }
      bind._opcodes.push_back(SMO_window_size);
    } else if (pieces[1] == "cardcenter") {
      if (!errchk_cg_parameter_float(p, 2, 2)) {
        return false;
      }
      bind._opcodes.push_back(SMO_card_center);
    } else {
      report_cg_parameter_error(p,"unknown system parameter");
      return false;
    }
    _mat_spec.push_back(bind);
    return true;
  }
  
  // Keywords to access textures.
  
  if (pieces[0] == "tex") {
    if ((!errchk_cg_parameter_in(p)) ||
        (!errchk_cg_parameter_uniform(p)) ||
        (!errchk_cg_parameter_sampler(p)))
      return false;
    if ((pieces.size() != 2)&&(pieces.size() != 3)) {
      report_cg_parameter_error(p, "Invalid parameter name");
      return false;
    }
    ShaderTexSpec bind;
    bind._parameter = (void*)p;
    bind._name = 0;
    bind._stage = atoi(pieces[1].c_str());
    switch (cgGetParameterType(p)) {
    case CG_SAMPLER1D:   bind._desired_type = Texture::TT_1d_texture; break;
    case CG_SAMPLER2D:   bind._desired_type = Texture::TT_2d_texture; break;
    case CG_SAMPLER3D:   bind._desired_type = Texture::TT_3d_texture; break;
    case CG_SAMPLERCUBE: bind._desired_type = Texture::TT_cube_map; break;
    default:
      report_cg_parameter_error(p, "Invalid type for a tex-parameter");
      return false;
    }
    if (pieces.size()==3) {
      bind._suffix = InternalName::make(((string)"-") + pieces[2]);
    }
    _tex_spec.push_back(bind);
    return true;
  }

  // Keywords to access constants.

  if (pieces[0] == "k") {
    if ((!errchk_cg_parameter_words(p,2)) ||
        (!errchk_cg_parameter_in(p)) ||
        (!errchk_cg_parameter_uniform(p)))
      return false;
    switch (cgGetParameterType(p)) {
    case CG_FLOAT4: {
      ShaderMatSpec bind;
      bind._parameter = (void*)p;
      bind._trans_dependent = false;
      bind._piece = SMP_row3;
      bind._opcodes.push_back(SMO_vec_constant_x);
      bind._args.push_back(InternalName::make(pieces[1]));
      _mat_spec.push_back(bind);
      break;
    }
    case CG_FLOAT4x4: {
      ShaderMatSpec bind;
      bind._parameter = (void*)p;
      bind._trans_dependent = false;
      bind._piece = SMP_whole;
      bind._opcodes.push_back(SMO_mat_constant_x);
      bind._args.push_back(InternalName::make(pieces[1]));
      _mat_spec.push_back(bind);
      break;
    }
    case CG_SAMPLER1D: {
      ShaderTexSpec bind;
      bind._parameter = (void*)p;
      bind._name = InternalName::make(pieces[1]);
      bind._desired_type=Texture::TT_1d_texture;
      _tex_spec.push_back(bind);
      break;
    }
    case CG_SAMPLER2D: {
      ShaderTexSpec bind;
      bind._parameter = (void*)p;
      bind._name = InternalName::make(pieces[1]);
      bind._desired_type=Texture::TT_2d_texture;
      _tex_spec.push_back(bind);
      break;
    }
    case CG_SAMPLER3D: {
      ShaderTexSpec bind;
      bind._parameter = (void*)p;
      bind._name = InternalName::make(pieces[1]);
      bind._desired_type=Texture::TT_3d_texture;
      _tex_spec.push_back(bind);
      break;
    }
    case CG_SAMPLERCUBE: {
      ShaderTexSpec bind;
      bind._parameter = (void*)p;
      bind._name = InternalName::make(pieces[1]);
      bind._desired_type = Texture::TT_cube_map;
      _tex_spec.push_back(bind);
      break;
    }
    default:
      report_cg_parameter_error(p, "Invalid type for a k-parameter");
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

  report_cg_parameter_error(p, "unrecognized parameter name");
  return false;
}

#endif // HAVE_CG
