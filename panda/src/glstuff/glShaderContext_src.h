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
  typedef CLP(GraphicsStateGuardian) GSG;

  CLP(ShaderContext)(ShaderExpansion *s, GSG *gsg);
  ~CLP(ShaderContext)();

  INLINE bool valid(void);
  void bind(GSG *gsg);
  void unbind();
  void issue_parameters(GSG *gsg, bool all);
  void disable_shader_vertex_arrays(GSG *gsg);
  void update_shader_vertex_arrays(CLP(ShaderContext) *prev, GSG *gsg);
  void disable_shader_texture_bindings(GSG *gsg);
  void update_shader_texture_bindings(CLP(ShaderContext) *prev, GSG *gsg);
  bool _state;

private:

#ifdef HAVE_CGGL
  CGcontext _cg_context;
  CGprofile _cg_profile[2];
  CGprogram _cg_program[2];

  bool try_cg_compile(ShaderExpansion *s, GSG *gsg);
  void suggest_cg_profile(const string &vpro, const string &fpro);
  CGprofile parse_cg_profile(const string &id, bool vertex);
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

