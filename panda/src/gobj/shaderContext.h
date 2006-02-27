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
#ifdef HAVE_CG
// Instead of including the whole header file, just include these stubs.
typedef struct _CGcontext *CGcontext;
typedef struct _CGprogram *CGprogram;
typedef struct _CGparameter *CGparameter;
#endif


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

protected:
  pvector <ShaderMatSpec> _mat_spec;
  pvector <ShaderTexSpec> _tex_spec;
  pvector <ShaderVarSpec> _var_spec;
  
#ifdef HAVE_CG
private:
  // These functions are only called by 'compile_cg_parameter'
  NotifyCategory *_cg_report_cat;
  void report_cg_parameter_error(CGparameter p, const string &msg);
  bool errchk_cg_parameter_words(CGparameter p, int len);
  bool errchk_cg_parameter_in(CGparameter p);
  bool errchk_cg_parameter_varying(CGparameter p);
  bool errchk_cg_parameter_uniform(CGparameter p);
  bool errchk_cg_parameter_float(CGparameter p, int lo, int hi);
  bool errchk_cg_parameter_sampler(CGparameter p);
  bool parse_cg_trans_clause(CGparameter p,
                             ShaderMatSpec &spec,
                             const vector_string &pieces,
                             int &next,
                             ShaderMatOp ofop,
                             ShaderMatOp op);

protected:
  void report_cg_compile_errors(const string &file, CGcontext ctx,
                                NotifyCategory *cat);
  bool compile_cg_parameter(CGparameter p, NotifyCategory *cat);
#endif

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
