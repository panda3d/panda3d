/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vulkanGraphicsStateGuardian.cxx
 * @author rdb
 * @date 2016-02-16
 */

#include "vulkanGraphicsStateGuardian.h"
#include "vulkanBufferContext.h"
#include "vulkanIndexBufferContext.h"
#include "vulkanTextureContext.h"
#include "vulkanVertexBufferContext.h"
#include "graphicsEngine.h"
#include "pStatGPUTimer.h"
#include "standardMunger.h"
#include "shaderModuleSpirV.h"

#include "colorAttrib.h"
#include "colorBlendAttrib.h"
#include "colorScaleAttrib.h"
#include "colorWriteAttrib.h"
#include "cullFaceAttrib.h"
#include "depthTestAttrib.h"
#include "depthWriteAttrib.h"
#include "lightAttrib.h"
#include "logicOpAttrib.h"
#include "renderModeAttrib.h"
#include "transparencyAttrib.h"

#include "lightLensNode.h"
#include "pointLight.h"
#include "paramTexture.h"

static const std::string default_vshader =
  "#version 330\n"
  "in vec4 p3d_Vertex;\n"
  "in vec4 p3d_Color;\n"
  "in vec2 p3d_MultiTexCoord0;\n"
  "out vec2 texcoord;\n"
  "out vec4 color;\n"
  "uniform mat4 p3d_ModelViewProjectionMatrix;\n"
  "uniform vec4 p3d_ColorScale;\n"
  "void main(void) {\n"
  "  gl_Position = p3d_ModelViewProjectionMatrix * p3d_Vertex;\n"
  "  texcoord = p3d_MultiTexCoord0;\n"
  "  color = p3d_Color * p3d_ColorScale;\n"
  "}\n";

static const std::string default_fshader =
  "#version 330\n"
  "in vec2 texcoord;\n"
  "in vec4 color;\n"
  "out vec4 p3d_FragColor;\n"
  "uniform sampler2D p3d_Texture0;\n"
  "void main(void) {\n"
  "  p3d_FragColor = texture(p3d_Texture0, texcoord);\n"
  "  //p3d_FragColor += p3d_TexAlphaOnly;\n" // Hack for text rendering
  "  p3d_FragColor *= color;\n"
  "}\n";

static PStatCollector _make_pipeline_pcollector("Draw:Primitive:Make Pipeline");
static PStatCollector _update_lattr_descriptor_set_pcollector("Draw:Update Descriptor Sets:LightAttrib");
static PStatCollector _bind_descriptor_sets_pcollector("Draw:Set State:Bind Descriptor Sets");
static PStatCollector _finish_frame_pcollector("Draw:Finish Frame");
static PStatCollector _wait_semaphore_pcollector("Wait:Semaphore");

TypeHandle VulkanGraphicsStateGuardian::_type_handle;

/**
 * Creates a Vulkan device and queue.
 */
VulkanGraphicsStateGuardian::
VulkanGraphicsStateGuardian(GraphicsEngine *engine, VulkanGraphicsPipe *pipe,
                            VulkanGraphicsStateGuardian *share_with,
                            uint32_t queue_family_index) :
  GraphicsStateGuardian(CS_default, engine, pipe),
  _graphics_queue_family_index(queue_family_index)
{
}

/**
 *
 */
VulkanGraphicsStateGuardian::
~VulkanGraphicsStateGuardian() {
  destroy_device();
}

/**
 * Resets all internal state as if the gsg were newly created.
 */
void VulkanGraphicsStateGuardian::
reset() {
  if (_device != VK_NULL_HANDLE) {
    close_gsg();
    destroy_device();
    _closing_gsg = false;
    _prepared_objects = new PreparedGraphicsObjects;
  }

  GraphicsStateGuardian::reset();
  _needs_reset = true;
  _is_valid = false;

  _current_shader = nullptr;
  _current_sc = nullptr;

  VulkanGraphicsPipe *pipe;
  DCAST_INTO_V(pipe, get_pipe());

  const VkPhysicalDeviceLimits &limits = pipe->_gpu_properties.limits;
  const VkPhysicalDeviceFeatures &features = pipe->_gpu_features;

  std::vector<const char*> extensions;
  if (pipe->has_device_extension("VK_KHR_swapchain")) {
    extensions.push_back("VK_KHR_swapchain");
  }

  // Enable all features we may want to use, if they are supported.
  VkPhysicalDeviceFeatures2 enabled_features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
  enabled_features.features.imageCubeArray = features.imageCubeArray;
  enabled_features.features.geometryShader = features.geometryShader;
  enabled_features.features.tessellationShader = features.tessellationShader;
  enabled_features.features.dualSrcBlend = features.dualSrcBlend;
  enabled_features.features.logicOp = features.logicOp;
  enabled_features.features.fillModeNonSolid = features.fillModeNonSolid;
  enabled_features.features.wideLines = features.wideLines;
  enabled_features.features.largePoints = features.largePoints;
  enabled_features.features.alphaToOne = features.alphaToOne;
  enabled_features.features.samplerAnisotropy = features.samplerAnisotropy;
  enabled_features.features.textureCompressionETC2 = features.textureCompressionETC2;
  enabled_features.features.textureCompressionBC = features.textureCompressionBC;
  enabled_features.features.fragmentStoresAndAtomics = features.fragmentStoresAndAtomics;
  enabled_features.features.shaderFloat64 = features.shaderFloat64;

  // Vulkan 1.2
  VkPhysicalDeviceVulkan12Features v_1_2_features = {
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
    enabled_features.pNext,
  };
  v_1_2_features.timelineSemaphore = VK_TRUE;
#ifdef DO_PSTATS
  v_1_2_features.hostQueryReset = VK_TRUE;
#endif
  enabled_features.pNext = &v_1_2_features;

  // synchronization2 from 1.3 core
  VkPhysicalDeviceSynchronization2Features sync2_features = {
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES,
    enabled_features.pNext,
    VK_TRUE,
  };
  enabled_features.pNext = &sync2_features;

  VkPhysicalDeviceDynamicRenderingFeatures dr_features = {
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES,
    enabled_features.pNext,
  };
  if (pipe->_gpu_supports_dynamic_rendering) {
    dr_features.dynamicRendering = VK_TRUE;
    enabled_features.pNext = &dr_features;

    if (pipe->_gpu_properties.apiVersion < VK_MAKE_VERSION(1, 3, 0)) {
      extensions.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    }
    _supports_dynamic_rendering = true;
  } else {
    _supports_dynamic_rendering = false;
  }

  VkPhysicalDeviceCustomBorderColorFeaturesEXT cbc_features = {
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_FEATURES_EXT,
    enabled_features.pNext,
  };
  if (pipe->_gpu_supports_custom_border_colors &&
      vulkan_support_custom_border_color) {
    cbc_features.customBorderColors = VK_TRUE;
    cbc_features.customBorderColorWithoutFormat = VK_TRUE;
    enabled_features.pNext = &cbc_features;

    extensions.push_back(VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME);
    _supports_custom_border_colors = true;
  } else {
    _supports_custom_border_colors = false;
  }

  bool supports_null_descriptor = false;
  VkPhysicalDeviceRobustness2FeaturesEXT ro2_features = {
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT,
    enabled_features.pNext,
  };
  if (pipe->_gpu_supports_null_descriptor) {
    ro2_features.nullDescriptor = VK_TRUE;
    enabled_features.pNext = &ro2_features;

    if (pipe->has_device_extension("VK_KHR_robustness2")) {
      extensions.push_back("VK_KHR_robustness2");
    } else {
      extensions.push_back(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);
    }
    supports_null_descriptor = true;
  }

  // VK_KHR_vertex_attribute_divisor / VK_EXT_vertex_attribute_divisor
  VkPhysicalDeviceVertexAttributeDivisorFeaturesKHR div_features = {
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_FEATURES_KHR,
    enabled_features.pNext,
  };
  if (pipe->_gpu_supports_vertex_attrib_divisor) {
    div_features.vertexAttributeInstanceRateDivisor = VK_TRUE;
    div_features.vertexAttributeInstanceRateZeroDivisor = pipe->_gpu_supports_vertex_attrib_zero_divisor;
    enabled_features.pNext = &div_features;

    if (pipe->has_device_extension(VK_KHR_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME)) {
      extensions.push_back(VK_KHR_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME);
    } else {
      extensions.push_back(VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME);
    }
    _supports_vertex_attrib_divisor = true;
    _supports_vertex_attrib_zero_divisor = pipe->_gpu_supports_vertex_attrib_zero_divisor;
  } else {
    _supports_vertex_attrib_divisor = false;
    _supports_vertex_attrib_zero_divisor = false;
  }

  // VK_EXT_extended_dynamic_state and VK_EXT_extended_dynamic_state2
  VkPhysicalDeviceExtendedDynamicStateFeaturesEXT eds_features = {
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
    enabled_features.pNext,
  };


  bool supports_eds_and_eds2;
  if (pipe->_gpu_properties.apiVersion >= VK_MAKE_VERSION(1, 3, 0)) {
    // Supported in the core without enable.
    supports_eds_and_eds2 = true;
    _supports_extended_dynamic_state2 = true;
  } else {
    supports_eds_and_eds2 = pipe->_gpu_supports_extended_dynamic_state && pipe->_gpu_supports_extended_dynamic_state2;

    if (supports_eds_and_eds2) {
      eds_features.extendedDynamicState = VK_TRUE;
      enabled_features.pNext = &eds_features;

      extensions.push_back(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
      _supports_extended_dynamic_state2 = true;
    } else {
      _supports_extended_dynamic_state2 = false;
    }
  }

  // VK_EXT_extended_dynamic_state2
  VkPhysicalDeviceExtendedDynamicState2FeaturesEXT eds2_features = {
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT,
    enabled_features.pNext,
  };
  if (supports_eds_and_eds2 ||
      pipe->_gpu_supports_extended_dynamic_state2_patch_control_points) {
    eds2_features.extendedDynamicState2 = supports_eds_and_eds2;
    eds2_features.extendedDynamicState2PatchControlPoints = pipe->_gpu_supports_extended_dynamic_state2_patch_control_points;
    enabled_features.pNext = &eds2_features;

    extensions.push_back(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
    _supports_extended_dynamic_state2_patch_control_points = pipe->_gpu_supports_extended_dynamic_state2_patch_control_points;
  } else {
    _supports_extended_dynamic_state2_patch_control_points = false;
  }

  // VK_KHR_portability_subset
  VkPhysicalDevicePortabilitySubsetFeaturesKHR portability_features = {
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PORTABILITY_SUBSET_FEATURES_KHR,
    enabled_features.pNext,
  };
  if (pipe->has_device_extension(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME)) {
    portability_features.imageViewFormatSwizzle = VK_TRUE;
    portability_features.vertexAttributeAccessBeyondStride = VK_TRUE;
    enabled_features.pNext = &portability_features;

    extensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
  }

  // Create a queue in the given queue family.  For now, we assume NVIDIA,
  // which has only one queue family, but we want to separate this out for
  // the sake of AMD cards.
  const float queue_priorities[2] = {0.0f, 0.0f};
  VkDeviceQueueCreateInfo queue_info;
  queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_info.pNext = nullptr;
  queue_info.flags = 0;
  queue_info.queueFamilyIndex = _graphics_queue_family_index;
  queue_info.queueCount = 1;
  queue_info.pQueuePriorities = queue_priorities;

  // Can we afford a second queue for transfer operations?
  if (pipe->_queue_families[queue_info.queueFamilyIndex].queueCount > 1) {
    queue_info.queueCount = 2;
  }

  VkDeviceCreateInfo device_info;
  device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  device_info.pNext = nullptr;
  device_info.flags = 0;
  device_info.queueCreateInfoCount = 1;
  device_info.pQueueCreateInfos = &queue_info;
  device_info.enabledLayerCount = 0;
  device_info.ppEnabledLayerNames = nullptr;
  device_info.enabledExtensionCount = extensions.size();
  device_info.ppEnabledExtensionNames = &extensions[0];
  device_info.pEnabledFeatures = nullptr;

  if (pipe->_gpu_properties.apiVersion >= VK_MAKE_VERSION(1, 1, 0) ||
      pipe->has_instance_extension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
    device_info.pNext = &enabled_features;
  } else {
    device_info.pEnabledFeatures = &enabled_features.features;
  }

  VkResult
  err = vkCreateDevice(pipe->_gpu, &device_info, nullptr, &_device);
  if (err) {
    vulkan_error(err, "Failed to create device");
    _is_valid = false;
    _needs_reset = true;
    return;
  }

  vkGetDeviceQueue(_device, _graphics_queue_family_index, 0, &_queue);
  if (queue_info.queueCount > 1) {
    vkGetDeviceQueue(_device, _graphics_queue_family_index, 1, &_dma_queue);
  } else {
    _dma_queue = _queue;
  }

  // Get direct function pointers for functions called frequently.
  _vkCmdBindIndexBuffer = (PFN_vkCmdBindIndexBuffer)vkGetDeviceProcAddr(_device, "vkCmdBindIndexBuffer");
  _vkCmdBindPipeline = (PFN_vkCmdBindPipeline)vkGetDeviceProcAddr(_device, "vkCmdBindPipeline");
  _vkCmdBindVertexBuffers = (PFN_vkCmdBindVertexBuffers)vkGetDeviceProcAddr(_device, "vkCmdBindVertexBuffers");
  _vkCmdDraw = (PFN_vkCmdDraw)vkGetDeviceProcAddr(_device, "vkCmdDraw");
  _vkCmdDrawIndexed = (PFN_vkCmdDrawIndexed)vkGetDeviceProcAddr(_device, "vkCmdDrawIndexed");
  _vkCmdPushConstants = (PFN_vkCmdPushConstants)vkGetDeviceProcAddr(_device, "vkCmdPushConstants");
  _vkCmdWriteTimestamp2 = (PFN_vkCmdWriteTimestamp2)vkGetDeviceProcAddr(_device, "vkCmdWriteTimestamp2");
  _vkUpdateDescriptorSets = (PFN_vkUpdateDescriptorSets)vkGetDeviceProcAddr(_device, "vkUpdateDescriptorSets");

  if (_supports_dynamic_rendering) {
    _vkCmdBeginRendering = (PFN_vkCmdBeginRendering)vkGetDeviceProcAddr(_device, "vkCmdBeginRendering");
    _vkCmdEndRendering = (PFN_vkCmdEndRendering)vkGetDeviceProcAddr(_device, "vkCmdEndRendering");
  }

  if (_supports_extended_dynamic_state2) {
    if (pipe->_gpu_properties.apiVersion >= VK_MAKE_VERSION(1, 3, 0)) {
      _vkCmdSetPrimitiveTopology = (PFN_vkCmdSetPrimitiveTopology)vkGetDeviceProcAddr(_device, "vkCmdSetPrimitiveTopology");
      _vkCmdSetPrimitiveRestartEnable = (PFN_vkCmdSetPrimitiveRestartEnable)vkGetDeviceProcAddr(_device, "vkCmdSetPrimitiveRestartEnable");
    } else {
      _vkCmdSetPrimitiveTopology = (PFN_vkCmdSetPrimitiveTopology)vkGetDeviceProcAddr(_device, "vkCmdSetPrimitiveTopologyEXT");
      _vkCmdSetPrimitiveRestartEnable = (PFN_vkCmdSetPrimitiveRestartEnable)vkGetDeviceProcAddr(_device, "vkCmdSetPrimitiveRestartEnableEXT");
    }
  }
  if (_supports_extended_dynamic_state2_patch_control_points) {
    _vkCmdSetPatchControlPointsEXT = (PFN_vkCmdSetPatchControlPointsEXT)vkGetDeviceProcAddr(_device, "vkCmdSetPatchControlPointsEXT");
  }

  // Create a command pool to allocate command buffers from.
  VkCommandPoolCreateInfo cmd_pool_info;
  cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  cmd_pool_info.pNext = nullptr;
  cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT |
                        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  cmd_pool_info.queueFamilyIndex = _graphics_queue_family_index;

  err = vkCreateCommandPool(_device, &cmd_pool_info, nullptr, &_cmd_pool);
  if (err) {
    vulkan_error(err, "Failed to create command pool");
    return;
  }

  // Create two command buffers per frame.
  const uint32_t num_command_buffers = 8 * _frame_data_capacity;
  _free_command_buffers.resize(num_command_buffers);
  VkCommandBufferAllocateInfo alloc_info;
  alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info.pNext = nullptr;
  alloc_info.commandPool = _cmd_pool;
  alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  alloc_info.commandBufferCount = num_command_buffers;

  err = vkAllocateCommandBuffers(_device, &alloc_info, &_free_command_buffers[0]);
  nassertv(!err);

  // Create a timeline semaphore so we can track when rendering is done.
  {
    VkSemaphoreTypeCreateInfo timeline_info;
    timeline_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
    timeline_info.pNext = NULL;
    timeline_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    timeline_info.initialValue = 0;

    VkSemaphoreCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    create_info.pNext = &timeline_info;
    create_info.flags = 0;

    err = vkCreateSemaphore(_device, &create_info, nullptr, &_timeline_semaphore);
    if (err) {
      vulkan_error(err, "Failed to create timeline semaphore");
      return;
    }
  }

  // Create a pipeline cache, which may help with performance.
  VkPipelineCacheCreateInfo cache_info;
  cache_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
  cache_info.pNext = nullptr;
  cache_info.flags = 0;
  cache_info.initialDataSize = 0;
  cache_info.pInitialData = nullptr;

  err = vkCreatePipelineCache(_device, &cache_info, nullptr, &_pipeline_cache);
  if (err) {
    // This isn't a fatal error, since we don't strictly need the cache.
    vulkan_error(err, "Failed to create pipeline cache");
  }

  //TODO: dynamic allocation, create more pools if we run out
  VkDescriptorPoolSize pool_sizes[] = {
    {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4096},
    {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 512},
    {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 256},
    {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 256},
    {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 256},
    {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 256},
    {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 128},
  };

  VkDescriptorPoolCreateInfo pool_info;
  pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_info.pNext = nullptr;
  pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  pool_info.maxSets = 1024;
  pool_info.poolSizeCount = sizeof(pool_sizes) / sizeof(VkDescriptorPoolSize);
  pool_info.pPoolSizes = pool_sizes;

  err = vkCreateDescriptorPool(_device, &pool_info, nullptr, &_descriptor_pool);
  if (err) {
    vulkan_error(err, "Failed to create descriptor pool");
    return;
  }

  // Create a "null" vertex buffer.  This will be used to store default values
  // for attributes when they are not bound to a vertex buffer.
  if (!supports_null_descriptor) {
    if (!create_buffer(4, _null_vertex_buffer, _null_vertex_memory,
                       VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                       VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
      vulkandisplay_cat.error()
        << "Failed to create null vertex buffer.\n";
      return;
    }
    _needs_write_null_vertex_data = true;
  } else {
    _null_vertex_buffer = VK_NULL_HANDLE;
    _needs_write_null_vertex_data = false;
  }

  // Create a push constant layout based on the available space.
  {
    ShaderType::Struct struct_type;
    struct_type.add_member(ShaderType::register_type(ShaderType::Matrix(ShaderType::ST_float, 4, 4)), "p3d_ModelViewProjectionMatrix");
    struct_type.add_member(ShaderType::register_type(ShaderType::Vector(ShaderType::ST_float, 4)), "p3d_ColorScale");
    _push_constant_block_type = ShaderType::register_type(std::move(struct_type));
  }

  // Create a descriptor set layout for our LightAttrib descriptor set.
  const uint32_t num_shadow_maps = 8;
  {
    // The depth map is managed by Panda, so we can bake the sampler state into
    // the pipeline layout, which allows lower-latency samples on some cards.
    VkSamplerCreateInfo sampler_info;
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.pNext = nullptr;
    sampler_info.flags = 0;
    sampler_info.magFilter = VK_FILTER_NEAREST;
    sampler_info.minFilter = VK_FILTER_NEAREST;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    sampler_info.mipLodBias = 0;
    sampler_info.anisotropyEnable = VK_FALSE;
    sampler_info.maxAnisotropy = 0;
    sampler_info.compareEnable = VK_TRUE;
    sampler_info.compareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    sampler_info.minLod = 0;
    sampler_info.maxLod = 0.25;
    sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    sampler_info.unnormalizedCoordinates = VK_FALSE;

    VkResult err;
    err = vkCreateSampler(_device, &sampler_info, nullptr, &_shadow_sampler);
    if (err) {
      vulkan_error(err, "Failed to create shadow sampler object");
      return;
    }

    VkSampler immutable_samplers[num_shadow_maps];
    for (size_t i = 0; i < num_shadow_maps; ++i) {
      immutable_samplers[i] = _shadow_sampler;
    }

    VkDescriptorSetLayoutBinding binding;
    binding.binding = 0;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    binding.descriptorCount = num_shadow_maps;
    binding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
    binding.pImmutableSamplers = immutable_samplers;

    VkDescriptorSetLayoutCreateInfo set_info;
    set_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    set_info.pNext = nullptr;
    set_info.flags = 0;
    set_info.bindingCount = 1;
    set_info.pBindings = &binding;

    err = vkCreateDescriptorSetLayout(_device, &set_info, nullptr,
      &_lattr_descriptor_set_layout);
    if (err) {
      vulkan_error(err, "Failed to create descriptor set layout for LightAttrib");
      return;
    }
  }

  // Make a descriptor set that we apply when there are no lights.
  {
    VkDescriptorSetAllocateInfo alloc_info;
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.pNext = nullptr;
    alloc_info.descriptorPool = _descriptor_pool;
    alloc_info.descriptorSetCount = 1;
    alloc_info.pSetLayouts = &_lattr_descriptor_set_layout;
    VkResult err = vkAllocateDescriptorSets(_device, &alloc_info, &_empty_lattr_descriptor_set);
    if (err) {
      vulkan_error(err, "Failed to allocate descriptor set for empty light attribute");
      return;
    }
  }

  // Create a uniform buffer that we'll use for everything.
  // Some cards set aside 256 MiB of device-local host-visible memory for data
  // like this, so we use that.
  // We also use it as a vertex buffer for flat vertex colors.
  VkDeviceSize uniform_buffer_size = vulkan_global_uniform_buffer_size;
  if (!create_buffer(uniform_buffer_size, _uniform_buffer, _uniform_buffer_memory,
                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                     (VkMemoryPropertyFlagBits)(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))) {
    // No?  Put it in GPU-accessible CPU memory, then.
    if (!create_buffer(uniform_buffer_size, _uniform_buffer, _uniform_buffer_memory,
                       VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                       (VkMemoryPropertyFlagBits)(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))) {
      vulkandisplay_cat.error()
        << "Failed to create global uniform buffer.\n";
    }
    return;
  }
  _uniform_buffer_ptr = _uniform_buffer_memory.map_persistent();
  if (_uniform_buffer_ptr == nullptr) {
    vulkandisplay_cat.error()
      << "Failed to map global uniform buffer.\n";
    return;
  }
  _uniform_buffer_allocator = CircularAllocator(uniform_buffer_size, limits.minUniformBufferOffsetAlignment);

  // If we have only one heap, it's safe to assume we're on a UMA system.
  _has_unified_memory = (pipe->_memory_properties.memoryHeapCount == 1);

  // Create a staging buffer for CPU-to-GPU uploads.
  VkDeviceSize staging_buffer_size = vulkan_staging_buffer_size;
  if (staging_buffer_size > 0) {
    if (create_buffer(staging_buffer_size, _staging_buffer, _staging_buffer_memory,
                      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                      (VkMemoryPropertyFlagBits)(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))) {
      _staging_buffer_ptr = _staging_buffer_memory.map_persistent();
      if (_staging_buffer_ptr != nullptr) {
        _staging_buffer_allocator = CircularAllocator(staging_buffer_size, limits.optimalBufferCopyOffsetAlignment);
      }
    }
  }

  // Fill in the features supported by this physical device.
  _is_hardware = (pipe->_gpu_properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_CPU);

  _max_vertices_per_array = std::max((uint32_t)0x7fffffff, limits.maxDrawIndexedIndexValue);
  _max_vertices_per_primitive = INT_MAX;

  _max_texture_stages = std::max((uint32_t)0x7fffffff, std::min(limits.maxPerStageDescriptorSampledImages, std::min(limits.maxDescriptorSetSampledImages, limits.maxPerStageDescriptorSampledImages)));
  _max_texture_dimension = limits.maxImageDimension2D;
  _max_3d_texture_dimension = limits.maxImageDimension3D;
  _max_2d_texture_array_layers = limits.maxImageArrayLayers;
  _max_cube_map_dimension = limits.maxImageDimensionCube;
  _max_buffer_texture_size = limits.maxTexelBufferElements;

  _max_compute_work_group_invocations = limits.maxComputeWorkGroupInvocations;
  _max_compute_work_group_count.set(limits.maxComputeWorkGroupCount[0],
                                    limits.maxComputeWorkGroupCount[1],
                                    limits.maxComputeWorkGroupCount[2]);
  _max_compute_work_group_size.set(limits.maxComputeWorkGroupSize[0],
                                   limits.maxComputeWorkGroupSize[1],
                                   limits.maxComputeWorkGroupSize[2]);

  _supports_3d_texture = true;
  _supports_2d_texture_array = true;
  _supports_cube_map = true;
  _supports_buffer_texture = true;
  _supports_cube_map_array = (features.imageCubeArray != VK_FALSE);
  _supports_tex_non_pow2 = true;
  _supports_texture_srgb = true;
  _supports_compressed_texture = (features.textureCompressionBC != VK_FALSE ||
                                  features.textureCompressionETC2 != VK_FALSE);

  if (features.textureCompressionBC) {
    _compressed_texture_formats.set_bit(Texture::CM_dxt1);
    _compressed_texture_formats.set_bit(Texture::CM_dxt3);
    _compressed_texture_formats.set_bit(Texture::CM_dxt5);
    _compressed_texture_formats.set_bit(Texture::CM_rgtc);
  }
  if (features.textureCompressionETC2) {
    _compressed_texture_formats.set_bit(Texture::CM_etc1);
    _compressed_texture_formats.set_bit(Texture::CM_etc2);
    _compressed_texture_formats.set_bit(Texture::CM_eac);
  }

  // Assume no limits on number of lights or clip planes.
  _max_lights = -1;
  _max_clip_planes = -1;

  _supports_occlusion_query = false;
  _supports_timer_query = limits.timestampComputeAndGraphics;
  _timer_query_factor = 0.000000001 * limits.timestampPeriod;

  // Set to indicate that we get an inverted result when we copy the
  // framebuffer to a texture.
  _copy_texture_inverted = true;

  // Similarly with these capabilities flags.
  _supports_multisample = true;
  _supports_generate_mipmap = false;
  _supports_depth_texture = true;
  _supports_depth_stencil = true;
  _supports_shadow_filter = true;
  _supports_sampler_objects = true;
  _supports_glsl = false;
  _supports_hlsl = false;
  _supports_framebuffer_multisample = true;
  _supports_framebuffer_blit = true;

  _supports_stencil = true;
  _supports_stencil_wrap = true;
  _supports_two_sided_stencil = true;
  _supports_geometry_instancing = true;
  _supports_indirect_draw = true;

  _supported_shader_caps =
    Shader::C_basic_shader |
    Shader::C_vertex_texture |
    Shader::C_point_coord |
    Shader::C_standard_derivatives |
    Shader::C_shadow_samplers |
    Shader::C_non_square_matrices |
    Shader::C_texture_lod |
    Shader::C_unified_model |
    Shader::C_noperspective_interpolation |
    Shader::C_texture_array |
    Shader::C_texture_integer |
    Shader::C_texture_query_size |
    Shader::C_sampler_cube_shadow |
    Shader::C_vertex_id |
    Shader::C_draw_buffers |
    Shader::C_instance_id |
    Shader::C_texture_buffer |
    Shader::C_bit_encoding |
    Shader::C_texture_query_lod |
    Shader::C_texture_gather_red |
    Shader::C_texture_gather_any |
    Shader::C_extended_arithmetic |
    Shader::C_multisample_interpolation |
    Shader::C_image_query_size |
    Shader::C_texture_query_levels |
    Shader::C_compute_shader |
    Shader::C_enhanced_layouts |
    Shader::C_derivative_control |
    Shader::C_texture_query_samples;

  if (features.shaderClipDistance) {
    _supported_shader_caps |= Shader::C_clip_distance;
  }

  if (features.shaderCullDistance) {
    _supported_shader_caps |= Shader::C_cull_distance;
  }

  if (features.geometryShader) {
    _supported_shader_caps |=
      Shader::C_geometry_shader |
      Shader::C_primitive_id;

    if (limits.maxGeometryShaderInvocations > 1) {
      _supported_shader_caps |= Shader::C_geometry_shader_instancing;
    }
  }

  if (features.tessellationShader) {
    _supported_shader_caps |= Shader::C_tessellation_shader;
  }

  if (features.shaderFloat64) {
    _supported_shader_caps |= Shader::C_double;
  }

  if (features.imageCubeArray) {
    _supported_shader_caps |= Shader::C_cube_map_array;
  }

  if (features.sampleRateShading) {
    _supported_shader_caps |= Shader::C_sample_variables;
  }

  if (features.shaderSampledImageArrayDynamicIndexing) {
    _supported_shader_caps |= Shader::C_dynamic_indexing;
  }

  if (limits.maxDescriptorSetStorageImages > 0 &&
      features.vertexPipelineStoresAndAtomics &&
      features.fragmentStoresAndAtomics) {
    _supported_shader_caps |= Shader::C_image_load_store | Shader::C_image_atomic;
  }

  if (limits.maxDescriptorSetStorageBuffers > 0) {
    _supported_shader_caps |= Shader::C_storage_buffer;
  }

  _max_color_targets = limits.maxColorAttachments;
  _supports_dual_source_blending = (features.dualSrcBlend != VK_FALSE);

  _supported_geom_rendering =
    Geom::GR_indexed_point |
    Geom::GR_point |
    Geom::GR_indexed_other |
    Geom::GR_triangle_strip | Geom::GR_triangle_fan |
    Geom::GR_line_strip |
    Geom::GR_flat_first_vertex | //TODO: is this correct?
    Geom::GR_strip_cut_index;

  if (features.fillModeNonSolid) {
    _supported_geom_rendering |= Geom::GR_render_mode_wireframe |
                                 Geom::GR_render_mode_point;
  }

  if (features.largePoints) {
    _supported_geom_rendering |= Geom::GR_point_uniform_size;
  }

  // Load the default shader.  Temporary hack.
  static PT(Shader) default_shader;
  if (default_shader.is_null()) {
    default_shader = Shader::make(Shader::SL_GLSL, default_vshader, default_fshader);
    nassertv(default_shader);
  }
  if (_default_sc == nullptr) {
    ShaderContext *sc = default_shader->prepare_now(get_prepared_objects(), this);
    nassertv(sc);
    _default_sc = DCAST(VulkanShaderContext, sc);
  }

  _is_valid = true;
  _needs_reset = false;
}

/**
 * Releases the device and all associated resources.
 */
void VulkanGraphicsStateGuardian::
destroy_device() {
  if (_device == VK_NULL_HANDLE) {
    nassertv(_memory_pages.empty());
    return;
  }

  // Remove the things we created in the constructor, in reverse order.
  vkDestroyBuffer(_device, _uniform_buffer, nullptr);
  vkDestroyDescriptorSetLayout(_device, _lattr_descriptor_set_layout, nullptr);
  vkDestroyDescriptorPool(_device, _descriptor_pool, nullptr);
  vkDestroySampler(_device, _shadow_sampler, nullptr);
  vkDestroyPipelineCache(_device, _pipeline_cache, nullptr);
  vkDestroyCommandPool(_device, _cmd_pool, nullptr);

  if (_null_vertex_buffer != VK_NULL_HANDLE) {
    vkDestroyBuffer(_device, _null_vertex_buffer, nullptr);
    _null_vertex_buffer = VK_NULL_HANDLE;
  }

  if (_staging_buffer != VK_NULL_HANDLE) {
    vkDestroyBuffer(_device, _staging_buffer, nullptr);
    _staging_buffer = VK_NULL_HANDLE;
  }

  vkDestroySemaphore(_device, _timeline_semaphore, nullptr);

  // Also free all the memory pages before destroying the device.
  _memory_pages.clear();

  vkDestroyDevice(_device, nullptr);
  _device = VK_NULL_HANDLE;
}

/**
 * This is called by the associated GraphicsWindow when close_window() is
 * called.  It should null out the _win pointer and possibly free any open
 * resources associated with the GSG.
 */
void VulkanGraphicsStateGuardian::
close_gsg() {
  // All pending work should be submitted.
  flush();

  // Make sure it's no longer doing anything before we destroy it.
  vkDeviceWaitIdle(_device);

  // Call finish_frame() on all frames.  We don't need to wait on the
  // semaphore due to the above call.
  if (_frame_data_head != _frame_data_capacity) {
    do {
      FrameData &frame_data = _frame_data_pool[_frame_data_tail];
      finish_frame(frame_data);

      _frame_data_tail = (_frame_data_tail + 1) % _frame_data_capacity;
    }
    while (_frame_data_tail != _frame_data_head);

    _frame_data_head = _frame_data_capacity;
    _frame_data_tail = 0;
  }

  // We need to release all prepared resources, since the upcall to close_gsg
  // will cause the PreparedGraphicsObjects to be cleared out.
  {
    PT(PreparedGraphicsObjects) pgo = std::move(_prepared_objects);
    if (pgo != nullptr) {
      // Create a temporary FrameData to hold all objects we need to destroy.
      FrameData frame_data;
      frame_data._frame_index = ++_frame_counter;
      _frame_data = &frame_data;
      pgo->release_all_now(this);
      _frame_data = nullptr;
      finish_frame(frame_data);
    }
    _default_sc = nullptr;
  }

  GraphicsStateGuardian::close_gsg();
}

/**
 * Returns the vendor of the video card driver
 */
std::string VulkanGraphicsStateGuardian::
get_driver_vendor() {
  VulkanGraphicsPipe *vkpipe;
  DCAST_INTO_R(vkpipe, get_pipe(), nullptr);

  const char *vendor = vkpipe->get_vendor_name();
  if (vendor != nullptr) {
    return std::string(vendor);
  } else {
    char vendor[24];
    sprintf(vendor, "Unknown vendor 0x%04X", vkpipe->_gpu_properties.vendorID);
    return std::string(vendor);
  }
}

/**
 * Returns GL_Renderer
 */
std::string VulkanGraphicsStateGuardian::
get_driver_renderer() {
  VulkanGraphicsPipe *vkpipe;
  DCAST_INTO_R(vkpipe, get_pipe(), nullptr);
  return std::string(vkpipe->_gpu_properties.deviceName);
}

/**
 * Returns driver version This has an implementation-defined meaning, and may
 * be "" if the particular graphics implementation does not provide a way to
 * query this information.
 */
std::string VulkanGraphicsStateGuardian::
get_driver_version() {
  return std::string();
}

/**
 * Allocates a block of graphics memory, using the given requirements.
 */
bool VulkanGraphicsStateGuardian::
allocate_memory(VulkanMemoryBlock &block, const VkMemoryRequirements &reqs,
                VkFlags required_flags, bool linear) {
  //MutexHolder holder(_allocator_lock);

  for (VulkanMemoryPage &page : _memory_pages) {
    if (page.meets_requirements(reqs, required_flags, linear)) {
      SimpleAllocatorBlock *result = page.alloc(reqs.size, reqs.alignment);
      if (result != nullptr) {
        block = std::move(*result);
        delete result;
        return true;
      }
    }
  }

  VulkanGraphicsPipe *vkpipe;
  DCAST_INTO_R(vkpipe, get_pipe(), false);

  // We don't have a matching allocator.  Create a new one.
  uint32_t type_index;
  if (!vkpipe->find_memory_type(type_index, reqs, required_flags)) {
    return false;
  }
  VkFlags flags = vkpipe->_memory_properties.memoryTypes[type_index].propertyFlags;

  VkMemoryAllocateInfo alloc_info;
  alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  alloc_info.pNext = nullptr;
  alloc_info.memoryTypeIndex = type_index;
  alloc_info.allocationSize = std::max(std::min(vkpipe->_max_allocation_size, (VkDeviceSize)vulkan_memory_page_size), reqs.size);

  VkDeviceMemory memory;
  VkResult err;
  do {
    err = vkAllocateMemory(_device, &alloc_info, nullptr, &memory);
    if (!err) {
      break;
    }

    // No?  Try a smaller allocation.
    alloc_info.allocationSize >>= 1;
  } while (alloc_info.allocationSize >= reqs.size);

  if (err) {
    vulkan_error(err, "Failed to allocate new memory page");
    return false;
  }

  _total_allocated += alloc_info.allocationSize;

  if (vulkandisplay_cat.is_debug()) {
    size_t size_kb = alloc_info.allocationSize >> 10u;
    if (size_kb > 4096) {
      vulkandisplay_cat.debug()
        << "Allocated new " << (size_kb >> 10u) << " MiB page";
    } else {
      vulkandisplay_cat.debug()
        << "Allocated new " << size_kb << " KiB page";
    }

    vulkandisplay_cat.debug(false)
      << ", type " << std::dec << type_index;
    if (flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
      vulkandisplay_cat.debug(false) << ", device-local";
    }
    if (flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
      vulkandisplay_cat.debug(false) << ", host-visible";
    }
    if (flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
      vulkandisplay_cat.debug(false) << ", host-coherent";
    }
    if (flags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) {
      vulkandisplay_cat.debug(false) << ", host-cached";
    }
    if (flags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) {
      vulkandisplay_cat.debug(false) << ", lazily-allocated";
    }
    if (flags & VK_MEMORY_PROPERTY_PROTECTED_BIT) {
      vulkandisplay_cat.debug(false) << ", protected";
    }
    if (flags & VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD) {
      vulkandisplay_cat.debug(false) << ", device-coherent";
    }
    if (flags & VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD) {
      vulkandisplay_cat.debug(false) << ", device-uncached";
    }
    if (flags & VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV) {
      vulkandisplay_cat.debug(false) << ", rdma-capable";
    }
    vulkandisplay_cat.debug(false) << "\n";
  }

  VulkanMemoryPage page(_device, memory, alloc_info.allocationSize, type_index, flags, _allocator_lock);

  // We use a separate page for images with optimal tiling since they need to
  // have a certain distance to regular tiling allocations, according to the
  // bufferImageGranularity limit.  It's easier to just have a separate page.
  page._linear_tiling = linear;

  SimpleAllocatorBlock *result = page.alloc(reqs.size, reqs.alignment);
  nassertr_always(result != nullptr, false);
  block = std::move(*result);
  delete result;

  _memory_pages.push_back(std::move(page));
  return true;
}

/**
 * Prepares the texture for the given usage of the texture, performing any
 * updates as necessary.  If discard is true, the existing contents are
 * disregarded, and any pending upload is discarded.
 */
VulkanTextureContext *VulkanGraphicsStateGuardian::
use_texture(Texture *texture, VkImageLayout layout,
            VkPipelineStageFlags2 stage_mask, VkAccessFlags2 access_mask,
            bool discard) {
  nassertr(_render_cmd, nullptr);

  VulkanTextureContext *tc;
  DCAST_INTO_R(tc, texture->prepare_now(_prepared_objects, this), nullptr);

  if (_fb_color_tc == tc || _fb_depth_tc == tc) {
    vulkandisplay_cat.warning()
      << "Attempt to use framebuffer texture " << *texture << " during render!\n";
    return nullptr;
  }

  // We only update the texture the first time it is used in a frame.
  // Otherwise, we would have to invalidate the descriptor sets.
  if (tc->_read_seq < _render_cmd._seq) {
    if (tc->was_modified()) {
      if (tc->needs_recreation()) {
        tc->release(get_frame_data());
        if (!create_texture(tc)) {
          return nullptr;
        }
      }

      // If discard is true, we are about to replace the texture contents, so we
      // just skip the upload step and mark it as loaded anyway.
      if (!discard && !upload_texture(tc)) {
        return nullptr;
      }

      tc->mark_loaded();
    }

    tc->set_active(true);
    tc->enqueue_lru(&_prepared_objects->_graphics_memory_lru);
  }

  if (discard) {
    // Flags it as not caring if the current contents get stomped over.
    tc->discard();
  }

  _render_cmd.add_barrier(tc, layout, stage_mask, access_mask);
  return tc;
}

/**
 * Creates whatever structures the GSG requires to represent the texture
 * internally, and returns a newly-allocated TextureContext object with this
 * data.  It is the responsibility of the calling function to later call
 * release_texture() with this same pointer (which will also delete the
 * pointer).
 *
 * This function should not be called directly to prepare a texture.  Instead,
 * call Texture::prepare().
 */
TextureContext *VulkanGraphicsStateGuardian::
prepare_texture(Texture *texture) {
  PStatTimer timer(_prepare_texture_pcollector);

  VulkanTextureContext *tc = new VulkanTextureContext(get_prepared_objects(), texture);
  if (tc != nullptr && create_texture(tc)) {
    return tc;
  } else {
    delete tc;
    return nullptr;
  }
}

/**
 *
 */
bool VulkanGraphicsStateGuardian::
create_texture(VulkanTextureContext *tc) {
  using std::swap;

  VulkanGraphicsPipe *vkpipe;
  DCAST_INTO_R(vkpipe, get_pipe(), false);

  nassertr(tc->_image == VK_NULL_HANDLE, false);

  Texture *texture = tc->get_texture();

  VkImageCreateFlags flags = 0;
  VkImageType type;
  VkExtent3D extent;
  extent.width = texture->get_x_size();
  extent.height = texture->get_y_size();
  extent.depth = 1;
  int num_views = texture->get_num_views();
  uint32_t num_layers_per_view = 1;
  uint32_t num_layers = 1;
  uint32_t num_levels = 1;
  bool is_buffer = false;

  if (extent.width == 0) {
    extent.width = 1;
  }
  if (extent.height == 0) {
    extent.height = 1;
  }

  switch (texture->get_texture_type()) {
  case Texture::TT_1d_texture:
  case Texture::TT_1d_texture_array:
    type = VK_IMAGE_TYPE_1D;
    swap(extent.height, num_layers);
    break;

  case Texture::TT_cube_map:
  case Texture::TT_cube_map_array:
    flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    // Fall through
  case Texture::TT_2d_texture:
  case Texture::TT_2d_texture_array:
    type = VK_IMAGE_TYPE_2D;
    num_layers = texture->get_z_size();
    break;

  case Texture::TT_3d_texture:
    type = VK_IMAGE_TYPE_3D;
    extent.depth = texture->get_z_size();
    break;

  case Texture::TT_buffer_texture:
    type = VK_IMAGE_TYPE_1D; // Just to shut up compiler warning
    is_buffer = true;
    break;

  default:
    vulkandisplay_cat.error()
      << "Unsupported texture type " << texture->get_texture_type() << "!\n";
    return false;
  }
  const VkExtent3D orig_extent = extent;

  num_layers_per_view = num_layers;
  num_layers *= num_views;

  // Check if the format is actually supported.
  VkFormat format = get_image_format(texture);
  VkFormatProperties fmt_props;
  vkGetPhysicalDeviceFormatProperties(vkpipe->_gpu, format, &fmt_props);

  Texture::Format tex_format = texture->get_format();
  bool is_depth = (tex_format == Texture::F_depth_stencil
                || tex_format == Texture::F_depth_component
                || tex_format == Texture::F_depth_component16
                || tex_format == Texture::F_depth_component24
                || tex_format == Texture::F_depth_component32);

  bool supported;
  if (is_buffer) {
    supported = (fmt_props.bufferFeatures & VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT) != 0;
    if (!is_depth && supported) {
      supported = (fmt_props.bufferFeatures & VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT) != 0;
    }
  } else {
    supported = (fmt_props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) != 0;
    if (!is_depth && supported) {
      supported = (fmt_props.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT) != 0;
    }
  }

  bool pack_bgr8 = false;
  bool swap_bgra8 = false;
  if (!supported) {
    // Not fully supported.  Can we convert it to a format that is supported?
    // We pick ones with mandatory support in Vulkan; no need to check things.
    switch (format) {
    case VK_FORMAT_B8G8R8_UNORM:
      format = VK_FORMAT_R8G8B8A8_UNORM;
      pack_bgr8 = true;
      break;

    case VK_FORMAT_B8G8R8_SNORM:
      format = VK_FORMAT_R8G8B8A8_SNORM;
      pack_bgr8 = true;
      break;

    case VK_FORMAT_B8G8R8_UINT:
      format = VK_FORMAT_R8G8B8A8_UINT;
      pack_bgr8 = true;
      break;

    case VK_FORMAT_B8G8R8_SINT:
      format = VK_FORMAT_R8G8B8A8_SINT;
      pack_bgr8 = true;
      break;

    case VK_FORMAT_B8G8R8A8_UNORM:
      format = VK_FORMAT_R8G8B8A8_UNORM;
      swap_bgra8 = true;
      break;

    case VK_FORMAT_B8G8R8A8_SNORM:
      format = VK_FORMAT_R8G8B8A8_SNORM;
      swap_bgra8 = true;
      break;

    case VK_FORMAT_B8G8R8A8_UINT:
      format = VK_FORMAT_R8G8B8A8_UINT;
      swap_bgra8 = true;
      break;

    case VK_FORMAT_B8G8R8A8_SINT:
      format = VK_FORMAT_R8G8B8A8_SINT;
      swap_bgra8 = true;
      break;

    default:
      // We'll still allow it if it's supported in a limited fashion.
      if (is_buffer) {
        supported = (fmt_props.bufferFeatures & VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT) != 0;
      } else {
        supported = (fmt_props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) != 0;
      }

      if (!supported) {
        vulkandisplay_cat.error()
          << "Texture format " << format << " not supported.\n";
        return false;
      }
    }

    // Update the properties for the new format.
    vkGetPhysicalDeviceFormatProperties(vkpipe->_gpu, format, &fmt_props);
  }

  bool render_to_texture = false;
  if (!is_buffer) {
    // Image texture.  Is the size supported for this format?
    VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (fmt_props.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT) {
      usage |= VK_IMAGE_USAGE_STORAGE_BIT;
    }

    render_to_texture = texture->get_render_to_texture();
    if (render_to_texture) {
      if (is_depth) {
        usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
      } else {
        usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
      }
    }

    VkImageFormatProperties img_props;
    if (vkGetPhysicalDeviceImageFormatProperties(vkpipe->_gpu, format, type,
                                                 VK_IMAGE_TILING_OPTIMAL, usage,
                                                 flags, &img_props) == VK_ERROR_FORMAT_NOT_SUPPORTED) {
      vulkandisplay_cat.error()
        << "Texture format " << format << " is not supported.\n";
      return false;
    }
    nassertr(img_props.maxArrayLayers > 0, false);
    if (num_layers > img_props.maxArrayLayers) {
      //TODO: more elegant solution to reduce layer count.
      vulkandisplay_cat.error()
        << "Texture has too many layers, format " << format << " has a maximum of "
        << img_props.maxArrayLayers << " layers\n";
      return false;
    }
    int mipmap_begin = 0;
    while (extent.width > img_props.maxExtent.width ||
           extent.height > img_props.maxExtent.height ||
           extent.depth > img_props.maxExtent.depth) {
      // Reduce the size by bumping the first mipmap level uploaded.
      extent.width = std::max(1U, extent.width >> 1);
      extent.height = std::max(1U, extent.height >> 1);
      extent.depth = std::max(1U, extent.depth >> 1);
      ++mipmap_begin;
    }

    if (mipmap_begin != 0) {
      vulkandisplay_cat.info()
        << "Reducing image " << texture->get_name() << " from "
        << orig_extent.width << " x " << orig_extent.height << " x "
        << orig_extent.depth << " to " << extent.width << " x "
        << extent.height << " x " << extent.depth << "\n";

      if (texture->has_ram_image() && !texture->has_ram_mipmap_image(mipmap_begin)) {
        // Ugh, and to do this, we have to generate mipmaps on the CPU.
        texture->generate_ram_mipmap_images();
      }
    }

    int mipmap_end = mipmap_begin + 1;
    if (texture->uses_mipmaps()) {
      mipmap_end = texture->get_expected_num_mipmap_levels();
      nassertr(mipmap_end > mipmap_begin, false);
    }

    // Do we need to generate any mipmaps?
    bool generate_mipmaps = false;
    for (int i = mipmap_begin; i < mipmap_end; ++i) {
      if (!texture->has_ram_mipmap_image(i)) {
        generate_mipmaps = true;
        break;
      }
    }

    if (generate_mipmaps) {
      if ((fmt_props.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT) == 0 ||
          (fmt_props.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT) == 0 ||
          !driver_generate_mipmaps) {
        // Hang on, we don't support blitting using this format.  We'll have to
        // generate the mipmaps on the CPU instead.
        if (texture->has_ram_image()) {
          texture->generate_ram_mipmap_images();

          // We now have as many levels as we're going to get.
          mipmap_end = texture->get_num_ram_mipmap_images();
        }
        generate_mipmaps = false;
      } else {
        // We may be generating mipmaps from it, so mark it as transfer source.
        usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
      }
    }

    num_levels = mipmap_end - mipmap_begin;
    if (num_levels > img_props.maxMipLevels) {
      mipmap_end -= num_levels - img_props.maxMipLevels;
      num_levels = img_props.maxMipLevels;
    }

    if (!create_image(tc, type, format, extent, num_levels, num_layers,
                      VK_SAMPLE_COUNT_1_BIT, usage, flags)) {
      return false;
    }
    tc->_mipmap_begin = mipmap_begin;
    tc->_mipmap_end = mipmap_end;
    tc->_generate_mipmaps = generate_mipmaps;
    tc->_pack_bgr8 = pack_bgr8;
    tc->_supports_render_to_texture = render_to_texture;

    if (is_depth) {
      tc->_aspect_mask = VK_IMAGE_ASPECT_DEPTH_BIT;
    } else {
      tc->_aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    if (vulkandisplay_cat.is_debug()) {
      vulkandisplay_cat.debug()
        << "Created image " << tc->_image << " for texture " << *texture << "\n";
    }

    // Now we'll create an image view that describes how we interpret the image.
    VkImageViewCreateInfo view_info;
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.pNext = nullptr;
    view_info.flags = 0;
    view_info.image = tc->_image;

    switch (texture->get_texture_type()) {
    case Texture::TT_1d_texture:
      view_info.viewType = VK_IMAGE_VIEW_TYPE_1D;
      break;
    case Texture::TT_2d_texture:
      view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
      break;
    case Texture::TT_3d_texture:
      view_info.viewType = VK_IMAGE_VIEW_TYPE_3D;
      break;
    case Texture::TT_2d_texture_array:
      view_info.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
      break;
    case Texture::TT_cube_map:
      view_info.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
      break;
    case Texture::TT_buffer_texture: //TODO: figure out buffer textures in Vulkan.
      view_info.viewType = VK_IMAGE_VIEW_TYPE_1D;
      break;
    case Texture::TT_cube_map_array:
      view_info.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
      break;
    case Texture::TT_1d_texture_array:
      view_info.viewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
      break;
    }

    view_info.format = format;

    // We use the swizzle mask to emulate deprecated formats.
    switch (texture->get_format()) {
    case Texture::F_green:
      view_info.components.r = VK_COMPONENT_SWIZZLE_ZERO;
      view_info.components.g = VK_COMPONENT_SWIZZLE_R;
      view_info.components.b = VK_COMPONENT_SWIZZLE_ZERO;
      view_info.components.a = VK_COMPONENT_SWIZZLE_ONE;
      break;

    case Texture::F_blue:
      view_info.components.r = VK_COMPONENT_SWIZZLE_ZERO;
      view_info.components.g = VK_COMPONENT_SWIZZLE_ZERO;
      view_info.components.b = VK_COMPONENT_SWIZZLE_R;
      view_info.components.a = VK_COMPONENT_SWIZZLE_ZERO;
      break;

    case Texture::F_alpha:
      //FIXME: better solution for fixing black text issue
      view_info.components.r = VK_COMPONENT_SWIZZLE_ONE;
      view_info.components.g = VK_COMPONENT_SWIZZLE_ONE;
      view_info.components.b = VK_COMPONENT_SWIZZLE_ONE;
      view_info.components.a = VK_COMPONENT_SWIZZLE_R;
      break;

    case Texture::F_luminance:
    case Texture::F_sluminance:
      view_info.components.r = VK_COMPONENT_SWIZZLE_R;
      view_info.components.g = VK_COMPONENT_SWIZZLE_R;
      view_info.components.b = VK_COMPONENT_SWIZZLE_R;
      view_info.components.a = VK_COMPONENT_SWIZZLE_ONE;
      break;

    case Texture::F_luminance_alpha:
      // F_sluminance_alpha can't be emulated using R8G8 and a swizzle mask
      // because we need the second channel to be linear.  Beh.
      view_info.components.r = VK_COMPONENT_SWIZZLE_R;
      view_info.components.g = VK_COMPONENT_SWIZZLE_R;
      view_info.components.b = VK_COMPONENT_SWIZZLE_R;
      view_info.components.a = VK_COMPONENT_SWIZZLE_G;
      break;

    default:
      view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
      view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
      view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
      view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
      break;
    }

    view_info.subresourceRange.aspectMask = tc->_aspect_mask;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = num_levels;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = num_layers_per_view;

    for (int view = 0; view < num_views; ++view) {
      VkImageView image_view;
      VkResult err;
      err = vkCreateImageView(_device, &view_info, nullptr, &image_view);
      if (err) {
        vulkan_error(err, "Failed to create image view for texture");
        tc->destroy_now(_device);
        return false;
      }

      tc->_image_views.push_back(image_view);
      view_info.subresourceRange.baseArrayLayer += num_layers_per_view;
    }
  }
  else {
    // Buffer texture.
    if (extent.width > (uint32_t)_max_buffer_texture_size) {
      vulkandisplay_cat.error()
        << "Buffer texture size " << extent.width << " is too large, maximum size is "
        << _max_buffer_texture_size << " texels\n";
      return false;
    }

    VkBufferUsageFlags usage = 0;
    usage |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
    usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    if (fmt_props.bufferFeatures & VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT) {
      usage |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
    }

    VkBuffer buffer;
    VulkanMemoryBlock block;
    //VkDeviceSize view_size = texture->get_expected_ram_view_size();
    VkDeviceSize view_size = texture->get_expected_ram_image_size() / (size_t)num_views;
    if (pack_bgr8) {
      view_size = view_size / 3 * 4;
    }
    VkDeviceSize total_size = view_size * num_views;
    if (!create_buffer(total_size, buffer, block, usage,
                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
      return false;
    }

    if (vulkandisplay_cat.is_debug()) {
      vulkandisplay_cat.debug()
        << "Created buffer " << buffer << " with size " << total_size
        << " for texture " << *texture << "\n";
    }

    VkBufferViewCreateInfo view_info;
    view_info.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
    view_info.pNext = nullptr;
    view_info.flags = 0;
    view_info.buffer = buffer;
    view_info.format = format;
    view_info.offset = 0;
    view_info.range = view_size;

    small_vector<VkBufferView> buffer_views;
    for (int view = 0; view < num_views; ++view) {
      VkBufferView buffer_view;
      VkResult err;
      err = vkCreateBufferView(_device, &view_info, nullptr, &buffer_view);
      if (err) {
        vulkan_error(err, "Failed to create buffer view for texture");
        for (VkBufferView buffer_view : buffer_views) {
          vkDestroyBufferView(_device, buffer_view, nullptr);
        }
        vkDestroyBuffer(_device, buffer, nullptr);
        return false;
      }

      buffer_views.push_back(buffer_view);
      view_info.offset += view_size;
    }

    tc->_format = format;
    tc->_extent = extent;
    tc->_buffer = buffer;
    tc->_buffer_views = std::move(buffer_views);
    tc->_block = std::move(block);
    tc->_pack_bgr8 = pack_bgr8;
    tc->_swap_bgra8 = swap_bgra8;
    tc->_supports_render_to_texture = render_to_texture;
  }

  // We can't upload it at this point because the texture lock is currently
  // held, so accessing the RAM image will cause a deadlock.
  return true;
}

/**
 * Uploads the texture data for the given texture.
 */
bool VulkanGraphicsStateGuardian::
upload_texture(VulkanTextureContext *tc, CompletionToken token) {
  nassertr(_render_cmd, false);
  PStatGPUTimer timer(this, _load_texture_pcollector);

  // Textures can only be updated before the first time they are used in a
  // frame.  This prevents out-of-order calls to transition(), which would
  // otherwise generate invalid barriers, and also prevents invalid descriptor
  // sets (which are also only updated the first time in a frame).
  nassertr(tc->_read_seq < _render_cmd._seq, false);

  Texture *texture = tc->get_texture();
  VkImage image = tc->_image;

  //TODO: check if the image is currently in use on a different queue, and if
  // so, use a semaphore to control order or create a new image and discard
  // the old one.

  VulkanGraphicsPipe *vkpipe;
  DCAST_INTO_R(vkpipe, get_pipe(), false);

  // Do we even have an image to upload?
  if (texture->get_ram_image().is_null() || texture->get_ram_image_size() == 0) {
    if (texture->has_clear_color()) {
      // No, but we have to clear it to a solid color.
      LColor col = texture->get_clear_color();
      if (tc->_buffer != VK_NULL_HANDLE) {
        // Vulkan can only clear buffers using a multiple of 4 bytes.  If it's
        // all zeroes or texels are 4 bytes, it's easy.
        if (col == LColor::zero()) {
          tc->clear_buffer(_transfer_cmd, 0);
          token.complete(true);
          return true;
        }
        vector_uchar data = texture->get_clear_data();
        if (data.size() == sizeof(uint32_t)) {
          tc->clear_buffer(_transfer_cmd, *(uint32_t *)&data[0]);
          token.complete(true);
          return true;
        }
        // Otherwise, make a RAM mipmap image and fall through.
        texture->make_ram_image();
      }
      else if (tc->_aspect_mask == VK_IMAGE_ASPECT_COLOR_BIT) {
        VkClearColorValue value; //TODO: handle integer clears?
        value.float32[0] = col[0];
        value.float32[1] = col[1];
        value.float32[2] = col[2];
        value.float32[3] = col[3];
        tc->clear_color_image(_transfer_cmd, value);
        token.complete(true);
        return true;
      }
      else {
        VkClearDepthStencilValue value;
        value.depth = col[0];
        value.stencil = 0;
        tc->clear_depth_stencil_image(_transfer_cmd, value);
        token.complete(true);
        return true;
      }
    } else {
      // No clear color means we get an uninitialized image.
      token.complete(true);
      return true;
    }
  }

  // Create the staging buffer, where we will write the image to on the CPU.
  // This will then be copied to the device-local memory.
  VkDeviceSize buffer_size = 0;
  VkDeviceSize optimal_align = vkpipe->_gpu_properties.limits.optimalBufferCopyOffsetAlignment;
  nassertd(optimal_align > 0) {
    optimal_align = 1;
  }
  if (optimal_align < 4) {
    //FIXME: texel size
    optimal_align = 4;
  }

  // Add up the total size of the staging buffer to create.
  for (int n = tc->_mipmap_begin; n < tc->_mipmap_end; ++n) {
    if (texture->has_ram_mipmap_image(n)) {
      // Add for optimal alignment.
      VkDeviceSize remain = buffer_size % optimal_align;
      if (remain > 0) {
        buffer_size += optimal_align - remain;
      }

      if (tc->_pack_bgr8) {
        buffer_size += texture->get_ram_mipmap_image_size(n) / 3 * 4;
      } else {
        buffer_size += texture->get_ram_mipmap_image_size(n);
      }
    }
  }
  nassertr(buffer_size > 0, false);

  VkBuffer buffer;
  uint32_t buffer_offset = 0;
  void *data = alloc_staging_buffer(buffer_size + optimal_align - 1, buffer, buffer_offset);
  if (!data) {
    vulkandisplay_cat.error()
      << "Failed to allocate staging buffer for texture "
      << texture->get_name() << std::endl;
    return false;
  }

  if (tc->_image != VK_NULL_HANDLE) {
    // Issue a command to transition the image into a layout optimal for
    // transferring into.
    _transfer_cmd.add_barrier(tc, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                              VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                              VK_ACCESS_2_TRANSFER_WRITE_BIT);

    // Schedule a copy from our staging buffer to the image.
    VkBufferImageCopy region = {};
    region.bufferOffset = buffer_offset;
    region.imageSubresource.aspectMask = tc->_aspect_mask;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.layerCount = tc->_array_layers;
    region.imageExtent = tc->_extent;

    VkImageBlit blit = {};
    blit.srcSubresource = region.imageSubresource;
    blit.srcOffsets[1].x = region.imageExtent.width;
    blit.srcOffsets[1].y = region.imageExtent.height;
    blit.srcOffsets[1].z = region.imageExtent.depth;
    blit.dstSubresource = blit.srcSubresource;

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.pNext = nullptr;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = tc->_aspect_mask;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = tc->_array_layers;

    SparseArray levels_in_dst_optimal_layout;
    levels_in_dst_optimal_layout.set_range(0, tc->_mip_levels);

    for (int n = tc->_mipmap_begin; n < tc->_mipmap_end; ++n) {
      // Get a pointer to the RAM image data.
      const uint8_t *src = (const uint8_t *)texture->get_ram_mipmap_pointer((int)n);
      size_t src_size;
      CPTA_uchar ptimage;
      if (src == nullptr) {
        ptimage = texture->get_ram_mipmap_image(n);
        src = (const uint8_t *)ptimage.p();
        src_size = ptimage.size();
      } else {
        // It's a "pointer texture"; we trust it to be the expected size.
        src_size = texture->get_expected_ram_mipmap_image_size((int)n);
      }

      if (src == nullptr) {
        // There's no image for this level.  Are we supposed to generate it?
        if (n > 0 && tc->_generate_mipmaps) {
          // Transition the previous mipmap level to optimal read layout.
          barrier.subresourceRange.baseMipLevel = blit.srcSubresource.mipLevel;
          vkCmdPipelineBarrier(_transfer_cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
                               VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                               0, nullptr, 0, nullptr, 1, &barrier);
          levels_in_dst_optimal_layout.clear_bit(barrier.subresourceRange.baseMipLevel);

          blit.dstSubresource.mipLevel = region.imageSubresource.mipLevel;
          blit.dstOffsets[1].x = region.imageExtent.width;
          blit.dstOffsets[1].y = region.imageExtent.height;
          blit.dstOffsets[1].z = region.imageExtent.depth;
          vkCmdBlitImage(_transfer_cmd, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                         image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit,
                         VK_FILTER_LINEAR);

          blit.srcSubresource.mipLevel = blit.dstSubresource.mipLevel;
          blit.srcOffsets[1] = blit.dstOffsets[1];

        } else {
          // No, shoot.  Um, panic?
          vulkandisplay_cat.warning() << "No RAM mipmap level " << n
            << " found for texture " << texture->get_name() << "\n";
        }

      } else {
        // We do have an image.  This means we can write it to the appropriate
        // location in the staging buffer, and schedule a copy to the image.
        nassertr(buffer != VK_NULL_HANDLE, false);

        // Pad for optimal alignment.
        VkDeviceSize remain = region.bufferOffset % optimal_align;
        if (remain > 0) {
          region.bufferOffset += optimal_align - remain;
        }

        uint8_t *dest = (uint8_t *)data + (region.bufferOffset - buffer_offset);

        if (tc->_pack_bgr8) {
          // Pack RGB data into RGBA, since most cards don't support RGB8.
          nassertr(((uintptr_t)dest & 0x3) == 0, false);
          const uint8_t *src_end = src + src_size;
          uint32_t *dest32 = (uint32_t *)dest;

          for (; src < src_end; src += 3) {
            *dest32++ = 0xff000000 | (src[0] << 16) | (src[1] << 8) | src[2];
          }
          src_size = src_size / 3 * 4;
        }
        else if (tc->_swap_bgra8) {
          nassertr(((uintptr_t)src & 0x3) == 0, false);
          nassertr(((uintptr_t)dest & 0x3) == 0, false);
          const uint32_t *src32 = (const uint32_t *)src;
          const uint32_t *src32_end = (const uint32_t *)(src + src_size);
          uint32_t *dest32 = (uint32_t *)dest;

          for (; src32 < src32_end; ++src32) {
            uint32_t v = *src32++;
            *dest32++ = (v & 0xff00ff00) | ((v & 0x00ff0000) >> 16) | ((v & 0x000000ff) << 16);
          }
        }
        else {
          memcpy(dest, src, src_size);
        }

        // Schedule a copy from the staging buffer.
        vkCmdCopyBufferToImage(_transfer_cmd, buffer, image,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        region.bufferOffset += src_size;
        _data_transferred_pcollector.add_level(src_size);

        blit.srcSubresource.mipLevel = region.imageSubresource.mipLevel;
        blit.srcOffsets[1].x = region.imageExtent.width;
        blit.srcOffsets[1].y = region.imageExtent.height;
        blit.srcOffsets[1].z = region.imageExtent.depth;
      }

      region.imageExtent.width = std::max(1U, region.imageExtent.width >> 1);
      region.imageExtent.height = std::max(1U, region.imageExtent.height >> 1);
      region.imageExtent.depth = std::max(1U, region.imageExtent.depth >> 1);
      ++region.imageSubresource.mipLevel;
    }

    // Now, ensure that all the mipmap levels are in the same layout.
    if (!levels_in_dst_optimal_layout.has_all_of(0, tc->_mip_levels)) {
      for (size_t ri = 0; ri < levels_in_dst_optimal_layout.get_num_subranges(); ++ri) {
        barrier.subresourceRange.baseMipLevel = levels_in_dst_optimal_layout.get_subrange_begin(ri);
        barrier.subresourceRange.levelCount = levels_in_dst_optimal_layout.get_subrange_end(ri)
                                            - levels_in_dst_optimal_layout.get_subrange_begin(ri);

        vkCmdPipelineBarrier(_transfer_cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0,
                             nullptr, 1, &barrier);
      }
      tc->_layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    }
  }
  else if (tc->_buffer != VK_NULL_HANDLE) {
    // Get a pointer to the RAM image data.
    const uint8_t *src = (const uint8_t *)texture->get_ram_mipmap_pointer(0);
    size_t src_size;
    CPTA_uchar ptimage;
    if (src == nullptr) {
      ptimage = texture->get_ram_mipmap_image(0);
      src = (const uint8_t *)ptimage.p();
      src_size = ptimage.size();
    } else {
      // It's a "pointer texture"; we trust it to be the expected size.
      src_size = texture->get_expected_ram_mipmap_image_size(0);
    }

    if (tc->_pack_bgr8) {
      // Pack RGB data into RGBA, since most cards don't support RGB8.
      nassertr(((uintptr_t)data & 0x3) == 0, false);
      const uint8_t *src_end = src + src_size;
      uint32_t *dest32 = (uint32_t *)data;

      for (; src < src_end; src += 3) {
        *dest32++ = 0xff000000 | (src[0] << 16) | (src[1] << 8) | src[2];
      }
      src_size = src_size / 3 * 4;
    }
    else if (tc->_swap_bgra8) {
      nassertr(((uintptr_t)src & 0x3) == 0, false);
      nassertr(((uintptr_t)data & 0x3) == 0, false);
      const uint32_t *src32 = (const uint32_t *)src;
      const uint32_t *src32_end = (const uint32_t *)(src + src_size);
      uint32_t *dest32 = (uint32_t *)data;

      for (; src32 < src32_end; ++src32) {
        uint32_t v = *src32++;
        *dest32++ = (v & 0xff00ff00) | ((v & 0x00ff0000) >> 16) | ((v & 0x000000ff) << 16);
      }
    }
    else {
      memcpy(data, src, src_size);
    }

    VkBufferCopy region;
    region.srcOffset = buffer_offset;
    region.dstOffset = 0;
    region.size = src_size;
    vkCmdCopyBuffer(_transfer_cmd, buffer, tc->_buffer, 1, &region);
  }

  tc->mark_loaded();

  // Tell the GraphicsEngine that we uploaded the texture.  This may cause
  // it to unload the data from RAM at the end of this frame.
  GraphicsEngine *engine = get_engine();
  if (engine != nullptr) {
    engine->texture_uploaded(texture);
  }

  token.complete(true);
  return true;
}

/**
 * Ensures that the current Texture data is refreshed onto the GSG.  This
 * means updating the texture properties and/or re-uploading the texture
 * image, if necessary.  This should only be called within the draw thread.
 *
 * If force is true, this function will not return until the texture has been
 * fully uploaded.  If force is false, the function may choose to upload a
 * simple version of the texture instead, if the texture is not fully resident
 * (and if get_incomplete_render() is true).
 */
bool VulkanGraphicsStateGuardian::
update_texture(TextureContext *tc, bool force, CompletionToken token) {
  VulkanTextureContext *vtc;
  DCAST_INTO_R(vtc, tc, false);

  // Note that this path is only hit through PreparedGraphicsObjects, all
  // internal uses should go through use_texture().

  if (vtc->was_modified()) {
    if (vtc->needs_recreation()) {
      // We need to recreate the image entirely.
      vtc->release(get_frame_data());
      if (!create_texture(vtc)) {
        return false;
      }
    }

    if (!upload_texture(vtc, std::move(token))) {
      return false;
    }

    vtc->mark_loaded();
  }
  else {
    token.complete(true);
  }

  vtc->enqueue_lru(&_prepared_objects->_graphics_memory_lru);
  return true;
}

/**
 * Frees the resources previously allocated via a call to prepare_texture(),
 * including deleting the TextureContext itself, if it is non-NULL.
 */
void VulkanGraphicsStateGuardian::
release_texture(TextureContext *tc) {
  VulkanTextureContext *vtc;
  DCAST_INTO_V(vtc, tc);

  vtc->release(get_frame_data());
  delete vtc;
}

/**
 * This method should only be called by the GraphicsEngine.  Do not call it
 * directly; call GraphicsEngine::extract_texture_data() instead.
 *
 * Please note that this may be a very expensive operation as it stalls the
 * graphics pipeline while waiting for the rendered results to become
 * available.  The graphics implementation may choose to defer writing the ram
 * image until the next end_frame() call.
 *
 * This method will be called in the draw thread between begin_frame() and
 * end_frame() to download the texture memory's image into its ram_image
 * value.  It may not be called between begin_scene() and end_scene().
 *
 * @return true on success, false otherwise
 */
bool VulkanGraphicsStateGuardian::
extract_texture_data(Texture *tex) {
  nassertr(_render_cmd, false);

  VulkanTextureContext *tc;
  DCAST_INTO_R(tc, tex->prepare_now(get_prepared_objects(), this), false);

  bool success = true;

  // If we wanted to optimize this use-case, we could allocate a single buffer
  // to hold all texture views and copy that in one go.
  int num_views = tex->get_num_views();
  for (int view = 0; view < num_views; ++view) {
    if (!do_extract_image(tc, tex, view)) {
      success = false;
    }
  }

  return success;
}

/**
 * Creates whatever structures the GSG requires to represent the sampler
 * internally, and returns a newly-allocated SamplerContext object with this
 * data.  It is the responsibility of the calling function to later call
 * release_sampler() with this same pointer (which will also delete the
 * pointer).
 *
 * This function should not be called directly to prepare a sampler.  Instead,
 * call Texture::prepare().
 */
SamplerContext *VulkanGraphicsStateGuardian::
prepare_sampler(const SamplerState &sampler) {
  PStatTimer timer(_prepare_sampler_pcollector);

  SamplerState::FilterType magfilter = sampler.get_effective_magfilter();
  SamplerState::FilterType minfilter = sampler.get_effective_minfilter();

  VkSamplerAddressMode wrap_map[] = {VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                                     VK_SAMPLER_ADDRESS_MODE_REPEAT,
                                     VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
                                     VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE,
                                     VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
                                     VK_SAMPLER_ADDRESS_MODE_REPEAT};
  VkSamplerCreateInfo sampler_info;
  sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  sampler_info.pNext = nullptr;
  sampler_info.flags = 0;
  if (minfilter == SamplerState::FT_shadow || magfilter == SamplerState::FT_shadow) {
    sampler_info.compareEnable = VK_TRUE;
    sampler_info.compareOp = VK_COMPARE_OP_LESS;
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;
  } else {
    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = VK_COMPARE_OP_NEVER;
    sampler_info.magFilter = (VkFilter)(magfilter & 1);
    sampler_info.minFilter = (VkFilter)(minfilter & 1);
  }
  sampler_info.mipmapMode = (VkSamplerMipmapMode)
    (minfilter == SamplerState::FT_nearest_mipmap_linear ||
     minfilter == SamplerState::FT_linear_mipmap_linear);
  sampler_info.addressModeU = wrap_map[sampler.get_wrap_u()];
  sampler_info.addressModeV = wrap_map[sampler.get_wrap_v()];
  sampler_info.addressModeW = wrap_map[sampler.get_wrap_w()];
  sampler_info.mipLodBias = sampler.get_lod_bias();
  sampler_info.anisotropyEnable = (sampler.get_effective_anisotropic_degree() > 1);
  sampler_info.maxAnisotropy = sampler.get_effective_anisotropic_degree();
  sampler_info.minLod = sampler.get_min_lod();
  sampler_info.maxLod = sampler.get_max_lod();
  sampler_info.unnormalizedCoordinates = VK_FALSE;

  LColor border_color = sampler.get_border_color();
  VkSamplerCustomBorderColorCreateInfoEXT custom_border_color;
  if (border_color.almost_equal(LColor(0.0, 0.0, 0.0, 0.0))) {
    sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
  }
  else if (border_color.almost_equal(LColor(0.0, 0.0, 0.0, 1.0))) {
    sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
  }
  else if (border_color.almost_equal(LColor(1.0, 1.0, 1.0, 1.0))) {
    sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
  }
  else if (_supports_custom_border_colors) {
    sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_CUSTOM_EXT;

    custom_border_color.sType = VK_STRUCTURE_TYPE_SAMPLER_CUSTOM_BORDER_COLOR_CREATE_INFO_EXT;
    custom_border_color.pNext = nullptr;
    custom_border_color.customBorderColor.float32[0] = border_color[0];
    custom_border_color.customBorderColor.float32[1] = border_color[1];
    custom_border_color.customBorderColor.float32[2] = border_color[2];
    custom_border_color.customBorderColor.float32[3] = border_color[3];
    custom_border_color.format = VK_FORMAT_UNDEFINED;

    sampler_info.pNext = &custom_border_color;
  }
  else if (border_color[3] >= 0.5f) {
    sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
  }
  else {
    sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
  }

  VkResult err;
  VkSampler vk_sampler;
  err = vkCreateSampler(_device, &sampler_info, nullptr, &vk_sampler);
  if (err) {
    return nullptr;
  }

  return new VulkanSamplerContext(sampler, vk_sampler);
}

/**
 * Frees the resources previously allocated via a call to prepare_sampler(),
 * including deleting the SamplerContext itself, if it is non-NULL.
 */
void VulkanGraphicsStateGuardian::
release_sampler(SamplerContext *sc) {
  VulkanSamplerContext *vsc;
  DCAST_INTO_V(vsc, sc);

  nassertv(vsc->_sampler != VK_NULL_HANDLE);
  get_frame_data()._pending_destroy_samplers.push_back(vsc->_sampler);
  delete vsc;
}

/**
 * Prepares the indicated Geom for retained-mode rendering, by creating
 * whatever structures are necessary in the GSG (for instance, vertex
 * buffers). Returns the newly-allocated GeomContext that can be used to
 * render the geom.
 */
GeomContext *VulkanGraphicsStateGuardian::
prepare_geom(Geom *) {
  return nullptr;
}

/**
 * Frees the resources previously allocated via a call to prepare_geom(),
 * including deleting the GeomContext itself, if it is non-NULL.
 *
 * This function should not be called directly to prepare a Geom.  Instead,
 * call Geom::prepare().
 */
void VulkanGraphicsStateGuardian::
release_geom(GeomContext *) {
}

/**
 * Compile a vertex/fragment shader body.
 */
ShaderContext *VulkanGraphicsStateGuardian::
prepare_shader(Shader *shader) {
  PStatTimer timer(_prepare_shader_pcollector);

  VulkanShaderContext *sc = new VulkanShaderContext(shader);

  if (!sc->create_modules(_device, _push_constant_block_type)) {
    delete sc;
    return nullptr;
  }

  // Create the pipeline layout.  We use a predetermined number of sets, with
  // specific sets corresponding to different render attributes.
  VkDescriptorSetLayout ds_layouts[DS_SET_COUNT] = {};
  ds_layouts[DS_light_attrib] = _lattr_descriptor_set_layout;
  ds_layouts[DS_texture_attrib] = sc->make_texture_attrib_descriptor_set_layout(_device);
  ds_layouts[DS_shader_attrib] = sc->make_shader_attrib_descriptor_set_layout(_device);
  ds_layouts[DS_dynamic_uniforms] = sc->make_dynamic_uniform_descriptor_set_layout(_device);

  VkResult err;

  VkPipelineLayoutCreateInfo layout_info;
  layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  layout_info.pNext = nullptr;
  layout_info.flags = 0;
  layout_info.setLayoutCount = DS_SET_COUNT;
  layout_info.pSetLayouts = ds_layouts;
  layout_info.pushConstantRangeCount = 0;
  layout_info.pPushConstantRanges = nullptr;

  // Define the push constant range.  Because we have declared the same block
  // in all stages that use any push constant, we only need to define one range.
  // I'm happy with that, because I've gone down the route of trying to sort out
  // the ranges exactly in the way that Vulkan wants it, and it's not fun.
  VkPushConstantRange range;
  if (sc->_push_constant_stage_mask != 0) {
    range.stageFlags = sc->_push_constant_stage_mask;
    range.offset = 0;
    range.size = _push_constant_block_type->get_size_bytes();

    layout_info.pushConstantRangeCount = 1;
    layout_info.pPushConstantRanges = &range;
  }

  err = vkCreatePipelineLayout(_device, &layout_info, nullptr, &sc->_pipeline_layout);
  if (err) {
    vulkan_error(err, "Failed to create pipeline layout");
    delete sc;
    return nullptr;
  }

  if (ds_layouts[DS_dynamic_uniforms] != VK_NULL_HANDLE) {
    if (!update_dynamic_uniform_descriptor_set(sc)) {
      delete sc;
      return nullptr;
    }
  }

  return sc;
}

/**
 * Releases the resources allocated by prepare_shader
 */
void VulkanGraphicsStateGuardian::
release_shader(ShaderContext *context) {
  VulkanShaderContext *sc;
  DCAST_INTO_V(sc, context);

  // According to the Vulkan spec, it is safe to delete a shader module even
  // if pipelines using it are still in use, so let's do it now.
  sc->destroy_modules(_device);

  // Destroy the pipeline states that use these modules.
  //TODO: is this safe?
  for (const auto &item : sc->_pipeline_map) {
    vkDestroyPipeline(_device, item.second, nullptr);
  }
  sc->_pipeline_map.clear();

  if (sc->_compute_pipeline != VK_NULL_HANDLE) {
    vkDestroyPipeline(_device, sc->_compute_pipeline, nullptr);
    sc->_compute_pipeline = VK_NULL_HANDLE;
  }

  vkDestroyDescriptorSetLayout(_device, sc->_tattr_descriptor_set_layout, nullptr);
  vkDestroyDescriptorSetLayout(_device, sc->_sattr_descriptor_set_layout, nullptr);
  vkDestroyDescriptorSetLayout(_device, sc->_dynamic_uniform_descriptor_set_layout, nullptr);
  vkDestroyPipelineLayout(_device, sc->_pipeline_layout, nullptr);

  delete sc;
}

/**
 * Prepares the indicated buffer for retained-mode rendering.
 */
VertexBufferContext *VulkanGraphicsStateGuardian::
prepare_vertex_buffer(GeomVertexArrayData *array_data) {
  PStatTimer timer(_prepare_vertex_buffer_pcollector);

  VulkanGraphicsPipe *vkpipe;
  DCAST_INTO_R(vkpipe, get_pipe(), nullptr);

  CPT(GeomVertexArrayDataHandle) handle = array_data->get_handle();
  VkDeviceSize data_size = handle->get_data_size_bytes();

  VkBuffer buffer;
  VulkanMemoryBlock block;
  if (!create_buffer(data_size, buffer, block,
                     VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
    vulkandisplay_cat.error()
      << "Failed to create vertex buffer.\n";
    return nullptr;
  }

  VulkanVertexBufferContext *vbc = new VulkanVertexBufferContext(_prepared_objects, array_data);
  vbc->_buffer = buffer;
  vbc->_block = std::move(block);
  vbc->update_data_size_bytes(data_size);

  update_vertex_buffer(vbc, handle, false);

  return vbc;
}

/**
 * Makes sure that the data in the vertex buffer is up-to-date.
 */
bool VulkanGraphicsStateGuardian::
update_vertex_buffer(VulkanVertexBufferContext *vbc,
                     const GeomVertexArrayDataHandle *reader,
                     bool force) {

  if (vbc->was_modified(reader)) {
    PStatGPUTimer timer(this, _load_vertex_buffer_pcollector);

    VkDeviceSize num_bytes = reader->get_data_size_bytes();
    if (num_bytes != 0) {
      const unsigned char *client_pointer = reader->get_read_pointer(force);
      if (client_pointer == nullptr) {
        return false;
      }

      bool use_staging_buffer = true;
      if (vbc->_last_use_frame > _last_finished_frame) {
        // Still in use, so insert an execution dependency.
        // Surely there should be a more optimal way to do this...
        VkBufferMemoryBarrier barrier;
        barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        barrier.pNext = nullptr;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.buffer = vbc->_buffer;
        barrier.offset = 0;
        barrier.size = VK_WHOLE_SIZE;
        vkCmdPipelineBarrier(_transfer_cmd,
                             VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             0, 0, nullptr, 1, &barrier, 0, nullptr);
      }
      else if (_has_unified_memory) {
        // If we have UMA, and the buffer is not in use, we can skip the
        // staging buffer and write directly to buffer memory.
        use_staging_buffer = false;
      }

      if (use_staging_buffer) {
        VkBuffer buffer;
        uint32_t buffer_offset;
        void *data = alloc_staging_buffer(num_bytes, buffer, buffer_offset);
        if (!data) {
          vulkandisplay_cat.error()
            << "Failed to allocate staging buffer for updating vertex buffer.\n";
          return false;
        }
        memcpy(data, client_pointer, num_bytes);

        VkBufferCopy region;
        region.srcOffset = buffer_offset;
        region.dstOffset = 0;
        region.size = num_bytes;
        vkCmdCopyBuffer(_transfer_cmd, buffer, vbc->_buffer, 1, &region);
      } else {
        VulkanMemoryMapping mapping = vbc->_block.map();
        memcpy(mapping, client_pointer, num_bytes);
        vbc->_block.flush();
      }
      _data_transferred_pcollector.add_level(num_bytes);

      VkBufferMemoryBarrier barrier;
      barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
      barrier.pNext = nullptr;
      barrier.srcAccessMask = use_staging_buffer ? VK_ACCESS_TRANSFER_WRITE_BIT : VK_ACCESS_HOST_WRITE_BIT;
      barrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
      barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      barrier.buffer = vbc->_buffer;
      barrier.offset = 0;
      barrier.size = VK_WHOLE_SIZE;
      vkCmdPipelineBarrier(_transfer_cmd,
                           use_staging_buffer ? VK_PIPELINE_STAGE_TRANSFER_BIT : VK_PIPELINE_STAGE_HOST_BIT,
                           VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                           0, 0, nullptr, 1, &barrier, 0, nullptr);
    }

    vbc->mark_loaded(reader);
  }
  vbc->mark_used_this_frame(get_frame_data());
  vbc->enqueue_lru(&_prepared_objects->_graphics_memory_lru);

  return true;
}

/**
 * Frees the resources previously allocated via a call to prepare_data(),
 * including deleting the VertexBufferContext itself, if necessary.
 */
void VulkanGraphicsStateGuardian::
release_vertex_buffer(VertexBufferContext *context) {
  VulkanVertexBufferContext *vbc;
  DCAST_INTO_V(vbc, context);

  VulkanFrameData &frame_data = get_frame_data();
  frame_data._pending_destroy_buffers.push_back(vbc->_buffer);
  frame_data._pending_free.push_back(std::move(vbc->_block));
  delete vbc;
}

/**
 * Prepares the indicated buffer for retained-mode rendering.
 */
IndexBufferContext *VulkanGraphicsStateGuardian::
prepare_index_buffer(GeomPrimitive *primitive) {
  PStatTimer timer(_prepare_index_buffer_pcollector);

  VulkanGraphicsPipe *vkpipe;
  DCAST_INTO_R(vkpipe, get_pipe(), nullptr);

  CPT(GeomVertexArrayDataHandle) handle = primitive->get_vertices()->get_handle();
  VkDeviceSize data_size = handle->get_data_size_bytes();

  GeomEnums::NumericType index_type = primitive->get_index_type();
  if (index_type == GeomEnums::NT_uint8) {
    // We widen 8-bits indices to 16-bits.
    data_size *= 2;
  }
  else if (index_type != GeomEnums::NT_uint16 &&
           index_type != GeomEnums::NT_uint32) {
    vulkandisplay_cat.error()
      << "Unsupported index type: " << index_type;
    return nullptr;
  }

  VkBuffer buffer;
  VulkanMemoryBlock block;
  if (!create_buffer(data_size, buffer, block,
                     VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
    vulkandisplay_cat.error()
      << "Failed to create index buffer.\n";
    return nullptr;
  }

  VulkanIndexBufferContext *ibc = new VulkanIndexBufferContext(_prepared_objects, primitive);
  ibc->_buffer = buffer;
  ibc->_block = std::move(block);
  ibc->update_data_size_bytes(data_size);

  if (index_type == GeomEnums::NT_uint32) {
    ibc->_index_type = VK_INDEX_TYPE_UINT32;
  } else {
    // NT_uint8 is automatically promoted to uint16 below.
    ibc->_index_type = VK_INDEX_TYPE_UINT16;
  }

  GeomPrimitivePipelineReader reader(primitive, Thread::get_current_thread());
  update_index_buffer(ibc, &reader, false);

  return ibc;
}

/**
 * Makes sure that the data in the index buffer is up-to-date.
 */
bool VulkanGraphicsStateGuardian::
update_index_buffer(VulkanIndexBufferContext *ibc,
                    const GeomPrimitivePipelineReader *reader,
                    bool force) {

  if (ibc->was_modified(reader)) {
    PStatGPUTimer timer(this, _load_index_buffer_pcollector);

    VkDeviceSize num_bytes = reader->get_data_size_bytes();
    if (num_bytes != 0) {
      const unsigned char *client_pointer = reader->get_read_pointer(force);
      if (client_pointer == nullptr) {
        return false;
      }

      GeomEnums::NumericType index_type = reader->get_index_type();
      if (index_type == GeomEnums::NT_uint8) {
        // We widen 8-bits indices to 16-bits.
        num_bytes *= 2;
      }
      else if (index_type != GeomEnums::NT_uint16
            && index_type != GeomEnums::NT_uint32) {
        vulkandisplay_cat.error()
          << "Unsupported index type: " << index_type;
        return false;
      }

      bool use_staging_buffer = true;
      if (ibc->_last_use_frame > _last_finished_frame) {
        // Still in use, so insert an execution dependency.
        // Surely there should be a more optimal way to do this...
        VkBufferMemoryBarrier barrier;
        barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        barrier.pNext = nullptr;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.buffer = ibc->_buffer;
        barrier.offset = 0;
        barrier.size = VK_WHOLE_SIZE;
        vkCmdPipelineBarrier(_transfer_cmd,
                             VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             0, 0, nullptr, 1, &barrier, 0, nullptr);
      }
      else if (_has_unified_memory) {
        // If we have UMA, and the buffer is not in use, we can skip the
        // staging buffer and write directly to buffer memory.
        use_staging_buffer = false;
      }

      if (use_staging_buffer) {
        VkBuffer buffer;
        uint32_t buffer_offset;
        void *data = alloc_staging_buffer(num_bytes, buffer, buffer_offset);
        if (!data) {
          vulkandisplay_cat.error()
            << "Failed to allocate staging buffer for updating index buffer.\n";
          return false;
        }

        if (index_type == GeomEnums::NT_uint8) {
          // Widen to 16-bits, as Vulkan doesn't support 8-bits indices.
          uint16_t *ptr = (uint16_t *)data;
          for (size_t i = 0; i < num_bytes; i += 2) {
            *ptr++ = (uint16_t)*client_pointer++;
          }
        } else {
          memcpy(data, client_pointer, num_bytes);
        }

        VkBufferCopy region;
        region.srcOffset = buffer_offset;
        region.dstOffset = 0;
        region.size = num_bytes;
        vkCmdCopyBuffer(_transfer_cmd, buffer, ibc->_buffer, 1, &region);
      } else {
        VulkanMemoryMapping mapping = ibc->_block.map();
        if (index_type == GeomEnums::NT_uint8) {
          uint16_t *ptr = (uint16_t *)mapping;
          for (size_t i = 0; i < num_bytes; i += 2) {
            *ptr++ = (uint16_t)*client_pointer++;
          }
        } else {
          memcpy(mapping, client_pointer, num_bytes);
        }
        ibc->_block.flush();
      }
      _data_transferred_pcollector.add_level(num_bytes);

      VkBufferMemoryBarrier barrier;
      barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
      barrier.pNext = nullptr;
      barrier.srcAccessMask = use_staging_buffer ? VK_ACCESS_TRANSFER_WRITE_BIT : VK_ACCESS_HOST_WRITE_BIT;
      barrier.dstAccessMask = VK_ACCESS_INDEX_READ_BIT;
      barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      barrier.buffer = ibc->_buffer;
      barrier.offset = 0;
      barrier.size = VK_WHOLE_SIZE;
      vkCmdPipelineBarrier(_transfer_cmd,
                           use_staging_buffer ? VK_PIPELINE_STAGE_TRANSFER_BIT : VK_PIPELINE_STAGE_HOST_BIT,
                           VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                           0, 0, nullptr, 1, &barrier, 0, nullptr);
    }

    ibc->mark_loaded(reader);
  }
  ibc->mark_used_this_frame(get_frame_data());
  ibc->enqueue_lru(&_prepared_objects->_graphics_memory_lru);

  return true;
}

/**
 * Frees the resources previously allocated via a call to prepare_data(),
 * including deleting the IndexBufferContext itself, if necessary.
 */
void VulkanGraphicsStateGuardian::
release_index_buffer(IndexBufferContext *context) {
  nassertv(_frame_data != nullptr);

  VulkanIndexBufferContext *vbc;
  DCAST_INTO_V(vbc, context);

  _frame_data->_pending_destroy_buffers.push_back(vbc->_buffer);
  _frame_data->_pending_free.push_back(std::move(vbc->_block));
  delete vbc;
}

/**
 * Prepares the buffer for the given usage of the buffer.
 */
VulkanBufferContext *VulkanGraphicsStateGuardian::
use_shader_buffer(ShaderBuffer *buffer, VkPipelineStageFlags2 stage_mask,
                  VkAccessFlags2 access_mask) {
  nassertr(_render_cmd, nullptr);

  VulkanBufferContext *bc;
  DCAST_INTO_R(bc, buffer->prepare_now(_prepared_objects, this), nullptr);

  _render_cmd.add_barrier(bc, stage_mask, access_mask);
  return bc;
}

/**
 * Creates a new retained-mode representation of the given data, and returns a
 * newly-allocated BufferContext pointer to reference it.  It is the
 * responsibility of the calling function to later call release_shader_buffer()
 * with this same pointer (which will also delete the pointer).
 *
 * This function should not be called directly to prepare a buffer.  Instead,
 * call ShaderBuffer::prepare().
 */
BufferContext *VulkanGraphicsStateGuardian::
prepare_shader_buffer(ShaderBuffer *data) {
  PStatTimer timer(_prepare_shader_buffer_pcollector);

  VkDeviceSize data_size = data->get_data_size_bytes();

  bool use_staging_buffer = !_has_unified_memory;
  VkMemoryPropertyFlagBits mem_flags = (VkMemoryPropertyFlagBits)0;
  if (data->get_usage_hint() == GeomEnums::UH_client) {
    use_staging_buffer = false;
  } else {
    mem_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
  }

  int usage_flags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  if (use_staging_buffer) {
    usage_flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  } else {
    mem_flags = (VkMemoryPropertyFlagBits)(mem_flags | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
  }

  VkBuffer buffer;
  VulkanMemoryBlock block;
  if (!create_buffer(data_size, buffer, block, usage_flags, mem_flags)) {
    vulkandisplay_cat.error()
      << "Failed to create shader buffer.\n";
    return nullptr;
  }

  VulkanBufferContext *bc = new VulkanBufferContext(_prepared_objects, data);
  bc->_buffer = buffer;
  bc->_block = std::move(block);
  bc->_host_visible = !use_staging_buffer;
  bc->update_data_size_bytes(data_size);

  const unsigned char *initial_data = data->get_initial_data();
  if (initial_data != nullptr) {
    if (!_has_unified_memory) {
      VkBuffer buffer;
      uint32_t buffer_offset;
      void *data = alloc_staging_buffer(data_size, buffer, buffer_offset);
      if (!data) {
        vulkandisplay_cat.error()
          << "Failed to allocate staging buffer for updating shader buffer.\n";
        return nullptr;
      }
      memcpy(data, initial_data, data_size);

      VkBufferCopy region;
      region.srcOffset = buffer_offset;
      region.dstOffset = 0;
      region.size = data_size;
      vkCmdCopyBuffer(_transfer_cmd, buffer, bc->_buffer, 1, &region);
    }
    else {
      VulkanMemoryMapping mapping = bc->_block.map();
      memcpy(mapping, initial_data, data_size);
      bc->_block.flush();
    }
    _data_transferred_pcollector.add_level(data_size);

    VkBufferMemoryBarrier2 barrier;
    barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    barrier.pNext = nullptr;
    barrier.srcStageMask = use_staging_buffer ? VK_PIPELINE_STAGE_2_TRANSFER_BIT : VK_PIPELINE_STAGE_2_HOST_BIT;
    barrier.srcAccessMask = use_staging_buffer ? VK_ACCESS_2_TRANSFER_WRITE_BIT : VK_ACCESS_2_HOST_WRITE_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT
                         | VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT
                         | VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT
                         | VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT
                         | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT
                         | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.buffer = bc->_buffer;
    barrier.offset = 0;
    barrier.size = VK_WHOLE_SIZE;

    VkDependencyInfo info = {
      VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      nullptr, 0, // pNext, dependencyFlags
      0, nullptr, // memory barriers
      1, &barrier, // buffer barriers
      0, nullptr, // image barriers
    };
    vkCmdPipelineBarrier2(_transfer_cmd, &info);
  }

  //bc->enqueue_lru(&_prepared_objects->_graphics_memory_lru);

  return bc;
}

/**
 * Frees the GL resources previously allocated for the data.  This function
 * should never be called directly; instead, call Data::release() (or simply
 * let the Data destruct).
 */
void VulkanGraphicsStateGuardian::
release_shader_buffer(BufferContext *context) {
  nassertv(_frame_data != nullptr);

  VulkanBufferContext *bc;
  DCAST_INTO_V(bc, context);

  _frame_data->_pending_destroy_buffers.push_back(bc->_buffer);
  _frame_data->_pending_free.push_back(std::move(bc->_block));
  delete bc;
}

/**
 * This method should only be called by the GraphicsEngine.  Do not call it
 * directly; call GraphicsEngine::extract_shader_buffer_data() instead.
 *
 * This method will be called in the draw thread between begin_frame() and
 * end_frame() to download the shader buffer data into the given vector.
 * It may not be called between begin_scene() and end_scene().
 */
bool VulkanGraphicsStateGuardian::
extract_shader_buffer_data(ShaderBuffer *buffer, vector_uchar &data) {
  nassertr(!_render_cmd, false);

  VulkanBufferContext *bc;
  DCAST_INTO_R(bc, buffer->prepare_now(get_prepared_objects(), this), false);
  bc->set_active(true);

  if (!_transfer_cmd) {
    _transfer_cmd = begin_command_buffer();
    nassertr_always(_transfer_cmd, false);
  }

  return do_extract_buffer(bc, data);
}

/**
 * Adds a timer query to the command stream, associated with the given PStats
 * collector index.
 */
void VulkanGraphicsStateGuardian::
issue_timer_query(int pstats_index) {
  uint32_t query = get_next_timer_query(pstats_index);

  bool is_end = pstats_index & 0x8000;
  _vkCmdWriteTimestamp2(_render_cmd, is_end ? VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT : VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, _timer_query_pool, query);
}

/**
 * Returns the next available timer query.
 */
uint32_t VulkanGraphicsStateGuardian::
get_next_timer_query(int pstats_index) {
  nassertr(_frame_data != nullptr, 0);

  uint32_t new_head = (_timer_query_head + 1) & _timer_query_pool_size;
  if (UNLIKELY(new_head == _timer_query_tail)) {
    replace_timer_query_pool();
    new_head = 1;
  }

  _frame_data->_timer_query_pool._pstats_indices.push_back(pstats_index);
  return std::exchange(_timer_query_head, new_head);
}

/**
 * Creates a new timer query pool, storing it in _timer_query_pool.  Used when
 * space runs out in the current pool.
 * Must be called with transfer command buffer begun.
 */
void VulkanGraphicsStateGuardian::
replace_timer_query_pool() {
  VkQueryPoolCreateInfo info;
  info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
  info.pNext = nullptr;
  info.flags = 0;
  info.queryType = VK_QUERY_TYPE_TIMESTAMP;
  info.queryCount = (_timer_query_pool_size + 1) << 1;
  info.pipelineStatistics = 0;

  VkResult err = vkCreateQueryPool(_device, &info, nullptr, &_timer_query_pool);
  if (err != VK_SUCCESS) {
    vulkan_error(err, "Failed to create timestamp query pool");
    return;
  }

  _timer_query_pool_size = info.queryCount - 1;
  _timer_query_head = 0;
  _timer_query_tail = 0;

  vkResetQueryPool(_device, _timer_query_pool, 0, info.queryCount);

  _frame_data->replace_timer_query_pool(_timer_query_pool, _timer_query_pool_size);
}

/**
 * Dispatches a currently bound compute shader using the given work group
 * counts.
 */
void VulkanGraphicsStateGuardian::
dispatch_compute(int num_groups_x, int num_groups_y, int num_groups_z) {
  nassertv(_render_cmd);
  nassertv(_current_shader != nullptr);

#ifdef DO_PSTATS
  _compute_work_groups_pcollector.add_level(num_groups_x * num_groups_y * num_groups_z);
  PStatGPUTimer timer(this, _current_sc->_compute_dispatch_pcollector);
#endif

  //TODO: must actually be outside render pass, and on a queue that supports
  // compute.  Should we have separate pool/queue/buffer for compute?
  VkPipeline pipeline = _current_sc->get_compute_pipeline(this);
  nassertv(pipeline != VK_NULL_HANDLE);
  _vkCmdBindPipeline(_render_cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);

  vkCmdDispatch(_render_cmd, num_groups_x, num_groups_y, num_groups_z);
}

/**
 * Creates a new GeomMunger object to munge vertices appropriate to this GSG
 * for the indicated state.
 */
PT(GeomMunger) VulkanGraphicsStateGuardian::
make_geom_munger(const RenderState *state, Thread *current_thread) {
  PT(StandardMunger) munger = new StandardMunger(this, state, 1, GeomEnums::NT_packed_dabc, GeomEnums::C_color);
  return GeomMunger::register_munger(munger, current_thread);
}

/**
 * Simultaneously resets the render state and the transform state.
 *
 * This transform specified is the "internal" net transform, already converted
 * into the GSG's internal coordinate space by composing it to
 * get_cs_transform().  (Previously, this used to be the "external" net
 * transform, with the assumption that that GSG would convert it internally,
 * but that is no longer the case.)
 */
void VulkanGraphicsStateGuardian::
set_state_and_transform(const RenderState *state,
                        const TransformState *trans) {
  PStatTimer timer(_draw_set_state_pcollector);

  // This does not actually set the state just yet, because we can't make a
  // pipeline state without knowing the vertex format.
  _target_rs = state;
  _internal_transform = trans;

  determine_target_shader();

  bool shader_changed = false;
  VulkanShaderContext *sc = _current_sc;
  Shader *shader = (Shader *)_target_shader->get_shader();
  if (sc == nullptr || shader != _current_shader) {
    _current_shader = shader;

    if (shader != nullptr) {
      sc = DCAST(VulkanShaderContext, shader->prepare_now(get_prepared_objects(), this));
    } else {
      sc = _default_sc;
    }
    _current_sc = sc;
    shader_changed = true;
  }
  if (UNLIKELY(sc == nullptr)) {
    // Don't bother updating the state if we don't have a valid shader, because
    // begin_draw_primitives will return false anyway.
    return;
  }

  VkCommandBuffer cmd = _render_cmd;

  // Put the modelview projection matrix and color scale in the push constants.
  if (sc->_projection_mat_stage_mask != 0) {
    CPT(TransformState) combined = _projection_mat->compose(trans);
    LMatrix4f matrix = LCAST(float, combined->get_mat());
    _vkCmdPushConstants(cmd, sc->_pipeline_layout,
                        sc->_push_constant_stage_mask, 0, 64, matrix.get_data());
  }

  if (sc->_color_scale_stage_mask != 0) {
    const ColorScaleAttrib *target_color_scale;
    state->get_attrib(target_color_scale);
    if (target_color_scale != _state_rs->get_attrib(ColorScaleAttrib::get_class_slot()) ||
        shader_changed) {

      LColorf color(1.0f, 1.0f, 1.0f, 1.0f);
      if (target_color_scale != nullptr) {
        color = LCAST(float, target_color_scale->get_scale());
      }
      _vkCmdPushConstants(cmd, sc->_pipeline_layout,
                          sc->_push_constant_stage_mask, 64, 16, color.get_data());
    }
  }

  // Update and bind descriptor sets.
  VkDescriptorSet descriptor_sets[DS_SET_COUNT] = {};

  uint32_t first_set = 0;
  const LightAttrib *target_light;
  state->get_attrib(target_light);
  if (shader_changed ||
      target_light != _state_rs->get_attrib(LightAttrib::get_class_slot())) {
    if (!sc->_uses_lattr_descriptors ||
        target_light == nullptr ||
        target_light->get_num_on_lights() == 0) {
      descriptor_sets[DS_light_attrib] = _empty_lattr_descriptor_set;
    }
    else if (get_attrib_descriptor_set(descriptor_sets[DS_light_attrib],
                                      _lattr_descriptor_set_layout,
                                      target_light)) {
      // The first time this set is bound in this frame, we update it.  Once we
      // use it in a command buffer, we can't update it again anyway.
      update_lattr_descriptor_set(descriptor_sets[DS_light_attrib], target_light);
    }
  } else {
    first_set = 1;
  }

  const TextureAttrib *target_texture;
  state->get_attrib_def(target_texture);
  if (first_set != 1 || shader_changed ||
      target_texture != _state_rs->get_attrib_def(TextureAttrib::get_class_slot())) {
    if (get_attrib_descriptor_set(descriptor_sets[DS_texture_attrib],
                                  sc->_tattr_descriptor_set_layout,
                                  target_texture)) {
      sc->update_tattr_descriptor_set(this, descriptor_sets[DS_texture_attrib]);
    }
  } else {
    first_set = 2;
  }

  if (get_attrib_descriptor_set(descriptor_sets[DS_shader_attrib],
                                sc->_sattr_descriptor_set_layout,
                                _target_shader)) {
    sc->update_sattr_descriptor_set(this, descriptor_sets[DS_shader_attrib]);
  }

  const ColorAttrib *target_color;
  state->get_attrib(target_color);
  if (target_color != _state_rs->get_attrib(ColorAttrib::get_class_slot()) ||
      shader_changed) {

    if (sc->_uses_vertex_color && target_color != nullptr &&
        target_color->get_color_type() == ColorAttrib::T_flat) {
      //FIXME: this doesn't need to be aligned to minUniformBufferOffsetAlignment,
      // which can be way more excessive (up to 256 bytes) than needed.
      float *ptr = (float *)alloc_dynamic_uniform_buffer(16, _current_color_buffer, _current_color_offset);
      ptr[0] = target_color->get_color()[0];
      ptr[1] = target_color->get_color()[1];
      ptr[2] = target_color->get_color()[2];
      ptr[3] = target_color->get_color()[3];
    } else {
      _current_color_buffer = _uniform_buffer;
      _current_color_offset = _uniform_buffer_white_offset;
    }
  }

  //TODO: properly compute altered field.
  uint32_t num_offsets = 0;
  uint32_t offset = 0;
  if (sc->_other_state_block._size > 0) {
    offset = sc->update_dynamic_uniforms(this, ~0);
    num_offsets = 1;
  }

  // Note that this set may be recreated by update_dynamic_uniforms, above.
  descriptor_sets[DS_dynamic_uniforms] = sc->_uniform_descriptor_set;

  _state_rs = state;

  PStatTimer timer2(_bind_descriptor_sets_pcollector);
  vkCmdBindDescriptorSets(cmd, sc->_bind_point, sc->_pipeline_layout,
                          first_set, DS_SET_COUNT - first_set, descriptor_sets + first_set,
                          num_offsets, &offset);
}

/**
 * Clears the framebuffer within the current DisplayRegion, according to the
 * flags indicated by the given DrawableRegion object.
 *
 * This does not set the DisplayRegion first.  You should call
 * prepare_display_region() to specify the region you wish the clear operation
 * to apply to.
 */
void VulkanGraphicsStateGuardian::
clear(DrawableRegion *clearable) {
  nassertv(_render_cmd);
  nassertv(clearable->is_any_clear_active());

  VkClearAttachment attachments[2];
  uint32_t ai = 0;

  if (clearable->get_clear_color_active() &&
      _current_properties->get_color_bits() > 0) {
    LColor color = clearable->get_clear_color();
    VkClearAttachment &attachment = attachments[ai++];
    attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    attachment.colorAttachment = 0;
    attachment.clearValue.color.float32[0] = color[0];
    attachment.clearValue.color.float32[1] = color[1];
    attachment.clearValue.color.float32[2] = color[2];
    attachment.clearValue.color.float32[3] = color[3];
  }

  if ((clearable->get_clear_depth_active() ||
       clearable->get_clear_stencil_active()) &&
      (_current_properties->get_depth_bits() > 0 ||
       _current_properties->get_stencil_bits() > 0)) {
    VkClearAttachment &attachment = attachments[ai++];
    attachment.aspectMask = 0;
    if (clearable->get_clear_depth_active() &&
        _current_properties->get_depth_bits() > 0) {
      attachment.aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    if (clearable->get_clear_stencil_active() &&
        _current_properties->get_stencil_bits() > 0) {
      attachment.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
    attachment.colorAttachment = 0; // unused
    attachment.clearValue.depthStencil.depth = clearable->get_clear_depth();
    attachment.clearValue.depthStencil.stencil = clearable->get_clear_stencil();
  }

  // Collect the rectangles we want to clear.
  VkClearRect *rects = (VkClearRect *)alloca(sizeof(VkClearRect) * _viewports.size());
  for (size_t i = 0; i < _viewports.size(); ++i) {
    rects[i].rect = _viewports[i];
    rects[i].baseArrayLayer = 0;
    rects[i].layerCount = 1;
  }

  if (ai > 0) {
    vkCmdClearAttachments(_render_cmd, ai, attachments, _viewports.size(), rects);
  }
}

/**
 * Prepare a display region for rendering (set up scissor region and viewport)
 */
void VulkanGraphicsStateGuardian::
prepare_display_region(DisplayRegionPipelineReader *dr) {
  nassertv(dr != nullptr);
  GraphicsStateGuardian::prepare_display_region(dr);

  int count = dr->get_num_regions();
  VkViewport *viewports = (VkViewport *)alloca(sizeof(VkViewport) * count);
  _viewports.resize(count);

  for (int i = 0; i < count; ++i) {
    int x, y, w, h;
    dr->get_region_pixels(i, x, y, w, h);
    viewports[i].x = x;
    viewports[i].y = y;
    viewports[i].width = w;
    viewports[i].height = h;
    viewports[i].minDepth = 0.0;
    viewports[i].maxDepth = 1.0;

    // Also save this in the _viewports array for later use.
    _viewports[i].offset.x = x;
    _viewports[i].offset.y = y;
    _viewports[i].extent.width = w;
    _viewports[i].extent.height = h;
  }

  vkCmdSetViewport(_render_cmd, 0, count, viewports);
  vkCmdSetScissor(_render_cmd, 0, count, &_viewports[0]);
}


/**
 * Given a lens, calculates the appropriate projection matrix for use with
 * this gsg.  Note that the projection matrix depends a lot upon the
 * coordinate system of the rendering API.
 *
 * The return value is a TransformState if the lens is acceptable, NULL if it
 * is not.
 */
CPT(TransformState) VulkanGraphicsStateGuardian::
calc_projection_mat(const Lens *lens) {
  if (lens == nullptr) {
    return nullptr;
  }

  if (!lens->is_linear()) {
    return nullptr;
  }

  // Vulkan also uses a Z range of 0 to 1, whereas the Panda convention is
  // for the projection matrix to produce a Z range of -1 to 1.  We have to
  // rescale to compensate.
  static const LMatrix4 rescale_mat
    (1, 0, 0, 0,
     0, 1, 0, 0,
     0, 0, 0.5, 0,
     0, 0, 0.5, 1);

  LMatrix4 result = lens->get_projection_mat(_current_stereo_channel) * rescale_mat;

  if (!_scene_setup->get_inverted()) {
    // Vulkan uses an upside down coordinate system.
    result *= LMatrix4::scale_mat(1.0f, -1.0f, 1.0f);
  }

  return TransformState::make_mat(result);
}

/**
 * Makes the current lens (whichever lens was most recently specified with
 * set_scene()) active, so that it will transform future rendered geometry.
 * Normally this is only called from the draw process, and usually it is
 * called by set_scene().
 *
 * The return value is true if the lens is acceptable, false if it is not.
 */
bool VulkanGraphicsStateGuardian::
prepare_lens() {
  return true;
}

/**
 * Called before each frame is rendered, to allow the GSG a chance to do any
 * internal cleanup before beginning the frame.
 *
 * The return value is true if successful (in which case the frame will be
 * drawn and end_frame() will be called later), or false if unsuccessful (in
 * which case nothing will be drawn and end_frame() will not be called).
 */
bool VulkanGraphicsStateGuardian::
begin_frame(Thread *current_thread) {
  return begin_frame(current_thread, VK_NULL_HANDLE);
}

/**
 * Called between begin_frame() and end_frame() to mark the beginning of
 * drawing commands for a "scene" (usually a particular DisplayRegion) within
 * a frame.  All 3-D drawing commands, except the clear operation, must be
 * enclosed within begin_scene() .. end_scene(). This must be called in the
 * draw thread.
 *
 * The return value is true if successful (in which case the scene will be
 * drawn and end_scene() will be called later), or false if unsuccessful (in
 * which case nothing will be drawn and end_scene() will not be called).
 */
bool VulkanGraphicsStateGuardian::
begin_scene() {
  return GraphicsStateGuardian::begin_scene();
}

/**
 * Called between begin_frame() and end_frame() to mark the end of drawing
 * commands for a "scene" (usually a particular DisplayRegion) within a frame.
 * All 3-D drawing commands, except the clear operation, must be enclosed
 * within begin_scene() .. end_scene().
 */
void VulkanGraphicsStateGuardian::
end_scene() {
  GraphicsStateGuardian::end_scene();

  // The end_scene() upcall above clears the _state_rs, forcing us to respecify
  // the state.  We accomplish this by setting the shader to null, which causes
  // set_state_and_transform to respecify all relevant state.
  _current_shader = nullptr;
  _current_sc = nullptr;
}

/**
 * Called after each frame is rendered, to allow the GSG a chance to do any
 * internal cleanup after rendering the frame, and before the window flips.
 */
void VulkanGraphicsStateGuardian::
end_frame(Thread *current_thread) {
  end_frame(current_thread, VK_NULL_HANDLE);
}

/**
 * Version of begin_frame that transfers ownership of the given wait_for
 * semaphore to the frame data object.  Rendering will not commence (though
 * transfers may already take place) until the given semaphore is signalled.
 */
bool VulkanGraphicsStateGuardian::
begin_frame(Thread *current_thread, VkSemaphore wait_for) {
  nassertr_always(!_closing_gsg, false);
  nassertr_always(!_render_cmd, false);

  int clock_frame = ClockObject::get_global_clock()->get_frame_count(current_thread);
  bool is_new_clock_frame = (clock_frame != _current_clock_frame_number);
  if (is_new_clock_frame) {
    // First Vulkan frame in this clock frame.
    _current_clock_frame_number = clock_frame;

    // Make sure any commands from previous frame are submitted.
    flush();
    _frame_data = nullptr;
  }
  if (_frame_data == nullptr) {
    _frame_data = &get_next_frame_data(true);
    _frame_data->_clock_frame_number = clock_frame;
  }

  // Make sure we have a transfer command buffer.
  if (!_transfer_cmd) {
    _transfer_cmd = begin_command_buffer();

    if (!_transfer_cmd) {
      return false;
    }
  }

  if (is_new_clock_frame) {
#ifdef DO_PSTATS
    if (!_timer_queries_active) {
      if (pstats_gpu_timing && _supports_timer_query && PStatClient::is_connected()) {
        _timer_queries_active = true;

        if (_timer_query_pool == VK_NULL_HANDLE) {
          replace_timer_query_pool();
        }
      }
    }

    if (_timer_queries_active) {
      // Issue the first timer query on the transfer command buffer, since that
      // marks the first command we will submit belonging to this frame.
      uint32_t query = get_next_timer_query(0);
      _vkCmdWriteTimestamp2(_transfer_cmd, VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, _timer_query_pool, query);
    }
#endif
  }

  // Make sure we have a white texture.
  if (_white_texture.is_null()) {
    _white_texture = new Texture();
    _white_texture->setup_2d_texture(1, 1, Texture::T_unsigned_byte, Texture::F_rgba8);
    _white_texture->set_clear_color(LColor(1, 1, 1, 1));
    _white_texture->prepare_now(0, get_prepared_objects(), this);
  }

  if (_needs_write_null_vertex_data) {
    _needs_write_null_vertex_data = false;
    vkCmdFillBuffer(_transfer_cmd, _null_vertex_buffer, 0, 4, 0);
  }

  // Add a "white" color to be used for ColorAttribs in this frame, since this
  // color occurs frequently.
  {
    VkBuffer buffer;
    void *ptr = alloc_dynamic_uniform_buffer(16, buffer, _uniform_buffer_white_offset);
    *(LVecBase4f *)ptr = LVecBase4f(1, 1, 1, 1);
  }

  // Call the GSG's begin_frame, which will cause any queued-up release() and
  // prepare() methods to be called.  Note that some of them may add to the
  // transfer command buffer, which is why we've begun it already.
  if (!GraphicsStateGuardian::begin_frame(current_thread)) {
    return false;
  }

  // Now begin the main (ie. graphics) command buffer.
  _render_cmd = begin_command_buffer(wait_for);

  _current_shader = nullptr;
  _current_sc = nullptr;

#ifdef DO_PSTATS
  if (_timer_queries_active && wait_for != VK_NULL_HANDLE) {
    // Measure the gap between the end of the transfer command buffer and
    // the beginning of the render command buffer, which is mostly waiting for
    // the semaphore.  The transfer end query will be created now but written
    // in end_frame().
    _transfer_end_query = get_next_timer_query(_wait_semaphore_pcollector.get_index());
    _transfer_end_query_pool = _timer_query_pool;
    uint32_t query = get_next_timer_query(_wait_semaphore_pcollector.get_index() | 0x8000);
    _vkCmdWriteTimestamp2(_render_cmd, VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, _timer_query_pool, query);
  } else {
    _transfer_end_query_pool = VK_NULL_HANDLE;
  }
#endif

  // Bind the "null" vertex buffer.
  const VkDeviceSize offset = 0;
  _vkCmdBindVertexBuffers(_render_cmd, 0, 1, &_null_vertex_buffer, &offset);
  return true;
}

/**
 * Version of end_frame that signals a given semaphore when it's done.
 */
void VulkanGraphicsStateGuardian::
end_frame(Thread *current_thread, VkSemaphore signal_done) {
  GraphicsStateGuardian::end_frame(current_thread);

#ifdef DO_PSTATS
  if (_transfer_end_query_pool != VK_NULL_HANDLE) {
    _vkCmdWriteTimestamp2(_transfer_cmd, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
                        _transfer_end_query_pool, _transfer_end_query);
    _transfer_end_query_pool = VK_NULL_HANDLE;
  }
#endif

  end_command_buffer(std::move(_transfer_cmd));

  // Note down the current watermark of the ring buffers.
  _frame_data->_uniform_buffer_head = _uniform_buffer_allocator.get_head();
  _frame_data->_staging_buffer_head = _staging_buffer_allocator.get_head();

  // Report how much UBO memory was used.
  size_t used = _uniform_buffer_allocator.get_size();
  if (used > _uniform_buffer_max_used) {
    _uniform_buffer_max_used = used;

    if (vulkandisplay_cat.is_debug()) {
      vulkandisplay_cat.debug()
        << "Used at most " << _uniform_buffer_max_used << " of "
        << _uniform_buffer_allocator.get_capacity()
        << " bytes of global uniform buffer.\n";
    }
  }

  // Issue commands to transition the staging buffers of the texture downloads
  // to make sure that the previous copy operations are visible to host reads.
  if (!_frame_data->_download_queue.empty()) {
    size_t num_downloads = _frame_data->_download_queue.size();
    VkBufferMemoryBarrier *barriers = (VkBufferMemoryBarrier *)
      alloca(sizeof(VkBufferMemoryBarrier) * num_downloads);

    for (size_t i = 0; i < num_downloads; ++i) {
      barriers[i].sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
      barriers[i].pNext = nullptr;
      barriers[i].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      barriers[i].dstAccessMask = VK_ACCESS_HOST_READ_BIT;
      barriers[i].srcQueueFamilyIndex = _graphics_queue_family_index;
      barriers[i].dstQueueFamilyIndex = _graphics_queue_family_index;
      barriers[i].buffer = _frame_data->_download_queue[i]._buffer;
      barriers[i].offset = 0;
      barriers[i].size = VK_WHOLE_SIZE;
    }

    // It's safe to issue a barrier on the render cmd since we're no longer
    // inside an active render pass.
    vkCmdPipelineBarrier(_render_cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0,
                         0, nullptr, (uint32_t)num_downloads, barriers, 0, nullptr);
  }

#ifdef DO_PSTATS
  if (_timer_queries_active) {
    issue_timer_query(0x8000);

    if (_gpu_sync_time == 0) {
      // Get a synchronized timestamp by waiting for the frame.
      _frame_data->_wait_for_finish = true;
    }
  }
#endif

  // We only submit the work now if we know something is waiting on it.
  if (signal_done == VK_NULL_HANDLE && !_frame_data->_wait_for_finish) {
    // The render CB stays open and becomes the new transfer CB.
    _transfer_cmd = std::move(_render_cmd);
    return;
  }

  // Submit the command buffers.
  end_command_buffer(std::move(_render_cmd), signal_done);
  uint64_t watermark = flush();

  // If we queued up synchronous texture downloads, wait for the queue to finish
  // (slow!) and then copy the data from Vulkan host memory to Panda memory.
  if (_frame_data->_wait_for_finish) {
    if (!wait_semaphore(_timeline_semaphore, watermark)) {
      vkQueueWaitIdle(_queue);
    }

#ifdef DO_PSTATS
    if (_timer_queries_active) {
      PStatClient *client = PStatClient::get_global_pstats();
      if (client != nullptr && client->client_is_connected()) {
        _frame_data->_finish_time = client->get_real_time();
      }
    }
#endif

    nassertv(_frame_data_head != _frame_data_capacity);
    do {
      FrameData &frame_data = _frame_data_pool[_frame_data_tail];

      // This frame has completed execution.
      finish_frame(frame_data);

      _frame_data_tail = (_frame_data_tail + 1) % _frame_data_capacity;
    }
    while (_frame_data_tail != _frame_data_head);

    _frame_data_head = _frame_data_capacity;
    _frame_data_tail = 0;
  }

  _last_frame_data = _frame_data;
  _frame_data = nullptr;

  //TODO: delete command buffer, schedule for deletion, or recycle.
}

/**
 * Called after the frame has finished executing to clean up any resources.
 * All frames *must* be finished in order!  It is illegal to call this for a
 * frame when a preceding frame has not finished yet!
 */
void VulkanGraphicsStateGuardian::
finish_frame(FrameData &frame_data) {
#ifdef DO_PSTATS
  PStatClient *client = PStatClient::get_global_pstats();
  double finish_time;
  if (client != nullptr) {
    if (client->client_is_connected()) {
      finish_time = client->get_real_time();
      _finish_frame_pcollector.start(client->get_current_thread(), finish_time);
    } else {
      client = nullptr;
    }
  }
#endif

#ifndef NDEBUG
  if (vulkandisplay_cat.is_spam()) {
    vulkandisplay_cat.spam()
      << "GPU finished CB #" << frame_data._watermark - 1
      << ", cleaning up frame data\n";
  }
#endif

  // Return the used command buffers to the pool.
  _free_command_buffers.insert(
    _free_command_buffers.end(),
    frame_data._pending_command_buffers.begin(),
    frame_data._pending_command_buffers.end());
  frame_data._pending_command_buffers.clear();

  for (VkBufferView buffer_view : frame_data._pending_destroy_buffer_views) {
    vkDestroyBufferView(_device, buffer_view, nullptr);
  }
  frame_data._pending_destroy_buffer_views.clear();

  for (VkBuffer buffer : frame_data._pending_destroy_buffers) {
    vkDestroyBuffer(_device, buffer, nullptr);
  }
  frame_data._pending_destroy_buffers.clear();

  for (VkFramebuffer framebuffer : frame_data._pending_destroy_framebuffers) {
    vkDestroyFramebuffer(_device, framebuffer, nullptr);
  }
  frame_data._pending_destroy_framebuffers.clear();

  for (VkImageView image_view : frame_data._pending_destroy_image_views) {
    vkDestroyImageView(_device, image_view, nullptr);
  }
  frame_data._pending_destroy_image_views.clear();

  for (VkImage image : frame_data._pending_destroy_images) {
    vkDestroyImage(_device, image, nullptr);
  }
  frame_data._pending_destroy_images.clear();

  for (VkRenderPass render_pass : frame_data._pending_destroy_render_passes) {
    vkDestroyRenderPass(_device, render_pass, nullptr);
  }
  frame_data._pending_destroy_render_passes.clear();

  for (VkSampler sampler : frame_data._pending_destroy_samplers) {
    vkDestroySampler(_device, sampler, nullptr);
  }
  frame_data._pending_destroy_samplers.clear();

  for (VkSemaphore semaphore : frame_data._pending_destroy_semaphores) {
    vkDestroySemaphore(_device, semaphore, nullptr);
  }
  frame_data._pending_destroy_semaphores.clear();

  // This will return all the allocated memory blocks to the pool.
  frame_data._pending_free.clear();

  if (!frame_data._pending_free_descriptor_sets.empty()) {
    VkResult err;
    err = vkFreeDescriptorSets(_device, _descriptor_pool,
                               frame_data._pending_free_descriptor_sets.size(),
                               &frame_data._pending_free_descriptor_sets[0]);
    if (err) {
      vulkan_error(err, "Failed to free descriptor sets");
    }
    frame_data._pending_free_descriptor_sets.clear();
  }

  // Make the used uniform / staging buffer space available.
  if (frame_data._uniform_buffer_head != 0) {
    _uniform_buffer_allocator.set_tail(frame_data._uniform_buffer_head);
  }
  if (frame_data._staging_buffer_head != 0) {
    _staging_buffer_allocator.set_tail(frame_data._staging_buffer_head);
  }

  // Process texture-to-RAM downloads.
  frame_data.finish_downloads(_device);

#ifdef DO_PSTATS
  if (client != nullptr && frame_data._timer_query_pool._pool != VK_NULL_HANDLE) {
    if (_pstats_frame_number != frame_data._clock_frame_number) {
      if (!_pstats_frame_data.is_empty()) {
        // Implicitly add an end-of-frame marker.
        _pstats_frame_data.add_stop(0, _pstats_frame_end_time);
        PStatThread gpu_thread = get_pstats_thread();
        gpu_thread.add_frame(_pstats_frame_number, std::move(_pstats_frame_data));
        _pstats_frame_data.clear();
      }

      _pstats_frame_number = frame_data._clock_frame_number;
    }

    if (frame_data._wait_for_finish) {
      // We have an opportunity to synchronize the frame timing.
      // Find the end-of-frame marker.
      for (size_t i = frame_data._timer_query_pool._pstats_indices.size(); i > 0; --i) {
        uint64_t result;
        if (frame_data._timer_query_pool._pstats_indices[i - 1] == 0x8000) {
          vkGetQueryPoolResults(_device, frame_data._timer_query_pool._pool, frame_data._timer_query_pool._offset + i - 1, 1,
                                sizeof(result), &result, sizeof(uint64_t), VK_QUERY_RESULT_64_BIT);
          _gpu_sync_time = result;
          _cpu_sync_time = std::max(frame_data._submit_time, frame_data._finish_time);
          break;
        }
      }
    }

    // Process timer queries.
    small_vector<VulkanFrameData::TimerQueryPool *, 1> rpools;
    size_t data_size = 0;
    auto *pool = &frame_data._timer_query_pool;
    while (pool != nullptr) {
      if (!pool->_pstats_indices.empty()) {
        // We want to iterate over the pools in reverse order.
        rpools.insert(rpools.begin(), pool);
        data_size = std::max(data_size, pool->_pstats_indices.size() * sizeof(uint64_t));
      }
      pool = pool->_prev;
    }

    if (data_size > 0) {
      uint64_t *results = (uint64_t *)alloca(data_size);

      for (auto *pool : rpools) {
        size_t split = (pool->_pool_size + 1) - pool->_offset;
        if (pool->_pstats_indices.size() <= split) {
          vkGetQueryPoolResults(_device, pool->_pool, pool->_offset, pool->_pstats_indices.size(),
                                data_size, results, sizeof(uint64_t), VK_QUERY_RESULT_64_BIT);
        } else {
          vkGetQueryPoolResults(_device, pool->_pool, pool->_offset, split,
                                data_size, results, sizeof(uint64_t), VK_QUERY_RESULT_64_BIT);
          vkGetQueryPoolResults(_device, pool->_pool, 0, pool->_pstats_indices.size() - split,
                                data_size, results + split, sizeof(uint64_t), VK_QUERY_RESULT_64_BIT);
        }

        for (size_t i = 0; i < pool->_pstats_indices.size(); ++i) {
          uint16_t index = pool->_pstats_indices[i];
          double time = (int64_t)(results[i] - _gpu_sync_time) * _timer_query_factor + _cpu_sync_time;
          if (time < frame_data._submit_time) {
            // Can't have executed before submission, shift the timestamp.
            _cpu_sync_time += frame_data._submit_time - time;
            time = frame_data._submit_time;
          }

          if (index == 0x8000) {
            // We don't want multiple of these in a PStats frame, so we add
            // only the last one.
            _pstats_frame_end_time = time;
            continue;
          } else if (index & 0x8000) {
            _pstats_frame_data.add_stop(index & 0x7fff, time);
          } else {
            _pstats_frame_data.add_start(index & 0x7fff, time);
          }
        }

        if (pool != &frame_data._timer_query_pool) {
          // This was an old pool, no longer current.  Throw it away.
          vkDestroyQueryPool(_device, pool->_pool, nullptr);
        } else {
          // This is the current pool.  Reset the used ranges.
          if (pool->_pstats_indices.size() <= split) {
            vkResetQueryPool(_device, pool->_pool, pool->_offset, pool->_pstats_indices.size());
          } else {
            vkResetQueryPool(_device, pool->_pool, pool->_offset, split);
            vkResetQueryPool(_device, pool->_pool, 0, pool->_pstats_indices.size() - split);
          }
          _timer_query_tail = (pool->_offset + pool->_pstats_indices.size()) & _timer_query_pool_size;
        }

        pool->_pstats_indices.clear();
        pool = pool->_prev;
      }
    }

    _finish_frame_pcollector.stop();
  }
#endif

  frame_data._finish_time = 0.0;
  frame_data._wait_for_finish = false;

  if (_last_frame_data == &frame_data) {
    _last_frame_data = nullptr;
  }
}

/**
 * Returns the next available VulkanFrameData object.
 * If finish_frames is true, will finish any old frames that are already done
 * first.  Otherwise, will only do that if the frame queue is full.
 */
VulkanFrameData &VulkanGraphicsStateGuardian::
get_next_frame_data(bool finish_frames) {
  // First, finish old, finished frames.
  if (finish_frames && _frame_data_head != _frame_data_capacity) {
    uint64_t watermark = 0;
    vkGetSemaphoreCounterValue(_device, _timeline_semaphore, &watermark);

    while (true) {
      FrameData &frame_data = _frame_data_pool[_frame_data_tail];

      if (frame_data._watermark >= watermark) {
        // This frame is not yet ready.
        if (frame_data._clock_frame_number < _current_clock_frame_number - 3) {
          // This frame is too old, we will wait for it before continuing.
          wait_semaphore(_timeline_semaphore, frame_data._watermark);
        } else {
          break;
        }
      }

      // This frame has completed execution.
      finish_frame(frame_data);

      _frame_data_tail = (_frame_data_tail + 1) % _frame_data_capacity;
      if (_frame_data_tail == _frame_data_head) {
        // This was the last one, it's now empty.
        _frame_data_head = _frame_data_capacity;
        _frame_data_tail = 0;
        break;
      }
    }
  }

  if (_frame_data != nullptr) {
    return *_frame_data;
  }

  // If the frame queue is full, we must wait until a frame is done.
  if (_frame_data_tail == _frame_data_head) {
    finish_one_frame();
  }

  VulkanFrameData &frame_data = _frame_data_pool[_frame_data_head % _frame_data_capacity];
  frame_data._timer_query_pool._pool = _timer_query_pool;
  frame_data._timer_query_pool._pool_size = _timer_query_pool_size;
  frame_data._timer_query_pool._offset = _timer_query_head;

  // Increase the frame counter, which we use to determine whether we've
  // updated any resources in this frame.
  frame_data._frame_index = ++_frame_counter;
  _frame_data_head = (_frame_data_head + 1) % _frame_data_capacity;

  return frame_data;
}

/**
 * Finishes the last frame.  Returns true unless there were no more unfinished
 * frames.
 */
bool VulkanGraphicsStateGuardian::
finish_one_frame() {
  if (_frame_data_head == _frame_data_capacity) {
    return false;
  }

  FrameData &frame_data = _frame_data_pool[_frame_data_tail];
  if (!wait_semaphore(_timeline_semaphore, frame_data._watermark)) {
    vkQueueWaitIdle(_queue);
  }

  // This frame has completed execution.
  finish_frame(frame_data);
  _frame_data_tail = (_frame_data_tail + 1) % _frame_data_capacity;
  return true;
}

/**
 *
 */
VulkanCommandBuffer VulkanGraphicsStateGuardian::
begin_command_buffer(VkSemaphore wait_for) {
  static const VkCommandBufferBeginInfo begin_info = {
    VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    nullptr,
    VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    nullptr,
  };

  while (_free_command_buffers.empty() && finish_one_frame()) {
  }

  VkCommandBuffer handle = _free_command_buffers.back();
  _free_command_buffers.pop_back();

#ifndef NDEBUG
  if (vulkandisplay_cat.is_spam()) {
    auto &out = vulkandisplay_cat.spam()
      << "Beginning CB #" << _next_begin_command_buffer_seq;

    if (wait_for) {
      out << " (will wait for semaphore " << wait_for << ")";
    }
    out << "\n";
  }
#endif

  VkResult err;
  err = vkBeginCommandBuffer(handle, &begin_info);
  if (err == VK_SUCCESS) {
    return VulkanCommandBuffer(handle, _next_begin_command_buffer_seq++, wait_for);
  } else {
    vulkan_error(err, "Can't begin command buffer");
    return VulkanCommandBuffer();
  }
}

/**
 * Queues the given command buffer for submission.  It may no longer be used
 * to record commands after this call.
 */
void VulkanGraphicsStateGuardian::
end_command_buffer(VulkanCommandBuffer &&cmd, VkSemaphore signal_done) {
  // We must call begin_command_buffer() and end_command_buffer() in the same
  // order.
  nassertv(cmd._seq == _next_end_command_buffer_seq);
  ++_next_end_command_buffer_seq;

#ifndef NDEBUG
  if (vulkandisplay_cat.is_spam()) {
    auto &out = vulkandisplay_cat.spam()
      << "Finished recording CB #" << cmd._seq;

    if (signal_done) {
      out << " (will signal semaphore " << signal_done << ")";
    }
    out << "\n";
  }
#endif

  if (_pending_command_buffers.empty()) {
    _first_pending_command_buffer_seq = cmd._seq;
  }

  if (!cmd._image_barriers.empty() || !cmd._buffer_barriers.empty()) {
    // Need to issue barriers.  We do that on the preceding command buffer.
    if (_pending_command_buffers.empty()) {
      // We don't have a preceding one, so we need to create one just to issue
      // the barriers.
      while (_free_command_buffers.empty() && finish_one_frame()) {
      }
      VkCommandBuffer handle = _free_command_buffers.back();
      _free_command_buffers.pop_back();

      if (vulkandisplay_cat.is_spam()) {
        vulkandisplay_cat.spam()
          << "Using fresh command buffer for issuing barriers before CB #"
          << cmd._seq << ".\n";
      }

      static const VkCommandBufferBeginInfo begin_info = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        nullptr,
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        nullptr,
      };

      VkResult err;
      err = vkBeginCommandBuffer(handle, &begin_info);
      if (err != VK_SUCCESS) {
        vulkan_error(err, "Can't begin command buffer");
        return;
      }

      _pending_command_buffers.push_back(handle);
      _pending_submissions.push_back({VK_NULL_HANDLE, VK_NULL_HANDLE, 0u, 1u});
    }

    VkDependencyInfo info = {
      VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      nullptr, // pNext
      0, // dependencyFlags
      0, // memoryBarrierCount
      nullptr, // pMemoryBarriers
      (uint32_t)cmd._buffer_barriers.size(), cmd._buffer_barriers.data(),
      (uint32_t)cmd._image_barriers.size(), cmd._image_barriers.data(),
    };
    vkCmdPipelineBarrier2(_pending_command_buffers.back(), &info);
  }

  size_t i = _pending_command_buffers.size();
  _pending_command_buffers.push_back(cmd._cmd);

  if (cmd._wait_semaphore != VK_NULL_HANDLE ||
      _pending_submissions.empty() ||
      _pending_submissions.back()._signal_semaphore != VK_NULL_HANDLE) {
    // Create new submission.
    _pending_submissions.push_back({cmd._wait_semaphore, signal_done, (uint32_t)i, 1u});
  } else {
    // No semaphore, so add on to existing submission.
    auto &submit = _pending_submissions.back();
    size_t i2 = submit._first_command_buffer + submit._num_command_buffers++;
    assert(i == i2);
    if (signal_done != VK_NULL_HANDLE) {
      submit._signal_semaphore = signal_done;
    }
  }
  cmd._wait_semaphore = VK_NULL_HANDLE;
  cmd._cmd = VK_NULL_HANDLE;

  // We end the penultimate one.  We don't actually end the one we just added
  // because we want to still be able to insert barriers at the end.
  if (_pending_command_buffers.size() >= 2) {
    vkEndCommandBuffer(_pending_command_buffers[_pending_command_buffers.size() - 2]);
  }
}

/**
 * Makes sure that any pending command buffers are submitted to the queue.
 * Should not be called between begin_frame() and end_frame().
 *
 * Calls to this should be limited to only when something is waiting for this
 * work, as many intermediate submits may be inefficient.
 *
 * Returns a watermark that can be waited for using the timeline semaphore if
 * you want to wait for the results to be available.
 */
uint64_t VulkanGraphicsStateGuardian::
flush() {
  // Shouldn't be called between begin_frame() and end_frame().
  nassertr_always(!_render_cmd, 0);

  if (_transfer_cmd) {
    end_command_buffer(std::move(_transfer_cmd));
    nassertr(!_transfer_cmd, 0);
  }

  if (_pending_submissions.empty()) {
    nassertr(_pending_command_buffers.empty(), 0);
    return _last_submitted_watermark;
  }

  // End the last command buffer, which we left open in case we wanted to
  // record transitions.
  vkEndCommandBuffer(_pending_command_buffers.back());

  VulkanFrameData &frame_data = get_frame_data();
  // As a watermark for the timeline semaphore, we use one past the last
  // command buffer seq, since otherwise the semaphore's initial state of 0
  // would indicate that the first command buffer has already completed.
  frame_data._watermark = _first_pending_command_buffer_seq + _pending_command_buffers.size();

  PStatTimer timer(_flush_pcollector);

  VkCommandBufferSubmitInfo *cb_infos = (VkCommandBufferSubmitInfo *)alloca(sizeof(VkCommandBufferSubmitInfo) * _pending_command_buffers.size());
  for (size_t i = 0; i < _pending_command_buffers.size(); ++i) {
    cb_infos[i] = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO, nullptr, _pending_command_buffers[i], 0u};
  }

  VkSemaphoreSubmitInfo *sem_infos = (VkSemaphoreSubmitInfo *)
    alloca(sizeof(VkSemaphoreSubmitInfo) * (_pending_submissions.size() * 2 + 1));
  size_t sem_i = 0;

  VkSubmitInfo2 *submit_infos = (VkSubmitInfo2 *)alloca(sizeof(VkSubmitInfo2) * _pending_submissions.size());
  for (size_t i = 0; i < _pending_submissions.size(); ++i) {
    auto &pending = _pending_submissions[i];
    VkSubmitInfo2 &submit_info = submit_infos[i];
    submit_info.pNext = nullptr;
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    submit_info.flags = 0;
    submit_info.waitSemaphoreInfoCount = 0;
    submit_info.pWaitSemaphoreInfos = nullptr;
    submit_info.commandBufferInfoCount = pending._num_command_buffers;
    submit_info.pCommandBufferInfos = &cb_infos[pending._first_command_buffer];
    submit_info.signalSemaphoreInfoCount = 0;
    submit_info.pSignalSemaphoreInfos = nullptr;

#ifndef NDEBUG
    if (vulkandisplay_cat.is_spam()) {
      auto &out = vulkandisplay_cat.spam();
      size_t first_seq = _first_pending_command_buffer_seq + pending._first_command_buffer;
      if (pending._num_command_buffers == 1) {
        out << "Submitting CB #" << first_seq;
      } else {
        out << "Submitting CB #" << first_seq << "-#"
            << (first_seq + pending._num_command_buffers - 1);
      }
      if (pending._wait_semaphore) {
        out << ", waiting for semaphore " << pending._wait_semaphore;
      }
      if (pending._signal_semaphore) {
        out << ", signalling semaphore " << pending._signal_semaphore;
      }
      out << "\n";
    }
#endif

    if (pending._wait_semaphore != VK_NULL_HANDLE) {
      // We may need to wait until the attachments are available for writing.
      // TOP_OF_PIPE placates the validation layer, not sure why it's needed.
      VkSemaphoreSubmitInfo &sem_info = sem_infos[sem_i++];
      sem_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
      sem_info.pNext = nullptr;
      sem_info.semaphore = pending._wait_semaphore;
      sem_info.value = 0;
      sem_info.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
      sem_info.deviceIndex = 0;

      submit_info.waitSemaphoreInfoCount = 1;
      submit_info.pWaitSemaphoreInfos = &sem_info;
      frame_data._pending_destroy_semaphores.push_back(pending._wait_semaphore);
    }

    if (pending._signal_semaphore != VK_NULL_HANDLE) {
      VkSemaphoreSubmitInfo &sem_info = sem_infos[sem_i++];
      sem_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
      sem_info.pNext = nullptr;
      sem_info.semaphore = pending._signal_semaphore;
      sem_info.value = 0;
      sem_info.stageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT; //FIXME
      sem_info.deviceIndex = 0;

      submit_info.signalSemaphoreInfoCount = 1;
      submit_info.pSignalSemaphoreInfos = &sem_info;
    }
  }

  // Signal the timeline semaphore in the last submission.
  {
    VkSubmitInfo2 &submit_info = submit_infos[_pending_submissions.size() - 1];
    submit_info.pSignalSemaphoreInfos = &sem_infos[sem_i - submit_info.signalSemaphoreInfoCount];
    submit_info.signalSemaphoreInfoCount++;
    VkSemaphoreSubmitInfo &sem_info = sem_infos[sem_i];
    sem_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    sem_info.pNext = nullptr;
    sem_info.semaphore = _timeline_semaphore;
    sem_info.value = frame_data._watermark;
    sem_info.stageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
    sem_info.deviceIndex = 0;
  }

#ifdef DO_PSTATS
  PStatClient *client = PStatClient::get_global_pstats();
  if (client != nullptr && client->client_is_connected()) {
    frame_data._submit_time = client->get_real_time();
  }
#endif

  VkResult err = vkQueueSubmit2(_queue, _pending_submissions.size(), submit_infos, VK_NULL_HANDLE);
  if (err) {
    vulkan_error(err, "Error submitting command buffers");
    if (err == VK_ERROR_DEVICE_LOST) {
      mark_new();
    }
    return _last_submitted_watermark;
  }

  // Move these command buffers to the frame data, so we can release them when
  // the frame is done.
  frame_data._pending_command_buffers.insert(
    frame_data._pending_command_buffers.end(),
    _pending_command_buffers.begin(),
    _pending_command_buffers.end());

  _pending_command_buffers.clear();
  _pending_submissions.clear();
  _last_submitted_watermark = frame_data._watermark;
  return frame_data._watermark;
}

/**
 * Called before a sequence of draw_primitive() functions are called, this
 * should prepare the vertex data for rendering.  It returns true if the
 * vertices are ok, false to abort this group of primitives.
 */
bool VulkanGraphicsStateGuardian::
begin_draw_primitives(const GeomPipelineReader *geom_reader,
                      const GeomVertexDataPipelineReader *data_reader,
                      size_t num_instances, bool force) {
  if (!GraphicsStateGuardian::begin_draw_primitives(geom_reader, data_reader, num_instances, force)) {
    return false;
  }

  // We need to have a valid shader to be able to render anything.
  if (_current_sc == nullptr) {
    return false;
  }

  _instance_count = (uint32_t)std::min(num_instances, (size_t)0xffffffff);

  // Prepare and bind the vertex buffers.
  size_t num_arrays = data_reader->get_num_arrays();
  size_t num_buffers = num_arrays + _current_sc->_uses_vertex_color;
  VkBuffer *buffers = (VkBuffer *)alloca(sizeof(VkBuffer) * num_buffers);
  VkDeviceSize *offsets = (VkDeviceSize *)alloca(sizeof(VkDeviceSize) * num_buffers);

  size_t i;
  for (i = 0; i < num_arrays; ++i) {
    CPT(GeomVertexArrayDataHandle) handle = data_reader->get_array_reader(i);
    VulkanVertexBufferContext *vbc;
    DCAST_INTO_R(vbc, handle->prepare_now(get_prepared_objects(), this), false);

    if (!update_vertex_buffer(vbc, handle, force)) {
      return false;
    }

    buffers[i] = vbc->_buffer;
    offsets[i] = 0;
  }

  if (_current_sc->_uses_vertex_color) {
    buffers[i] = _current_color_buffer;
    offsets[i] = _current_color_offset;
    ++i;
  }

  nassertr(i == num_buffers, false);

  _vkCmdBindVertexBuffers(_render_cmd, 1, num_buffers, buffers, offsets);

  // If we support setting the topology as dynamic state, or we're rendering
  // points, we can bind the pipeline here.  Otherwise, we have to bind a new
  // pipeline before every draw call.
  Geom::PrimitiveType prim_type = geom_reader->get_primitive_type();
  if (prim_type == Geom::PT_points ||
      (prim_type != Geom::PT_patches && _supports_extended_dynamic_state2) ||
      (prim_type == Geom::PT_patches && _supports_extended_dynamic_state2_patch_control_points)) {
    // With extended dynamic state, Vulkan might still want to know the
    // primitive class up front, it just doesn't care about strip vs fan, etc.
    VkPrimitiveTopology topology;
    switch (prim_type) {
    case Geom::PT_polygons:
      topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
      break;

    case Geom::PT_lines:
      topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
      break;

    case Geom::PT_points:
      topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
      break;

    case Geom::PT_patches:
      topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
      break;

    default:
      return false;
    }

    VkPipeline pipeline = _current_sc->get_pipeline(this, _state_rs, data_reader->get_format(), topology, 0, _fb_config);
    nassertr(pipeline != VK_NULL_HANDLE, false);
    _vkCmdBindPipeline(_render_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
  } else {
    _format = data_reader->get_format();
  }

  return true;
}

/**
 * Draws a series of disconnected triangles.
 */
bool VulkanGraphicsStateGuardian::
draw_triangles(const GeomPrimitivePipelineReader *reader, bool force) {
  return do_draw_primitive_with_topology(reader, force, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, false);
}

/**
 * Draws a series of disconnected triangles with adjacency information.
 */
bool VulkanGraphicsStateGuardian::
draw_triangles_adj(const GeomPrimitivePipelineReader *reader, bool force) {
  return do_draw_primitive_with_topology(reader, force, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY, false);
}

/**
 * Draws a series of triangle strips.
 */
bool VulkanGraphicsStateGuardian::
draw_tristrips(const GeomPrimitivePipelineReader *reader, bool force) {
  return do_draw_primitive_with_topology(reader, force, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, false);
}

/**
 * Draws a series of triangle strips with adjacency information.
 */
bool VulkanGraphicsStateGuardian::
draw_tristrips_adj(const GeomPrimitivePipelineReader *reader, bool force) {
  return do_draw_primitive_with_topology(reader, force, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY, true);
}

/**
 * Draws a series of triangle fans.
 */
bool VulkanGraphicsStateGuardian::
draw_trifans(const GeomPrimitivePipelineReader *reader, bool force) {
  return do_draw_primitive_with_topology(reader, force, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN, false);
}

/**
 * Draws a series of "patches", which can only be processed by a tessellation
 * shader.
 */
bool VulkanGraphicsStateGuardian::
draw_patches(const GeomPrimitivePipelineReader *reader, bool force) {
  PStatGPUTimer timer(this, _draw_primitive_pcollector);

  uint32_t patch_control_points = ((const GeomPrimitive *)reader->get_object())->get_num_vertices_per_primitive();
  if (_supports_extended_dynamic_state2_patch_control_points) {
    // No need to set topology, there's no reason not to bake that into the
    // pipeline for a pipeline with tessellation shaders.
    _vkCmdSetPatchControlPointsEXT(_render_cmd, patch_control_points);
  } else {
    // Bind a pipeline which has both the topology and number of patch control
    // points baked in.
    VkPipeline pipeline = _current_sc->get_pipeline(this, _state_rs, _format, VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, patch_control_points, _fb_config);
    nassertr(pipeline != VK_NULL_HANDLE, false);
    _vkCmdBindPipeline(_render_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
  }

  return do_draw_primitive(reader, force);
}

/**
 * Draws a series of disconnected line segments.
 */
bool VulkanGraphicsStateGuardian::
draw_lines(const GeomPrimitivePipelineReader *reader, bool force) {
  return do_draw_primitive_with_topology(reader, force, VK_PRIMITIVE_TOPOLOGY_LINE_LIST, false);
}

/**
 * Draws a series of disconnected line segments with adjacency information.
 */
bool VulkanGraphicsStateGuardian::
draw_lines_adj(const GeomPrimitivePipelineReader *reader, bool force) {
  return do_draw_primitive_with_topology(reader, force, VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY, false);
}

/**
 * Draws a series of line strips.
 */
bool VulkanGraphicsStateGuardian::
draw_linestrips(const GeomPrimitivePipelineReader *reader, bool force) {
  return do_draw_primitive_with_topology(reader, force, VK_PRIMITIVE_TOPOLOGY_LINE_STRIP, true);
}

/**
 * Draws a series of line strips with adjacency information.
 */
bool VulkanGraphicsStateGuardian::
draw_linestrips_adj(const GeomPrimitivePipelineReader *reader, bool force) {
  return do_draw_primitive_with_topology(reader, force, VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY, true);
}

/**
 * Draws a series of disconnected points.
 */
bool VulkanGraphicsStateGuardian::
draw_points(const GeomPrimitivePipelineReader *reader, bool force) {
  // For point-type primitives, there is only one choice of topology, so we set
  // the pipeline only once in begin_draw_primitives.
  return do_draw_primitive(reader, force);
}

/**
 * Called after a sequence of draw_primitive() functions are called, this
 * should do whatever cleanup is appropriate.
 */
void VulkanGraphicsStateGuardian::
end_draw_primitives() {
  GraphicsStateGuardian::end_draw_primitives();
}

/**
 * Creates a depth buffer for shadow mapping.  A derived GSG can override this
 * if it knows that a particular buffer type works best for shadow rendering.
 */
GraphicsOutput *VulkanGraphicsStateGuardian::
make_shadow_buffer(LightLensNode *light, Texture *tex, GraphicsOutput *host) {
  // We override this to circumvent the fact that GraphicsEngine::make_output
  // can only be called from the app thread.
  bool is_point = light->is_of_type(PointLight::get_class_type());

  // Determine the properties for creating the depth buffer.
  FrameBufferProperties fbp;
  fbp.set_depth_bits(32);

  WindowProperties props = WindowProperties::size(light->get_shadow_buffer_size());
  int flags = GraphicsPipe::BF_refuse_window;
  if (is_point) {
    flags |= GraphicsPipe::BF_size_square;
  }

  if (host != nullptr) {
    host = host->get_host();
  }

  VulkanGraphicsBuffer *sbuffer = new VulkanGraphicsBuffer(get_engine(), get_pipe(), light->get_name(), fbp, props, flags, this, host);
  sbuffer->add_render_texture(tex, GraphicsOutput::RTM_bind_or_copy, GraphicsOutput::RTP_depth);
  get_engine()->add_window(sbuffer, light->get_shadow_buffer_sort());
  return sbuffer;
}

/**
 * Copy the pixels within the indicated display region from the framebuffer
 * into texture memory.
 *
 * This should be called between begin_frame() and end_frame(), but not be
 * called between begin_scene() and end_scene().
 *
 * @param z if z > -1, it is the cube map index into which to copy
 * @return true on success, false on failure
 */
bool VulkanGraphicsStateGuardian::
framebuffer_copy_to_texture(Texture *tex, int view, int z,
                            const DisplayRegion *dr, const RenderBuffer &rb) {

  // You're not allowed to call this while a render pass is active.
  nassertr(_render_pass == VK_NULL_HANDLE, false);
  nassertr(!_closing_gsg, false);

  VulkanTextureContext *fbtc;
  if (rb._buffer_type & (RenderBuffer::T_depth | RenderBuffer::T_stencil)) {
    fbtc = _fb_depth_tc;
    nassertr(_fb_depth_tc != nullptr, false);
  } else {
    fbtc = _fb_color_tc;
    nassertr(_fb_color_tc != nullptr, false);
  }

  int xo, yo, w, h;
  dr->get_region_pixels(xo, yo, w, h);
  tex->set_size_padded(w, h, tex->get_z_size());

  if (tex->get_match_framebuffer_format()) {
    Texture::ComponentType type = Texture::T_unsigned_byte;
    Texture::Format format = Texture::F_rgba8;

    if (lookup_image_format(fbtc->_format, format, type) &&
        (tex->get_component_type() != type || tex->get_format() != format)) {
      tex->set_component_type(type);
      tex->set_format(format);
    }
  }

  VulkanTextureContext *tc;
  tc = use_texture(tex, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                   VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
                   true);
  nassertr(tc != nullptr, false);

  nassertr(fbtc->_extent.width <= tc->_extent.width &&
           fbtc->_extent.height <= tc->_extent.height &&
           fbtc->_extent.depth <= tc->_extent.depth, false);
  nassertr(fbtc->_mip_levels == tc->_mip_levels, false);
  nassertr(fbtc->_array_layers == tc->_array_layers, false);
  nassertr(fbtc->_aspect_mask == tc->_aspect_mask, false);

  // Issue a command to transition the image into a layout optimal for
  // transferring from.
  _render_cmd.add_barrier(fbtc, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                          VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                          VK_ACCESS_2_TRANSFER_READ_BIT);

  if (fbtc->_format == tc->_format) {
    // The formats are the same.  This is just an image copy.
    VkImageCopy region;
    region.srcSubresource.aspectMask = fbtc->_aspect_mask;
    region.srcSubresource.mipLevel = 0;
    if (z != -1) {
      nassertr(z >= 0 && (uint32_t)z < fbtc->_array_layers, false);
      region.srcSubresource.baseArrayLayer = z;
      region.srcSubresource.layerCount = 1;
    } else {
      region.srcSubresource.baseArrayLayer = 0;
      region.srcSubresource.layerCount = fbtc->_array_layers;
    }
    region.srcOffset.x = 0;
    region.srcOffset.y = 0;
    region.srcOffset.z = 0;
    region.dstSubresource = region.srcSubresource;
    region.dstOffset.x = 0;
    region.dstOffset.y = 0;
    region.dstOffset.z = 0;
    region.extent = fbtc->_extent;

    vkCmdCopyImage(_render_cmd, fbtc->_image, fbtc->_layout, tc->_image, tc->_layout, 1, &region);
  } else {
    // The formats are not the same, so we need a blit operation.
    VkImageBlit region;
    region.srcSubresource.aspectMask = fbtc->_aspect_mask;
    region.srcSubresource.mipLevel = 0;
    if (z != -1) {
      nassertr(z >= 0 && (uint32_t)z < fbtc->_array_layers, false);
      region.srcSubresource.baseArrayLayer = z;
      region.srcSubresource.layerCount = 1;
    } else {
      region.srcSubresource.baseArrayLayer = 0;
      region.srcSubresource.layerCount = fbtc->_array_layers;
    }
    region.srcOffsets[0].x = 0;
    region.srcOffsets[0].y = 0;
    region.srcOffsets[0].z = 0;
    region.srcOffsets[1].x = fbtc->_extent.width;
    region.srcOffsets[1].y = fbtc->_extent.height;
    region.srcOffsets[1].z = fbtc->_extent.depth;
    region.dstSubresource = region.srcSubresource;
    region.dstOffsets[0].x = 0;
    region.dstOffsets[0].y = 0;
    region.dstOffsets[0].z = 0;
    region.dstOffsets[1].x = fbtc->_extent.width;
    region.dstOffsets[1].y = fbtc->_extent.height;
    region.dstOffsets[1].z = fbtc->_extent.depth;

    vkCmdBlitImage(_render_cmd, fbtc->_image, fbtc->_layout, tc->_image, tc->_layout, 1, &region, VK_FILTER_NEAREST);
  }
  return true;
}

/**
 * Copy the pixels within the indicated display region from the framebuffer
 * into system memory, not texture memory.  Please note that this may be a
 * very expensive operation as it stalls the graphics pipeline while waiting
 * for the rendered results to become available.  The graphics implementation
 * may choose to defer writing the ram image until the next end_frame() call.
 *
 * This completely redefines the ram image of the indicated texture.
 *
 * This should be called between begin_frame() and end_frame(), but not be
 * called between begin_scene() and end_scene().
 *
 * @return true on success, false on failure
 */
bool VulkanGraphicsStateGuardian::
framebuffer_copy_to_ram(Texture *tex, int view, int z, const DisplayRegion *dr,
                        const RenderBuffer &rb, ScreenshotRequest *request) {

  // Please note that this doesn't complete immediately, but instead queues it
  // until the next end_frame().  This seems to be okay given existing usage,
  // and it prevents having to do the equivalent of glFinish() mid-render.

  // You're not allowed to call this while a render pass is active.
  nassertr(_render_pass == VK_NULL_HANDLE, false);
  nassertr(!_closing_gsg, false);

  // Are we reading the color attachment or the depth attachment?
  VulkanTextureContext *fbtc;
  if (rb._buffer_type & RenderBuffer::T_color) {
    fbtc = _fb_color_tc;
  } else {
    fbtc = _fb_depth_tc;
  }

  nassertr(fbtc != nullptr, false);

  //TODO: proper format checking and size calculation.
  Texture::ComponentType type = Texture::T_unsigned_byte;
  Texture::Format format = Texture::F_rgba8;
  if (!lookup_image_format(fbtc->_format, format, type)) {
    return false;
  }

  tex->setup_2d_texture(fbtc->_extent.width, fbtc->_extent.height, type, format);

  return do_extract_image(fbtc, tex, view, z, request);
}

/**
 * Internal method used by extract_texture_data and framebuffer_copy_to_ram.
 * Queues up a texture-to-RAM download.
 */
bool VulkanGraphicsStateGuardian::
do_extract_image(VulkanTextureContext *tc, Texture *tex, int view, int z, ScreenshotRequest *request) {
  VkDeviceSize buffer_size = tex->get_expected_ram_image_size();

  // Create a temporary buffer for transferring into.
  FrameData::QueuedDownload down;
  if (!create_buffer(buffer_size, down._buffer, down._block,
                     VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
    vulkandisplay_cat.error()
      << "Failed to create staging buffer for framebuffer-to-RAM copy.\n";
    return false;
  }

  // We tack this onto the existing command buffer, for now.
  VulkanCommandBuffer &cmd = _render_cmd;

  // Issue a command to transition the image into a layout optimal for
  // transferring from.
  cmd.add_barrier(tc, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                  VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                  VK_ACCESS_2_TRANSFER_READ_BIT);

  if (tc->_image != VK_NULL_HANDLE) {
    VkBufferImageCopy region;
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = tc->_aspect_mask;
    region.imageSubresource.mipLevel = 0;
    if (z != -1) {
      nassertr(z >= 0 && (uint32_t)z < tc->_array_layers, false);
      region.imageSubresource.baseArrayLayer = z;
      region.imageSubresource.layerCount = 1;
    } else {
      region.imageSubresource.baseArrayLayer = 0;
      region.imageSubresource.layerCount = tc->_array_layers;
    }
    region.imageOffset.x = 0;
    region.imageOffset.y = 0;
    region.imageOffset.z = 0;
    region.imageExtent = tc->_extent;
    vkCmdCopyImageToBuffer(cmd, tc->_image, tc->_layout, down._buffer, 1, &region);
  }
  else {
    VkBufferCopy region;
    region.srcOffset = 0;
    region.dstOffset = 0;
    region.size = buffer_size;
    vkCmdCopyBuffer(cmd, tc->_buffer, down._buffer, 1, &region);
  }

  down._texture = tex;
  down._view = view;
  down._request = request;

  VulkanFrameData &frame_data = get_frame_data();
  frame_data._download_queue.push_back(std::move(down));

  if (request == nullptr) {
    // If we download synchronously, we need to wait for the frame to finish in
    // the end_frame() method, so we can process the results right away.
    frame_data._wait_for_finish = true;
  }
  return true;
}

/**
 * Assumes there is a transfer command buffer started, which is finished and
 * submitted.  Caller is responsible for reopening that buffer if needed.
 */
bool VulkanGraphicsStateGuardian::
do_extract_buffer(VulkanBufferContext *bc, vector_uchar &data) {
  size_t num_bytes = bc->get_data_size_bytes();

  // Create a temporary buffer for transferring into.
  VkBuffer tmp_buffer = VK_NULL_HANDLE;
  VulkanMemoryBlock tmp_block;
  if (!bc->_host_visible) {
    if (!create_buffer(num_bytes, tmp_buffer, tmp_block,
                       VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
      vulkandisplay_cat.error()
        << "Failed to create staging buffer for buffer data extraction.\n";
      return false;
    }

    VkBufferMemoryBarrier2 barrier;
    barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    barrier.pNext = nullptr;
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    barrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.buffer = bc->_buffer;
    barrier.offset = 0;
    barrier.size = VK_WHOLE_SIZE;

    VkDependencyInfo info = {
      VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      nullptr, 0, // pNext, dependencyFlags
      0, nullptr, // memory barriers
      1, &barrier, // buffer barriers
      0, nullptr, // image barriers
    };
    vkCmdPipelineBarrier2(_transfer_cmd, &info);

    VkBufferCopy region;
    region.srcOffset = 0;
    region.dstOffset = 0;
    region.size = num_bytes;
    vkCmdCopyBuffer(_transfer_cmd, bc->_buffer, tmp_buffer, 1, &region);

    // Issue a new barrier to make the copy visible on the host.
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_HOST_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_HOST_READ_BIT;
    barrier.buffer = tmp_buffer;
    barrier.offset = 0;
    barrier.size = VK_WHOLE_SIZE;
    vkCmdPipelineBarrier2(_transfer_cmd, &info);
  } else {
    VkBufferMemoryBarrier2 barrier;
    barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    barrier.pNext = nullptr;
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    barrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_HOST_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_HOST_READ_BIT;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.buffer = bc->_buffer;
    barrier.offset = 0;
    barrier.size = VK_WHOLE_SIZE;

    VkDependencyInfo info = {
      VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      nullptr, 0, // pNext, dependencyFlags
      0, nullptr, // memory barriers
      1, &barrier, // buffer barriers
      0, nullptr, // image barriers
    };
    vkCmdPipelineBarrier2(_transfer_cmd, &info);
  }

  uint64_t watermark = flush();
  if (!wait_semaphore(_timeline_semaphore, watermark)) {
    vkQueueWaitIdle(_queue);
  }

  vkDestroyBuffer(_device, tmp_buffer, nullptr);

  data.resize(num_bytes);
  {
    VulkanMemoryBlock &block = (_has_unified_memory ? bc->_block : tmp_block);
    VulkanMemoryMapping mapping = block.map();
    block.invalidate();
    memcpy(&data[0], mapping, num_bytes);
  }

  return true;
}

/**
 * Sets the primitive topology (or binds the entire pipeline, if extended
 * dynamic state if not supported) and calls do_draw_primitive.
 *
 * Not used for patches.
 */
bool VulkanGraphicsStateGuardian::
do_draw_primitive_with_topology(const GeomPrimitivePipelineReader *reader,
                                bool force, VkPrimitiveTopology topology,
                                bool primitive_restart_enable) {

  PStatGPUTimer timer(this, _draw_primitive_pcollector);

  if (_supports_extended_dynamic_state2) {
    _vkCmdSetPrimitiveTopology(_render_cmd, topology);
    _vkCmdSetPrimitiveRestartEnable(_render_cmd, primitive_restart_enable && reader->is_indexed());
  } else {
    VkPipeline pipeline = _current_sc->get_pipeline(this, _state_rs, _format, topology, 0, _fb_config);
    nassertr(pipeline != VK_NULL_HANDLE, false);
    _vkCmdBindPipeline(_render_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
  }

  return do_draw_primitive(reader, force);
}

/**
 * Invoked by all the draw_xyz methods.
 */
bool VulkanGraphicsStateGuardian::
do_draw_primitive(const GeomPrimitivePipelineReader *reader, bool force) {
  int num_vertices = reader->get_num_vertices();

  if (reader->is_indexed()) {
    // This is an indexed primitive.  Set up and bind the index buffer.
    VulkanIndexBufferContext *ibc;
    DCAST_INTO_R(ibc, reader->prepare_now(get_prepared_objects(), this), false);

    if (!update_index_buffer(ibc, reader, force)) {
      return false;
    }

    _vkCmdBindIndexBuffer(_render_cmd, ibc->_buffer, 0, ibc->_index_type);
    _vkCmdDrawIndexed(_render_cmd, num_vertices, _instance_count, 0, 0, 0);
  } else {
    // A non-indexed primitive.
    _vkCmdDraw(_render_cmd, num_vertices, _instance_count, reader->get_first_vertex(), 0);
  }
  return true;
}

/**
 * Shared code for creating a buffer and allocating memory for it.
 * @return true on success.
 */
bool VulkanGraphicsStateGuardian::
create_buffer(VkDeviceSize size, VkBuffer &buffer, VulkanMemoryBlock &block,
              int usage_flags, VkMemoryPropertyFlagBits flags) {
  VulkanGraphicsPipe *vkpipe;
  DCAST_INTO_R(vkpipe, get_pipe(), false);

  VkBufferCreateInfo buf_info;
  buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buf_info.pNext = nullptr;
  buf_info.flags = 0;
  buf_info.size = size;
  buf_info.usage = usage_flags;
  buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  buf_info.queueFamilyIndexCount = 0;
  buf_info.pQueueFamilyIndices = nullptr;

  VkResult
  err = vkCreateBuffer(_device, &buf_info, nullptr, &buffer);
  if (err)  {
    vulkan_error(err, "Failed to create buffer");
    return false;
  }

  VkMemoryRequirements mem_reqs;
  vkGetBufferMemoryRequirements(_device, buffer, &mem_reqs);

  if (!allocate_memory(block, mem_reqs, flags, true)) {
    vulkandisplay_cat.error()
      << "Failed to allocate " << mem_reqs.size << " bytes for buffer\n";
    vkDestroyBuffer(_device, buffer, nullptr);
    return false;
  }

  if (!block.bind_buffer(buffer)) {
    vkDestroyBuffer(_device, buffer, nullptr);
    return false;
  }

  return true;
}

/**
 * Shared code for creating an image and allocating memory for it.
 * @return true on success.
 */
bool VulkanGraphicsStateGuardian::
create_image(VulkanTextureContext *tc, VkImageType type, VkFormat format,
             const VkExtent3D &extent, uint32_t levels, uint32_t layers,
             VkSampleCountFlagBits samples, VkImageUsageFlags usage,
             VkImageCreateFlags flags) {
  nassertr(extent.width > 0, false);
  nassertr(extent.height > 0, false);
  nassertr(extent.depth > 0, false);

  VkImageCreateInfo img_info;
  img_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  img_info.pNext = nullptr;
  img_info.flags = flags;
  img_info.imageType = type;
  img_info.format = format;
  img_info.extent = extent;
  img_info.mipLevels = levels;
  img_info.arrayLayers = layers;
  img_info.samples = samples;
  img_info.tiling = VK_IMAGE_TILING_OPTIMAL;
  img_info.usage = usage;
  img_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  img_info.queueFamilyIndexCount = 0;
  img_info.pQueueFamilyIndices = nullptr;
  img_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

  VkImage image;
  VkResult err = vkCreateImage(_device, &img_info, nullptr, &image);
  if (err) {
    vulkan_error(err, "Failed to create image");
    return false;
  }

  // Get the memory requirements, and find an appropriate heap to alloc in.
  VkMemoryRequirements mem_reqs;
  vkGetImageMemoryRequirements(_device, image, &mem_reqs);

  VulkanMemoryBlock block;
  if (!allocate_memory(block, mem_reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, false)) {
    vulkandisplay_cat.error()
      << "Failed to allocate " << mem_reqs.size << " bytes for image.\n";
    vkDestroyImage(_device, image, nullptr);
    return false;
  }

  // Bind memory to image.
  if (!block.bind_image(image)) {
    vulkan_error(err, "Failed to bind memory to multisample color image");
    vkDestroyImage(_device, image, nullptr);
    return false;
  }

  tc->_image = image;
  tc->_format = format;
  tc->_extent = extent;
  tc->_mip_levels = levels;
  tc->_array_layers = layers;
  tc->_block = std::move(block);

  tc->set_resident(true);
  tc->update_data_size_bytes(mem_reqs.size);
  return true;
}

/**
 * Creates a new binary semaphore on this device.
 */
VkSemaphore VulkanGraphicsStateGuardian::
create_semaphore() {
  VkSemaphoreCreateInfo semaphore_info = {};
  semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkSemaphore semaphore;
  VkResult
  err = vkCreateSemaphore(_device, &semaphore_info, nullptr, &semaphore);
  nassertr_always(err == VK_SUCCESS, VK_NULL_HANDLE);
  return semaphore;
}

/**
 * Waits for the given timeline semaphore to have reached the given value.
 */
bool VulkanGraphicsStateGuardian::
wait_semaphore(VkSemaphore semaphore, uint64_t value, uint64_t timeout) {
  PStatTimer timer(_wait_semaphore_pcollector);
  VkSemaphoreWaitInfo wait_info;
  wait_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
  wait_info.pNext = nullptr;
  wait_info.flags = 0;
  wait_info.semaphoreCount = 1;
  wait_info.pSemaphores = &semaphore;
  wait_info.pValues = &value;
  VkResult err = vkWaitSemaphores(_device, &wait_info, timeout);
  if (err == VK_ERROR_DEVICE_LOST) {
    mark_new();
  }
  return err == VK_SUCCESS;
}

/**
 * Chooses a framebuffer configuration.
 */
uint32_t VulkanGraphicsStateGuardian::
choose_fb_config(FbConfig &out, FrameBufferProperties &props,
                 VkFormat preferred_format) {
  VulkanGraphicsPipe *vkpipe;
  DCAST_INTO_R(vkpipe, get_pipe(), 0);

  static const struct {
    int rgb, r, g, b, a;
    bool has_float;
    VkFormat format;
  } formats[] = {
    { 8,  8,  0,  0,  0, false, VK_FORMAT_R8_UNORM},
    {16,  8,  8,  0,  0, false, VK_FORMAT_R8G8_UNORM},
    {16,  5,  6,  5,  0, false, VK_FORMAT_R5G6B5_UNORM_PACK16},
    {15,  5,  5,  5,  1, false, VK_FORMAT_A1R5G5B5_UNORM_PACK16},
    {16, 16,  0,  0,  0,  true, VK_FORMAT_R16_SFLOAT},
    {24,  8,  8,  8,  8, false, VK_FORMAT_B8G8R8A8_UNORM},
    {32, 16, 16,  0,  0, false, VK_FORMAT_R16G16_SFLOAT},
    {30, 10, 10, 10,  2, false, VK_FORMAT_A2B10G10R10_UNORM_PACK32},
    {48, 16, 16, 16, 16,  true, VK_FORMAT_R16G16B16A16_SFLOAT},
    {32, 32,  0,  0,  0,  true, VK_FORMAT_R32_SFLOAT},
    {64, 32, 32,  0,  0,  true, VK_FORMAT_R32G32_SFLOAT},
    {96, 32, 32, 32, 32,  true, VK_FORMAT_R32G32B32A32_SFLOAT},
    {0}
  };

  if (preferred_format != VK_FORMAT_UNDEFINED) {
    // Caller's responsibility to modify props.
    out._color_formats.push_back(preferred_format);
  }
  else if (props.get_srgb_color()) {
    // This the only sRGB format.  Deal with it.
    out._color_formats.push_back(VK_FORMAT_B8G8R8A8_SRGB);
    props.set_rgba_bits(8, 8, 8, 8);
    props.set_float_color(false);
  }
  else if (props.get_rgb_color() || props.get_color_bits() > 0) {
    for (int i = 0; formats[i].r; ++i) {
      if (formats[i].r >= props.get_red_bits() &&
          formats[i].g >= props.get_green_bits() &&
          formats[i].b >= props.get_blue_bits() &&
          formats[i].a >= props.get_alpha_bits() &&
          formats[i].rgb >= props.get_color_bits() &&
          formats[i].has_float >= props.get_float_color()) {

        // This format meets the requirements.
        out._color_formats.push_back(VK_FORMAT_B8G8R8A8_SRGB);
        props.set_rgba_bits(formats[i].r, formats[i].g,
                            formats[i].b, formats[i].a);
        break;
      }
    }
  }

  // Choose a suitable depth/stencil format that satisfies the requirements.
  VkFormatProperties fmt_props;
  bool request_depth32 = props.get_depth_bits() > 24 ||
                         props.get_float_depth();

  if (props.get_depth_bits() > 0 && props.get_stencil_bits() > 0) {
    // Vulkan requires support for at least of one of these two formats.
    vkGetPhysicalDeviceFormatProperties(vkpipe->_gpu, VK_FORMAT_D32_SFLOAT_S8_UINT, &fmt_props);
    bool supports_depth32 = (fmt_props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0;
    vkGetPhysicalDeviceFormatProperties(vkpipe->_gpu, VK_FORMAT_D24_UNORM_S8_UINT, &fmt_props);
    bool supports_depth24 = (fmt_props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0;

    if ((supports_depth32 && request_depth32) || !supports_depth24) {
      out._depth_format = VK_FORMAT_D32_SFLOAT_S8_UINT;
      props.set_depth_bits(32);
    } else {
      out._depth_format = VK_FORMAT_D24_UNORM_S8_UINT;
      props.set_depth_bits(24);
    }
    props.set_stencil_bits(8);
  }
  else if (props.get_depth_bits() > 0) {
    // Vulkan requires support for at least of one of these two formats.
    vkGetPhysicalDeviceFormatProperties(vkpipe->_gpu, VK_FORMAT_D32_SFLOAT, &fmt_props);
    bool supports_depth32 = (fmt_props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0;
    vkGetPhysicalDeviceFormatProperties(vkpipe->_gpu, VK_FORMAT_X8_D24_UNORM_PACK32, &fmt_props);
    bool supports_depth24 = (fmt_props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0;

    if ((supports_depth32 && request_depth32) || !supports_depth24) {
      out._depth_format = VK_FORMAT_D32_SFLOAT;
      props.set_depth_bits(32);
    } else {
      out._depth_format = VK_FORMAT_X8_D24_UNORM_PACK32;
      props.set_depth_bits(24);
    }
  }
  else {
    out._depth_format = VK_FORMAT_UNDEFINED;
  }

  // Choose a sample count.
  if (props.get_multisamples() > 1) {
    const VkPhysicalDeviceLimits &limits = vkpipe->_gpu_properties.limits;

    VkSampleCountFlags supported = limits.framebufferColorSampleCounts;
    if (props.get_depth_bits() > 0) {
      supported &= limits.framebufferDepthSampleCounts;
    }
    if (props.get_stencil_bits() > 0) {
      supported &= limits.framebufferStencilSampleCounts;
    }

    // Round up requested bits to next power of two, and flood down.
    VkSampleCountFlags accepted = ::flood_bits_down((uint32_t)(props.get_multisamples() - 1) << 1u);

    // Select the highest overlapping bit.
    out._sample_count = (VkSampleCountFlagBits)(1u << ::get_highest_on_bit(accepted & supported));
  } else {
    out._sample_count = VK_SAMPLE_COUNT_1_BIT;
  }
  props.set_multisamples(out._sample_count);

  // Make a unique identifier for this fb config, for easy hashing.
  uint32_t id = 0;
  while (id < _fb_configs.size()) {
    if (out == _fb_configs[id]) {
      break;
    }
    ++id;
  }
  if (id == _fb_configs.size()) {
    _fb_configs.push_back(out);
  }
  if (vulkandisplay_cat.is_debug()) {
    vulkandisplay_cat.debug()
      << "Framebuffer config " << id << ": " << props << "\n";
  }
  return id;
}

/**
 * Creates a VkPipeline for the given RenderState+GeomVertexFormat combination.
 */
VkPipeline VulkanGraphicsStateGuardian::
make_pipeline(VulkanShaderContext *sc,
              const VulkanShaderContext::PipelineKey &key) {

  if (vulkandisplay_cat.is_spam()) {
    vulkandisplay_cat.spam()
      << "Making pipeline for"
      << " format=" << key._format
      << " topology=" << key._topology
      << " patch_control_points=" << key._patch_control_points
      << " fb_config=" << key._fb_config
      << " color_type=" << key._color_type
      << " render_mode_attrib=" << key._render_mode_attrib
      << " cull_face_mode=" << key._cull_face_mode
      << " depth_write_mode=" << key._depth_write_mode
      << " depth_test_mode=" << key._depth_test_mode
      << " color_blend_attrib=" << key._color_blend_attrib
      << " color_write_mask=" << key._color_write_mask
      << " logic_op=" << key._logic_op
      << " transparency_mode=" << key._transparency_mode
      << " alpha_test_attrib=" << key._alpha_test_attrib
      << "\n";
  }

  FbConfig fb_config = _fb_configs[key._fb_config];

  PStatTimer timer(_make_pipeline_pcollector);

  VkPipelineShaderStageCreateInfo stages[(size_t)Shader::Stage::COMPUTE + 1];
  const VkShaderStageFlagBits stage_flags[(size_t)Shader::Stage::COMPUTE + 1] = {
    VK_SHADER_STAGE_VERTEX_BIT,
    VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
    VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
    VK_SHADER_STAGE_GEOMETRY_BIT,
    VK_SHADER_STAGE_FRAGMENT_BIT,
    VK_SHADER_STAGE_COMPUTE_BIT,
  };
  uint32_t num_stages = 0;

  VkSpecializationInfo fragment_spec_info;
  VkSpecializationMapEntry fragment_spec_map_entry;
  float alpha_test_ref;

  RenderAttrib::PandaCompareFunc alpha_test_mode = RenderAttrib::M_none;
  if (key._alpha_test_attrib != nullptr && !fb_config._color_formats.empty()) {
    alpha_test_mode = key._alpha_test_attrib->get_mode();
    if (alpha_test_mode != RenderAttrib::M_never) {
      alpha_test_ref = key._alpha_test_attrib->get_reference_alpha();
    } else {
      // Rather than create special case code for the rare case of M_never, we
      // instead turn it into a equal test with NaN.
      alpha_test_ref = make_nan((float)0);
    }

    fragment_spec_info.mapEntryCount = 1;
    fragment_spec_info.pMapEntries = &fragment_spec_map_entry;
    fragment_spec_info.dataSize = 4;
    fragment_spec_info.pData = &alpha_test_ref;

    fragment_spec_map_entry.constantID = 0;
    fragment_spec_map_entry.offset = 0;
    fragment_spec_map_entry.size = 4;
  }

  for (size_t i = 0; i <= (size_t)Shader::Stage::COMPUTE; ++i) {
    if (sc->_modules[i] != VK_NULL_HANDLE) {
      VkPipelineShaderStageCreateInfo &stage = stages[num_stages++];
      stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      stage.pNext = nullptr;
      stage.flags = 0;
      stage.stage = stage_flags[i];
      stage.pSpecializationInfo = nullptr;
      if (i == (size_t)Shader::Stage::FRAGMENT && alpha_test_mode != RenderAttrib::M_none) {
        stage.module = sc->get_fragment_module(_device, alpha_test_mode);
        stage.pSpecializationInfo = &fragment_spec_info;
      } else {
        stage.module = sc->get_module((Shader::Stage)i);
      }
      stage.pName = "main";
    }
  }

  // Describe each vertex input binding (ie. GeomVertexArray).  Leave two extra
  // slots for the "color" and "null" bindings, see below.
  size_t num_arrays = key._format->get_num_arrays();
  VkVertexInputBindingDescription *binding_descs = (VkVertexInputBindingDescription *)
    alloca(sizeof(VkVertexInputBindingDescription) * (num_arrays + 2));

  VkVertexInputBindingDivisorDescriptionEXT *divisors = nullptr;
  int num_divisors = 0;
  if (_supports_vertex_attrib_divisor) {
    divisors = (VkVertexInputBindingDivisorDescriptionEXT *)
      alloca(sizeof(VkVertexInputBindingDivisorDescriptionEXT) * (num_arrays + 2));
  }

  int num_bindings = 0;

  // Always create a "null binding" for missing vertex attributes.
  int null_binding = num_bindings;
  {
    VkVertexInputBindingDescription &binding_desc = binding_descs[num_bindings];
    binding_desc.binding = num_bindings;
    binding_desc.stride = 0;
    if (_supports_vertex_attrib_zero_divisor) {
      binding_desc.stride = 4;
      binding_desc.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
      divisors[num_divisors].binding = binding_desc.binding;
      divisors[num_divisors].divisor = 0;
      ++num_divisors;
    } else {
      binding_desc.stride = 0;
      binding_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    }
    ++num_bindings;
  }

  for (size_t i = 0; i < num_arrays; ++i) {
    const GeomVertexArrayFormat *array = key._format->get_array(i);
    VkVertexInputBindingDescription &binding_desc = binding_descs[num_bindings];
    binding_desc.binding = num_bindings;
    binding_desc.stride = array->get_stride();
    if (array->get_divisor() == 0) {
      binding_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    } else {
      binding_desc.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
      if (_supports_vertex_attrib_divisor) {
        divisors[num_divisors].binding = binding_desc.binding;
        divisors[num_divisors].divisor = array->get_divisor();
        ++num_divisors;
      }
    }
    ++num_bindings;
  }

  // Create an extra binding for flat vertex colors.
  int color_binding = -1;
  if (sc->_uses_vertex_color) {
    color_binding = num_bindings;
    VkVertexInputBindingDescription &binding_desc = binding_descs[num_bindings];
    binding_desc.binding = num_bindings;

    if (_supports_vertex_attrib_zero_divisor) {
      // MoltenVK uses portability subset, which doesn't allow zero stride,
      // but it does support zero divisor.
      binding_desc.stride = 16;
      binding_desc.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
      divisors[num_divisors].binding = binding_desc.binding;
      divisors[num_divisors].divisor = 0;
      ++num_divisors;
    } else {
      binding_desc.stride = 0;
      binding_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    }
    ++num_bindings;
  }

  // Now describe each vertex attribute (ie. GeomVertexColumn).
  const Shader *shader = sc->get_shader();
  nassertr(shader != nullptr, VK_NULL_HANDLE);
  pvector<Shader::ShaderVarSpec>::const_iterator it;

  VkVertexInputAttributeDescription *attrib_desc = (VkVertexInputAttributeDescription *)
    alloca(sizeof(VkVertexInputAttributeDescription) * shader->_var_spec.size());

  uint32_t i = 0;
  for (it = shader->_var_spec.begin(); it != shader->_var_spec.end(); ++it) {
    const Shader::ShaderVarSpec &spec = *it;
    int array_index;
    const GeomVertexColumn *column;

    attrib_desc[i].location = spec._id._location;

    if ((key._color_type != ColorAttrib::T_vertex && spec._name == InternalName::get_color()) ||
        !key._format->get_array_info(spec._name, array_index, column)) {
      // The shader references a non-existent vertex column.  To make this a
      // well-defined operation (as in OpenGL), we bind a "null" vertex buffer
      // containing a fixed value with a stride of 0.
      if (spec._name == InternalName::get_color()) {
        // Except for vertex colors, which get a flat color applied.
        assert(color_binding >= 0);
        attrib_desc[i].binding = color_binding;
        attrib_desc[i].offset = 0;
        attrib_desc[i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
      }
      else {
        attrib_desc[i].binding = null_binding;
        attrib_desc[i].offset = 0;
        attrib_desc[i].format = VK_FORMAT_R32_SFLOAT;
      }
      ++i;
      continue;
    }

    attrib_desc[i].binding = array_index + 1;
    attrib_desc[i].offset = column->get_start();
    assert(attrib_desc[i].offset < 2048);

    bool normalized = (column->get_contents() == GeomEnums::C_color);

    // Determine which Vulkan format to map this column to.  The formats are
    // laid out somewhat (though not entirely) consistently, so we can use a
    // trick to jump to the format for the right number of components.
    int fmt_jump = column->get_num_components() - 1;
    switch (column->get_numeric_type()) {
    case GeomEnums::NT_uint8:
      if (normalized) {
        if (fmt_jump < 3) {
          attrib_desc[i].format = (VkFormat)(VK_FORMAT_R8_UNORM + 7 * fmt_jump);
        } else {
          attrib_desc[i].format = VK_FORMAT_R8G8B8A8_UNORM;
        }
      } else {
        if (fmt_jump < 3) {
          attrib_desc[i].format = (VkFormat)(VK_FORMAT_R8_UINT + 7 * fmt_jump);
        } else {
          attrib_desc[i].format = VK_FORMAT_R8G8B8A8_UINT;
        }
      }
      break;
    case GeomEnums::NT_uint16:
      attrib_desc[i].format = (VkFormat)(VK_FORMAT_R16_UINT + 7 * fmt_jump);
      break;
    case GeomEnums::NT_uint32:
      attrib_desc[i].format = (VkFormat)(VK_FORMAT_R32_UINT + 3 * fmt_jump);
      break;
    case GeomEnums::NT_packed_dcba:
      if (normalized) {
        attrib_desc[i].format = VK_FORMAT_A8B8G8R8_UNORM_PACK32;
      } else {
        attrib_desc[i].format = VK_FORMAT_A8B8G8R8_UINT_PACK32;
      }
      break;
    case GeomEnums::NT_packed_dabc:
      if (normalized) {
        attrib_desc[i].format = VK_FORMAT_B8G8R8A8_UNORM;
      } else {
        attrib_desc[i].format = VK_FORMAT_B8G8R8A8_UINT;
      }
      break;
#ifndef STDFLOAT_DOUBLE
    case GeomEnums::NT_stdfloat:
#endif
    case GeomEnums::NT_float32:
      attrib_desc[i].format = (VkFormat)(VK_FORMAT_R32_SFLOAT + 3 * fmt_jump);
      break;
#ifdef STDFLOAT_DOUBLE
    case GeomEnums::NT_stdfloat:
#endif
    case GeomEnums::NT_float64:
      attrib_desc[i].format = (VkFormat)(VK_FORMAT_R64_SFLOAT + 3 * fmt_jump);
      break;
    case GeomEnums::NT_int8:
      if (fmt_jump < 3) {
        attrib_desc[i].format = (VkFormat)(VK_FORMAT_R8_SINT + 7 * fmt_jump);
      } else {
        attrib_desc[i].format = VK_FORMAT_R8G8B8A8_SINT;
      }
      break;
    case GeomEnums::NT_int16:
      attrib_desc[i].format = (VkFormat)(VK_FORMAT_R16_SINT + 7 * fmt_jump);
      break;
    case GeomEnums::NT_int32:
      attrib_desc[i].format = (VkFormat)(VK_FORMAT_R32_SINT + 3 * fmt_jump);
      break;
    case GeomEnums::NT_packed_ufloat:
      attrib_desc[i].format = VK_FORMAT_B10G11R11_UFLOAT_PACK32;
      break;
    }
    ++i;
  }

  VkPipelineVertexInputStateCreateInfo vertex_info;
  vertex_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertex_info.pNext = nullptr;
  vertex_info.flags = 0;
  vertex_info.vertexBindingDescriptionCount = num_bindings;
  vertex_info.pVertexBindingDescriptions = binding_descs;
  vertex_info.vertexAttributeDescriptionCount = i;
  vertex_info.pVertexAttributeDescriptions = attrib_desc;

  VkPipelineVertexInputDivisorStateCreateInfoEXT divisor_info;
  if (num_divisors > 0) {
    divisor_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_DIVISOR_STATE_CREATE_INFO_EXT;
    divisor_info.pNext = vertex_info.pNext;
    divisor_info.vertexBindingDivisorCount = num_divisors;
    divisor_info.pVertexBindingDivisors = divisors;
    vertex_info.pNext = &divisor_info;
  }

  VkPipelineInputAssemblyStateCreateInfo assembly_info;
  assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  assembly_info.pNext = nullptr;
  assembly_info.flags = 0;
  assembly_info.topology = key._topology;
  if (_supports_extended_dynamic_state2) {
    // This is enabled dynamically if needed.
    assembly_info.primitiveRestartEnable = VK_FALSE;
  } else {
    assembly_info.primitiveRestartEnable = (
      key._topology == VK_PRIMITIVE_TOPOLOGY_LINE_STRIP ||
      //key._topology == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP ||
      //key._topology == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN ||
      key._topology == VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY ||
      key._topology == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY);
  }

  VkPipelineTessellationStateCreateInfo tess_info;
  tess_info.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
  tess_info.pNext = nullptr;
  tess_info.flags = 0;
  tess_info.patchControlPoints = key._patch_control_points;

  VkPipelineViewportStateCreateInfo viewport_info;
  viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport_info.pNext = nullptr;
  viewport_info.flags = 0;
  viewport_info.viewportCount = 1;
  viewport_info.pViewports = nullptr;
  viewport_info.scissorCount = 1;
  viewport_info.pScissors = nullptr;

  VkPipelineRasterizationStateCreateInfo raster_info;
  raster_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  raster_info.pNext = nullptr;
  raster_info.flags = 0;
  raster_info.depthClampEnable = VK_FALSE;
  raster_info.rasterizerDiscardEnable = VK_FALSE;

  if (key._render_mode_attrib != nullptr &&
      (_supported_geom_rendering & Geom::GR_render_mode_wireframe) != 0) {
    switch (key._render_mode_attrib->get_mode()) {
    case RenderModeAttrib::M_filled:
    default:
      raster_info.polygonMode = VK_POLYGON_MODE_FILL;
      break;
    case RenderModeAttrib::M_wireframe:
      raster_info.polygonMode = VK_POLYGON_MODE_LINE;
      break;
    case RenderModeAttrib::M_point:
      raster_info.polygonMode = VK_POLYGON_MODE_POINT;
      break;
    }
    raster_info.lineWidth = key._render_mode_attrib->get_thickness();
  } else {
    // Not supported.  The geometry will have been changed at munge time.
    raster_info.polygonMode = VK_POLYGON_MODE_FILL;
    raster_info.lineWidth = 1.0f;
  }

  raster_info.cullMode = (VkCullModeFlagBits)key._cull_face_mode;
  raster_info.frontFace = VK_FRONT_FACE_CLOCKWISE; // Flipped
  raster_info.depthBiasEnable = VK_FALSE;
  raster_info.depthBiasConstantFactor = 0;
  raster_info.depthBiasClamp = 0;
  raster_info.depthBiasSlopeFactor = 0;

  VkPipelineMultisampleStateCreateInfo ms_info;
  ms_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  ms_info.pNext = nullptr;
  ms_info.flags = 0;
  ms_info.rasterizationSamples = fb_config._sample_count;
  ms_info.sampleShadingEnable = VK_FALSE;
  ms_info.minSampleShading = 0.0;
  ms_info.pSampleMask = nullptr;
  ms_info.alphaToCoverageEnable = VK_FALSE;
  ms_info.alphaToOneEnable = VK_FALSE;

  VkPipelineDepthStencilStateCreateInfo ds_info;
  ds_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  ds_info.pNext = nullptr;
  ds_info.flags = 0;
  ds_info.depthTestEnable = (key._depth_test_mode != RenderAttrib::M_none);
  ds_info.depthWriteEnable = key._depth_write_mode;
  ds_info.depthCompareOp = (VkCompareOp)std::max(0, key._depth_test_mode - 1);
  ds_info.depthBoundsTestEnable = VK_FALSE;
  ds_info.stencilTestEnable = VK_FALSE;
  ds_info.front.failOp = VK_STENCIL_OP_KEEP;
  ds_info.front.passOp = VK_STENCIL_OP_KEEP;
  ds_info.front.depthFailOp = VK_STENCIL_OP_KEEP;
  ds_info.front.compareOp = VK_COMPARE_OP_ALWAYS;
  ds_info.front.compareMask = 0;
  ds_info.front.writeMask = 0;
  ds_info.front.reference = 0;
  ds_info.back = ds_info.front;
  ds_info.minDepthBounds = 0;
  ds_info.maxDepthBounds = 0;

  VkPipelineColorBlendAttachmentState att_state[1];
  if (key._color_blend_attrib != nullptr) {
    const ColorBlendAttrib *color_blend = key._color_blend_attrib;
    nassertr(color_blend->get_mode() != ColorBlendAttrib::M_none, VK_NULL_HANDLE);
    att_state[0].blendEnable = VK_TRUE;
    att_state[0].srcColorBlendFactor = (VkBlendFactor)color_blend->get_operand_a();
    att_state[0].dstColorBlendFactor = (VkBlendFactor)color_blend->get_operand_b();
    att_state[0].colorBlendOp = (VkBlendOp)(color_blend->get_mode() - 1);
    att_state[0].srcAlphaBlendFactor = (VkBlendFactor)color_blend->get_alpha_operand_a();
    att_state[0].dstAlphaBlendFactor = (VkBlendFactor)color_blend->get_alpha_operand_b();
    att_state[0].alphaBlendOp = (VkBlendOp)(color_blend->get_alpha_mode() - 1);
  } else {
    att_state[0].blendEnable = VK_FALSE;

    // No color blend mode enabled; was there a transparency attribute?
    switch (key._transparency_mode) {
    case TransparencyAttrib::M_none:
    case TransparencyAttrib::M_binary:
      att_state[0].blendEnable = VK_FALSE;
      att_state[0].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
      att_state[0].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
      att_state[0].colorBlendOp = VK_BLEND_OP_ADD;
      att_state[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
      att_state[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
      att_state[0].alphaBlendOp = VK_BLEND_OP_ADD;
      break;

    case TransparencyAttrib::M_alpha:
    case TransparencyAttrib::M_dual:
      att_state[0].blendEnable = VK_TRUE;
      att_state[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
      att_state[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
      att_state[0].colorBlendOp = VK_BLEND_OP_ADD;
      att_state[0].srcAlphaBlendFactor = old_alpha_blend ? VK_BLEND_FACTOR_SRC_ALPHA : VK_BLEND_FACTOR_ONE;
      att_state[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
      att_state[0].alphaBlendOp = VK_BLEND_OP_ADD;
      break;

    case TransparencyAttrib::M_premultiplied_alpha:
      att_state[0].blendEnable = VK_TRUE;
      att_state[0].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
      att_state[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
      att_state[0].colorBlendOp = VK_BLEND_OP_ADD;
      att_state[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
      att_state[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
      att_state[0].alphaBlendOp = VK_BLEND_OP_ADD;
      break;

    case TransparencyAttrib::M_multisample:
      // We need to enable *both* of these in M_multisample case.
      ms_info.alphaToCoverageEnable = VK_TRUE;
      ms_info.alphaToOneEnable = VK_TRUE;
      att_state[0].blendEnable = VK_FALSE;
      break;

    case TransparencyAttrib::M_multisample_mask:
      ms_info.alphaToCoverageEnable = VK_TRUE;
      ms_info.alphaToOneEnable = VK_FALSE;
      att_state[0].blendEnable = VK_FALSE;
      break;

    default:
      att_state[0].blendEnable = VK_FALSE;
      vulkandisplay_cat.error()
        << "invalid transparency mode " << (int)key._transparency_mode << std::endl;
      break;
    }
  }
  att_state[0].colorWriteMask = key._color_write_mask;

  VkPipelineColorBlendStateCreateInfo blend_info;
  blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  blend_info.pNext = nullptr;
  blend_info.flags = 0;

  if (key._logic_op != LogicOpAttrib::O_none) {
    blend_info.logicOpEnable = VK_TRUE;
    blend_info.logicOp = (VkLogicOp)(key._logic_op - 1);
  } else {
    blend_info.logicOpEnable = VK_FALSE;
    blend_info.logicOp = VK_LOGIC_OP_COPY;
  }

  blend_info.attachmentCount = 1;
  blend_info.pAttachments = att_state;

  LColor constant_color = LColor::zero();
  if (key._color_blend_attrib != nullptr) {
    constant_color = key._color_blend_attrib->get_color();
  }
  blend_info.blendConstants[0] = constant_color[0];
  blend_info.blendConstants[1] = constant_color[1];
  blend_info.blendConstants[2] = constant_color[2];
  blend_info.blendConstants[3] = constant_color[3];

  // Tell Vulkan that we'll be specifying the viewport and scissor separately.
  VkDynamicState dynamic_states[4] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
  uint32_t num_dynamic_states = 2;
  if (key._topology == VK_PRIMITIVE_TOPOLOGY_PATCH_LIST) {
    if (key._patch_control_points == 0) {
      nassertr(_supports_extended_dynamic_state2_patch_control_points, VK_NULL_HANDLE);
      dynamic_states[2] = VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT;
      num_dynamic_states = 3;
    }
  }
  else if (key._topology != VK_PRIMITIVE_TOPOLOGY_POINT_LIST &&
           _supports_extended_dynamic_state2) {
    dynamic_states[2] = VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY_EXT;
    dynamic_states[3] = VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE_EXT;
    num_dynamic_states = 4;
  }

  VkPipelineDynamicStateCreateInfo dynamic_info;
  dynamic_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamic_info.pNext = nullptr;
  dynamic_info.flags = 0;
  dynamic_info.dynamicStateCount = num_dynamic_states;
  dynamic_info.pDynamicStates = dynamic_states;

  VkGraphicsPipelineCreateInfo pipeline_info;
  pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipeline_info.pNext = nullptr;
  pipeline_info.flags = 0;
  pipeline_info.stageCount = num_stages;
  pipeline_info.pStages = stages;
  pipeline_info.pVertexInputState = &vertex_info;
  pipeline_info.pInputAssemblyState = &assembly_info;
  pipeline_info.pTessellationState = (key._patch_control_points > 0) ? &tess_info : nullptr;
  pipeline_info.pViewportState = &viewport_info;
  pipeline_info.pRasterizationState = &raster_info;
  pipeline_info.pMultisampleState = &ms_info;
  pipeline_info.pDepthStencilState = &ds_info;
  pipeline_info.pColorBlendState = &blend_info;
  pipeline_info.pDynamicState = &dynamic_info;
  pipeline_info.layout = sc->_pipeline_layout;
  pipeline_info.renderPass = _render_pass;
  pipeline_info.subpass = 0;
  pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
  pipeline_info.basePipelineIndex = 0;

  VkPipelineRenderingCreateInfo render_info;
  if (_render_pass == VK_NULL_HANDLE) {
    pipeline_info.pNext = &render_info;
    render_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    render_info.pNext = nullptr;
    render_info.viewMask = 0;
    render_info.colorAttachmentCount = fb_config._color_formats.size();
    render_info.pColorAttachmentFormats = fb_config._color_formats.data();
    render_info.depthAttachmentFormat = fb_config._depth_format;
    render_info.stencilAttachmentFormat = fb_config._stencil_format;
  }

  VkResult err;
  VkPipeline pipeline;
  err = vkCreateGraphicsPipelines(_device, _pipeline_cache, 1, &pipeline_info, nullptr, &pipeline);
  if (err) {
    vulkan_error(err, "Failed to create graphics pipeline");
    return VK_NULL_HANDLE;
  }

  return pipeline;
}

/**
 * Creates a pipeline for the given compute shader context.
 */
VkPipeline VulkanGraphicsStateGuardian::
make_compute_pipeline(VulkanShaderContext *sc) {
  if (vulkandisplay_cat.is_debug()) {
    vulkandisplay_cat.debug()
      << "Making compute pipeline for shader " << sc->_shader->get_filename() << "\n";
  }

  VkComputePipelineCreateInfo pipeline_info;
  pipeline_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
  pipeline_info.pNext = nullptr;
  pipeline_info.flags = 0;
  pipeline_info.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  pipeline_info.stage.pNext = nullptr;
  pipeline_info.stage.flags = 0;
  pipeline_info.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
  pipeline_info.stage.module = sc->_modules[(size_t)Shader::Stage::COMPUTE];
  pipeline_info.stage.pName = "main";
  pipeline_info.stage.pSpecializationInfo = nullptr;
  pipeline_info.layout = sc->_pipeline_layout;
  pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
  pipeline_info.basePipelineIndex = 0;

  VkResult err;
  VkPipeline pipeline;
  err = vkCreateComputePipelines(_device, _pipeline_cache, 1, &pipeline_info, nullptr, &pipeline);
  if (err) {
    vulkan_error(err, "Failed to create compute pipeline");
    return VK_NULL_HANDLE;
  }

  return pipeline;
}

/**
 * Returns a VkDescriptorSet for the resources of the given render state.
 * Returns true if this was the first use of this set in this frame, false
 * otherwise.
 */
bool VulkanGraphicsStateGuardian::
get_attrib_descriptor_set(VkDescriptorSet &out, VkDescriptorSetLayout layout, const RenderAttrib *attrib) {
  nassertr(_current_sc != nullptr, false);

  // Look it up in the attribute map.
  auto it = _current_sc->_attrib_descriptor_set_map.find(attrib);
  if (it != _current_sc->_attrib_descriptor_set_map.end()) {
    // Found something.  Check that it's not just a different state that has
    // been allocated in the memory of a previous state.
    VulkanShaderContext::DescriptorSet &set = it->second;
    if (!set._weak_ref->was_deleted()) {
      // Nope, it's not deleted, which must mean it's the same one, which must
      // mean we have a live pointer to it (so no need to lock anything).
      out = set._handle;

      bool is_current = _frame_counter == set._last_update_frame;
      if (is_current) {
        return false;
      }
      else if (set._last_update_frame <= _last_finished_frame) {
        // We have a descriptor set from a frame that has finished, so we can
        // safely update it.
        set._last_update_frame = _frame_counter;
        return true;
      }
    }

    // It's been deleted, which means it's for a very different state.  We can
    // let go of this one and create a new one instead.
    if (!set._weak_ref->unref()) {
      delete set._weak_ref;
    }

    _frame_data->_pending_free_descriptor_sets.push_back(set._handle);
    set._handle = VK_NULL_HANDLE;
  }

  VulkanShaderContext::DescriptorSet set;

  VkDescriptorSetAllocateInfo alloc_info;
  alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  alloc_info.pNext = nullptr;
  alloc_info.descriptorPool = _descriptor_pool;
  alloc_info.descriptorSetCount = 1;
  alloc_info.pSetLayouts = &layout;
  VkResult err = vkAllocateDescriptorSets(_device, &alloc_info, &set._handle);
  if (err) {
    vulkan_error(err, "Failed to allocate descriptor set for attribute");
    return false;
  }

  // Keep a weak reference, so we'll know if it's been deleted.
  set._weak_ref = ((RenderAttrib *)attrib)->weak_ref();

  set._last_update_frame = _frame_counter;

  out = set._handle;
  _current_sc->_attrib_descriptor_set_map[attrib] = std::move(set);
  return true;
}

/**
 * Updates the descriptor set containing all the light attributes.
 */
bool VulkanGraphicsStateGuardian::
update_lattr_descriptor_set(VkDescriptorSet ds, const LightAttrib *attr) {
  PStatTimer timer(_update_lattr_descriptor_set_pcollector);

  const size_t num_shadow_maps = 8;
  VkDescriptorImageInfo *image_infos = (VkDescriptorImageInfo *)alloca(num_shadow_maps * sizeof(VkDescriptorImageInfo));

  VkWriteDescriptorSet write;
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.pNext = nullptr;
  write.dstSet = ds;
  write.dstBinding = 0;
  write.dstArrayElement = 0;
  write.descriptorCount = num_shadow_maps;
  write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  write.pImageInfo = image_infos;
  write.pBufferInfo = nullptr;
  write.pTexelBufferView = nullptr;

  size_t num_lights = attr->get_num_non_ambient_lights();

  PT(Texture) dummy = get_dummy_shadow_map(false);

  for (size_t i = 0; i < num_shadow_maps; ++i) {
    PT(Texture) texture;

    if (i < num_lights) {
      NodePath light = attr->get_on_light(i);
      nassertr(!light.is_empty(), false);
      Light *light_obj = light.node()->as_light();
      nassertr(light_obj != nullptr, false);

      LightLensNode *lln = DCAST(LightLensNode, light.node());
      if (lln != nullptr && lln->is_shadow_caster()) {
        texture = get_shadow_map(light);
      } else {
        texture = dummy;
      }
    } else {
      texture = dummy;
    }

    // We don't know at this point which stages is using them, and finding out
    // would require duplication of descriptor sets, so we flag all stages.
    VkPipelineStageFlags2 stage_flags = 0
      | VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT
      | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT
      | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT
      ;

    if (_supported_shader_caps & ShaderModule::C_tessellation_shader) {
      //stage_flags |= VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT;
      //stage_flags |= VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT;
    }
    if (_supported_shader_caps & ShaderModule::C_geometry_shader) {
      //stage_flags |= VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT;
    }

    VulkanTextureContext *tc;
    tc = use_texture(texture, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                     stage_flags, VK_ACCESS_2_SHADER_READ_BIT);

    if (tc == nullptr) {
      // We can't bind this because we're currently rendering into it.
      texture = dummy;
      tc = use_texture(texture, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                       stage_flags, VK_ACCESS_2_SHADER_READ_BIT);
    }

    VkDescriptorImageInfo &image_info = image_infos[i];
    image_info.sampler = VK_NULL_HANDLE;
    image_info.imageView = tc ? tc->get_image_view(0) : VK_NULL_HANDLE;
    image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  }

  _vkUpdateDescriptorSets(_device, 1, &write, 0, nullptr);
  return true;
}

/**
 * Updates the descriptor set containing the dynamic uniform buffer.  This only
 * needs to happen rarely, when the global uniform buffer is swapped out due to
 * running out of size.
 */
bool VulkanGraphicsStateGuardian::
update_dynamic_uniform_descriptor_set(VulkanShaderContext *sc) {
  nassertr(sc->_dynamic_uniform_descriptor_set_layout != VK_NULL_HANDLE, false);

  // Create a descriptor set for the UBOs.
  VkDescriptorSetAllocateInfo alloc_info;
  alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  alloc_info.pNext = nullptr;
  alloc_info.descriptorPool = _descriptor_pool;
  alloc_info.descriptorSetCount = 1;
  alloc_info.pSetLayouts = &sc->_dynamic_uniform_descriptor_set_layout;
  VkResult
  err = vkAllocateDescriptorSets(_device, &alloc_info, &sc->_uniform_descriptor_set);
  if (err) {
    vulkan_error(err, "Failed to allocate descriptor set for dynamic uniforms");
    return false;
  }

  // We set the offsets to 0, since we use dynamic offsets.
  size_t count = 0;
  VkDescriptorBufferInfo buffer_info[2];
  if (sc->_other_state_block._size > 0) {
    buffer_info[count].buffer = _uniform_buffer;
    buffer_info[count].offset = 0;
    buffer_info[count].range = sc->_other_state_block._size;
    ++count;
  }

  VkWriteDescriptorSet write[1];
  for (size_t i = 0; i < count; ++i) {
    write[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write[i].pNext = nullptr;
    write[i].dstSet = sc->_uniform_descriptor_set;
    write[i].dstBinding = i;
    write[i].dstArrayElement = 0;
    write[i].descriptorCount = 1;
    write[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    write[i].pImageInfo = nullptr;
    write[i].pBufferInfo = &buffer_info[i];
    write[i].pTexelBufferView = nullptr;
  }
  vkUpdateDescriptorSets(_device, count, write, 0, nullptr);

  return true;
}

/**
 * Returns a writable pointer to the dynamic uniform buffer.
 */
void *VulkanGraphicsStateGuardian::
alloc_dynamic_uniform_buffer(VkDeviceSize size, VkBuffer &buffer, uint32_t &offset) {
  buffer = _uniform_buffer;
  if (size == 0) {
    offset = 0;
    return nullptr;
  }

  ptrdiff_t result = _uniform_buffer_allocator.alloc(size);
  if (result >= 0) {
    offset = (uint32_t)result;
    return (char *)_uniform_buffer_ptr + result;
  }

  // It's full?  Just toss it at the end of this frame and create a fresh one.
  // Make the new one twice as large, to avoid this happening again.
  VulkanMemoryBlock block;
  VkDeviceSize new_capacity = _uniform_buffer_allocator.get_capacity() * 2;

  if (vulkandisplay_cat.is_debug()) {
    vulkandisplay_cat.debug()
      << "Changing global uniform buffer, ran out of space allocating "
      << size << " bytes, new capacity will be " << new_capacity << "\n";
  }

  if (!create_buffer(new_capacity, buffer, block,
                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                     (VkMemoryPropertyFlagBits)(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))) {
    // No?  Put it in GPU-accessible CPU memory, then.
    if (!create_buffer(new_capacity, buffer, block,
                       VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                       (VkMemoryPropertyFlagBits)(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))) {
      vulkandisplay_cat.error()
        << "Failed to create global uniform buffer.\n";
      return nullptr;
    }
  }

  void *ptr = block.map_persistent();
  if (ptr == nullptr) {
    vulkandisplay_cat.error()
      << "Failed to map global uniform buffer.\n";
    return nullptr;
  }

  _uniform_buffer_memory.unmap_persistent();

  {
    VulkanFrameData &frame_data = get_frame_data();
    frame_data._pending_free.push_back(std::move(_uniform_buffer_memory));
    frame_data._pending_destroy_buffers.push_back(_uniform_buffer);
  }

  _uniform_buffer = buffer;
  _uniform_buffer_memory = std::move(block);
  _uniform_buffer_ptr = ptr;

  _uniform_buffer_allocator.reset(new_capacity);

  // We have a new buffer now, so any frames that used the old buffer should
  // not try to free from the new buffer.
  for (FrameData &frame_data : _frame_data_pool) {
    frame_data._uniform_buffer_head = 0;
  }

  // Also add a white color to the end of the new allocation (aligning it only
  // to 16 bytes), since it is frequently used.
  _uniform_buffer_white_offset = (size + 15) & ~15;
  result = _uniform_buffer_allocator.alloc(_uniform_buffer_white_offset + 16);
  nassertr(result == 0, nullptr);
  offset = 0;
  *(LVecBase4f *)((char *)ptr + _uniform_buffer_white_offset) = LVecBase4f(1, 1, 1, 1);
  return ptr;
}

/**
 * Allocates memory in the staging buffer.  Note that the staging buffer may
 * only be used in this frame, since it will be cleaned up automatically when
 * the frame is done.
 */
void *VulkanGraphicsStateGuardian::
alloc_staging_buffer(VkDeviceSize size, VkBuffer &buffer, uint32_t &offset) {
  nassertr(size > 0, nullptr);

  // It won't fit (easily).  Create a fresh staging buffer just for this
  // resource, to be destroyed at the end of the frame.
  if (size * 2 > _staging_buffer_allocator.get_capacity()) {
    if (vulkandisplay_cat.is_debug()) {
      vulkandisplay_cat.debug()
        << "Creating dedicated staging buffer for size " << size << "\n";
    }
    VulkanMemoryBlock block;
    if (create_buffer(size, buffer, block,
                      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                      (VkMemoryPropertyFlagBits)(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))) {
      void *ptr = block.map_persistent();
      offset = 0;
      VulkanFrameData &frame_data = get_frame_data();
      frame_data._pending_free.push_back(std::move(block));
      frame_data._pending_destroy_buffers.push_back(buffer);
      return ptr;
    } else {
      return nullptr;
    }
  }

  ptrdiff_t result = _staging_buffer_allocator.alloc(size);
  if (result >= 0) {
    buffer = _staging_buffer;
    offset = (uint32_t)result;
    return (char *)_staging_buffer_ptr + result;
  }

  if (vulkandisplay_cat.is_debug()) {
    vulkandisplay_cat.debug()
      << "Changing staging buffer, ran out of space allocating "
      << size << " bytes\n";
  }

  // It's full?  Just toss it at the end of this frame and create a fresh one.
  VulkanMemoryBlock block;
  if (!create_buffer(_staging_buffer_allocator.get_capacity(),
                     buffer, block,
                     VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                     (VkMemoryPropertyFlagBits)(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))) {
    return nullptr;
  }

  void *ptr = block.map_persistent();
  if (ptr == nullptr) {
    vulkandisplay_cat.error()
      << "Failed to map staging buffer.\n";
    return nullptr;
  }

  _staging_buffer_memory.unmap_persistent();

  VulkanFrameData &frame_data = get_frame_data();
  frame_data._pending_free.push_back(std::move(_staging_buffer_memory));
  frame_data._pending_destroy_buffers.push_back(_staging_buffer);

  _staging_buffer = buffer;
  _staging_buffer_memory = std::move(block);
  _staging_buffer_ptr = ptr;

  _staging_buffer_allocator.reset();

  // We have a new buffer now, so any frames that used the old buffer should
  // not try to free from the new buffer.
  for (FrameData &frame_data : _frame_data_pool) {
    frame_data._staging_buffer_head = 0;
  }

  result = _staging_buffer_allocator.alloc(size);
  nassertr(result == 0, nullptr);
  offset = 0;
  return (char *)_staging_buffer_ptr;
}

/**
 * Returns a VkFormat suitable for the given texture.
 */
VkFormat VulkanGraphicsStateGuardian::
get_image_format(const Texture *texture) const {
  Texture::Format format = texture->get_format();
  Texture::ComponentType component_type = texture->get_component_type();
  Texture::CompressionMode compression = texture->get_compression();
  if (compression == Texture::CM_default) {
    compression = compressed_textures ? Texture::CM_on : Texture::CM_off;
  }

  if (!_supports_compressed_texture) {
    compression = Texture::CM_off;
  }
  if (compression != Texture::CM_on && compression != Texture::CM_off &&
      !_compressed_texture_formats.get_bit(compression)) {
    compression = Texture::CM_off;
  }

  VulkanGraphicsPipe *vkpipe;
  DCAST_INTO_R(vkpipe, get_pipe(), VK_FORMAT_UNDEFINED);
  const VkPhysicalDeviceFeatures &features = vkpipe->_gpu_features;

  bool is_signed = !Texture::is_unsigned(component_type);
  bool is_srgb = Texture::is_srgb(format);

  switch (compression) {
  case Texture::CM_on:
    // Select an appropriate compression mode automatically.
    if (features.textureCompressionBC) {
      if (!is_srgb && texture->get_num_components() == 1) {
        return (VkFormat)(VK_FORMAT_BC4_UNORM_BLOCK + is_signed);

      } else if (!is_srgb && texture->get_num_components() == 2) {
        return (VkFormat)(VK_FORMAT_BC5_UNORM_BLOCK + is_signed);

      } else if (Texture::has_binary_alpha(format)) {
        return (VkFormat)(VK_FORMAT_BC1_RGBA_UNORM_BLOCK + is_srgb);

      } else if (format == Texture::F_rgba4 || format == Texture::F_rgb10_a2) {
        return (VkFormat)(VK_FORMAT_BC2_UNORM_BLOCK + is_srgb);

      } else if (Texture::has_alpha(format)) {
        return (VkFormat)(VK_FORMAT_BC3_UNORM_BLOCK + is_srgb);

      } else {
        return (VkFormat)(VK_FORMAT_BC1_RGB_UNORM_BLOCK + is_srgb);
      }
    }
    if (!features.textureCompressionETC2) {
      break;
    }
    // Fall through
  case Texture::CM_etc1:
  case Texture::CM_etc2:
    if (!Texture::has_alpha(format)) {
      return (VkFormat)(VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK + is_srgb);
    } else if (Texture::has_binary_alpha(format)) {
      return (VkFormat)(VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK + is_srgb);
    } else {
      return (VkFormat)(VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK + is_srgb);
    }
    break;

  case Texture::CM_dxt1:
    if (Texture::has_alpha(format)) {
      return (VkFormat)(VK_FORMAT_BC1_RGBA_UNORM_BLOCK + is_srgb);
    } else {
      return (VkFormat)(VK_FORMAT_BC1_RGB_UNORM_BLOCK + is_srgb);
    }
  case Texture::CM_dxt3:
    return (VkFormat)(VK_FORMAT_BC2_UNORM_BLOCK + is_srgb);

  case Texture::CM_dxt5:
    return (VkFormat)(VK_FORMAT_BC3_UNORM_BLOCK + is_srgb);

  case Texture::CM_rgtc:
    if (texture->get_num_components() == 1) {
      return (VkFormat)(VK_FORMAT_BC4_UNORM_BLOCK + is_signed);
    } else {
      return (VkFormat)(VK_FORMAT_BC5_UNORM_BLOCK + is_signed);
    }

  case Texture::CM_eac:
    if (texture->get_num_components() == 1) {
      return (VkFormat)(VK_FORMAT_EAC_R11_UNORM_BLOCK + is_signed);
    } else {
      return (VkFormat)(VK_FORMAT_EAC_R11G11_UNORM_BLOCK + is_signed);
    }
    break;

  default:
    // Compression mode not supported.
    break;
  }

  switch (format) {
  case Texture::F_depth_stencil:
    return VK_FORMAT_D24_UNORM_S8_UINT;
  case Texture::F_color_index:
    return VK_FORMAT_R8_UINT;
  case Texture::F_red:
  case Texture::F_green:
  case Texture::F_blue:
  case Texture::F_alpha:
    return (VkFormat)(VK_FORMAT_R8_UNORM + is_signed);
  case Texture::F_rgb:
    return (VkFormat)(VK_FORMAT_B8G8R8_UNORM + is_signed);
  case Texture::F_rgb5:
    return VK_FORMAT_B5G6R5_UNORM_PACK16;
  case Texture::F_rgb8:
    return (VkFormat)(VK_FORMAT_B8G8R8_UNORM + is_signed);
  case Texture::F_rgb12:
    return (VkFormat)(VK_FORMAT_R16G16B16_UNORM + is_signed);
  case Texture::F_rgb332:
    return VK_FORMAT_B5G6R5_UNORM_PACK16;
  case Texture::F_rgba:
    return (VkFormat)(VK_FORMAT_B8G8R8A8_UNORM + is_signed);
  case Texture::F_rgbm:
    return (VkFormat)(VK_FORMAT_B8G8R8A8_UNORM + is_signed);
  case Texture::F_rgba4:
    return VK_FORMAT_R4G4B4A4_UNORM_PACK16;
  case Texture::F_rgba5:
    return VK_FORMAT_R5G5B5A1_UNORM_PACK16;
  case Texture::F_rgba8:
    return (VkFormat)(VK_FORMAT_B8G8R8A8_UNORM + is_signed);
  case Texture::F_rgba12:
    return (VkFormat)(VK_FORMAT_R16G16B16A16_UNORM + is_signed);
  case Texture::F_luminance:
    return (VkFormat)(VK_FORMAT_R8_UNORM + is_signed);
  case Texture::F_luminance_alpha:
    return (VkFormat)(VK_FORMAT_R8G8_UNORM + is_signed);
  case Texture::F_luminance_alphamask:
    return (VkFormat)(VK_FORMAT_R8G8_UNORM + is_signed);
  case Texture::F_rgba16:
    return (VkFormat)(VK_FORMAT_R16G16B16A16_UNORM + is_signed);
  case Texture::F_rgba32:
    return VK_FORMAT_R32G32B32A32_SFLOAT;
  case Texture::F_depth_component:
    return (VkFormat)(VK_FORMAT_D16_UNORM + is_signed);
  case Texture::F_depth_component16:
    return (VkFormat)(VK_FORMAT_D16_UNORM + is_signed);
  case Texture::F_depth_component24:
    return (VkFormat)(VK_FORMAT_X8_D24_UNORM_PACK32 + is_signed);
  case Texture::F_depth_component32:
    return VK_FORMAT_D32_SFLOAT;
  case Texture::F_r16:
    return (VkFormat)(VK_FORMAT_R16_UNORM + is_signed);
  case Texture::F_rg16:
    return (VkFormat)(VK_FORMAT_R16G16_UNORM + is_signed);
  case Texture::F_rgb16:
    return (VkFormat)(VK_FORMAT_R16G16B16_UNORM + is_signed);
  case Texture::F_srgb:
    return VK_FORMAT_B8G8R8A8_SRGB;
  case Texture::F_srgb_alpha:
    return VK_FORMAT_B8G8R8A8_SRGB;
  case Texture::F_sluminance:
    return VK_FORMAT_R8_SRGB;
  case Texture::F_sluminance_alpha:
    return VK_FORMAT_B8G8R8A8_SRGB;
  case Texture::F_r32i:
    return (VkFormat)(VK_FORMAT_R32_UINT + is_signed);
  case Texture::F_r32:
    return VK_FORMAT_R32_SFLOAT;
  case Texture::F_rg32:
    return VK_FORMAT_R32G32_SFLOAT;
  case Texture::F_rgb32:
    return VK_FORMAT_R32G32B32_SFLOAT;
  case Texture::F_r8i:
    return (VkFormat)(VK_FORMAT_R8_UINT + is_signed);
  case Texture::F_rg8i:
    return (VkFormat)(VK_FORMAT_R8G8_UINT + is_signed);
  case Texture::F_rgb8i:
    //XXX not flipped in OpenGL, so not flipped here
    return (VkFormat)(VK_FORMAT_R8G8B8_UINT + is_signed);
  case Texture::F_rgba8i:
    //XXX not flipped in OpenGL, so not flipped here
    return (VkFormat)(VK_FORMAT_R8G8B8A8_UINT + is_signed);
  case Texture::F_r11_g11_b10:
    return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
  case Texture::F_rgb9_e5:
    return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
  case Texture::F_rgb10_a2:
    return (VkFormat)(VK_FORMAT_A2R10G10B10_UNORM_PACK32 + is_signed);
  case Texture::F_rg:
    return (VkFormat)(VK_FORMAT_R8G8_UNORM + is_signed);
  case Texture::F_r16i:
    return (VkFormat)(VK_FORMAT_R16_UINT + is_signed);
  case Texture::F_rg16i:
    return (VkFormat)(VK_FORMAT_R16G16_UINT + is_signed);
  case Texture::F_rgb16i:
    return (VkFormat)(VK_FORMAT_R16G16B16_UINT + is_signed);
  case Texture::F_rgba16i:
    return (VkFormat)(VK_FORMAT_R16G16B16A16_UINT + is_signed);
  case Texture::F_rg32i:
    return (VkFormat)(VK_FORMAT_R32G32_UINT + is_signed);
  case Texture::F_rgb32i:
    return (VkFormat)(VK_FORMAT_R32G32B32_UINT + is_signed);
  case Texture::F_rgba32i:
    return (VkFormat)(VK_FORMAT_R32G32B32A32_UINT + is_signed);
  }

  return VK_FORMAT_UNDEFINED;
}

/**
 * Does the inverse of get_image_format.  If the VkFormat has a corresponding
 * Texture representation, sets those and returns true, otherwise it returns
 * false and leaves the arguments untouched.
 */
bool VulkanGraphicsStateGuardian::
lookup_image_format(VkFormat vk_format, Texture::Format &format,
                    Texture::ComponentType &type) {
  switch (vk_format) {
  case VK_FORMAT_R8_UNORM:
    type = Texture::T_unsigned_byte;
    format = Texture::F_red;
    break;
  case VK_FORMAT_R8G8_UNORM:
    type = Texture::T_unsigned_byte;
    format = Texture::F_rg;
    break;
  case VK_FORMAT_R16_SFLOAT:
    type = Texture::T_float;
    format = Texture::F_r16;
    break;
  case VK_FORMAT_R8G8B8_UNORM:
  case VK_FORMAT_B8G8R8_UNORM:
    type = Texture::T_unsigned_byte;
    format = Texture::F_rgb8;
    break;
  case VK_FORMAT_R8G8B8_SRGB:
  case VK_FORMAT_B8G8R8_SRGB:
    type = Texture::T_unsigned_byte;
    format = Texture::F_srgb;
    break;
  case VK_FORMAT_R8G8B8A8_UNORM:
  case VK_FORMAT_B8G8R8A8_UNORM:
    type = Texture::T_unsigned_byte;
    format = Texture::F_rgba8;
    break;
  case VK_FORMAT_R8G8B8A8_SRGB:
  case VK_FORMAT_B8G8R8A8_SRGB:
    type = Texture::T_unsigned_byte;
    format = Texture::F_srgb_alpha;
    break;
  case VK_FORMAT_R16G16_SFLOAT:
    type = Texture::T_float;
    format = Texture::F_rg16;
    break;
  case VK_FORMAT_R16G16B16A16_SFLOAT:
    type = Texture::T_float;
    format = Texture::F_rgba16;
    break;
  case VK_FORMAT_R32_SFLOAT:
    type = Texture::T_float;
    format = Texture::F_r32;
    break;
  case VK_FORMAT_R32G32_SFLOAT:
    type = Texture::T_float;
    format = Texture::F_rg32;
    break;
  case VK_FORMAT_R32G32B32A32_SFLOAT:
    type = Texture::T_float;
    format = Texture::F_rgba32;
    break;
  case VK_FORMAT_D32_SFLOAT_S8_UINT:
    type = Texture::T_unsigned_int;
    format = Texture::F_depth_stencil;
    break;
  case VK_FORMAT_D24_UNORM_S8_UINT:
    type = Texture::T_unsigned_int_24_8;
    format = Texture::F_depth_stencil;
    break;
  case VK_FORMAT_D32_SFLOAT:
    type = Texture::T_float;
    format = Texture::F_depth_component32;
    break;
  case VK_FORMAT_X8_D24_UNORM_PACK32:
    type = Texture::T_unsigned_int_24_8;
    format = Texture::F_depth_component24;
    break;
  default:
    return false;
  }
  return true;
}
