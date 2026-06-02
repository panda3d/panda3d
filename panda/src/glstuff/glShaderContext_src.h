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
#include "ordered_vector.h"

class CLP(GraphicsStateGuardian);
class ShaderModuleSpirV;

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

  struct LinkedProgram;

public:
  friend class CLP(GraphicsStateGuardian);

  CLP(ShaderContext)(CLP(GraphicsStateGuardian) *glgsg, Shader *s);
  ~CLP(ShaderContext)();
  ALLOC_DELETED_CHAIN(CLP(ShaderContext));

  bool valid(void);
  bool bind(CLP(GraphicsStateGuardian) *glgsg);
  void unbind();

  bool compile_variant(int variant);

private:
  static void r_count_locations_bindings(const ShaderType *type,
                                         GLint &num_locations,
                                         GLint &num_ssbo_bindings,
                                         GLint &num_image_bindings);

  void r_collect_uniforms(LinkedProgram &linked_program, const Shader::Parameter &param,
                          UniformCalls &calls, const ShaderType *type,
                          const char *name, const char *sym, int &cur_location,
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
  bool set_state_and_transform(const RenderState *state,
                               const TransformState *modelview_transform,
                               const TransformState *camera_transform,
                               const TransformState *projection_transform,
                               bool inject_instancing, bool inject_animation);

  void issue_parameters(int altered);
  void disable_shader_vertex_arrays();
  bool update_shader_vertex_arrays(ShaderContext *prev, bool force);
  void disable_shader_texture_bindings();
  void update_shader_texture_bindings();
  void update_shader_buffer_bindings();
  void issue_memory_barriers();

  bool uses_standard_vertex_arrays(void) {
    return _uses_standard_vertex_arrays;
  }
  bool uses_custom_vertex_arrays(void) {
    return true;
  }

private:
  bool _validated = false;

  // We can compile multiple versions of a program, keyed by VariantBits.
  // M_always itself isn't included since it's the same as M_none
  enum VariantBits {
    VB_alpha_test_mode_mask = 7,
    VB_instancing = 8,
    VB_animation = 16,
  };
  int _variant_mask = 0;
  int _bound_variant = -1;

  // Stores the modules that aren't dependent on a particular variant.
  struct Module {
    Shader::Stage _stage;
    GLuint _handle;
  };
  small_vector<Module, 1> _invariant_modules;

  // Cached shader code when alpha testing is being injected.
  std::string _fragment_shader_code;

  bool _is_legacy = false;

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
    UniformCalls _calls;
  };
  //pvector<UniformBlock> _uniform_blocks;
  int _uniform_data_deps = 0;
  size_t _scratch_space_size = 0;
  int _force_respecify = 0;

  struct TextureUnit {
    PT(ShaderInputBinding) _binding;
    ShaderInputBinding::ResourceId _resource_id;
    GLenum _target;
    pmap<int, GLint> _size_loc; // keyed by variant
  };
  typedef pvector<TextureUnit> TextureUnits;
  TextureUnits _texture_units;

  struct ImageUnit {
    PT(ShaderInputBinding) _binding;
    ShaderInputBinding::ResourceId _resource_id;
    CLP(TextureContext) *_gtc = nullptr;
    ShaderType::Access _access;
    bool _written = false;
    pmap<int, GLint> _size_loc; // keyed by variant
  };
  typedef pvector<ImageUnit> ImageUnits;
  ImageUnits _image_units;

  struct VertexAttrib {
    InternalName *_name;
    GLint _location;
    int _num_locations;
    int _variant_mask;
    int _append_uv;
    Shader::ScalarType _scalar_type;
  };
  pvector<VertexAttrib> _vertex_attribs;
  BitMask32 _enabled_attribs {0};
  GLint _color_attrib_index = -1;
  GLint _instance_mat_index = -1;
  GLint _transform_index_index = -1;
  GLint _transform_weight_index = -1;
  uint32_t _animate_attrib_locations = 0;
  uint32_t _animate_point_attrib_locations = 0;
  BitMask32 _bind_attrib_locations = 0;
  BitMask32 _bind_frag_data_locations = 0;

  struct StorageBlock {
    PT(ShaderInputBinding) _binding;
    CLP(BufferContext) *_gbc = nullptr;
    ShaderInputBinding::ResourceId _resource_id;
    GLint _binding_index;
    bool _writable;
  };
  typedef pvector<StorageBlock> StorageBlocks;
  StorageBlocks _storage_blocks;
  uint32_t _storage_block_bindings = 0;
  bool _force_rebind_ssbos = false;

  CLP(GraphicsStateGuardian) *_glgsg;
  uint64_t _emulated_caps = 0u;

  struct LinkedProgram {
    int _variant;
    GLuint _program = 0;

    pvector<UniformBlock> _uniform_blocks {};
    GLint _alpha_test_ref_location = -1;

    bool operator < (const LinkedProgram &other) const {
      return _variant < other._variant;
    }
  };
  typedef ordered_vector<LinkedProgram> LinkedPrograms;
  LinkedPrograms _linked_programs;
  int _bound_linked_program_index = -1;

  bool _remap_locations = false;
  LocationMap _locations;
  LocationMap _bindings;

  pmap<const InternalName *, std::pair<bool, bool> > _model_matrices;

  bool _uses_standard_vertex_arrays;

#ifdef DO_PSTATS
  PStatCollector _compute_dispatch_pcollector;
#endif

  void report_shader_errors(GLuint handle, Shader::Stage stage, bool fatal);
  void report_program_errors(GLuint program, bool fatal);
  GLuint create_shader(const ShaderModule *module, size_t mi,
                       const Shader::ModuleSpecConstants &spec_consts,
                       int variant);
  std::vector<uint32_t>
    compile_spirv_to_spirv(const ShaderModuleSpirV *module, size_t mi,
                           pmap<uint32_t, int> &id_to_location,
                           const pvector<uint32_t> &binding_ids,
                           int variant);
  std::string compile_spirv_to_glsl(const ShaderModuleSpirV *module, size_t mi,
                                    pmap<uint32_t, int> &id_to_location,
                                    const pvector<uint32_t> &binding_ids,
                                    int variant);
  GLuint compile_and_link(int variant);
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
