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
#include "shaderInputBinding.h"
#include "deletedChain.h"
#include "paramTexture.h"
#include "small_vector.h"

class CLP(GraphicsStateGuardian);

/**
 * xyz
 */
class EXPCL_GL CLP(ShaderContext) final : public ShaderContext {
private:
  struct UniformBlock;
  typedef pmap<const InternalName *, GLint> LocationMap;

  struct UniformCall {
    GLint _location;
    GLuint _count;
    void *_func;
    size_t _offset;
  };

  struct UniformCalls {
    pvector<UniformCall> _matrices;
    pvector<UniformCall> _vectors;
  };

public:
  friend class CLP(GraphicsStateGuardian);

  CLP(ShaderContext)(CLP(GraphicsStateGuardian) *glgsg, Shader *s);
  ~CLP(ShaderContext)();
  ALLOC_DELETED_CHAIN(CLP(ShaderContext));

  bool valid(void);
  bool bind(CLP(GraphicsStateGuardian) *glgsg,
            RenderAttrib::PandaCompareFunc alpha_test_mode);
  void unbind();

  bool compile_for(RenderAttrib::PandaCompareFunc alpha_test_mode);

private:
  static void r_count_locations_bindings(const ShaderType *type,
                                         GLint &num_locations,
                                         GLint &num_ssbo_bindings,
                                         GLint &num_image_bindings);

  void r_collect_uniforms(RenderAttrib::PandaCompareFunc alpha_test_mode,
                          const Shader::Parameter &param, UniformCalls &calls,
                          const ShaderType *type, const char *name,
                          const char *sym, int &location,
                          const SparseArray &active_locations,
                          int &resource_index, int &binding,
                          size_t offset = 0);

  void reflect_program(GLuint program, SparseArray &active_locations);
  void reflect_attribute(GLuint program, int i, char *name_buf, GLsizei name_buflen);
  void reflect_uniform_block(GLuint program, int i, const char *block_name,
                             char *name_buffer, GLsizei name_buflen);
  bool get_sampler_texture_type(int &out, GLenum param_type);
  const ShaderType *get_param_type(GLenum type);

  INLINE void set_display_region(const DisplayRegion *display_region);
  void set_state_and_transform(const RenderState *state,
                               const TransformState *modelview_transform,
                               const TransformState *camera_transform,
                               const TransformState *projection_transform);

  void issue_parameters(int altered);
  void disable_shader_vertex_arrays();
  bool update_shader_vertex_arrays(ShaderContext *prev, bool force);
  void disable_shader_texture_bindings();
  void update_shader_texture_bindings(ShaderContext *prev);
  void update_shader_buffer_bindings(ShaderContext *prev);

  bool uses_standard_vertex_arrays(void) {
    return _uses_standard_vertex_arrays;
  }
  bool uses_custom_vertex_arrays(void) {
    return true;
  }

private:
  bool _validated = false;
  bool _inject_alpha_test = false;
  GLuint _program = 0;
  GLuint _linked_programs[RenderAttrib::M_always] {0u};
  RenderAttrib::PandaCompareFunc _alpha_test_mode = RenderAttrib::M_none;
  GLint _alpha_test_ref_locations[RenderAttrib::M_always];

  // May exclude the fragment shader if _inject_alpha_test is set.
  struct Module {
    Shader::Stage _stage;
    GLuint _handle;
  };
  small_vector<Module, 2> _modules;
  bool _is_legacy = false;
  bool _emulate_float_attribs = false;

  WCPT(RenderState) _state_rs;
  const TransformState *_modelview_transform;
  const TransformState *_camera_transform;
  const TransformState *_projection_transform;
  const ColorAttrib *_color_attrib;
  const ShaderAttrib *_shader_attrib;
  const DisplayRegion *_display_region = nullptr;
  int _frame_number = -1;

  pvector<LMatrix4> _matrix_cache;
  int _matrix_cache_deps = ShaderEnums::D_none;

  struct UniformBlock {
    struct Binding {
      PT(ShaderInputBinding) _binding;
      size_t _offset;
    };

    small_vector<Binding, 1> _bindings;
    int _dep;

    // When UBOs are not used or supported, we use an array of glUniform
    // calls instead.
    small_vector<UniformCalls, 1> _calls;
  };
  pvector<UniformBlock> _uniform_blocks;
  int _uniform_data_deps = 0;
  size_t _scratch_space_size = 0;

  struct TextureUnit {
    PT(ShaderInputBinding) _binding;
    ShaderInputBinding::ResourceId _resource_id;
    GLenum _target;
    GLint _size_loc[RenderAttrib::M_always];
  };
  typedef pvector<TextureUnit> TextureUnits;
  TextureUnits _texture_units;

  struct ImageUnit {
    PT(ShaderInputBinding) _binding;
    ShaderInputBinding::ResourceId _resource_id;
    CLP(TextureContext) *_gtc = nullptr;
    ShaderType::Access _access;
    bool _written = false;
    GLint _size_loc[RenderAttrib::M_always];
  };
  typedef pvector<ImageUnit> ImageUnits;
  ImageUnits _image_units;

  BitMask32 _enabled_attribs;
  GLint _color_attrib_index;
  uint32_t _bind_attrib_locations = 0;

  struct StorageBlock {
    PT(ShaderInputBinding) _binding;
    ShaderInputBinding::ResourceId _resource_id;
    GLint _binding_index;
  };
  typedef pvector<StorageBlock> StorageBlocks;
  StorageBlocks _storage_blocks;
  uint32_t _storage_block_bindings = 0;

  CLP(GraphicsStateGuardian) *_glgsg;
  uint64_t _emulated_caps = 0u;

  bool _remap_locations = false;
  LocationMap _locations;
  LocationMap _bindings;

  bool _uses_standard_vertex_arrays;

  void report_shader_errors(GLuint handle, Shader::Stage stage, bool fatal);
  void report_program_errors(GLuint program, bool fatal);
  GLuint create_shader(GLuint program, const ShaderModule *module, size_t mi,
                       const Shader::ModuleSpecConstants &spec_consts,
                       RenderAttrib::PandaCompareFunc alpha_test_mode);
  GLuint compile_and_link(RenderAttrib::PandaCompareFunc alpha_test_mode);
  void release_resources(CLP(GraphicsStateGuardian) *glgsg);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ShaderContext::init_type();
    register_type(_type_handle, CLASSPREFIX_QUOTED "ShaderContext",
                  ShaderContext::get_class_type());
  }
  virtual TypeHandle get_type() const override {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() override {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "glShaderContext_src.I"

#endif  // OPENGLES_1
