/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file glCgShaderContext_src.h
 * @author jyelon
 * @date 2005-09-01
 */

#if defined(HAVE_CG) && !defined(OPENGLES)

#include "pandabase.h"
#include "string_utils.h"
#include "internalName.h"
#include "shader.h"
#include "shaderContext.h"
#include "deletedChain.h"

class CLP(GraphicsStateGuardian);

/**
 * xyz
 */
class EXPCL_GL CLP(CgShaderContext) final : public ShaderContext {
public:
  friend class CLP(GraphicsStateGuardian);

  CLP(CgShaderContext)(CLP(GraphicsStateGuardian) *glgsg, Shader *s);
  ~CLP(CgShaderContext)();
  ALLOC_DELETED_CHAIN(CLP(CgShaderContext));

  bool valid(void) override;
  void bind() override;
  void unbind() override;

  void set_state_and_transform(const RenderState *state,
                               const TransformState *modelview_transform,
                               const TransformState *camera_transform,
                               const TransformState *projection_transform) override;

  void issue_parameters(int altered) override;
  void update_transform_table(const TransformTable *table);
  void update_slider_table(const SliderTable *table);
  void disable_shader_vertex_arrays() override;
  bool update_shader_vertex_arrays(ShaderContext *prev, bool force) override;
  void disable_shader_texture_bindings() override;
  void update_shader_texture_bindings(ShaderContext *prev) override;

  bool uses_standard_vertex_arrays(void) override { return false; }
  bool uses_custom_vertex_arrays(void) override { return true; }

  // Special values for location to indicate conventional attrib slots.
  enum ConventionalAttrib {
    CA_unknown = -1,
    CA_vertex = -2,
    CA_normal = -3,
    CA_color = -4,
    CA_secondary_color = -5,
    CA_texcoord = -32,
  };

private:
  CGprogram _cg_program;
  GLuint _glsl_program;

  pvector<GLint> _attributes;
  BitMask32 _used_generic_attribs;
  GLint _color_attrib_index;
  CGparameter _transform_table_param;
  CGparameter _slider_table_param;
  long _transform_table_size;
  long _slider_table_size;

  pvector<CGparameter> _cg_parameter_map;

  WCPT(RenderState) _state_rs;
  CPT(TransformState) _modelview_transform;
  CPT(TransformState) _camera_transform;
  CPT(TransformState) _projection_transform;
  GLint _frame_number;

  CLP(GraphicsStateGuardian) *_glgsg;

  void release_resources();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ShaderContext::init_type();
    register_type(_type_handle, CLASSPREFIX_QUOTED "CgShaderContext",
                  ShaderContext::get_class_type());
  }
  virtual TypeHandle get_type() const override {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() override {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "glCgShaderContext_src.I"

#endif  // OPENGLES_1
