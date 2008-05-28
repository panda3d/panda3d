// Filename: glShaderContext_src.h
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
#include "string_utils.h"
#include "internalName.h"
#include "shader.h"
#include "shaderContext.h"
#include "deletedChain.h"

class CLP(GraphicsStateGuardian);

////////////////////////////////////////////////////////////////////
//       Class : GLShaderContext
// Description : xyz
////////////////////////////////////////////////////////////////////

class EXPCL_GL CLP(ShaderContext): public ShaderContext {
public:
  typedef CLP(GraphicsStateGuardian) GSG;

  CLP(ShaderContext)(Shader *s, GSG *gsg);
  ~CLP(ShaderContext)();
  ALLOC_DELETED_CHAIN(CLP(ShaderContext));

  INLINE bool valid(void);
  void bind(GSG *gsg);
  void unbind();
  void issue_parameters(GSG *gsg, int altered);
  void disable_shader_vertex_arrays(GSG *gsg);
  bool update_shader_vertex_arrays(CLP(ShaderContext) *prev, GSG *gsg,
                                   bool force);
  void disable_shader_texture_bindings(GSG *gsg);
  void update_shader_texture_bindings(CLP(ShaderContext) *prev, GSG *gsg);

private:

#ifdef HAVE_CG
  CGcontext _cg_context;
  CGprogram _cg_vprogram;
  CGprogram _cg_fprogram;
  pvector <CGparameter> _cg_parameter_map;
  void cg_report_errors();
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

