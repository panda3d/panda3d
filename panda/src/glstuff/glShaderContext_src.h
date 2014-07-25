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

class CLP(GraphicsStateGuardian);

////////////////////////////////////////////////////////////////////
//       Class : GLShaderContext
// Description : xyz
////////////////////////////////////////////////////////////////////
class EXPCL_GL CLP(ShaderContext) : public ShaderContext {
public:
  friend class CLP(GraphicsStateGuardian);

  CLP(ShaderContext)(CLP(GraphicsStateGuardian) *glgsg, Shader *s);
  ~CLP(ShaderContext)();
  ALLOC_DELETED_CHAIN(CLP(ShaderContext));

  INLINE bool valid(void);
  void bind(bool reissue_parameters = true);
  void unbind();
  void issue_parameters(int altered);
  void disable_shader_vertex_arrays();
  bool update_shader_vertex_arrays(ShaderContext *prev, bool force);
  void disable_shader_texture_bindings();
  void update_shader_texture_bindings(ShaderContext *prev);

  INLINE bool uses_standard_vertex_arrays(void);
  INLINE bool uses_custom_vertex_arrays(void);
  INLINE bool uses_custom_texture_bindings(void);

private:
  GLuint _glsl_program;
  typedef pvector<GLuint> GLSLShaders;
  GLSLShaders _glsl_shaders;

  //struct ParamContext {
  //  CPT(InternalName) _name;
  //  GLint _location;
  //  GLsizei _count;
  //  WPT(ParamValue) _value;
  //  UpdateSeq _updated;
  //};
  //typedef pvector<ParamContext> ParamContexts;
  //ParamContexts _params;

  pvector<GLint> _glsl_parameter_map;
  pmap<GLint, GLuint64> _glsl_uniform_handles;

  pvector<CPT(InternalName)> _glsl_img_inputs;
  pvector<CLP(TextureContext)*> _glsl_img_textures;

  CLP(GraphicsStateGuardian) *_glgsg;

  bool _uses_standard_vertex_arrays;

  void glsl_report_shader_errors(GLuint shader);
  void glsl_report_program_errors(GLuint program);
  bool glsl_compile_shader(Shader::ShaderType type);
  bool glsl_compile_and_link();
  bool parse_and_set_short_hand_shader_vars(Shader::ShaderArgId &arg_id, Shader *s);
  void release_resources();

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
