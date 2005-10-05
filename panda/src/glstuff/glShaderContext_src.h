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
  struct ShaderAutoBind {
    CGparameter parameter;
    CGGLenum matrix;
    CGGLenum orient;
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
  
  // These arrays contain lists of "bindings." They
  // tell us how to fill the shader's input parameters.
  vector <ShaderAutoBind> _cg_autobind;
  vector <ShaderArgBind> _cg_fbind;
  vector <ShaderArgBind> _cg_npbind;
  vector <ShaderTexBind> _cg_texbind;
  vector <ShaderTransBind> _cg_transform_bind;
  vector <ShaderTransBind> _cg_parameter_bind;
  vector <ShaderVarying> _cg_varying;
  
  void bind_cg_transform(const ShaderTransBind &stb,
                         CLP(GraphicsStateGuardian) *gsg);
  
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

