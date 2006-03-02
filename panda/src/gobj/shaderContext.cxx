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

////////////////////////////////////////////////////////////////////
//     Function: Shader::cp_report_error
//       Access: Public
//  Description: Generate an error message including a description
//               of the specified parameter.
////////////////////////////////////////////////////////////////////
void ShaderContext::
cp_report_error(ShaderArgInfo &p, const string &msg)
{
  string vstr;
  if (p._varying) vstr = "varying ";
  else            vstr = "uniform ";

  string dstr = "unknown ";
  if (p._direction == SAD_in)    dstr = "in ";
  if (p._direction == SAD_out)   dstr = "out ";
  if (p._direction == SAD_inout) dstr = "inout ";

  string tstr = "unknown ";
  switch (p._type) {
  case SAT_float1: tstr = "float1 "; break;
  case SAT_float2: tstr = "float2 "; break;
  case SAT_float3: tstr = "float3 "; break;
  case SAT_float4: tstr = "float4 "; break;
  case SAT_float4x4: tstr = "float4x4 "; break;
  case SAT_sampler1d: tstr = "sampler1d "; break;
  case SAT_sampler2d: tstr = "sampler2d "; break;
  case SAT_sampler3d: tstr = "sampler3d "; break;
  case SAT_samplercube: tstr = "samplercube "; break;
  }

  string fn = _shader_expansion->get_name();
  p._cat->error() << fn << ": " << msg << " (" <<
    vstr << dstr << tstr << p._name << ")\n";
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::cp_errchk_parameter_words
//       Access: Public, Static
//  Description: Make sure the provided parameter contains
//               the specified number of words.  If not, print
//               error message and return false.
////////////////////////////////////////////////////////////////////
bool ShaderContext::
cp_errchk_parameter_words(ShaderArgInfo &p, int len)
{
  vector_string words;
  tokenize(p._name, words, "_");
  if (words.size() != len) {
    cp_report_error(p, "parameter name has wrong number of words");
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::cp_errchk_parameter_in
//       Access: Public, Static
//  Description: Make sure the provided parameter has the
//               'in' direction.  If not, print
//               error message and return false.
////////////////////////////////////////////////////////////////////
bool ShaderContext::
cp_errchk_parameter_in(ShaderArgInfo &p)
{
  if (p._direction != SAD_in) {
    cp_report_error(p, "parameter should be declared 'in'");
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::cp_errchk_parameter_varying
//       Access: Public, Static
//  Description: Make sure the provided parameter has the
//               correct variance.  If not, print
//               error message and return false.
////////////////////////////////////////////////////////////////////
bool ShaderContext::
cp_errchk_parameter_varying(ShaderArgInfo &p)
{
  if (!p._varying) {
    cp_report_error(p, "parameter should be declared 'varying'");
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::cp_errchk_parameter_uniform
//       Access: Public, Static
//  Description: Make sure the provided parameter has the
//               correct variance.  If not, print
//               error message and return false.
////////////////////////////////////////////////////////////////////
bool ShaderContext::
cp_errchk_parameter_uniform(ShaderArgInfo &p)
{
  if (p._varying) {
    cp_report_error(p, "parameter should be declared 'uniform'");
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::cp_errchk_parameter_float
//       Access: Public, Static
//  Description: Make sure the provided parameter has
//               a floating point type.  If not, print
//               error message and return false.
////////////////////////////////////////////////////////////////////
bool ShaderContext::
cp_errchk_parameter_float(ShaderArgInfo &p, int lo, int hi)
{
  int nfloat = 0;
  switch (p._type) {
  case SAT_float1: nfloat = 1; break;
  case SAT_float2: nfloat = 2; break;
  case SAT_float3: nfloat = 3; break;
  case SAT_float4: nfloat = 4; break;
  case SAT_float4x4: nfloat = 16; break;
  }
  if ((nfloat < lo)||(nfloat > hi)) {
    string msg = "wrong type for parameter: should be float";
    cp_report_error(p, msg);
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::cp_errchk_parameter_sampler
//       Access: Public, Static
//  Description: Make sure the provided parameter has
//               a texture type.  If not, print
//               error message and return false.
////////////////////////////////////////////////////////////////////
bool ShaderContext::
cp_errchk_parameter_sampler(ShaderArgInfo &p)
{
  if ((p._type!=SAT_sampler1d)&&
      (p._type!=SAT_sampler2d)&&
      (p._type!=SAT_sampler3d)&&
      (p._type!=SAT_samplercube)) {
    cp_report_error(p, "parameter should have a 'sampler' type");
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderContext::cp_parse_trans_clause
//       Access: Public
//  Description: Parses a single clause of a "trans" parameter.
////////////////////////////////////////////////////////////////////
bool ShaderContext::
cp_parse_trans_clause(ShaderArgInfo &p, ShaderMatSpec &spec, 
                      int part, const vector_string &pieces,
                      int &next, ShaderMatInput ofop, ShaderMatInput op) {
  if (pieces[next+1]=="of") {
    if (pieces[next+2]=="") {
      cp_report_error(p, "'of' should be followed by a name");
      return false;
    }
    spec._part[part] = ofop;
    spec._arg[part] = InternalName::make(pieces[next+2]);
    next += 3;
    return true;
  } else {
    spec._part[part] = op;
    spec._arg[part] = NULL;
    next += 1;
    return true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderContext::cp_optimize_mat_spec
//       Access: Public
//  Description: Analyzes a ShaderMatSpec and decides what it should
//               use its cache for.  It can cache the results of any
//               one opcode, or, it can cache the entire result.  This
//               routine needs to be smart enough to know which
//               data items can be correctly cached, and which cannot.
////////////////////////////////////////////////////////////////////
void ShaderContext::
cp_optimize_mat_spec(ShaderMatSpec &spec) {

  // If we're composing with identity, simplify.
  if (spec._func == SMF_compose) {
    if (spec._part[1] == SMO_identity) {
      spec._func = SMF_first;
    }
  }
  if (spec._func == SMF_compose) {
    if (spec._part[0] == SMO_identity) {
      spec._func = SMF_first;
      spec._part[0] = spec._part[1];
      spec._arg[0] = spec._arg[1];
    }
  }

  // See if either half can be cached.
  bool can_cache_part0 = true;
  bool can_cache_part1 = true;
  if ((spec._part[0] == SMO_model_to_view)||
      (spec._part[0] == SMO_view_to_model)) {
    can_cache_part0 = false;
  }
  if ((spec._part[1] == SMO_model_to_view)||
      (spec._part[1] == SMO_view_to_model)) {
    can_cache_part1 = false;
  }
  
  // See if we can use a compose-with-cache variant.
  if (spec._func == SMF_compose) {
    if (can_cache_part0) {
      spec._func = SMF_compose_cache_first;
    } else if (can_cache_part1) {
      spec._func = SMF_compose_cache_second;
    }
  }
  
  // Determine transform-dependence.
  if (can_cache_part0 && can_cache_part1) {
    spec._trans_dependent = false;
  } else {
    spec._trans_dependent = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderContext::compile_parameter
//       Access: Public
//  Description: Analyzes a parameter and decides how to
//               bind the parameter to some part of panda's
//               internal state.  Updates one of the bind
//               arrays to cause the binding to occur.
//
//               If there is an error, this routine will append
//               an error message onto the error messages.
////////////////////////////////////////////////////////////////////
bool ShaderContext::
compile_parameter(void *reference,
                  const string   &arg_name,
                  ShaderArgType   arg_type,
                  ShaderArgDir    arg_direction,
                  bool            arg_varying,
                  NotifyCategory *arg_cat)
{
  ShaderArgInfo p;
  p._name       = arg_name;
  p._type       = arg_type;
  p._direction  = arg_direction;
  p._varying    = arg_varying;
  p._cat        = arg_cat;

  if (p._name.size() == 0) return true;
  if (p._name[0] == '$') return true;
  vector_string pieces;
  tokenize(p._name, pieces, "_");

  if (pieces.size() < 2) {
    cp_report_error(p, "invalid parameter name");
    return false;
  }

  // Implement vtx parameters - the varying kind.
  
  if (pieces[0] == "vtx") {
    if ((!cp_errchk_parameter_in(p)) ||
        (!cp_errchk_parameter_varying(p)) ||
        (!cp_errchk_parameter_float(p, 1, 4))) {
      return false;
    }
    ShaderVarSpec bind;
    bind._parameter = reference;
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
    if (!cp_errchk_parameter_words(p, 2)) {
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
      cp_report_error(p,"unrecognized matrix name");
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
    
    if ((!cp_errchk_parameter_in(p)) ||
        (!cp_errchk_parameter_uniform(p)))
      return false;
    
    ShaderMatSpec bind;
    bind._parameter = reference;
    bind._func = SMF_compose;

    int next = 1;
    pieces.push_back("");

    // Decide whether this is a matrix or vector.
    if      (pieces[0]=="trans")   bind._piece = SMP_whole;
    else if (pieces[0]=="tpose")   bind._piece = SMP_transpose;
    else if (pieces[0]=="row0")    bind._piece = SMP_row0;
    else if (pieces[0]=="row1")    bind._piece = SMP_row1;
    else if (pieces[0]=="row2")    bind._piece = SMP_row2;
    else if (pieces[0]=="row3")    bind._piece = SMP_row3;
    else if (pieces[0]=="col0")    bind._piece = SMP_col0;
    else if (pieces[0]=="col1")    bind._piece = SMP_col1;
    else if (pieces[0]=="col2")    bind._piece = SMP_col2;
    else if (pieces[0]=="col3")    bind._piece = SMP_col3;
    if ((bind._piece == SMP_whole)||(bind._piece == SMP_transpose)) {
      if (!cp_errchk_parameter_float(p, 16, 16)) return false;
    } else {
      if (!cp_errchk_parameter_float(p, 4, 4)) return false;
    }
    
    // Parse the first half of the clause.
    bool ok = true;
    if ((pieces[next]=="")||(pieces[next]=="of")||(pieces[next]=="to")){
      cp_report_error(p, "argument missing");
      return false;
    } else if (pieces[next] == "world") {
      bind._part[0] = SMO_world_to_view;
      bind._arg[0] = NULL;
      next += 1;
    } else if (pieces[next] == "model") {
      ok &= cp_parse_trans_clause(p, bind, 0, pieces, next, SMO_view_x_to_view, SMO_model_to_view);
    } else if (pieces[next] == "clip") {
      ok &= cp_parse_trans_clause(p, bind, 0, pieces, next, SMO_clip_x_to_view, SMO_clip_to_view);
    } else if (pieces[next] == "view") {
      ok &= cp_parse_trans_clause(p, bind, 0, pieces, next, SMO_view_x_to_view, SMO_identity);
    } else if (pieces[next] == "apiview") {
      ok &= cp_parse_trans_clause(p, bind, 0, pieces, next, SMO_apiview_x_to_view, SMO_apiview_to_view);
    } else if (pieces[next] == "apiclip") {
      ok &= cp_parse_trans_clause(p, bind, 0, pieces, next, SMO_apiclip_x_to_view, SMO_apiclip_to_view);
    } else {
      bind._part[0] = SMO_view_x_to_view;
      bind._arg[0] = InternalName::make(pieces[next]);
      next += 1;
    }

    // Check for errors in the first clause.
    if (!ok) {
      return false;
    }

    // Check for syntactic well-formed-ness.
    if (pieces[next] != "to") {
      cp_report_error(p, "keyword 'to' expected");
      return false;
    } else {
      next += 1;
    }
    
    // Parse the second half of the clause.
    if ((pieces[next]=="")||(pieces[next]=="of")||(pieces[next]=="to")){
      cp_report_error(p, "argument missing");
      return false;
    } else if (pieces[next] == "world") {
      bind._part[1] = SMO_view_to_world;
      bind._arg[1] = NULL;
      next += 1;
    } else if (pieces[next] == "model") {
      ok &= cp_parse_trans_clause(p, bind, 1, pieces, next, SMO_view_to_view_x, SMO_view_to_model);
    } else if (pieces[next] == "clip") {
      ok &= cp_parse_trans_clause(p, bind, 1, pieces, next, SMO_view_to_clip_x, SMO_view_to_clip);
    } else if (pieces[next] == "view") {
      ok &= cp_parse_trans_clause(p, bind, 1, pieces, next, SMO_view_to_view_x, SMO_identity);
    } else if (pieces[next] == "apiview") {
      ok &= cp_parse_trans_clause(p, bind, 1, pieces, next, SMO_view_to_apiview_x, SMO_view_to_apiview);
    } else if (pieces[next] == "apiclip") {
      ok &= cp_parse_trans_clause(p, bind, 1, pieces, next, SMO_view_to_apiclip_x, SMO_view_to_apiclip);
    } else {
      bind._part[1] = SMO_view_to_view_x;
      bind._arg[1] = InternalName::make(pieces[next]);
      next += 1;
    }
    
    // Check for errors in the second clause.
    if (!ok) {
      return false;
    }

    // Check for syntactic well-formed-ness.
    if (pieces[next] != "") {
      cp_report_error(p, "end of line expected");
      return false;
    }
    
    cp_optimize_mat_spec(bind);
    _mat_spec.push_back(bind);
    return true;
  }

  // Keywords to access unusual parameters.
  
  if (pieces[0] == "sys") {
    if ((!cp_errchk_parameter_words(p,2)) ||
        (!cp_errchk_parameter_in(p)) ||
        (!cp_errchk_parameter_uniform(p))) {
      return false;
    }
    ShaderMatSpec bind;
    bind._parameter = reference;
    bind._piece = SMP_row3;
    bind._func = SMF_first;
    bind._part[1] = SMO_identity;
    bind._arg[1] = NULL;
    if (pieces[1] == "pixelsize") {
      if (!cp_errchk_parameter_float(p, 2, 2)) {
        return false;
      }
      bind._part[0] = SMO_pixel_size;
      bind._arg[0] = NULL;
    } else if (pieces[1] == "windowsize") {
      if (!cp_errchk_parameter_float(p, 2, 2)) {
        return false;
      }
      bind._part[0] = SMO_window_size;
      bind._arg[0] = NULL;
    } else if (pieces[1] == "cardcenter") {
      if (!cp_errchk_parameter_float(p, 2, 2)) {
        return false;
      }
      bind._part[0] = SMO_card_center;
      bind._arg[0] = NULL;
    } else {
      cp_report_error(p,"unknown system parameter");
      return false;
    }
    
    cp_optimize_mat_spec(bind);
    _mat_spec.push_back(bind);
    return true;
  }
  
  // Keywords to access textures.
  
  if (pieces[0] == "tex") {
    if ((!cp_errchk_parameter_in(p)) ||
        (!cp_errchk_parameter_uniform(p)) ||
        (!cp_errchk_parameter_sampler(p)))
      return false;
    if ((pieces.size() != 2)&&(pieces.size() != 3)) {
      cp_report_error(p, "Invalid parameter name");
      return false;
    }
    ShaderTexSpec bind;
    bind._parameter = reference;
    bind._name = 0;
    bind._stage = atoi(pieces[1].c_str());
    switch (p._type) {
    case SAT_sampler1d:   bind._desired_type = Texture::TT_1d_texture; break;
    case SAT_sampler2d:   bind._desired_type = Texture::TT_2d_texture; break;
    case SAT_sampler3d:   bind._desired_type = Texture::TT_3d_texture; break;
    case SAT_samplercube: bind._desired_type = Texture::TT_cube_map; break;
    default:
      cp_report_error(p, "Invalid type for a tex-parameter");
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
    if ((!cp_errchk_parameter_words(p,2)) ||
        (!cp_errchk_parameter_in(p)) ||
        (!cp_errchk_parameter_uniform(p)))
      return false;
    switch (p._type) {
    case SAT_float4: {
      ShaderMatSpec bind;
      bind._parameter = reference;
      bind._piece = SMP_row3;
      bind._func = SMF_first;
      bind._part[0] = SMO_vec_constant_x;
      bind._arg[0] = InternalName::make(pieces[1]);
      bind._part[1] = SMO_identity;
      bind._arg[1] = NULL;
      cp_optimize_mat_spec(bind);
      _mat_spec.push_back(bind);
      break;
    }
    case SAT_float4x4: {
      ShaderMatSpec bind;
      bind._parameter = reference;
      bind._piece = SMP_whole;
      bind._func = SMF_first;
      bind._part[0] = SMO_vec_constant_x;
      bind._arg[0] = InternalName::make(pieces[1]);
      bind._part[1] = SMO_identity;
      bind._arg[1] = NULL;
      cp_optimize_mat_spec(bind);
      _mat_spec.push_back(bind);
      break;
    }
    case SAT_sampler1d: {
      ShaderTexSpec bind;
      bind._parameter = reference;
      bind._name = InternalName::make(pieces[1]);
      bind._desired_type=Texture::TT_1d_texture;
      _tex_spec.push_back(bind);
      break;
    }
    case SAT_sampler2d: {
      ShaderTexSpec bind;
      bind._parameter = reference;
      bind._name = InternalName::make(pieces[1]);
      bind._desired_type=Texture::TT_2d_texture;
      _tex_spec.push_back(bind);
      break;
    }
    case SAT_sampler3d: {
      ShaderTexSpec bind;
      bind._parameter = reference;
      bind._name = InternalName::make(pieces[1]);
      bind._desired_type=Texture::TT_3d_texture;
      _tex_spec.push_back(bind);
      break;
    }
    case SAT_samplercube: {
      ShaderTexSpec bind;
      bind._parameter = reference;
      bind._name = InternalName::make(pieces[1]);
      bind._desired_type = Texture::TT_cube_map;
      _tex_spec.push_back(bind);
      break;
    }
    default:
      cp_report_error(p, "Invalid type for a k-parameter");
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

  cp_report_error(p, "unrecognized parameter name");
  return false;
}


