/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file glShaderContext_src.h
 * @author jyelon
 * @date 2005-09-01
 */

#ifndef OPENGLES_1

#include "pandabase.h"
#include "string_utils.h"
#include "internalName.h"
#include "shader.h"
#include "shaderContext.h"
#include "deletedChain.h"
#include "paramTexture.h"

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

  void reflect_attribute(int i, char *name_buf, GLsizei name_buflen);
#ifndef OPENGLES
  void reflect_uniform_block(int i, const char *block_name,
                             char *name_buffer, GLsizei name_buflen);
#endif
  void reflect_uniform(int i, char *name_buffer, GLsizei name_buflen);
  bool get_sampler_texture_type(int &out, GLenum param_type);

  INLINE bool valid(void);
  void bind();
  void unbind();

  void set_state_and_transform(const RenderState *state,
                               const TransformState *modelview_transform,
                               const TransformState *projection_transform);

  void issue_parameters(int altered);
  void update_transform_table(const TransformTable *table);
  void update_slider_table(const SliderTable *table);
  void disable_shader_vertex_arrays();
  bool update_shader_vertex_arrays(ShaderContext *prev, bool force);
  void disable_shader_texture_bindings() OVERRIDE;
  void update_shader_texture_bindings(ShaderContext *prev) OVERRIDE;

  INLINE bool uses_standard_vertex_arrays(void);
  INLINE bool uses_custom_vertex_arrays(void);

private:
  bool _validated;
  GLuint _glsl_program;
  typedef pvector<GLuint> GLSLShaders;
  GLSLShaders _glsl_shaders;

  WCPT(RenderState) _state_rs;
  CPT(TransformState) _modelview_transform;
  CPT(TransformState) _projection_transform;

  //struct ParamContext {
  //  CPT(InternalName) _name;
  //  GLint _location;
  //  GLsizei _count;
  //  WPT(ParamValue) _value;
  //  UpdateSeq _updated;
  //};
  //typedef pvector<ParamContext> ParamContexts;
  //ParamContexts _params;

  BitMask32 _enabled_attribs;
  GLint _color_attrib_index;
  GLint _transform_table_index;
  GLint _slider_table_index;
  GLsizei _transform_table_size;
  GLsizei _slider_table_size;
  GLint _frame_number_loc;
  GLint _frame_number;
  pmap<GLint, GLuint64> _glsl_uniform_handles;

  struct ImageInput {
    CPT(InternalName) _name;
    CLP(TextureContext) *_gtc;
    bool _writable;
  };
  pvector<ImageInput> _glsl_img_inputs;

  CLP(GraphicsStateGuardian) *_glgsg;

  bool _uses_standard_vertex_arrays;

  void glsl_report_shader_errors(GLuint shader, Shader::ShaderType type, bool fatal);
  void glsl_report_program_errors(GLuint program, bool fatal);
  bool glsl_compile_shader(Shader::ShaderType type);
  bool glsl_compile_and_link();
  bool parse_and_set_short_hand_shader_vars(Shader::ShaderArgId &arg_id, GLenum param_type, GLint param_size, Shader *s);
  void release_resources();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ShaderContext::init_type();
    register_type(_type_handle, CLASSPREFIX_QUOTED "ShaderContext",
                  ShaderContext::get_class_type());
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
