/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vulkanShaderContext.cxx
 * @author rdb
 * @date 2016-02-18
 */

#include "vulkanShaderContext.h"

TypeHandle VulkanShaderContext::_type_handle;

/**
 *
 */
bool VulkanShaderContext::
make_pipeline_layout(VkDevice device) {
  // Create a descriptor set layout.
  uint32_t binding_count = (uint32_t)_shader->_tex_spec.size();
  VkDescriptorSetLayoutBinding *ds_bindings = (VkDescriptorSetLayoutBinding *)
    alloca(sizeof(VkDescriptorSetLayoutBinding) * binding_count);

  uint32_t index = 0;
  for (Shader::ShaderTexSpec &spec : _shader->_tex_spec) {
    VkDescriptorSetLayoutBinding &binding = ds_bindings[index];
    binding.binding = spec._id._seqno;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    binding.descriptorCount = 1;
    binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    binding.pImmutableSamplers = nullptr;
    ++index;
  }

  VkDescriptorSetLayoutCreateInfo set_info;
  set_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  set_info.pNext = nullptr;
  set_info.flags = 0;
  set_info.bindingCount = binding_count;
  set_info.pBindings = ds_bindings;

  VkResult
  err = vkCreateDescriptorSetLayout(device, &set_info, nullptr, &_descriptor_set_layout);
  if (err) {
    vulkan_error(err, "Failed to create descriptor set layout");
    return false;
  }

  VkPushConstantRange ranges[2];
  ranges[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  ranges[0].offset = 0;
  ranges[0].size = 64;
  ranges[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  ranges[1].offset = 64;
  ranges[1].size = 16;

  VkPipelineLayoutCreateInfo layout_info;
  layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  layout_info.pNext = nullptr;
  layout_info.flags = 0;
  layout_info.setLayoutCount = 1;
  layout_info.pSetLayouts = &_descriptor_set_layout;
  layout_info.pushConstantRangeCount = 2;
  layout_info.pPushConstantRanges = ranges;

  err = vkCreatePipelineLayout(device, &layout_info, nullptr, &_pipeline_layout);
  if (err) {
    vulkan_error(err, "Failed to create pipeline layout");
    return false;
  }

  return true;
}

/**
 * Returns a VkPipeline for the given RenderState+GeomVertexFormat combination.
 */
VkPipeline VulkanShaderContext::
get_pipeline(VulkanGraphicsStateGuardian *gsg, const RenderState *state,
             const GeomVertexFormat *format, VkPrimitiveTopology topology) {
  PipelineKey key;
  key._state = state;
  key._format = format;
  key._topology = topology;

  PipelineMap::const_iterator it;
  it = _pipeline_map.find(key);
  if (it == _pipeline_map.end()) {
    VkPipeline pipeline = gsg->make_pipeline(this, state, format, topology);
    _pipeline_map[std::move(key)] = pipeline;
    return pipeline;
  } else {
    return it->second;
  }
}
