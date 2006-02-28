// Filename: shaderContext.h
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

#ifndef SHADERCONTEXT_H
#define SHADERCONTEXT_H

#include "pandabase.h"
#include "internalName.h"
#include "savedContext.h"
#include "shaderExpansion.h"

////////////////////////////////////////////////////////////////////
//       Class : ShaderContext
// Description : The ShaderContext is meant to contain the compiled
//               version of a shader string.  ShaderContext is an
//               abstract base class, there will be a subclass of it
//               for each shader language and graphics API.
//               Since the languages are so different and the
//               graphics APIs have so little in common, the base
//               class contains almost nothing.  All the implementation
//               details are in the subclasses.
////////////////////////////////////////////////////////////////////

class EXPCL_PANDA ShaderContext: public SavedContext {
public:
  INLINE ShaderContext(ShaderExpansion *se);
  
  ShaderExpansion *_shader_expansion;

public:
  enum {
    SHADER_type_vert=0,
    SHADER_type_frag=1,
    SHADER_type_both=2,
  };

  enum ShaderArgType {
    SAT_float1,
    SAT_float2,
    SAT_float3,
    SAT_float4,
    SAT_float4x4,
    SAT_sampler1d,
    SAT_sampler2d,
    SAT_sampler3d,
    SAT_samplercube,
    SAT_unknown,
  };

  enum ShaderArgDir {
    SAD_in,
    SAD_out,
    SAD_inout,
    SAD_unknown,
  };

  enum ShaderMatOp {
    SMO_identity,

    SMO_modelview,
    SMO_projection,
    SMO_modelproj,
    
    SMO_window_size,
    SMO_pixel_size,
    SMO_card_center,
    
    SMO_mat_constant_x,
    SMO_vec_constant_x,
    
    SMO_world_to_view,
    SMO_view_to_world_C,

    SMO_model_to_view,
    SMO_view_to_model_C,

    SMO_apiview_to_view,
    SMO_view_to_apiview_C,

    SMO_clip_to_view,
    SMO_view_to_clip_C,

    SMO_apiclip_to_view,
    SMO_view_to_apiclip_C,
    
    SMO_view_x_to_view,
    SMO_view_to_view_x_C,

    SMO_apiview_x_to_view,
    SMO_view_to_apiview_x_C,

    SMO_clip_x_to_view,
    SMO_view_to_clip_x_C,

    SMO_apiclip_x_to_view,
    SMO_view_to_apiclip_x_C,
    
    SMO_transpose,
    SMO_noop,
  };

  enum ShaderMatPiece {
    SMP_whole,
    SMP_transpose,
    SMP_row0,
    SMP_row1,
    SMP_row2,
    SMP_row3,
    SMP_col0,
    SMP_col1,
    SMP_col2,
    SMP_col3,
  };

  struct ShaderMatSpec {
    pvector<ShaderMatOp>      _opcodes;
    pvector<PT(InternalName)> _args;
    ShaderMatPiece            _piece;
    bool                      _trans_dependent;
    void                     *_parameter;
  };

  struct ShaderTexSpec {
    PT(InternalName)  _name;
    int               _stage;
    int               _desired_type;
    PT(InternalName)  _suffix;
    void             *_parameter;
  };

  struct ShaderVarSpec {
    PT(InternalName) _name;
    int              _append_uv;
    void            *_parameter;
  };

  struct ShaderArgInfo {
    string          _name;
    ShaderArgType   _type;
    ShaderArgDir    _direction;
    bool            _varying;
    NotifyCategory *_cat;
  };

protected:
  pvector <ShaderMatSpec> _mat_spec;
  pvector <ShaderTexSpec> _tex_spec;
  pvector <ShaderVarSpec> _var_spec;
  
private:
  // These functions are only called from compile_parameter.

  void cp_report_error(ShaderArgInfo &arg, const string &msg);
  bool cp_errchk_parameter_words(ShaderArgInfo &arg, int len);
  bool cp_errchk_parameter_in(ShaderArgInfo &arg);
  bool cp_errchk_parameter_varying(ShaderArgInfo &arg);
  bool cp_errchk_parameter_uniform(ShaderArgInfo &arg);
  bool cp_errchk_parameter_float(ShaderArgInfo &arg, int lo, int hi);
  bool cp_errchk_parameter_sampler(ShaderArgInfo &arg);
  bool cp_parse_trans_clause(ShaderArgInfo &arg,
                             ShaderMatSpec &spec,
                             const vector_string &pieces,
                             int &next,
                             ShaderMatOp ofop,
                             ShaderMatOp op);

protected:
  bool compile_parameter(void *reference,
                         const string   &arg_name,
                         ShaderArgType   arg_type,
                         ShaderArgDir    arg_direction,
                         bool            arg_varying,
                         NotifyCategory *arg_cat);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedObject::init_type();
    register_type(_type_handle, "ShaderContext",
                  TypedObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "shaderContext.I"

#endif
