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

public:
  friend class CLP(GraphicsStateGuardian);

  CLP(ShaderContext)(CLP(GraphicsStateGuardian) *glgsg, Shader *s);
  ~CLP(ShaderContext)();
  ALLOC_DELETED_CHAIN(CLP(ShaderContext));

  static void r_count_locations_bindings(const ShaderType *type,
                                         GLint &num_locations,
                                         GLint &num_ssbo_bindings);

  void r_collect_uniforms(const Shader::Parameter &param, UniformBlock &block,
                          const ShaderType *type, const char *name,
                          const char *sym, int &location,
                          const SparseArray &active_locations,
                          int &resource_index, int &ssbo_binding,
                          size_t offset = 0);

  void reflect_program(SparseArray &active_locations, LocationMap &locations, LocationMap &ssbo_bindings);
  void reflect_attribute(int i, char *name_buf, GLsizei name_buflen);
  void reflect_uniform_block(int i, const char *block_name,
                             char *name_buffer, GLsizei name_buflen);
  bool get_sampler_texture_type(int &out, GLenum param_type);
  const ShaderType *get_param_type(GLenum type);

  bool valid(void) override;
  void bind() override;
  void unbind() override;

  INLINE void set_display_region(const DisplayRegion *display_region);
  void set_state_and_transform(const RenderState *state,
                               const TransformState *modelview_transform,
                               const TransformState *camera_transform,
                               const TransformState *projection_transform) override;

  void issue_parameters(int altered) override;
  void disable_shader_vertex_arrays() override;
  bool update_shader_vertex_arrays(ShaderContext *prev, bool force) override;
  void disable_shader_texture_bindings() override;
  void update_shader_texture_bindings(ShaderContext *prev) override;
  void update_shader_buffer_bindings(ShaderContext *prev) override;

  bool uses_standard_vertex_arrays(void) override {
    return _uses_standard_vertex_arrays;
  }
  bool uses_custom_vertex_arrays(void) override {
    return true;
  }

private:
  bool _validated;
  GLuint _glsl_program;
  struct Module {
    const ShaderModule *_module;
    GLuint _handle;
    bool _needs_compile;
  };
  typedef small_vector<Module, 2> Modules;
  Modules _modules;
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
    struct Call {
      GLint _location;
      GLuint _count;
      void *_func;
      size_t _offset;
    };

    pvector<Call> _matrices;
    pvector<Call> _vectors;
  };
  pvector<UniformBlock> _uniform_blocks;
  int _uniform_data_deps = 0;
  size_t _scratch_space_size = 0;

  struct TextureUnit {
    PT(ShaderInputBinding) _binding;
    ShaderInputBinding::ResourceId _resource_id;
    GLenum _target;
  };
  typedef pvector<TextureUnit> TextureUnits;
  TextureUnits _texture_units;

  struct ImageUnit {
    PT(ShaderInputBinding) _binding;
    ShaderInputBinding::ResourceId _resource_id;
    CLP(TextureContext) *_gtc = nullptr;
    ShaderType::Access _access;
    bool _written = false;
  };
  typedef pvector<ImageUnit> ImageUnits;
  ImageUnits _image_units;

  BitMask32 _enabled_attribs;
  GLint _color_attrib_index;

  struct StorageBlock {
    PT(ShaderInputBinding) _binding;
    ShaderInputBinding::ResourceId _resource_id;
    GLint _binding_index;
  };
  typedef pvector<StorageBlock> StorageBlocks;
  StorageBlocks _storage_blocks;

  CLP(GraphicsStateGuardian) *_glgsg;

  bool _uses_standard_vertex_arrays;

  void report_shader_errors(const Module &module, bool fatal);
  void report_program_errors(GLuint program, bool fatal);
  bool attach_shader(const ShaderModule *module, Shader::ModuleSpecConstants &spec_consts,
                     const LocationMap &locations, bool &remap_locations,
                     const LocationMap &ssbo_bindings);
  bool compile_and_link(const LocationMap &locations, bool &remap_locations,
                        const LocationMap &ssbo_bindings);
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
  virtual TypeHandle get_type() const override {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() override {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "glShaderContext_src.I"

#endif  // OPENGLES_1
