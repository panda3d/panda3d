// Filename: shaderExpansion.h
// Created by:  jyelon (01Sep05)
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

#ifndef SHADEREXPANSION_H
#define SHADEREXPANSION_H

#include "pandabase.h"
#include "typedReferenceCount.h"
#include "namable.h"
#include "graphicsStateGuardianBase.h"
#include "internalName.h"

#ifdef HAVE_CG
// I don't want to include the Cg header file into panda as a 
// whole.  Instead, I'll just excerpt some opaque declarations.
typedef struct _CGcontext *CGcontext;
typedef struct _CGprogram *CGprogram;
typedef struct _CGparameter *CGparameter;
#endif

////////////////////////////////////////////////////////////////////
//       Class : ShaderExpansion
//      Summary: A shader can contain context-sensitive macros.
//               A ShaderExpansion is the output you get when you
//               run the macro preprocessor on a shader.
//               The ShaderExpansion contains the shader's 
//               macroexpanded text, an analysis of the shader's
//               parameters, and a map of ShaderContext
//               objects.
////////////////////////////////////////////////////////////////////

class EXPCL_PANDA ShaderExpansion: public TypedReferenceCount {

PUBLISHED:
  
  INLINE const string &get_name() const;
  INLINE const string &get_text() const;
  INLINE const string &get_header() const;
  INLINE bool get_error_flag() const;

  void prepare(PreparedGraphicsObjects *prepared_objects);
  bool release(PreparedGraphicsObjects *prepared_objects);
  int release_all();
  
public:

  enum ShaderMatInput {
    SMO_identity,

    SMO_window_size,
    SMO_pixel_size,
    SMO_card_center,
    
    SMO_mat_constant_x,
    SMO_vec_constant_x,
    
    SMO_world_to_view,
    SMO_view_to_world,

    SMO_model_to_view,
    SMO_view_to_model,

    SMO_apiview_to_view,
    SMO_view_to_apiview,

    SMO_clip_to_view,
    SMO_view_to_clip,

    SMO_apiclip_to_view,
    SMO_view_to_apiclip,
    
    SMO_view_x_to_view,
    SMO_view_to_view_x,

    SMO_apiview_x_to_view,
    SMO_view_to_apiview_x,

    SMO_clip_x_to_view,
    SMO_view_to_clip_x,

    SMO_apiclip_x_to_view,
    SMO_view_to_apiclip_x,
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

  enum ShaderMatFunc {
    SMF_compose,
    SMF_compose_cache_first,
    SMF_compose_cache_second,
    SMF_first,
  };

  struct ShaderArgId {
    string _name;
    bool   _fshader;
    int    _seqno;
  };
  
  struct ShaderMatSpec {
    ShaderArgId       _id;
    ShaderMatFunc     _func;
    ShaderMatInput    _part[2];
    PT(InternalName)  _arg[2];
    LMatrix4f         _cache;
    ShaderMatPiece    _piece;
    bool              _trans_dependent;
  };

  struct ShaderTexSpec {
    ShaderArgId       _id;
    PT(InternalName)  _name;
    int               _stage;
    int               _desired_type;
    PT(InternalName)  _suffix;
  };

  struct ShaderVarSpec {
    ShaderArgId       _id;
    PT(InternalName)  _name;
    int               _append_uv;
  };

  struct ShaderArgInfo {
    ShaderArgId       _id;
    ShaderArgType     _type;
    ShaderArgDir      _direction;
    bool              _varying;
    NotifyCategory   *_cat;
  };

  struct ShaderCaps {
#ifdef HAVE_CG
    int _active_vprofile;
    int _active_fprofile;
    int _ultimate_vprofile;
    int _ultimate_fprofile;
#endif;
    INLINE void clear();
    INLINE bool operator == (const ShaderCaps &other) const;
    INLINE ShaderCaps();
  };

 private:
  // These routines help split the shader into sections,
  // for those shader implementations that need to do so.
  void parse_init();
  void parse_line(string &result, bool rt, bool lt);
  void parse_upto(string &result, string pattern, bool include);
  void parse_rest(string &result);
  int  parse_lineno();
  bool parse_eof();
  
  void cp_report_error(ShaderArgInfo &arg, const string &msg);
  bool cp_errchk_parameter_words(ShaderArgInfo &arg, int len);
  bool cp_errchk_parameter_in(ShaderArgInfo &arg);
  bool cp_errchk_parameter_varying(ShaderArgInfo &arg);
  bool cp_errchk_parameter_uniform(ShaderArgInfo &arg);
  bool cp_errchk_parameter_float(ShaderArgInfo &arg, int lo, int hi);
  bool cp_errchk_parameter_sampler(ShaderArgInfo &arg);
  bool cp_parse_trans_clause(ShaderArgInfo &arg,
                             ShaderMatSpec &spec,
                             int part,
                             const vector_string &pieces,
                             int &next,
                             ShaderMatInput ofop,
                             ShaderMatInput op);
  void cp_optimize_mat_spec(ShaderMatSpec &spec);
  
  bool compile_parameter(const ShaderArgId    &arg_id,
                         ShaderArgType         arg_type,
                         ShaderArgDir          arg_direction,
                         bool                  arg_varying,
                         NotifyCategory       *arg_cat);

  void clear_parameters();

#ifdef HAVE_CG
 private:
  ShaderArgType cg_parameter_type(CGparameter p);
  ShaderArgDir  cg_parameter_dir(CGparameter p);
  CGprogram     cg_compile_entry_point(char *entry, int active, int ultimate);
  bool          cg_analyze_entry_point(CGprogram prog, bool fshader);
  bool          cg_analyze_shader(const ShaderCaps &caps);
  bool          cg_compile_shader(const ShaderCaps &caps);
  void          cg_release_resources();
  
  ShaderCaps _cg_last_caps;
  CGcontext  _cg_context;
  CGprogram  _cg_vprogram;
  CGprogram  _cg_fprogram;
  
 public:

  bool          cg_compile_for(const ShaderCaps &caps,
                               CGcontext &ctx,
                               CGprogram &vprogram,
                               CGprogram &fprogram,
                               pvector<CGparameter> &map);
  
#endif

 public:
  pvector <ShaderMatSpec> _mat_spec;
  pvector <ShaderTexSpec> _tex_spec;
  pvector <ShaderVarSpec> _var_spec;
  
 protected:
  string         _name;
  string         _text;
  string         _header;
  bool           _error_flag;
  int            _parse;
  
  typedef pair < string, string > ExpansionKey;
  typedef pmap < ExpansionKey, ShaderExpansion * > ExpansionCache;
  static ExpansionCache _expansion_cache;

  friend class ShaderContext;
  friend class PreparedGraphicsObjects;

  typedef pmap <PreparedGraphicsObjects *, ShaderContext *> Contexts;
  Contexts _contexts;

 public:  
  ShaderContext *prepare_now(PreparedGraphicsObjects *prepared_objects, 
                             GraphicsStateGuardianBase *gsg);
  
 private:  
  ShaderExpansion(const string &name, const string &text,
                  const ShaderCaps &caps);
  void clear_prepared(PreparedGraphicsObjects *prepared_objects);

 public:
  static PT(ShaderExpansion) make(const string &name, const string &body,
                                  const ShaderCaps &caps);
  
  ~ShaderExpansion();
  
 public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "ShaderExpansion",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

 private:
  static TypeHandle _type_handle;
};

#include "shaderExpansion.I"

#endif
