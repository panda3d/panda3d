/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderInputBinding_impls.h
 * @author rdb
 * @date 2024-09-22
 */

#ifndef SHADERINPUTBINDING_IMPLS_H
#define SHADERINPUTBINDING_IMPLS_H

#include "shaderInputBinding.h"

ShaderInputBinding *make_binding_cg(const InternalName *name, const ShaderType *type);
ShaderInputBinding *make_binding_glsl(const InternalName *name, const ShaderType *type);

/**
 * This binds a parameter to a specific transformation matrix or part thereof.
 */
class EXPCL_PANDA_DISPLAY ShaderMatrixBinding : public ShaderInputBinding {
public:
  INLINE ShaderMatrixBinding(ShaderEnums::StateMatrix input, CPT_InternalName arg,
                             bool transpose = false, size_t offset = 0,
                             size_t num_rows = 4, size_t num_cols = 4);

  virtual int get_state_dep() const override;
  virtual void setup(Shader *shader) override;

  virtual void fetch_data(const State &state, void *into, bool packed) const override;

protected:
  size_t _cache_index = 0;
  ShaderEnums::StateMatrix _input;
  CPT_InternalName _arg;
  bool _transpose = false;
  size_t _offset = 0;
  size_t _num_rows;
  size_t _num_cols;
};

/**
 * This binds a parameter to a composition of two matrices.
 */
class EXPCL_PANDA_DISPLAY ShaderMatrixComposeBinding : public ShaderInputBinding {
public:
  INLINE ShaderMatrixComposeBinding(
    ShaderEnums::StateMatrix input0, CPT_InternalName arg0,
    ShaderEnums::StateMatrix input1, CPT_InternalName arg1,
    bool transpose = false, size_t offset = 0,
    size_t num_rows = 4, size_t num_cols = 4);

  virtual int get_state_dep() const override;
  virtual void setup(Shader *shader) override;

  virtual void fetch_data(const State &state, void *into, bool packed) const override;

private:
  size_t _cache_index0 = 0;
  size_t _cache_index1 = 0;
  ShaderEnums::StateMatrix _input0;
  ShaderEnums::StateMatrix _input1;
  CPT_InternalName _arg0;
  CPT_InternalName _arg1;
  bool _transpose = false;
  size_t _offset = 0;
  size_t _num_rows;
  size_t _num_cols;
};

/**
 *
 */
class EXPCL_PANDA_DISPLAY ShaderPointParamsBinding : public ShaderInputBinding {
public:
  virtual int get_state_dep() const override;
  virtual void setup(Shader *shader) override;

  virtual void fetch_data(const State &state, void *into, bool packed) const override;

protected:
  size_t _cache_index = 0;
};

/**
 *
 */
class EXPCL_PANDA_DISPLAY ShaderPackedLightBinding : public ShaderInputBinding {
public:
  INLINE ShaderPackedLightBinding(int index);

  virtual int get_state_dep() const override;
  virtual void setup(Shader *shader) override;

  virtual void fetch_data(const State &state, void *into, bool packed) const override;

protected:
  size_t _index;
  size_t _world_mat_cache_index;
};

/**
 *
 */
class EXPCL_PANDA_DISPLAY ShaderLegacyLightBinding : public ShaderInputBinding {
public:
  INLINE ShaderLegacyLightBinding(CPT_InternalName input,
                                  ShaderEnums::StateMatrix matrix,
                                  CPT_InternalName arg);

  virtual int get_state_dep() const override;
  virtual void setup(Shader *shader) override;

protected:
  CPT_InternalName _input;
  ShaderEnums::StateMatrix _matrix;
  CPT_InternalName _arg;
  size_t _mat_cache_index;
};

/**
 *
 */
class EXPCL_PANDA_DISPLAY ShaderLegacyDirectionalLightBinding : public ShaderLegacyLightBinding {
public:
  using ShaderLegacyLightBinding::ShaderLegacyLightBinding;

  virtual void fetch_data(const State &state, void *into, bool packed) const override;
};

/**
 *
 */
class EXPCL_PANDA_DISPLAY ShaderLegacyPointLightBinding : public ShaderLegacyLightBinding {
public:
  using ShaderLegacyLightBinding::ShaderLegacyLightBinding;

  virtual void fetch_data(const State &state, void *into, bool packed) const override;
};

/**
 *
 */
class EXPCL_PANDA_DISPLAY ShaderLegacySpotlightBinding : public ShaderLegacyLightBinding {
public:
  using ShaderLegacyLightBinding::ShaderLegacyLightBinding;

  virtual void fetch_data(const State &state, void *into, bool packed) const override;
};

/**
 *
 */
class EXPCL_PANDA_DISPLAY ShaderLightStructBinding : public ShaderInputBinding {
public:
  ShaderLightStructBinding(const ShaderType *type,
                           const InternalName *input = nullptr);

  virtual int get_state_dep() const override;
  virtual void setup(Shader *shader) override;

  virtual void fetch_data(const State &state, void *into, bool packed) const override;

  virtual ResourceId get_resource_id(int index, const ShaderType *type) const;
  virtual PT(Texture) fetch_texture(const State &state,
                                    ResourceId index,
                                    SamplerState &sampler, int &view) const;

private:
  void fetch_light(const State &state, const NodePath &np, void *into) const;
  void fetch_from_input(const ShaderAttrib *target_shader, void *into) const;

protected:
  CPT(InternalName) _input;
  size_t _count;
  size_t _stride;
  size_t _world_to_apiview_mat_cache_index;
  size_t _apiview_to_world_mat_cache_index;
  bool _cube_shadow_map = false;
  int _color_offset = -1;
  int _specular_offset = -1;
  int _ambient_offset = -1;
  int _diffuse_offset = -1;
  int _position_offset = -1;
  int _half_vector_offset = -1;
  int _spot_direction_offset = -1;
  int _spot_cos_cutoff_offset = -1;
  int _spot_cutoff_offset = -1;
  int _spot_exponent_offset = -1;
  int _attenuation_offset = -1;
  int _constant_attenuation_offset = -1;
  int _linear_attenuation_offset = -1;
  int _quadratic_attenuation_offset = -1;
  int _radius_offset = -1;
  int _shadow_view_matrix_offset = -1;
};

/**
 * Binds a parameter to a texture stage.
 */
class EXPCL_PANDA_DISPLAY ShaderTextureStagesBinding : public ShaderInputBinding {
public:
  INLINE ShaderTextureStagesBinding(Texture::TextureType desired_type,
                                    size_t count,
                                    Texture *default_texture = nullptr,
                                    unsigned int mode_mask = ~0);

  virtual int get_state_dep() const override;

  virtual ResourceId get_resource_id(int index, const ShaderType *type) const;
  virtual PT(Texture) fetch_texture(const State &state,
                                    ResourceId resource_id,
                                    SamplerState &sampler, int &view) const;

protected:
  size_t const _count;
  Texture *const _default_texture;
  Texture::TextureType const _desired_type;
  unsigned int _mode_mask;
  mutable bool _shown_error = false;
};

/**
 * Binds a parameter to a generic texture shader input.
 */
class EXPCL_PANDA_DISPLAY ShaderTextureBinding : public ShaderInputBinding {
public:
  INLINE ShaderTextureBinding(CPT(InternalName) input, Texture::TextureType desired_type);

  virtual int get_state_dep() const override;

  virtual ResourceId get_resource_id(int index, const ShaderType *type) const;
  virtual PT(Texture) fetch_texture(const State &state,
                                    ResourceId resource_id,
                                    SamplerState &sampler, int &view) const;
  virtual PT(Texture) fetch_texture_image(const State &state,
                                          ResourceId resource_id,
                                          ShaderType::Access &access,
                                          int &z, int &n) const;

protected:
  CPT(InternalName) const _input;
  Texture::TextureType const _desired_type;
  mutable bool _shown_error = false;
};

/**
 * Binds a parameter to a generic storage buffer shader input.
 */
class EXPCL_PANDA_DISPLAY ShaderBufferBinding : public ShaderInputBinding {
public:
  INLINE ShaderBufferBinding(CPT(InternalName) input, size_t min_size = 0);

  virtual int get_state_dep() const override;

  virtual ResourceId get_resource_id(int index, const ShaderType *type) const;
  virtual PT(ShaderBuffer) fetch_shader_buffer(const State &state,
                                               ResourceId resource_id) const;

protected:
  CPT(InternalName) const _input;
  size_t const _min_size = 0;
  mutable bool _shown_error = false;
};

/**
 * This binds a parameter to a generic numeric data shader input.
 */
class EXPCL_PANDA_DISPLAY ShaderDataBinding : public ShaderInputBinding {
public:
  INLINE ShaderDataBinding(const InternalName *input, size_t num_elements,
                           size_t num_rows, size_t num_cols);

  virtual int get_state_dep() const override;

  virtual void fetch_data(const State &state, void *into, bool packed) const override=0;

protected:
  CPT_InternalName _input;
  size_t _num_elements;
  size_t _num_rows;
  size_t _num_cols;
};

/**
 * This binds a parameter to a single-precision float shader input.
 */
class EXPCL_PANDA_DISPLAY ShaderFloatBinding : public ShaderDataBinding {
public:
  using ShaderDataBinding::ShaderDataBinding;

  virtual void fetch_data(const State &state, void *into, bool packed) const override;
};

/**
 * This binds a parameter to a double-precision float shader input.
 */
class EXPCL_PANDA_DISPLAY ShaderDoubleBinding : public ShaderDataBinding {
public:
  using ShaderDataBinding::ShaderDataBinding;

  virtual void fetch_data(const State &state, void *into, bool packed) const override;
};

/**
 * This binds a parameter to an integer shader input.
 */
class EXPCL_PANDA_DISPLAY ShaderIntBinding : public ShaderDataBinding {
public:
  using ShaderDataBinding::ShaderDataBinding;

  virtual void fetch_data(const State &state, void *into, bool packed) const override;
};

/**
 * This binds a parameter to an boolean shader input.
 */
class EXPCL_PANDA_DISPLAY ShaderBoolBinding : public ShaderDataBinding {
public:
  using ShaderDataBinding::ShaderDataBinding;

  virtual void fetch_data(const State &state, void *into, bool packed) const override;
};

/**
 * This binds an aggregate parameter (such as a struct or an array of structs)
 * to a set of shader inputs.
 */
class EXPCL_PANDA_DISPLAY ShaderAggregateBinding : public ShaderInputBinding {
public:
  INLINE ShaderAggregateBinding(CPT_InternalName input, const ShaderType *type);

  virtual int get_state_dep() const override;

  virtual void fetch_data(const State &state, void *into, bool packed) const override;

  virtual ResourceId get_resource_id(int index, const ShaderType *type) const;
  virtual PT(Texture) fetch_texture(const State &state,
                                    ResourceId index,
                                    SamplerState &sampler, int &view) const;
  virtual PT(Texture) fetch_texture_image(const State &state,
                                          ResourceId index,
                                          ShaderType::Access &access,
                                          int &z, int &n) const;
  virtual PT(ShaderBuffer) fetch_shader_buffer(const State &state,
                                               ResourceId resource_id) const;

private:
  void r_collect_members(const InternalName *name, const ShaderType *type, size_t offset = 0);

private:
  struct DataMember {
    PT(ShaderDataBinding) _binding;
    size_t _offset;
  };
  pvector<DataMember> _data_members;
  pvector<CPT_InternalName> _resources;
};

#include "shaderInputBinding_impls.I"

#endif
