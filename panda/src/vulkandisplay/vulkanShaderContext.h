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

/**
 * Manages a set of Vulkan shader modules.
 */
class EXPCL_VULKANDISPLAY VulkanShaderContext : public ShaderContext {
public:
  INLINE VulkanShaderContext(Shader *shader);
  ~VulkanShaderContext() {};

  ALLOC_DELETED_CHAIN(VulkanShaderContext);

  bool make_pipeline_layout(VkDevice device);
  VkPipeline get_pipeline(VulkanGraphicsStateGuardian *gsg,
                          const RenderState *state,
                          const GeomVertexFormat *format,
                          VkPrimitiveTopology topology);

private:
  VkShaderModule _modules[2];
  VkDescriptorSetLayout _descriptor_set_layout;
  VkPipelineLayout _pipeline_layout;

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
  };

  // A map of all pipelines that use this shader.  This is in ShaderContext
  // because when a shader is released we have no more use of the pipelines
  // which use that shader.
  typedef pmap<PipelineKey, VkPipeline> PipelineMap;
  PipelineMap _pipeline_map;

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
