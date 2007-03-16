// Filename: shaderExpansion.cxx
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

#include "pandabase.h"
#include "shaderExpansion.h"
#include "preparedGraphicsObjects.h"

#ifdef HAVE_CG
#include "Cg/cg.h"
#endif

TypeHandle ShaderExpansion::_type_handle;
ShaderExpansion::ExpansionCache ShaderExpansion::_expansion_cache;

////////////////////////////////////////////////////////////////////
//     Function: ShaderExpansion::cp_report_error
//       Access: Public
//  Description: Generate an error message including a description
//               of the specified parameter.
////////////////////////////////////////////////////////////////////
void ShaderExpansion::
cp_report_error(ShaderArgInfo &p, const string &msg)
{
  string vstr;
  if (p._varying) vstr = "varying ";
  else            vstr = "uniform ";

  string dstr = "unknown ";
  if (p._direction == SAD_in)    dstr = "in ";
  if (p._direction == SAD_out)   dstr = "out ";
  if (p._direction == SAD_inout) dstr = "inout ";

  string tstr = "invalid ";
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
  case SAT_unknown: tstr = "unknown "; break;
  }

  string fn = get_name();
  p._cat->error() << fn << ": " << msg << " (" <<
    vstr << dstr << tstr << p._id._name << ")\n";
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderExpansion::cp_errchk_parameter_words
//       Access: Public, Static
//  Description: Make sure the provided parameter contains
//               the specified number of words.  If not, print
//               error message and return false.
////////////////////////////////////////////////////////////////////
bool ShaderExpansion::
cp_errchk_parameter_words(ShaderArgInfo &p, int len)
{
  vector_string words;
  tokenize(p._id._name, words, "_");
  if ((int)words.size() != len) {
    cp_report_error(p, "parameter name has wrong number of words");
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderExpansion::cp_errchk_parameter_in
//       Access: Public, Static
//  Description: Make sure the provided parameter has the
//               'in' direction.  If not, print
//               error message and return false.
////////////////////////////////////////////////////////////////////
bool ShaderExpansion::
cp_errchk_parameter_in(ShaderArgInfo &p)
{
  if (p._direction != SAD_in) {
    cp_report_error(p, "parameter should be declared 'in'");
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderExpansion::cp_errchk_parameter_varying
//       Access: Public, Static
//  Description: Make sure the provided parameter has the
//               correct variance.  If not, print
//               error message and return false.
////////////////////////////////////////////////////////////////////
bool ShaderExpansion::
cp_errchk_parameter_varying(ShaderArgInfo &p)
{
  if (!p._varying) {
    cp_report_error(p, "parameter should be declared 'varying'");
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderExpansion::cp_errchk_parameter_uniform
//       Access: Public, Static
//  Description: Make sure the provided parameter has the
//               correct variance.  If not, print
//               error message and return false.
////////////////////////////////////////////////////////////////////
bool ShaderExpansion::
cp_errchk_parameter_uniform(ShaderArgInfo &p)
{
  if (p._varying) {
    cp_report_error(p, "parameter should be declared 'uniform'");
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderExpansion::cp_errchk_parameter_float
//       Access: Public, Static
//  Description: Make sure the provided parameter has
//               a floating point type.  If not, print
//               error message and return false.
////////////////////////////////////////////////////////////////////
bool ShaderExpansion::
cp_errchk_parameter_float(ShaderArgInfo &p, int lo, int hi)
{
  int nfloat;
  switch (p._type) {
  case SAT_float1: nfloat = 1; break;
  case SAT_float2: nfloat = 2; break;
  case SAT_float3: nfloat = 3; break;
  case SAT_float4: nfloat = 4; break;
  case SAT_float4x4: nfloat = 16; break;
  default: nfloat = 0; break;
  }
  if ((nfloat < lo)||(nfloat > hi)) {
    string msg = "wrong type for parameter: should be float";
    cp_report_error(p, msg);
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderExpansion::cp_errchk_parameter_sampler
//       Access: Public, Static
//  Description: Make sure the provided parameter has
//               a texture type.  If not, print
//               error message and return false.
////////////////////////////////////////////////////////////////////
bool ShaderExpansion::
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
//     Function: ShaderExpansion::cp_parse_trans_clause
//       Access: Public
//  Description: Parses a single clause of a "trans" parameter.
////////////////////////////////////////////////////////////////////
bool ShaderExpansion::
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
//     Function: ShaderExpansion::cp_optimize_mat_spec
//       Access: Public
//  Description: Analyzes a ShaderMatSpec and decides what it should
//               use its cache for.  It can cache the results of any
//               one opcode, or, it can cache the entire result.  This
//               routine needs to be smart enough to know which
//               data items can be correctly cached, and which cannot.
////////////////////////////////////////////////////////////////////
void ShaderExpansion::
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
//     Function: ShaderExpansion::compile_parameter
//       Access: Public
//  Description: Analyzes a parameter and decides how to
//               bind the parameter to some part of panda's
//               internal state.  Updates one of the bind
//               arrays to cause the binding to occur.
//
//               If there is an error, this routine will append
//               an error message onto the error messages.
////////////////////////////////////////////////////////////////////
bool ShaderExpansion::
compile_parameter(const ShaderArgId  &arg_id,
                  ShaderArgType       arg_type,
                  ShaderArgDir        arg_direction,
                  bool                arg_varying,
                  NotifyCategory     *arg_cat)
{
  ShaderArgInfo p;
  p._id         = arg_id;
  p._type       = arg_type;
  p._direction  = arg_direction;
  p._varying    = arg_varying;
  p._cat        = arg_cat;

  if (p._id._name.size() == 0) return true;
  if (p._id._name[0] == '$') return true;
  vector_string pieces;
  tokenize(p._id._name, pieces, "_");

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
    bind._id = arg_id;
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
    bind._id = arg_id;
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
    bind._id = arg_id;
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
    bind._id = arg_id;
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
      bind._id = arg_id;
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
      bind._id = arg_id;
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
      bind._id = arg_id;
      bind._name = InternalName::make(pieces[1]);
      bind._desired_type=Texture::TT_1d_texture;
      _tex_spec.push_back(bind);
      break;
    }
    case SAT_sampler2d: {
      ShaderTexSpec bind;
      bind._id = arg_id;
      bind._name = InternalName::make(pieces[1]);
      bind._desired_type=Texture::TT_2d_texture;
      _tex_spec.push_back(bind);
      break;
    }
    case SAT_sampler3d: {
      ShaderTexSpec bind;
      bind._id = arg_id;
      bind._name = InternalName::make(pieces[1]);
      bind._desired_type=Texture::TT_3d_texture;
      _tex_spec.push_back(bind);
      break;
    }
    case SAT_samplercube: {
      ShaderTexSpec bind;
      bind._id = arg_id;
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


////////////////////////////////////////////////////////////////////
//     Function: ShaderExpansion::clear_parameters
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
void ShaderExpansion::
clear_parameters() {
  _mat_spec.clear();
  _var_spec.clear();
  _tex_spec.clear();
}

#ifdef HAVE_CG
////////////////////////////////////////////////////////////////////
//     Function: ShaderExpansion::cg_parameter_type
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
ShaderExpansion::ShaderArgType ShaderExpansion::
cg_parameter_type(CGparameter p) {
  switch (cgGetParameterType(p)) {
  case CG_FLOAT1:      return ShaderExpansion::SAT_float1;
  case CG_FLOAT2:      return ShaderExpansion::SAT_float2;
  case CG_FLOAT3:      return ShaderExpansion::SAT_float3;
  case CG_FLOAT4:      return ShaderExpansion::SAT_float4;
  case CG_FLOAT4x4:    return ShaderExpansion::SAT_float4x4;
  case CG_SAMPLER1D:   return ShaderExpansion::SAT_sampler1d;
  case CG_SAMPLER2D:   return ShaderExpansion::SAT_sampler2d;
  case CG_SAMPLER3D:   return ShaderExpansion::SAT_sampler3d;
  case CG_SAMPLERCUBE: return ShaderExpansion::SAT_samplercube;
  default:           return ShaderExpansion::SAT_unknown;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderExpansion::cg_parameter_dir
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
ShaderExpansion::ShaderArgDir ShaderExpansion::
cg_parameter_dir(CGparameter p) {
  switch (cgGetParameterDirection(p)) {
  case CG_IN:    return ShaderExpansion::SAD_in;
  case CG_OUT:   return ShaderExpansion::SAD_out;
  case CG_INOUT: return ShaderExpansion::SAD_inout;
  default:       return ShaderExpansion::SAD_unknown;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderExpansion::cg_release_resources
//       Access: Private
//  Description: xyz
////////////////////////////////////////////////////////////////////
void ShaderExpansion::
cg_release_resources() {
  if (_cg_vprogram != 0) {
    cgDestroyProgram(_cg_vprogram);
    _cg_vprogram = 0;
  }
  if (_cg_fprogram != 0) {
    cgDestroyProgram(_cg_fprogram);
    _cg_fprogram = 0;
  }
  if (_cg_context != 0) {
    cgDestroyContext(_cg_context);
    _cg_context = 0;
  }
}
////////////////////////////////////////////////////////////////////
//     Function: ShaderExpansion::cg_compile_entry_point
//       Access: Private
//  Description: xyz
////////////////////////////////////////////////////////////////////
CGprogram ShaderExpansion::
cg_compile_entry_point(char *entry, int active, int ultimate)
{
  CGprogram prog;
  CGerror err;

  if ((active != (int)CG_PROFILE_UNKNOWN) && (active != ultimate)) {
    prog = cgCreateProgram(_cg_context, CG_SOURCE, _text.c_str(),
                           (CGprofile)active, entry, (const char**)NULL);
    if (cgGetError() == CG_NO_ERROR) {
      return prog;
    }
    if (prog != 0) {
      cgDestroyProgram(prog);
    }
  }
  
  prog = cgCreateProgram(_cg_context, CG_SOURCE, _text.c_str(),
                         (CGprofile)ultimate, entry, (const char**)NULL);
  err = cgGetError();
  if (err == CG_NO_ERROR) {
    return prog;
  }
  if (err == CG_COMPILER_ERROR) {
    string listing = cgGetLastListing(_cg_context);
    vector_string errlines;
    tokenize(listing, errlines, "\n");
    for (int i=0; i<(int)errlines.size(); i++) {
      string line = trim(errlines[i]);
      if (line != "") {
        gobj_cat.error() << _name << " " << errlines[i] << "\n";
      }
    }
  } else {
    gobj_cat.error() << _name << ": " << cgGetErrorString(err) << "\n";
  }
  if (prog != 0) {
    cgDestroyProgram(prog);
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderExpansion::cg_compile_shader
//       Access: Private
//  Description: Compiles a Cg shader for a given set of capabilities.
//               If successful, the shader is stored in the instance
//               variables _cg_context, _cg_vprogram, _cg_fprogram.
////////////////////////////////////////////////////////////////////
bool ShaderExpansion::
cg_compile_shader(const ShaderCaps &caps) {

  // If we already tried compiling for this set of caps, there's no point
  // trying again.  Just return the results of the previous compile.
  if (caps == _cg_last_caps) {
    if (_cg_context == 0) {
      return false;
    } else {
      return true;
    }
  }

  _cg_last_caps = caps;

  _cg_context = cgCreateContext();
  
  if (_cg_context == 0) {
    gobj_cat.error() << "could not create a Cg context object.\n";
    return false;
  }
  
  _cg_vprogram = cg_compile_entry_point("vshader",
                                        caps._active_vprofile,
                                        caps._ultimate_vprofile);
  
  _cg_fprogram = cg_compile_entry_point("fshader",
                                        caps._active_fprofile,
                                        caps._ultimate_fprofile);

  if ((_cg_vprogram == 0)||(_cg_fprogram == 0)) {
    cg_release_resources();
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderExpansion::cg_analyze_entry_point
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
bool ShaderExpansion::
cg_analyze_entry_point(CGprogram prog, bool fshader) {
  CGparameter parameter;
  bool success = true;
  for (parameter = cgGetFirstLeafParameter(prog, CG_PROGRAM);
       parameter != 0;
       parameter = cgGetNextLeafParameter(parameter)) {
    CGenum vbl = cgGetParameterVariability(parameter);
    if ((vbl==CG_VARYING)||(vbl==CG_UNIFORM)) {
      ShaderArgId id;
      id._name = cgGetParameterName(parameter);
      id._fshader = fshader;
      id._seqno = -1;
      success &= compile_parameter(id,
                                   cg_parameter_type(parameter),
                                   cg_parameter_dir(parameter),
                                   (vbl == CG_VARYING),
                                   gobj_cat.get_safe_ptr());
    }
  }
  return success;
}


////////////////////////////////////////////////////////////////////
//     Function: ShaderExpansion::cg_analyze_shader
//       Access: Private
//  Description: This subroutine analyzes the parameters of a Cg 
//               shader. The output is stored in instance variables:
//               _mat_spec, _var_spec, and _tex_spec.
//               
//               In order to do this, it is necessary to compile the
//               shader.  It would be a waste of CPU time to compile
//               the shader, analyze the parameters, and then discard
//               the compiled shader.  This would force us to compile it
//               again later, when we need to build the ShaderContext.
//               Instead, we cache the compiled Cg program in instance
//               variables.  Later, a ShaderContext can pull the
//               compiled shader from these instance vars.
//
//               To compile a shader, you need to first choose a profile.
//               There are two contradictory objectives:
//
//               1. If you don't use the gsg's active profile,
//               then the cached compiled shader will not be useful to
//               the ShaderContext.
//
//               2. If you use too weak a profile, then the shader may
//               not compile.  So to guarantee success, you should use
//               the ultimate profile.
//
//               To resolve this conflict, we try the active profile
//               first, and if that doesn't work, we try the ultimate
//               profile.
//
////////////////////////////////////////////////////////////////////
bool ShaderExpansion::
cg_analyze_shader(const ShaderCaps &caps) {

  if (!cg_compile_shader(caps)) {
    return false;
  }

  if (!cg_analyze_entry_point(_cg_fprogram, true)) {
    cg_release_resources();
    clear_parameters();
    return false;
  }
    
  if (_var_spec.size() != 0) {
    gobj_cat.error() << "Cannot use vtx parameters in an fshader\n";
    cg_release_resources();
    clear_parameters();
    return false;
  }

  if (!cg_analyze_entry_point(_cg_vprogram, false)) {
    cg_release_resources();
    clear_parameters();
    return false;
  }
  
  // Assign sequence numbers to all parameters.
  int seqno = 0;
  for (int i=0; i<(int)_mat_spec.size(); i++) {
    _mat_spec[i]._id._seqno = seqno++;
  }
  for (int i=0; i<(int)_tex_spec.size(); i++) {
    _tex_spec[i]._id._seqno = seqno++;
  }
  for (int i=0; i<(int)_var_spec.size(); i++) {
    _var_spec[i]._id._seqno = seqno++;
  }

  // DEBUG: output the generated program
  if (gobj_cat.is_debug()) {
    const char *vertex_program;
    const char *pixel_program;

    vertex_program = cgGetProgramString (_cg_vprogram, CG_COMPILED_PROGRAM);
    pixel_program = cgGetProgramString (_cg_fprogram, CG_COMPILED_PROGRAM);

    gobj_cat.debug() << vertex_program << "\n";
    gobj_cat.debug() << pixel_program << "\n";
  }

  //  // The following code is present to work around a bug in the Cg compiler.
  //  // It does not generate correct code for shadow map lookups when using arbfp1.
  //  // This is a particularly onerous limitation, given that arbfp1 is the only
  //  // Cg target that works on radeons.  I suspect this is an intentional
  //  // omission on nvidia's part.  The following code fetches the output listing,
  //  // detects the error, repairs the code, and resumbits the repaired code to Cg.
  //  if ((_cg_fprofile == CG_PROFILE_ARBFP1) && (gsghint->_supports_shadow_filter)) {
  //    bool shadowunit[32];
  //    bool anyshadow = false;
  //    memset(shadowunit, 0, sizeof(shadowunit));
  //    vector_string lines;
  //    tokenize(cgGetProgramString(_cg_program[SHADER_type_frag],
  //                                CG_COMPILED_PROGRAM), lines, "\n");
  //    // figure out which texture units contain shadow maps.
  //    for (int lineno=0; lineno<(int)lines.size(); lineno++) {
  //      if (lines[lineno].compare(0,21,"#var sampler2DSHADOW ")) {
  //        continue;
  //      }
  //      vector_string fields;
  //      tokenize(lines[lineno], fields, ":");
  //      if (fields.size()!=5) {
  //        continue;
  //      }
  //      vector_string words;
  //      tokenize(trim(fields[2]), words, " ");
  //      if (words.size()!=2) {
  //        continue;
  //      }
  //      int unit = atoi(words[1].c_str());
  //      if ((unit < 0)||(unit >= 32)) {
  //        continue;
  //      }
  //      anyshadow = true;
  //      shadowunit[unit] = true;
  //    }
  //    // modify all TEX statements that use the relevant texture units.
  //    if (anyshadow) {
  //      for (int lineno=0; lineno<(int)lines.size(); lineno++) {
  //        if (lines[lineno].compare(0,4,"TEX ")) {
  //          continue;
  //        }
  //        vector_string fields;
  //        tokenize(lines[lineno], fields, ",");
  //        if ((fields.size()!=4)||(trim(fields[3]) != "2D;")) {
  //          continue;
  //        }
  //        vector_string texunitf;
  //        tokenize(trim(fields[2]), texunitf, "[]");
  //        if ((texunitf.size()!=3)||(texunitf[0] != "texture")||(texunitf[2]!="")) {
  //          continue;
  //        }
  //        int unit = atoi(texunitf[1].c_str());
  //        if ((unit < 0) || (unit >= 32) || (shadowunit[unit]==false)) {
  //          continue;
  //        }
  //        lines[lineno] = fields[0]+","+fields[1]+","+fields[2]+", SHADOW2D;";
  //      }
  //      string result = "!!ARBfp1.0\nOPTION ARB_fragment_program_shadow;\n";
  //      for (int lineno=1; lineno<(int)lines.size(); lineno++) {
  //        result += (lines[lineno] + "\n");
  //      }
  //      _cg_program[2] = _cg_program[SHADER_type_frag];
  //      _cg_program[SHADER_type_frag] =
  //        cgCreateProgram(_cg_context, CG_OBJECT, result.c_str(),
  //                        _cg_profile[SHADER_type_frag], "fshader", (const char**)NULL);
  //      cg_report_errors(s->get_name(), _cg_context);
  //      if (_cg_program[SHADER_type_frag]==0) {
  //        release_resources();
  //        return false;
  //      }
  //    }
  //  }

  return true;
}

////////////////////////////////////////////////////////////////////
//  Function: ShaderExpansion::cg_compile_for
//  Access: Public
//  Description: This routine is used by the ShaderContext constructor
//               to compile the shader.  The CGprogram
//               objects are turned over to the ShaderContext, we no
//               longer own them.
////////////////////////////////////////////////////////////////////
bool ShaderExpansion::
cg_compile_for(const ShaderCaps &caps,
               CGcontext &ctx,
               CGprogram &vprogram,
               CGprogram &fprogram,
               pvector<CGparameter> &map) {

  // Initialize the return values to empty.
  
  ctx = 0;
  vprogram = 0;
  fprogram = 0;
  map.clear();

  // Make sure the shader is compiled for the target caps.
  // Most of the time, it will already be - this is usually a no-op.
  
  if (!cg_compile_shader(caps)) {
    return false;
  }

  // If the compile routine used the ultimate profile instead of the
  // active one, it means the active one isn't powerful enough to
  // compile the shader.
  
  if ((cgGetProgramProfile(_cg_vprogram) != caps._active_vprofile)||
      (cgGetProgramProfile(_cg_fprogram) != caps._active_fprofile)) {
    gobj_cat.error() << "Cg program too complex for driver:" << get_name() << "\n";
    return false;
  }
  
  // Build a parameter map.
  
  int n_mat = (int)_mat_spec.size();
  int n_tex = (int)_tex_spec.size();
  int n_var = (int)_var_spec.size();
  
  map.resize(n_mat + n_tex + n_var);
  
  for (int i=0; i<n_mat; i++) {
    const ShaderArgId &id = _mat_spec[i]._id;
    CGprogram prog = (id._fshader) ? _cg_fprogram : _cg_vprogram;
    map[id._seqno] = cgGetNamedParameter(prog, id._name.c_str());
  }
  for (int i=0; i<n_tex; i++) {
    const ShaderArgId &id = _tex_spec[i]._id;
    CGprogram prog = (id._fshader) ? _cg_fprogram : _cg_vprogram;
    map[id._seqno] = cgGetNamedParameter(prog, id._name.c_str());
  }
  for (int i=0; i<n_var; i++) {
    const ShaderArgId &id = _var_spec[i]._id;
    CGprogram prog = (id._fshader) ? _cg_fprogram : _cg_vprogram;
    map[id._seqno] = cgGetNamedParameter(prog, id._name.c_str());
  }
  
  // Transfer ownership of the compiled shader.
  
  ctx = _cg_context;
  vprogram = _cg_vprogram;
  fprogram = _cg_fprogram;
  
  _cg_context = 0;
  _cg_vprogram = 0;
  _cg_fprogram = 0;
  _cg_last_caps.clear();

  return true;
}
#endif  // HAVE_CG

////////////////////////////////////////////////////////////////////
//  Function: ShaderExpansion::Constructor
//  Access: Private
//  Description: Construct a ShaderExpansion.
////////////////////////////////////////////////////////////////////
ShaderExpansion::
ShaderExpansion(const string &name, const string &text,
                const ShaderCaps &caps) :
  _name(name),
  _text(text),
  _header(""),
  _error_flag(true),
  _parse(0)
{
  string header;
  parse_init();
  parse_line(_header, true, true);
  
#ifdef HAVE_CG
  _cg_context = 0;
  _cg_vprogram = 0;
  _cg_fprogram = 0;
  if (_header == "//Cg") {
    if (cg_analyze_shader(caps)) {
      _error_flag = false;
    }
  } else {
    gobj_cat.error()
      << "Shader is not in a supported shader-language.\n";
    _error_flag = true;
  }
#endif

}

////////////////////////////////////////////////////////////////////
//     Function: ShaderExpansion::Destructor
//       Access: Public
//  Description: Delete the compiled code, if it exists.
////////////////////////////////////////////////////////////////////
ShaderExpansion::
~ShaderExpansion() {
  release_all();
  _expansion_cache.erase(ExpansionKey(_name,_text));
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderExpansion::make
//       Access: Public, Static
//  Description: Create a shader expansion (or reuse one from cache)
////////////////////////////////////////////////////////////////////
PT(ShaderExpansion) ShaderExpansion::
make(const string &name, const string &text,
     const ShaderCaps &caps) {
  ExpansionKey key(name, text);
  ExpansionCache::const_iterator i = _expansion_cache.find(key);
  if (i != _expansion_cache.end()) {
    return i->second;
  }
  ShaderExpansion *result = new ShaderExpansion(name, text, caps);
  _expansion_cache.insert(ExpansionCache::value_type(key,result));
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderExpansion::parse_init
//       Access: Public
//  Description: Set a 'parse pointer' to the beginning of the shader.
////////////////////////////////////////////////////////////////////
void ShaderExpansion::
parse_init() {
  _parse = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderExpansion::parse_line
//       Access: Public
//  Description: Parse a line of text. If 'lt' is true, trim blanks
//               from the left end of the line. If 'rt' is true, trim
//               blanks from the right end (the newline is always
//               trimmed).
////////////////////////////////////////////////////////////////////
void ShaderExpansion::
parse_line(string &result, bool lt, bool rt) {
  int len = _text.size();
  int head = _parse;
  int tail = head;
  while ((tail < len) && (_text[tail] != '\n')) {
    tail++;
  }
  if (tail < len) {
    _parse = tail+1;
  } else {
    _parse = tail;
  }
  if (lt) {
    while ((head < tail)&&(isspace(_text[head]))) head++;
    while ((tail > head)&&(isspace(_text[tail-1]))) tail--;
  }
  result = _text.substr(head, tail-head);
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderExpansion::parse_upto
//       Access: Public
//  Description: Parse lines until you read a line that matches the
//               specified pattern.  Returns all the preceding lines,
//               and if the include flag is set, returns the final
//               line as well.
////////////////////////////////////////////////////////////////////
void ShaderExpansion::
parse_upto(string &result, string pattern, bool include) {
  GlobPattern endpat(pattern);
  int start = _parse;
  int last = _parse;
  while (_parse < (int)(_text.size())) {
    string t;
    parse_line(t, true, true);
    if (endpat.matches(t)) break;
    last = _parse;
  }
  if (include) {
    result = _text.substr(start, _parse - start);
  } else {
    result = _text.substr(start, last - start);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderExpansion::parse_rest
//       Access: Public
//  Description: Returns the rest of the text from the current
//               parse location.
////////////////////////////////////////////////////////////////////
void ShaderExpansion::
parse_rest(string &result) {
  result = _text.substr(_parse, _text.size() - _parse);
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderExpansion::parse_lineno
//       Access: Public
//  Description: Returns the line number of the current parse pointer.
////////////////////////////////////////////////////////////////////
int ShaderExpansion::
parse_lineno() {
  int result = 1;
  for (int i=0; i<_parse; i++) {
    if (_text[i] == '\n') result += 1;
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderExpansion::parse_eof
//       Access: Public
//  Description: Returns true if the parse pointer is at the end of
//               the shader.
////////////////////////////////////////////////////////////////////
bool ShaderExpansion::
parse_eof() {
  return (int)_text.size() == _parse;
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderExpansion::prepare
//       Access: Published
//  Description: Indicates that the shader should be enqueued to be
//               prepared in the indicated prepared_objects at the
//               beginning of the next frame.  This will ensure the
//               texture is already loaded into texture memory if it
//               is expected to be rendered soon.
//
//               Use this function instead of prepare_now() to preload
//               textures from a user interface standpoint.
////////////////////////////////////////////////////////////////////
void ShaderExpansion::
prepare(PreparedGraphicsObjects *prepared_objects) {
  prepared_objects->enqueue_shader(this);
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderExpansion::release
//       Access: Published
//  Description: Frees the texture context only on the indicated object,
//               if it exists there.  Returns true if it was released,
//               false if it had not been prepared.
////////////////////////////////////////////////////////////////////
bool ShaderExpansion::
release(PreparedGraphicsObjects *prepared_objects) {
  Contexts::iterator ci;
  ci = _contexts.find(prepared_objects);
  if (ci != _contexts.end()) {
    ShaderContext *sc = (*ci).second;
    if (sc != (ShaderContext *)NULL) {
      prepared_objects->release_shader(sc);
    } else {
      _contexts.erase(ci);
    }
    return true;
  }

  // Maybe it wasn't prepared yet, but it's about to be.
  return prepared_objects->dequeue_shader(this);
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderExpansion::prepare_now
//       Access: Public
//  Description: Creates a context for the texture on the particular
//               GSG, if it does not already exist.  Returns the new
//               (or old) ShaderContext.  This assumes that the
//               GraphicsStateGuardian is the currently active
//               rendering context and that it is ready to accept new
//               textures.  If this is not necessarily the case, you
//               should use prepare() instead.
//
//               Normally, this is not called directly except by the
//               GraphicsStateGuardian; a texture does not need to be
//               explicitly prepared by the user before it may be
//               rendered.
////////////////////////////////////////////////////////////////////
ShaderContext *ShaderExpansion::
prepare_now(PreparedGraphicsObjects *prepared_objects, 
            GraphicsStateGuardianBase *gsg) {
  Contexts::const_iterator ci;
  ci = _contexts.find(prepared_objects);
  if (ci != _contexts.end()) {
    return (*ci).second;
  }

  ShaderContext *tc = prepared_objects->prepare_shader_now(this, gsg);
  _contexts[prepared_objects] = tc;

  return tc;
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderExpansion::clear_prepared
//       Access: Private
//  Description: Removes the indicated PreparedGraphicsObjects table
//               from the ShaderExpansion's table, without actually releasing
//               the texture.  This is intended to be called only from
//               PreparedGraphicsObjects::release_texture(); it should
//               never be called by user code.
////////////////////////////////////////////////////////////////////
void ShaderExpansion::
clear_prepared(PreparedGraphicsObjects *prepared_objects) {
  Contexts::iterator ci;
  ci = _contexts.find(prepared_objects);
  if (ci != _contexts.end()) {
    _contexts.erase(ci);
  } else {
    // If this assertion fails, clear_prepared() was given a
    // prepared_objects which the texture didn't know about.
    nassertv(false);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderExpansion::release_all
//       Access: Published
//  Description: Frees the context allocated on all objects for which
//               the texture has been declared.  Returns the number of
//               contexts which have been freed.
////////////////////////////////////////////////////////////////////
int ShaderExpansion::
release_all() {
  // We have to traverse a copy of the _contexts list, because the
  // PreparedGraphicsObjects object will call clear_prepared() in response
  // to each release_texture(), and we don't want to be modifying the
  // _contexts list while we're traversing it.
  Contexts temp = _contexts;
  int num_freed = (int)_contexts.size();

  Contexts::const_iterator ci;
  for (ci = temp.begin(); ci != temp.end(); ++ci) {
    PreparedGraphicsObjects *prepared_objects = (*ci).first;
    ShaderContext *sc = (*ci).second;
    if (sc != (ShaderContext *)NULL) {
      prepared_objects->release_shader(sc);
    }
  }
  
  // There might still be some outstanding contexts in the map, if
  // there were any NULL pointers there.  Eliminate them.
  _contexts.clear();

  return num_freed;
}

