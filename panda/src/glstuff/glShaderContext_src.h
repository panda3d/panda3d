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
#include "shaderContext.h"
#ifdef HAVE_CGGL
#include "Cg/cgGL.h"
#endif
#include "string_utils.h"

////////////////////////////////////////////////////////////////////
//       Class : GLShaderContext
// Description : xyz
////////////////////////////////////////////////////////////////////

class EXPCL_GL CLP(ShaderContext): public ShaderContext {
public:
  CLP(ShaderContext)(Shader *s);
  ~CLP(ShaderContext)();

  INLINE bool valid(void);
  void bind(ShaderMode *mode, GraphicsStateGuardianBase *gsg);
  void unbind();
  void rebind(ShaderMode *oldmode, ShaderMode *newmode);

private:
  
#ifdef HAVE_CGGL
  enum {
    ARGINDEX_WORLD =-1,
    ARGINDEX_CAMERA=-2,
    ARGINDEX_MODEL =-3
  };
  enum {
    TRANS_NORMAL,
    TRANS_TPOSE,
    TRANS_ROW0,
    TRANS_ROW1,
    TRANS_ROW2,
    TRANS_ROW3,
    TRANS_COL0,
    TRANS_COL1,
    TRANS_COL2,
    TRANS_COL3,
  };
  struct ShaderAutoBind {
    CGparameter parameter;
    CGGLenum matrix;
    CGGLenum orientation;
  };
  struct ShaderArgBind {
    CGparameter parameter;
    int argindex;
  };
  struct ShaderTransBind {
    CGparameter parameter;
    int src_argindex;
    int rel_argindex;
    int trans_piece;
  };
  CGcontext _cg_context;
  CGprofile _cg_profile[2];
  CGprogram _cg_program[2];
  int       _cg_linebase[2];
  
  // These arrays contain lists of "bindings." They
  // tell us how to fill the shader's input parameters.
  vector <ShaderAutoBind> _cg_autobind;
  vector <ShaderArgBind> _cg_tbind2d;
  vector <ShaderArgBind> _cg_tbind3d;
  vector <ShaderArgBind> _cg_vbind1;
  vector <ShaderArgBind> _cg_vbind2;
  vector <ShaderArgBind> _cg_vbind3;
  vector <ShaderArgBind> _cg_vbind4;
  vector <ShaderArgBind> _cg_npbind;
  vector <ShaderTransBind> _cg_trans_bind;
  vector <ShaderTransBind> _cg_trans_rebind;
  
  bool compile_cg_parameter(CGparameter p);
  bool errchk_cg_parameter_words(CGparameter p, int len);
  bool errchk_cg_parameter_direction(CGparameter p, CGenum dir);
  bool errchk_cg_parameter_variance(CGparameter p, CGenum var);
  bool errchk_cg_parameter_prog(CGparameter p, CGprogram prog, const string &msg);
  bool errchk_cg_parameter_semantic(CGparameter p, const string &semantic);
  bool errchk_cg_parameter_type(CGparameter p, CGtype dt);
  bool errchk_cg_parameter_float(CGparameter p);
  bool errchk_cg_parameter_sampler(CGparameter p);
  void errchk_cg_output(CGparameter p, const string &msg);
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

