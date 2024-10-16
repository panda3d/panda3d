/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vulkanShaderContext.h
 * @author rdb
 * @date 2016-02-18
 */

#ifndef VULKANSHADERCONTEXT_H
#define VULKANSHADERCONTEXT_H

#include "config_vulkandisplay.h"
#include "shaderModuleSpirV.h"
#include "spirVTransformPass.h"

#include "colorAttrib.h"
#include "renderModeAttrib.h"
#include "cullFaceAttrib.h"
#include "depthWriteAttrib.h"
#include "logicOpAttrib.h"
#include "colorBlendAttrib.h"
#include "transparencyAttrib.h"

#include "small_vector.h"

class VulkanGraphicsStateGuardian;

/**
 * Manages a set of Vulkan shader modules.
 */
class EXPCL_VULKANDISPLAY VulkanShaderContext : public ShaderContext {
private:
  struct Descriptor;

public:
  using AccessChain = SpirVTransformPass::AccessChain;

  INLINE VulkanShaderContext(Shader *shader);
  INLINE ~VulkanShaderContext();

  ALLOC_DELETED_CHAIN(VulkanShaderContext);

  bool create_modules(VkDevice device, const ShaderType::Struct *push_constant_block_type);

  const ShaderType *r_extract_resources(const Shader::Parameter &param, const AccessChain &chain,
                                        pmap<AccessChain, Descriptor> &descriptors,
                                        const ShaderType *type, int &resource_index);

  VkDescriptorSetLayout make_texture_attrib_descriptor_set_layout(VkDevice device);
  VkDescriptorSetLayout make_shader_attrib_descriptor_set_layout(VkDevice device);
  VkDescriptorSetLayout make_dynamic_uniform_descriptor_set_layout(VkDevice device);

  bool fetch_descriptor(VulkanGraphicsStateGuardian *gsg,
                        const Descriptor &desc, VkWriteDescriptorSet &write,
                        VkDescriptorImageInfo *&image_infos,
                        VkDescriptorBufferInfo *&buffer_infos,
                        VkBufferView *&texel_buffer_views);
  bool update_tattr_descriptor_set(VulkanGraphicsStateGuardian *gsg, VkDescriptorSet ds);
  bool update_sattr_descriptor_set(VulkanGraphicsStateGuardian *gsg, VkDescriptorSet ds);
  uint32_t update_sattr_uniforms(VulkanGraphicsStateGuardian *gsg, VkBuffer &buffer);
  uint32_t update_dynamic_uniforms(VulkanGraphicsStateGuardian *gsg, int altered);

  VkPipeline get_pipeline(VulkanGraphicsStateGuardian *gsg,
                          const RenderState *state,
                          const GeomVertexFormat *format,
                          VkPrimitiveTopology topology,
                          uint32_t patch_control_points,
                          VkSampleCountFlagBits multisamples);
  VkPipeline get_compute_pipeline(VulkanGraphicsStateGuardian *gsg);

  /**
   * Stores whatever is used to key a cached pipeline into the pipeline map.
   * This allows us to map Panda states to Vulkan pipelines effectively.
   */
  struct PipelineKey {
    INLINE bool operator ==(const PipelineKey &other) const;
    INLINE bool operator < (const PipelineKey &other) const;

    CPT(GeomVertexFormat) _format;
    VkPrimitiveTopology _topology;
    uint32_t _patch_control_points;
    VkSampleCountFlagBits _multisamples;

    ColorAttrib::Type _color_type;
    CPT(RenderModeAttrib) _render_mode_attrib;
    CullFaceAttrib::Mode _cull_face_mode;
    DepthWriteAttrib::Mode _depth_write_mode;
    RenderAttrib::PandaCompareFunc _depth_test_mode;
    int _color_write_mask;
    LogicOpAttrib::Operation _logic_op;
    CPT(ColorBlendAttrib) _color_blend_attrib;
    TransparencyAttrib::Mode _transparency_mode;
  };

private:
  VkShaderModule _modules[(size_t)Shader::Stage::COMPUTE + 1];
  VkDescriptorSetLayout _tattr_descriptor_set_layout = VK_NULL_HANDLE;
  VkDescriptorSetLayout _sattr_descriptor_set_layout = VK_NULL_HANDLE;
  VkDescriptorSetLayout _dynamic_uniform_descriptor_set_layout = VK_NULL_HANDLE;
  VkPipelineLayout _pipeline_layout = VK_NULL_HANDLE;

  // Keep track of a created descriptor set and the last frame in which it was
  // bound (since we can only update it once per frame).
  struct DescriptorSet {
    VkDescriptorSet _handle = VK_NULL_HANDLE;
    uint64_t _last_update_frame = 0;
    WeakReferenceList *_weak_ref = nullptr;
  };
  typedef pmap<const RenderAttrib *, DescriptorSet> AttribDescriptorSetMap;
  AttribDescriptorSetMap _attrib_descriptor_set_map;

  // Describe the two UBOs and push constant ranges we create.
  struct Block {
    struct Binding {
      PT(ShaderInputBinding) _binding;
      size_t _offset;
    };
    pvector<Binding> _bindings;
    VkDeviceSize _size = 0;
    int _stage_mask = 0;
    int _deps = 0;
  };
  Block _shader_input_block;
  Block _other_state_block;
  pvector<LMatrix4> _matrix_cache;
  int _matrix_cache_deps = 0;

  using ResourceId = ShaderInputBinding::ResourceId;
  struct Descriptor {
    VkDescriptorType _type;
    PT(ShaderInputBinding) _binding;
    small_vector<ResourceId, 1> _resource_ids;
    int _stage_mask = 0;
    ShaderType::Access _access = ShaderType::Access::READ_WRITE;
  };
  pvector<Descriptor> _tattr_descriptors;
  size_t _num_tattr_descriptor_elements = 0;

  pvector<Descriptor> _sattr_descriptors;
  size_t _num_sattr_descriptor_elements = 0;

  VkDescriptorSet _uniform_descriptor_set = VK_NULL_HANDLE;
  VkBuffer _uniform_buffer = VK_NULL_HANDLE;
  uint32_t _dynamic_uniform_offset = 0;

  bool _uses_vertex_color = false;

  // These are for the push constants; maybe in the future we'll replace this
  // with a more generic and flexible system.
  int _push_constant_stage_mask = 0;
  int _projection_mat_stage_mask = 0;
  int _color_scale_stage_mask = 0;

  // A map of all pipelines that use this shader.  This is in ShaderContext
  // because when a shader is released we have no more use of the pipelines
  // which use that shader.
  typedef pmap<PipelineKey, VkPipeline> PipelineMap;
  PipelineMap _pipeline_map;
  VkPipeline _compute_pipeline = VK_NULL_HANDLE;

  friend class VulkanGraphicsStateGuardian;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ShaderContext::init_type();
    register_type(_type_handle, "VulkanShaderContext",
                  ShaderContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "vulkanShaderContext.I"

#endif  // VULKANSHADERCONTEXT_H
