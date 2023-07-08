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

/**
 * Manages a set of Vulkan shader modules.
 */
class EXPCL_VULKANDISPLAY VulkanShaderContext : public ShaderContext {
public:
  INLINE VulkanShaderContext(Shader *shader);
  INLINE ~VulkanShaderContext();

  ALLOC_DELETED_CHAIN(VulkanShaderContext);

  bool create_modules(VkDevice device, const ShaderType::Struct *push_constant_block_type);

  VkDescriptorSetLayout make_shader_attrib_descriptor_set_layout(VkDevice device);
  VkDescriptorSetLayout make_dynamic_uniform_descriptor_set_layout(VkDevice device);

  uint32_t update_sattr_uniforms(VulkanGraphicsStateGuardian *gsg);
  uint32_t update_dynamic_uniforms(VulkanGraphicsStateGuardian *gsg, int altered);

  VkPipeline get_pipeline(VulkanGraphicsStateGuardian *gsg,
                          const RenderState *state,
                          const GeomVertexFormat *format,
                          VkPrimitiveTopology topology,
                          uint32_t patch_control_points,
                          VkSampleCountFlagBits multisamples);
  VkPipeline get_compute_pipeline(VulkanGraphicsStateGuardian *gsg);

private:
  VkShaderModule _modules[(size_t)Shader::Stage::compute + 1];
  VkDescriptorSetLayout _sattr_descriptor_set_layout = VK_NULL_HANDLE;
  VkPipelineLayout _pipeline_layout = VK_NULL_HANDLE;

  // Describe the two UBOs and push constant range we create.
  const ShaderType::Struct *_mat_block_type = nullptr;
  const ShaderType::Struct *_ptr_block_type = nullptr;
  VkDeviceSize _mat_block_size = 0;
  VkDeviceSize _ptr_block_size = 0;
  int _mat_block_stage_mask = 0;
  int _ptr_block_stage_mask = 0;
  int _mat_deps = Shader::SSD_NONE;
  LMatrix4 *_mat_part_cache = nullptr;
  pvector<Shader::ShaderMatSpec> _mat_spec;

  VkDescriptorSet _uniform_descriptor_set = VK_NULL_HANDLE;
  uint32_t _dynamic_uniform_offset = 0;

  // These are for the push constants; maybe in the future we'll replace this
  // with a more generic and flexible system.
  int _push_constant_stage_mask = 0;
  int _projection_mat_stage_mask = 0;
  int _color_scale_stage_mask = 0;

  /**
   * Stores whatever is used to key a cached pipeline into the pipeline map.
   * This allows us to map Panda states to Vulkan pipelines effectively.
   */
  struct PipelineKey {
    INLINE bool operator ==(const PipelineKey &other) const;
    INLINE bool operator < (const PipelineKey &other) const;

    CPT(RenderState) _state;
    CPT(GeomVertexFormat) _format;
    VkPrimitiveTopology _topology;
    uint32_t _patch_control_points;
    VkSampleCountFlagBits _multisamples;
  };

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
