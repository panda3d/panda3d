// Filename: shader.cxx
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

#include "pandabase.h"
#include "shader.h"
#include "preparedGraphicsObjects.h"
#include "virtualFileSystem.h"

#ifdef HAVE_CG
#include "Cg/cg.h"
#define JCG_PROFILE_GLSLV ((CGprofile)7007)
#define JCG_PROFILE_GLSLF ((CGprofile)7008)
#endif

TypeHandle Shader::_type_handle;
Shader::LoadTable Shader::_load_table;
Shader::MakeTable Shader::_make_table;
Shader::ShaderCaps Shader::_default_caps;
int Shader::_shaders_generated;
ShaderUtilization Shader::_shader_utilization = SUT_UNSPECIFIED;

////////////////////////////////////////////////////////////////////
//     Function: Shader::cp_report_error
//       Access: Public
//  Description: Generate an error message including a description
//               of the specified parameter.
////////////////////////////////////////////////////////////////////
void Shader::
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

  Filename fn = get_filename();
  p._cat->error() << fn << ": " << msg << " (" <<
    vstr << dstr << tstr << p._id._name << ")\n";
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::cp_errchk_parameter_words
//       Access: Public, Static
//  Description: Make sure the provided parameter contains
//               the specified number of words.  If not, print
//               error message and return false.
////////////////////////////////////////////////////////////////////
bool Shader::
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
//     Function: Shader::cp_errchk_parameter_in
//       Access: Public, Static
//  Description: Make sure the provided parameter has the
//               'in' direction.  If not, print
//               error message and return false.
////////////////////////////////////////////////////////////////////
bool Shader::
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
bool Shader::
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
bool Shader::
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
bool Shader::
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
//     Function: Shader::cp_errchk_parameter_sampler
//       Access: Public, Static
//  Description: Make sure the provided parameter has
//               a texture type.  If not, print
//               error message and return false.
////////////////////////////////////////////////////////////////////
bool Shader::
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
//     Function: Shader::cp_parse_eol
//       Access: Public
//  Description: Make sure the next thing on the word list is EOL
////////////////////////////////////////////////////////////////////
bool Shader::
cp_parse_eol(ShaderArgInfo &p, vector_string &words, int &next) {
  if (words[next] != "") {
    cp_report_error(p, "Too many words in parameter");
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::cp_parse_delimiter
//       Access: Public
//  Description: Pop a delimiter ('to' or 'rel') from the word list.
////////////////////////////////////////////////////////////////////
bool Shader::
cp_parse_delimiter(ShaderArgInfo &p, vector_string &words, int &next) {
  if ((words[next] != "to")&&(words[next] != "rel")) {
    cp_report_error(p, "Keyword 'to' or 'rel' expected");
    return false;
  }
  next += 1;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::cp_parse_non_delimiter
//       Access: Public
//  Description: Pop a non-delimiter word from the word list.
//               Delimiters are 'to' and 'rel.'
////////////////////////////////////////////////////////////////////
string Shader::
cp_parse_non_delimiter(vector_string &words, int &next) {
  const string &nword = words[next];
  if ((nword == "")||(nword == "to")||(nword == "rel")) {
    return "";
  }
  next += 1;
  return nword;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::cp_parse_coord_sys
//       Access: Public
//  Description: Convert a single-word coordinate system name into
//               a PART/ARG of a ShaderMatSpec.
////////////////////////////////////////////////////////////////////
bool Shader::
cp_parse_coord_sys(ShaderArgInfo &p,
                   vector_string &pieces, int &next,
                   ShaderMatSpec &bind, bool fromflag) {

  string word1 = cp_parse_non_delimiter(pieces, next);
  if (pieces[next] == "of") next++;
  string word2 = cp_parse_non_delimiter(pieces, next);

  ShaderMatInput from_single;
  ShaderMatInput from_double;
  ShaderMatInput to_single;
  ShaderMatInput to_double;

  if (word1 == "") {
    cp_report_error(p, "Could not parse coordinate system name");
    return false;
  } else if (word1 == "world") {
    from_single = SMO_world_to_view;
    from_double = SMO_INVALID;
    to_single   = SMO_view_to_world;
    to_double   = SMO_INVALID;
  } else if (word1 == "model") {
    from_single = SMO_model_to_view;
    from_double = SMO_view_x_to_view;
    to_single   = SMO_view_to_model;
    to_double   = SMO_view_to_view_x;
  } else if (word1 == "clip") {
    from_single = SMO_clip_to_view;
    from_double = SMO_clip_x_to_view;
    to_single   = SMO_view_to_clip;
    to_double   = SMO_view_to_clip_x;
  } else if (word1 == "view") {
    from_single = SMO_identity;
    from_double = SMO_view_x_to_view;
    to_single   = SMO_identity;
    to_double   = SMO_view_to_view_x;
  } else if (word1 == "apiview") {
    from_single = SMO_apiview_to_view;
    from_double = SMO_apiview_x_to_view;
    to_single   = SMO_view_to_apiview;
    to_double   = SMO_view_to_apiview_x;
  } else if (word1 == "apiclip") {
    from_single = SMO_apiclip_to_view;
    from_double = SMO_apiclip_x_to_view;
    to_single   = SMO_view_to_apiclip;
    to_double   = SMO_view_to_apiclip_x;
  } else {
    from_single = SMO_view_x_to_view;
    from_double = SMO_view_x_to_view;
    to_single   = SMO_view_to_view_x;
    to_double   = SMO_view_to_view_x;
    word2 = word1;
  }
  
  if (fromflag) {
    if (word2 == "") {
      bind._part[0] = from_single;
      bind._arg[0] = NULL;
    } else {
      if (from_double == SMO_INVALID) {
        cp_report_error(p, "Could not parse coordinate system name");
        return false;
      }
      bind._part[0] = from_double;
      bind._arg[0] = InternalName::make(word2);
    }
  } else {
    if (word2 == "") {
      bind._part[1] = to_single;
      bind._arg[1] = NULL;
    } else {
      if (to_double == SMO_INVALID) {
        cp_report_error(p, "Could not parse coordinate system name");
        return false;
      }
      bind._part[1] = to_double;
      bind._arg[1] = InternalName::make(word2);
    }
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::cp_dependency
//       Access: Public
//  Description: Given ShaderMatInput, returns an indication of what
//               part or parts of the state_and_transform the
//               ShaderMatInput depends upon.
////////////////////////////////////////////////////////////////////
int Shader::
cp_dependency(ShaderMatInput inp) {

  int dep = SSD_general;

  if (inp == SMO_INVALID) {
    return SSD_NONE;
  }
  if (inp == SMO_attr_material) {
    dep |= SSD_material;
  }
  if (inp == SMO_attr_color) {
    dep |= SSD_color;
  }
  if (inp == SMO_attr_colorscale) {
    dep |= SSD_colorscale;
  }
  if ((inp == SMO_model_to_view)||
      (inp == SMO_view_to_model)) {
    dep |= SSD_transform;
  }
  if ((inp == SMO_texpad_x)||
      (inp == SMO_texpix_x)||
      (inp == SMO_alight_x)||
      (inp == SMO_dlight_x)||
      (inp == SMO_plight_x)||
      (inp == SMO_slight_x)||
      (inp == SMO_satten_x)||
      (inp == SMO_clipplane_x)||
      (inp == SMO_mat_constant_x)||
      (inp == SMO_vec_constant_x)||
      (inp == SMO_view_x_to_view)||
      (inp == SMO_view_to_view_x)||
      (inp == SMO_apiview_x_to_view)||
      (inp == SMO_view_to_apiview_x)||
      (inp == SMO_clip_x_to_view)||
      (inp == SMO_view_to_clip_x)||
      (inp == SMO_apiclip_x_to_view)||
      (inp == SMO_view_to_apiclip_x)) {
    dep |= SSD_shaderinputs;
  }
  
  return dep;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::cp_optimize_mat_spec
//       Access: Public
//  Description: Analyzes a ShaderMatSpec and decides what it should
//               use its cache for.  It can cache the results of any
//               one opcode, or, it can cache the entire result.  This
//               routine needs to be smart enough to know which
//               data items can be correctly cached, and which cannot.
////////////////////////////////////////////////////////////////////
void Shader::
cp_optimize_mat_spec(ShaderMatSpec &spec) {

  // If we're composing with identity, simplify.
  
  if (spec._func == SMF_first) {
    spec._part[1] = SMO_INVALID;
    spec._arg[1] = 0;
  }
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

  // Calculate state and transform dependencies.
  
  spec._dep[0] = cp_dependency(spec._part[0]);
  spec._dep[1] = cp_dependency(spec._part[1]);
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::compile_parameter
//       Access: Public
//  Description: Analyzes a parameter and decides how to
//               bind the parameter to some part of panda's
//               internal state.  Updates one of the bind
//               arrays to cause the binding to occur.
//
//               If there is an error, this routine will append
//               an error message onto the error messages.
////////////////////////////////////////////////////////////////////
bool Shader::
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
    
    if (!cp_parse_coord_sys(p, pieces, next, bind, true)) {
      return false;
    }
    if (!cp_parse_delimiter(p, pieces, next)) {
      return false;
    }
    if (!cp_parse_coord_sys(p, pieces, next, bind, false)) {
      return false;
    }
    if (!cp_parse_eol(p, pieces, next)) {
      return false;
    }
    
    cp_optimize_mat_spec(bind);
    _mat_spec.push_back(bind);
    return true;
  }

  // Special parameter: attr_material or attr_color

  if (pieces[0] == "attr") {
    if ((!cp_errchk_parameter_words(p,2)) ||
        (!cp_errchk_parameter_in(p)) ||
        (!cp_errchk_parameter_uniform(p))) {
      return false;
    }
    ShaderMatSpec bind;
    if (pieces[1] == "material") {
      if (!cp_errchk_parameter_float(p,16,16)) {
        return false;
      }
      bind._id = arg_id;
      bind._piece = SMP_transpose;
      bind._func = SMF_first;
      bind._part[0] = SMO_attr_material;
      bind._arg[0] = NULL;
      bind._part[1] = SMO_identity;
      bind._arg[1] = NULL;
    } else if (pieces[1] == "color") {
      if (!cp_errchk_parameter_float(p,3,4)) {
        return false;
      }
      bind._id = arg_id;
      bind._piece = SMP_row3;
      bind._func = SMF_first;
      bind._part[0] = SMO_attr_color;
      bind._arg[0] = NULL;
      bind._part[1] = SMO_identity;
      bind._arg[1] = NULL;
    } else if (pieces[1] == "colorscale") {
      if (!cp_errchk_parameter_float(p,3,4)) {
        return false;
      }
      bind._id = arg_id;
      bind._piece = SMP_row3;
      bind._func = SMF_first;
      bind._part[0] = SMO_attr_colorscale;
      bind._arg[0] = NULL;
      bind._part[1] = SMO_identity;
      bind._arg[1] = NULL;
    } else {
      cp_report_error(p,"Unknown attr parameter.");
      return false;
    }
    
    cp_optimize_mat_spec(bind);
    _mat_spec.push_back(bind);
    return true;
  }

  if (pieces[0] == "color") {
    if ((!cp_errchk_parameter_words(p,1)) ||
        (!cp_errchk_parameter_in(p)) ||
        (!cp_errchk_parameter_uniform(p))) {
      return false;
    }
    ShaderMatSpec bind;
    
    cp_optimize_mat_spec(bind);
    _mat_spec.push_back(bind);
    return true;
  }

  // Keywords to access light properties.

  if (pieces[0] == "alight") {
    if ((!cp_errchk_parameter_words(p,2))||
        (!cp_errchk_parameter_in(p)) ||
        (!cp_errchk_parameter_uniform(p))||
        (!cp_errchk_parameter_float(p,3,4))) {
      return false;
    }
    ShaderMatSpec bind;
    bind._id = arg_id;
    bind._piece = SMP_row3;
    bind._func = SMF_first;
    bind._part[0] = SMO_alight_x;
    bind._arg[0] = InternalName::make(pieces[1]);
    bind._part[1] = SMO_identity;
    bind._arg[1] = NULL;

    cp_optimize_mat_spec(bind);
    _mat_spec.push_back(bind);
    return true;
  }

  if (pieces[0] == "satten") {
    if ((!cp_errchk_parameter_words(p,2))||
        (!cp_errchk_parameter_in(p)) ||
        (!cp_errchk_parameter_uniform(p))||
        (!cp_errchk_parameter_float(p,4,4))) {
      return false;
    }
    ShaderMatSpec bind;
    bind._id = arg_id;
    bind._piece = SMP_row3;
    bind._func = SMF_first;
    bind._part[0] = SMO_satten_x;
    bind._arg[0] = InternalName::make(pieces[1]);
    bind._part[1] = SMO_identity;
    bind._arg[1] = NULL;

    cp_optimize_mat_spec(bind);
    _mat_spec.push_back(bind);
    return true;
  }

  if ((pieces[0]=="dlight")||(pieces[0]=="plight")||(pieces[0]=="slight")) {
    if ((!cp_errchk_parameter_in(p)) ||
        (!cp_errchk_parameter_uniform(p))||
        (!cp_errchk_parameter_float(p,16,16))) {
      return false;
    }
    ShaderMatSpec bind;
    bind._id = arg_id;
    bind._piece = SMP_transpose;
    int next = 1;
    pieces.push_back("");
    if (pieces[next] == "") {
      cp_report_error(p, "Light name expected");
      return false;
    }
    if (pieces[0] == "dlight") {
      bind._func = SMF_transform_dlight;
      bind._part[0] = SMO_dlight_x;
    } else if (pieces[0] == "plight") {
      bind._func = SMF_transform_plight;
      bind._part[0] = SMO_plight_x;
    } else if (pieces[0] == "slight") {
      bind._func = SMF_transform_slight;
      bind._part[0] = SMO_slight_x;
    }
    bind._arg[0] = InternalName::make(pieces[next]);
    next += 1;
    if (!cp_parse_delimiter(p, pieces, next)) {
      return false;
    }
    if (!cp_parse_coord_sys(p, pieces, next, bind, false)) {
      return false;
    }
    if (!cp_parse_eol(p, pieces, next)) {
      return false;
    }
    cp_optimize_mat_spec(bind);
    _mat_spec.push_back(bind);
    return true;
  }
  
  if (pieces[0] == "plane") {
    if ((!cp_errchk_parameter_words(p,2))||
        (!cp_errchk_parameter_in(p)) ||
        (!cp_errchk_parameter_uniform(p))||
        (!cp_errchk_parameter_float(p,4,4))) {
      return false;
    }
    ShaderMatSpec bind;
    bind._id = arg_id;
    bind._piece = SMP_row3;
    bind._func = SMF_first;
    bind._part[0] = SMO_plane_x;
    bind._arg[0] = InternalName::make(pieces[1]);
    bind._part[1] = SMO_identity;
    bind._arg[1] = NULL;

    cp_optimize_mat_spec(bind);
    _mat_spec.push_back(bind);
    return true;
  }

  if (pieces[0] == "clipplane") {
    if ((!cp_errchk_parameter_words(p,2))||
        (!cp_errchk_parameter_in(p)) ||
        (!cp_errchk_parameter_uniform(p))||
        (!cp_errchk_parameter_float(p,4,4))) {
      return false;
    }
    ShaderMatSpec bind;
    bind._id = arg_id;
    bind._piece = SMP_row3;
    bind._func = SMF_first;
    bind._part[0] = SMO_clipplane_x;
    bind._arg[0] = InternalName::make(pieces[1]);
    bind._part[1] = SMO_identity;
    bind._arg[1] = NULL;

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
    if ((!cp_errchk_parameter_in(p)) ||
        (!cp_errchk_parameter_uniform(p)))
      return false;
    // In the case of k-parameters, we allow underscores in the name.
    PT(InternalName) kinputname = InternalName::make(p._id._name.substr(2));
    switch (p._type) {
    case SAT_float4: {
      ShaderMatSpec bind;
      bind._id = arg_id;
      bind._piece = SMP_row3;
      bind._func = SMF_first;
      bind._part[0] = SMO_vec_constant_x;
      bind._arg[0] = kinputname;
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
      bind._arg[0] = kinputname;
      bind._part[1] = SMO_identity;
      bind._arg[1] = NULL;
      cp_optimize_mat_spec(bind);
      _mat_spec.push_back(bind);
      break;
    }
    case SAT_sampler1d: {
      ShaderTexSpec bind;
      bind._id = arg_id;
      bind._name = kinputname;
      bind._desired_type=Texture::TT_1d_texture;
      _tex_spec.push_back(bind);
      break;
    }
    case SAT_sampler2d: {
      ShaderTexSpec bind;
      bind._id = arg_id;
      bind._name = kinputname;
      bind._desired_type=Texture::TT_2d_texture;
      _tex_spec.push_back(bind);
      break;
    }
    case SAT_sampler3d: {
      ShaderTexSpec bind;
      bind._id = arg_id;
      bind._name = kinputname;
      bind._desired_type=Texture::TT_3d_texture;
      _tex_spec.push_back(bind);
      break;
    }
    case SAT_samplercube: {
      ShaderTexSpec bind;
      bind._id = arg_id;
      bind._name = kinputname;
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

  // Keywords to fetch texture parameter data.
  
  if (pieces[0] == "texpad") {
    if ((!cp_errchk_parameter_words(p,2)) ||
        (!cp_errchk_parameter_in(p)) ||
        (!cp_errchk_parameter_uniform(p))||
        (!cp_errchk_parameter_float(p,3,4))) {
      return false;
    }
    ShaderMatSpec bind;
    bind._id = arg_id;
    bind._piece = SMP_row3;
    bind._func = SMF_first;
    bind._part[0] = SMO_texpad_x;
    bind._arg[0] = InternalName::make(pieces[1]);
    bind._part[1] = SMO_identity;
    bind._arg[1] = NULL;
    cp_optimize_mat_spec(bind);
    _mat_spec.push_back(bind);
    return true;
  }

  if (pieces[0] == "texpix") {
    if ((!cp_errchk_parameter_words(p,2)) ||
        (!cp_errchk_parameter_in(p)) ||
        (!cp_errchk_parameter_uniform(p))||
        (!cp_errchk_parameter_float(p,2,4))) {
      return false;
    }
    ShaderMatSpec bind;
    bind._id = arg_id;
    bind._piece = SMP_row3;
    bind._func = SMF_first;
    bind._part[0] = SMO_texpix_x;
    bind._arg[0] = InternalName::make(pieces[1]);
    bind._part[1] = SMO_identity;
    bind._arg[1] = NULL;
    cp_optimize_mat_spec(bind);
    _mat_spec.push_back(bind);
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
//     Function: Shader::clear_parameters
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
void Shader::
clear_parameters() {
  _mat_spec.clear();
  _var_spec.clear();
  _tex_spec.clear();
}

#ifdef HAVE_CG
////////////////////////////////////////////////////////////////////
//     Function: Shader::cg_parameter_type
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
Shader::ShaderArgType Shader::
cg_parameter_type(CGparameter p) {
  switch (cgGetParameterType(p)) {
  case CG_FLOAT1:      return Shader::SAT_float1;
  case CG_FLOAT2:      return Shader::SAT_float2;
  case CG_FLOAT3:      return Shader::SAT_float3;
  case CG_FLOAT4:      return Shader::SAT_float4;
  case CG_FLOAT4x4:    return Shader::SAT_float4x4;
  case CG_SAMPLER1D:   return Shader::SAT_sampler1d;
  case CG_SAMPLER2D:   return Shader::SAT_sampler2d;
  case CG_SAMPLER3D:   return Shader::SAT_sampler3d;
  case CG_SAMPLERCUBE: return Shader::SAT_samplercube;
  default:           return Shader::SAT_unknown;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::cg_parameter_dir
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
Shader::ShaderArgDir Shader::
cg_parameter_dir(CGparameter p) {
  switch (cgGetParameterDirection(p)) {
  case CG_IN:    return Shader::SAD_in;
  case CG_OUT:   return Shader::SAD_out;
  case CG_INOUT: return Shader::SAD_inout;
  default:       return Shader::SAD_unknown;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::cg_release_resources
//       Access: Private
//  Description: xyz
////////////////////////////////////////////////////////////////////
void Shader::
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
//     Function: Shader::cg_compile_entry_point
//       Access: Private
//  Description: xyz
////////////////////////////////////////////////////////////////////
CGprogram Shader::
cg_compile_entry_point(const char *entry, const ShaderCaps &caps, bool fshader)
{
  CGprogram prog;
  CGerror err;
  const char *compiler_args[100];
  int nargs = 0;
  int override = fshader ? _cg_fprofile : _cg_vprofile;
  int active = fshader ? caps._active_fprofile : caps._active_vprofile;
  int ultimate = fshader ? caps._ultimate_fprofile : caps._ultimate_vprofile;

  cgGetError();

  if (fshader && caps._bug_list.count(SBUG_ati_draw_buffers)) {
    compiler_args[nargs++] = "-po";
    compiler_args[nargs++] = "ATI_draw_buffers";
  }
  compiler_args[nargs] = 0;

  // If someone has explicitly set a profile, use it.
  if (override != (int)CG_PROFILE_UNKNOWN) {
    prog = cgCreateProgram(_cg_context, CG_SOURCE, _text.c_str(),
                           (CGprofile)override, entry, (const char **)compiler_args);
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
          gobj_cat.error() << get_filename() << ": " << errlines[i] << "\n";
        }
      }
    } else {
      gobj_cat.error() << get_filename() << ": " << cgGetErrorString(err) << "\n";
    }
    if (fshader) {
      gobj_cat.error() << "Fragment shader failed to compile with profile '"
        << cgGetProfileString((CGprofile)override) << "'!\n";
    } else {
      gobj_cat.error() << "Vertex shader failed to compile with profile '"
        << cgGetProfileString((CGprofile)override) << "'!\n";
    }
    if (prog != 0) {
      cgDestroyProgram(prog);
    }
    return 0;
  }
  
  if ((active != (int)CG_PROFILE_UNKNOWN) && (active != ultimate)) {
    prog = cgCreateProgram(_cg_context, CG_SOURCE, _text.c_str(),
                           (CGprofile)active, entry, (const char **)compiler_args);
    if (cgGetError() == CG_NO_ERROR) {
      return prog;
    }
    if (prog != 0) {
      cgDestroyProgram(prog);
    }
  }
  
  prog = cgCreateProgram(_cg_context, CG_SOURCE, _text.c_str(),
                         (CGprofile)ultimate, entry, (const char **)NULL);
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
        gobj_cat.error() << get_filename() << ": " << errlines[i] << "\n";
      }
    }
  } else {
    gobj_cat.error() << get_filename() << ": " << cgGetErrorString(err) << "\n";
  }
  if (prog != 0) {
    cgDestroyProgram(prog);
  }

  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::cg_compile_shader
//       Access: Private
//  Description: Compiles a Cg shader for a given set of capabilities.
//               If successful, the shader is stored in the instance
//               variables _cg_context, _cg_vprogram, _cg_fprogram.
////////////////////////////////////////////////////////////////////
bool Shader::
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
  
  gobj_cat.debug() << "Compiling Shader: \n" << _text << "\n";

  if (_cg_context == 0) {
    gobj_cat.error() << "could not create a Cg context object.\n";
    return false;
  }
  
  _cg_vprogram = cg_compile_entry_point("vshader", caps, false);
  _cg_fprogram = cg_compile_entry_point("fshader", caps, true);

  if ((_cg_vprogram == 0)||(_cg_fprogram == 0)) {
    cg_release_resources();
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::cg_analyze_entry_point
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
bool Shader::
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
//     Function: Shader::cg_analyze_shader
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
bool Shader::
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
//  Function: Shader::cg_compile_for
//  Access: Public
//  Description: This routine is used by the ShaderContext constructor
//               to compile the shader.  The CGprogram
//               objects are turned over to the ShaderContext, we no
//               longer own them.
////////////////////////////////////////////////////////////////////
bool Shader::
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
  
  _default_caps = caps;
  if (!cg_compile_shader(caps)) {
    return false;
  }

  // If the compile routine used the ultimate profile instead of the
  // active one, it means the active one isn't powerful enough to
  // compile the shader.
  // This does not apply when a custom profile is set.
  
  if ((_cg_vprofile == CG_PROFILE_UNKNOWN && cgGetProgramProfile(_cg_vprogram) != caps._active_vprofile) ||
      (_cg_fprofile == CG_PROFILE_UNKNOWN && cgGetProgramProfile(_cg_fprogram) != caps._active_fprofile)) {
    gobj_cat.error() << "Cg program too complex for driver: "
      << get_filename() << ". Try choosing a different profile.\n";
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
    if (cgGetParameterBaseResource(map[id._seqno]) == CG_UNDEFINED) {
      map[id._seqno] = 0;
    }
  }
  for (int i=0; i<n_var; i++) {
    const ShaderArgId &id = _var_spec[i]._id;
    CGprogram prog = (id._fshader) ? _cg_fprogram : _cg_vprogram;
    map[id._seqno] = cgGetNamedParameter(prog, id._name.c_str());
    if (cgGetParameterBaseResource(map[id._seqno]) == CG_UNDEFINED) {
      map[id._seqno] = 0;
    }
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
//  Function: Shader::Constructor
//  Access: Private
//  Description: Construct a Shader.
////////////////////////////////////////////////////////////////////
Shader::
Shader(const Filename &filename, const string &text, const string &vprofile, const string &fprofile) :
  _filename(filename),
  _text(text),
  _header(""),
  _error_flag(true),
  _parse(0),
  _loaded(false)
{
  string header;
  parse_init();
  parse_line(_header, true, true);
  
#ifdef HAVE_CG
  _error_flag = false;
  _cg_context = 0;
  _cg_vprogram = 0;
  _cg_fprogram = 0;
  _cg_vprofile = CG_PROFILE_UNKNOWN;
  _cg_fprofile = CG_PROFILE_UNKNOWN;
  if (vprofile != "") {
    CGprofile p = cgGetProfile(vprofile.c_str());
    if (p == CG_PROFILE_UNKNOWN) {
      gobj_cat.error() << "Invalid vertex profile: " << vprofile << "\n";
      _error_flag = true;
    }
    _cg_vprofile = (int)p;
  }
  if (fprofile != "") {
    CGprofile p = cgGetProfile(fprofile.c_str());
    if (p == CG_PROFILE_UNKNOWN) {
      gobj_cat.error() << "Invalid fragment profile: " << fprofile << "\n";
      _error_flag = true;
    }
    _cg_fprofile = (int)p;
  }
  if (_default_caps._ultimate_vprofile == 0) {
    _default_caps._active_vprofile = CG_PROFILE_UNKNOWN;
    _default_caps._active_fprofile = CG_PROFILE_UNKNOWN;
    _default_caps._ultimate_vprofile = JCG_PROFILE_GLSLV;
    _default_caps._ultimate_fprofile = JCG_PROFILE_GLSLF;
  }
  if (_header == "//Cg") {
    if (!cg_analyze_shader(_default_caps)) {
      _error_flag = true;
    }
  } else {
    gobj_cat.error()
      << "Shader is not in a supported shader-language.\n";
    _error_flag = true;
  }
#endif

}

////////////////////////////////////////////////////////////////////
//     Function: Shader::Destructor
//       Access: Public
//  Description: Delete the compiled code, if it exists.
////////////////////////////////////////////////////////////////////
Shader::
~Shader() {
  release_all();
  if (_loaded) {
    _load_table.erase(_filename);
  } else {
    _make_table.erase(_text);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::load
//       Access: Published, Static
//  Description: Loads the shader with the given filename.
//               If the optional vshader or fshader parameters
//               are set and non-empty, it will be used to override
//               all other profile settings (it even overrides
//               the basic-shaders-only flag) and forces the shader
//               to use the given profile.
////////////////////////////////////////////////////////////////////
PT(Shader) Shader::
load(const string &file, const string &vprofile, const string &fprofile) {
  return load(Filename(file), vprofile, fprofile);
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::load
//       Access: Published, Static
//  Description: Loads the shader with the given filename.
//               If the optional vshader or fshader parameters
//               are set and non-empty, it will be used to override
//               all other profile settings (it even overrides
//               the basic-shaders-only flag) and forces the shader
//               to use the given profile.
////////////////////////////////////////////////////////////////////
PT(Shader) Shader::
load(const Filename &file, const string &vprofile, const string &fprofile) {
  LoadTable::const_iterator i = _load_table.find(file);
  if (i != _load_table.end()) {
    return i->second;
  }
  string body;
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  if (!vfs->read_file(file, body, true)) {
    gobj_cat.error() << "Could not read shader file: " << file << "\n";
    return NULL;
  }
  PT(Shader) result = new Shader(file, body, vprofile, fprofile);
  result->_loaded = true;
  _load_table[file] = result;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::make
//       Access: Published, Static
//  Description: Loads the shader, using the string as shader body.
//               If the optional vshader or fshader parameters
//               are set and non-empty, it will be used to override
//               all other profile settings (it even overrides
//               the basic-shaders-only flag) and forces the shader
//               to use the given profile.
////////////////////////////////////////////////////////////////////
PT(Shader) Shader::
make(const string &body, const string &vprofile, const string &fprofile) {
  MakeTable::const_iterator i = _make_table.find(body);
  if (i != _make_table.end()) {
    return i->second;
  }
  PT(Shader) result = new Shader("created-shader", body, vprofile, fprofile);
  _make_table[body] = result;
  if (dump_generated_shaders) {
    ostringstream fns;
    int index = _shaders_generated ++;
    fns << "genshader" << index;
    string fn = fns.str();
    gobj_cat.warning() << "Dumping shader: " << fn << "\n";
    pofstream s;
    s.open(fn.c_str());
    s << body;
    s.close();
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::parse_init
//       Access: Public
//  Description: Set a 'parse pointer' to the beginning of the shader.
////////////////////////////////////////////////////////////////////
void Shader::
parse_init() {
  _parse = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::parse_line
//       Access: Public
//  Description: Parse a line of text. If 'lt' is true, trim blanks
//               from the left end of the line. If 'rt' is true, trim
//               blanks from the right end (the newline is always
//               trimmed).
////////////////////////////////////////////////////////////////////
void Shader::
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
//     Function: Shader::parse_upto
//       Access: Public
//  Description: Parse lines until you read a line that matches the
//               specified pattern.  Returns all the preceding lines,
//               and if the include flag is set, returns the final
//               line as well.
////////////////////////////////////////////////////////////////////
void Shader::
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
//     Function: Shader::parse_rest
//       Access: Public
//  Description: Returns the rest of the text from the current
//               parse location.
////////////////////////////////////////////////////////////////////
void Shader::
parse_rest(string &result) {
  result = _text.substr(_parse, _text.size() - _parse);
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::parse_lineno
//       Access: Public
//  Description: Returns the line number of the current parse pointer.
////////////////////////////////////////////////////////////////////
int Shader::
parse_lineno() {
  int result = 1;
  for (int i=0; i<_parse; i++) {
    if (_text[i] == '\n') result += 1;
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::parse_eof
//       Access: Public
//  Description: Returns true if the parse pointer is at the end of
//               the shader.
////////////////////////////////////////////////////////////////////
bool Shader::
parse_eof() {
  return (int)_text.size() == _parse;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::prepare
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
void Shader::
prepare(PreparedGraphicsObjects *prepared_objects) {
  prepared_objects->enqueue_shader(this);
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::is_prepared
//       Access: Published
//  Description: Returns true if the shader has already been prepared
//               or enqueued for preparation on the indicated GSG,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool Shader::
is_prepared(PreparedGraphicsObjects *prepared_objects) const {
  Contexts::const_iterator ci;
  ci = _contexts.find(prepared_objects);
  if (ci != _contexts.end()) {
    return true;
  }
  return prepared_objects->is_shader_queued(this);
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::release
//       Access: Published
//  Description: Frees the texture context only on the indicated object,
//               if it exists there.  Returns true if it was released,
//               false if it had not been prepared.
////////////////////////////////////////////////////////////////////
bool Shader::
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
//     Function: Shader::prepare_now
//       Access: Published
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
ShaderContext *Shader::
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
//     Function: Shader::clear_prepared
//       Access: Private
//  Description: Removes the indicated PreparedGraphicsObjects table
//               from the Shader's table, without actually releasing
//               the texture.  This is intended to be called only from
//               PreparedGraphicsObjects::release_texture(); it should
//               never be called by user code.
////////////////////////////////////////////////////////////////////
void Shader::
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
//     Function: Shader::release_all
//       Access: Published
//  Description: Frees the context allocated on all objects for which
//               the texture has been declared.  Returns the number of
//               contexts which have been freed.
////////////////////////////////////////////////////////////////////
int Shader::
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

////////////////////////////////////////////////////////////////////
//     Function: Shader::ShaderCapabilities::clear()
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void Shader::ShaderCaps::
clear() {
#ifdef HAVE_CG
  _active_vprofile = 0;
  _active_fprofile = 0;
  _ultimate_vprofile = 0;
  _ultimate_fprofile = 0;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::register_with_read_factory
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
void Shader::
register_with_read_factory() {
  // IMPLEMENT ME
}

