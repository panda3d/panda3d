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

#ifndef OPENGLES_1

#include "pandabase.h"
#include "string_utils.h"
#include "internalName.h"
#include "shader.h"
#include "shaderContext.h"
#include "deletedChain.h"

#if defined(HAVE_CG) && !defined(OPENGLES)
#include <Cg/cg.h>
#endif

class CLP(GraphicsStateGuardian);

////////////////////////////////////////////////////////////////////
//       Class : GLShaderContext
// Description : xyz
////////////////////////////////////////////////////////////////////
class EXPCL_GL CLP(ShaderContext): public ShaderContext {
public:
  friend class CLP(GraphicsStateGuardian);
  typedef CLP(GraphicsStateGuardian) GSG;

  CLP(ShaderContext)(Shader *s, GSG *gsg);
  ~CLP(ShaderContext)();
  ALLOC_DELETED_CHAIN(CLP(ShaderContext));

  INLINE bool valid(void);
  void bind(GSG *gsg, bool reissue_parameters = true);
  void unbind(GSG *gsg);
  void issue_parameters(GSG *gsg, int altered);
  void disable_shader_vertex_arrays(GSG *gsg);
  bool update_shader_vertex_arrays(CLP(ShaderContext) *prev, GSG *gsg,
                                   bool force);
  void disable_shader_texture_bindings(GSG *gsg);
  void update_shader_texture_bindings(CLP(ShaderContext) *prev, GSG *gsg);

  INLINE bool uses_standard_vertex_arrays(void);
  INLINE bool uses_custom_vertex_arrays(void);
  INLINE bool uses_custom_texture_bindings(void);

private:

#if defined(HAVE_CG) && !defined(OPENGLES)
  CGcontext _cg_context;
  CGprogram _cg_vprogram;
  CGprogram _cg_fprogram;
  CGprogram _cg_gprogram;
  CGprofile _cg_vprofile;
  CGprofile _cg_fprofile;
  CGprofile _cg_gprofile;

  pvector <CGparameter> _cg_parameter_map;
#endif

  GLuint _glsl_program;
  GLuint _glsl_vshader;
  GLuint _glsl_fshader;
  GLuint _glsl_gshader;
  GLuint _glsl_tcshader;
  GLuint _glsl_teshader;

  pvector <GLint> _glsl_parameter_map;

  int _stage_offset;
  // Avoid using this! It merely exists so the
  // destructor has access to the extension functions.
  WPT(GSG) _last_gsg;

  bool _uses_standard_vertex_arrays;

  void glsl_report_shader_errors(GSG *gsg, unsigned int shader);
  void glsl_report_program_errors(GSG *gsg, unsigned int program);
  unsigned int glsl_compile_entry_point(GSG *gsg, Shader::ShaderType type);
  bool glsl_compile_shader(GSG *gsg);
  bool parse_and_set_short_hand_shader_vars(Shader::ShaderArgId &arg_id, Shader *s);
  void release_resources(GSG *gsg);

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

#endif  // OPENGLES_1

