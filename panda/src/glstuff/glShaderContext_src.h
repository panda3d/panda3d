// Filename: glShaderContext_src.h
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
#ifdef HAVE_CGGL
#include "Cg/cgGL.h"
#endif
#include "string_utils.h"
#include "internalName.h"
#include "shaderExpansion.h"
#include "shaderContext.h"

class CLP(GraphicsStateGuardian);

////////////////////////////////////////////////////////////////////
//       Class : GLShaderContext
// Description : xyz
////////////////////////////////////////////////////////////////////

class EXPCL_GL CLP(ShaderContext): public ShaderContext {
public:
  CLP(ShaderContext)(ShaderExpansion *s);
  ~CLP(ShaderContext)();
  typedef CLP(GraphicsStateGuardian) GSG;

  INLINE bool valid(void);
  void bind(GSG *gsg);
  void unbind();
  void issue_parameters(GSG *gsg);
  void issue_transform(GSG *gsg);
  void disable_shader_vertex_arrays(GSG *gsg);
  void update_shader_vertex_arrays(CLP(ShaderContext) *prev, GSG *gsg);
  void disable_shader_texture_bindings(GSG *gsg);
  void update_shader_texture_bindings(CLP(ShaderContext) *prev, GSG *gsg);

private:

#ifdef HAVE_CGGL
  enum ShaderAutoValue {
    // This first batch of constants cleverly lines up
    // with the Cg constant values.  Don't insert anything.
    SIC_mat_modelview,
    SIC_inv_modelview,
    SIC_tps_modelview,
    SIC_itp_modelview,
    SIC_mat_projection,
    SIC_inv_projection,
    SIC_tps_projection,
    SIC_itp_projection,
    SIC_mat_texture,
    SIC_inv_texture,
    SIC_tps_texture,
    SIC_itp_texture,
    SIC_mat_modelproj,
    SIC_inv_modelproj,
    SIC_tps_modelproj,
    SIC_itp_modelproj,
    // From this point forward, it's okay to insert stuff.
    SIC_sys_windowsize,
    SIC_sys_pixelsize,
  };
  struct ShaderAutoBind {
    CGparameter parameter;
    int value;
  };
  struct ShaderArgBind {
    CGparameter parameter;
    PT(InternalName) name;
  };
  struct ShaderTexBind {
    CGparameter parameter;
    PT(InternalName) name;
    int stage;
    int desiredtype;
    PT(InternalName) suffix;
  };
  struct ShaderTransBind {
    CGparameter parameter;
    PT(InternalName) src_name;
    PT(InternalName) rel_name;
    int trans_piece;
  };
  struct ShaderVarying {
    CGparameter parameter;
    PT(InternalName) name;
    int append_uv;
  };
  CGcontext _cg_context;
  CGprofile _cg_profile[2];
  CGprogram _cg_program[2];
  string    _cg_errors;
  
  // These arrays contain lists of "bindings." They
  // tell us how to fill the shader's input parameters.
  vector <ShaderAutoBind> _cg_auto_trans;
  vector <ShaderAutoBind> _cg_auto_param;
  vector <ShaderArgBind> _cg_fbind;
  vector <ShaderArgBind> _cg_npbind;
  vector <ShaderTexBind> _cg_texbind;
  vector <ShaderTransBind> _cg_transform_bind;
  vector <ShaderTransBind> _cg_parameter_bind;
  vector <ShaderVarying> _cg_varying;
  
  bool try_cg_compile(ShaderExpansion *s);
  void bind_cg_transform(const ShaderTransBind &stb,
                         CLP(GraphicsStateGuardian) *gsg);
  void suggest_cg_profile(const string &vpro, const string &fpro);
  CGprofile parse_cg_profile(const string &id, bool vertex);
  void issue_cg_auto_bind(const ShaderAutoBind &bind, GSG *gsg);
  bool compile_cg_parameter(CGparameter p);
  bool errchk_cg_parameter_words(CGparameter p, int len);
  bool errchk_cg_parameter_direction(CGparameter p, CGenum dir);
  bool errchk_cg_parameter_variance(CGparameter p, CGenum var);
  bool errchk_cg_parameter_prog(CGparameter p, CGprogram prog, const string &msg);
  bool errchk_cg_parameter_type(CGparameter p, CGtype dt);
  bool errchk_cg_parameter_float(CGparameter p);
  bool errchk_cg_parameter_sampler(CGparameter p);
  void errchk_cg_output(CGparameter p, const string &msg);
  void print_cg_compile_errors(const string &file, CGcontext ctx);
#endif

  void release_resources(void);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedObject::init_type();
    register_type(_type_handle, CLASSPREFIX_QUOTED "ShaderContext",
                  TypedObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "glShaderContext_src.I"

