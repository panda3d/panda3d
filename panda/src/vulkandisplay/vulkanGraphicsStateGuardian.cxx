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
#include "vulkanVertexBufferContext.h"
#include "graphicsEngine.h"
#include "pStatTimer.h"
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

TypeHandle VulkanGraphicsStateGuardian::_type_handle;

/**
 * Creates a Vulkan device and queue.
 */
VulkanGraphicsStateGuardian::
VulkanGraphicsStateGuardian(GraphicsEngine *engine, VulkanGraphicsPipe *pipe,
                            VulkanGraphicsStateGuardian *share_with,
                            uint32_t queue_family_index) :
  GraphicsStateGuardian(CS_default, engine, pipe),
  _device(VK_NULL_HANDLE),
  _queue(VK_NULL_HANDLE),
  _dma_queue(VK_NULL_HANDLE),
  _graphics_queue_family_index(queue_family_index),
  _cmd_pool(VK_NULL_HANDLE),
  _render_pass(VK_NULL_HANDLE),
  _wait_semaphore(VK_NULL_HANDLE),
  _signal_semaphore(VK_NULL_HANDLE),
  _pipeline_cache(VK_NULL_HANDLE),
  _default_sc(nullptr),
  _total_allocated(0)
{
  const VkPhysicalDeviceLimits &limits = pipe->_gpu_properties.limits;
  const VkPhysicalDeviceFeatures &features = pipe->_gpu_features;

  const char *const layers[] = {
    "VK_LAYER_LUNARG_standard_validation",
  };

  std::vector<const char*> extensions;
  if (pipe->has_device_extension("VK_KHR_swapchain")) {
    extensions.push_back("VK_KHR_swapchain");
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
  device_info.enabledLayerCount = 1;
  device_info.ppEnabledLayerNames = layers;
  device_info.enabledExtensionCount = extensions.size();
  device_info.ppEnabledExtensionNames = &extensions[0];
  device_info.pEnabledFeatures = nullptr;

  VkResult
  err = vkCreateDevice(pipe->_gpu, &device_info, nullptr, &_device);
  if (err) {
    vulkan_error(err, "Failed to create device");
    return;
  }

  vkGetDeviceQueue(_device, _graphics_queue_family_index, 0, &_queue);
  if (queue_info.queueCount > 1) {
    vkGetDeviceQueue(_device, _graphics_queue_family_index, 1, &_dma_queue);
  } else {
    _dma_queue = _queue;
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
  const uint32_t num_command_buffers = 2 * _frame_data_capacity;
  VkCommandBufferAllocateInfo alloc_info;
  alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info.pNext = nullptr;
  alloc_info.commandPool = _cmd_pool;
  alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  alloc_info.commandBufferCount = num_command_buffers;

  VkCommandBuffer buffers[num_command_buffers];
  err = vkAllocateCommandBuffers(_device, &alloc_info, buffers);
  nassertv(!err);

  size_t ci = 0;
  for (FrameData &frame_data : _frame_data_pool) {
    frame_data._cmd = buffers[ci++];
    frame_data._transfer_cmd = buffers[ci++];

    // Create a fence to signal when this frame has finished.
    VkFenceCreateInfo fence_info;
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.pNext = nullptr;
    fence_info.flags = 0;
    err = vkCreateFence(_device, &fence_info, nullptr, &frame_data._fence);
    if (err) {
      vulkan_error(err, "Failed to create fence");
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
  VkDescriptorPoolSize pool_size;
  pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  pool_size.descriptorCount = 1024;

  VkDescriptorPoolCreateInfo pool_info;
  pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_info.pNext = nullptr;
  pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  pool_info.maxSets = 1024;
  pool_info.poolSizeCount = 1;
  pool_info.pPoolSizes = &pool_size;

  err = vkCreateDescriptorPool(_device, &pool_info, nullptr, &_descriptor_pool);
  if (err) {
    vulkan_error(err, "Failed to create descriptor pool");
    return;
  }

  // Create a dummy vertex buffer.  This will be used to store default values
  // for attributes when they are not bound to a vertex buffer, as well as any
  // flat color assigned via ColorAttrib.
  uint32_t palette_size = (uint32_t)std::max(2, vulkan_color_palette_size.get_value()) * 16;
  if (!create_buffer(palette_size, _color_vertex_buffer, _color_vertex_memory,
                     VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                     VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
    vulkandisplay_cat.error()
      << "Failed to create null vertex buffer.\n";
    return;
  }

  // The first two are reserved for opaque black and white, respectively.
  _color_palette[LColorf(0, 0, 0, 1)] = 0;
  _color_palette[LColorf(1, 1, 1, 1)] = 1;
  _next_palette_index = 2;

  // Create a push constant layout based on the available space.
  {
    ShaderType::Struct struct_type;
    struct_type.add_member(ShaderType::register_type(ShaderType::Matrix(ShaderType::ST_float, 4, 4)), "p3d_ModelViewProjectionMatrix");
    struct_type.add_member(ShaderType::register_type(ShaderType::Vector(ShaderType::ST_float, 4)), "p3d_ColorScale");
    _push_constant_block_type = ShaderType::register_type(std::move(struct_type));
  }

  // Create a descriptor set layout for our LightAttrib descriptor set.
  const uint32_t num_shadow_maps = 16;
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

    VkDescriptorSetLayoutBinding shadow_bindings[num_shadow_maps];

    for (uint32_t i = 0; i < num_shadow_maps; ++i) {
      VkDescriptorSetLayoutBinding &binding = shadow_bindings[i];
      binding.binding = i;
      binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      binding.descriptorCount = 1;
      binding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
      binding.pImmutableSamplers = &_shadow_sampler;
    }

    VkDescriptorSetLayoutCreateInfo set_info;
    set_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    set_info.pNext = nullptr;
    set_info.flags = 0;
    set_info.bindingCount = num_shadow_maps;
    set_info.pBindings = shadow_bindings;

    err = vkCreateDescriptorSetLayout(_device, &set_info, nullptr,
      &_lattr_descriptor_set_layout);
    if (err) {
      vulkan_error(err, "Failed to create descriptor set layout for LightAttrib");
      return;
    }
  }

  // Create a descriptor set layout for our TextureAttrib descriptor set.
  const uint32_t num_texture_stages = 16;
  {
    VkDescriptorSetLayoutBinding stage_bindings[num_texture_stages];

    for (uint32_t i = 0; i < num_texture_stages; ++i) {
      VkDescriptorSetLayoutBinding &binding = stage_bindings[i];
      binding.binding = i;
      binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      binding.descriptorCount = 1;
      binding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
      binding.pImmutableSamplers = nullptr;
    }

    VkDescriptorSetLayoutCreateInfo set_info;
    set_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    set_info.pNext = nullptr;
    set_info.flags = 0;
    set_info.bindingCount = num_texture_stages;
    set_info.pBindings = stage_bindings;

    VkResult
    err = vkCreateDescriptorSetLayout(_device, &set_info, nullptr,
      &_tattr_descriptor_set_layout);
    if (err) {
      vulkan_error(err, "Failed to create descriptor set layout for TextureAttrib");
      return;
    }
  }

  // Create a uniform buffer that we'll use for everything.
  // Some cards set aside 256 MiB of device-local host-visible memory for data
  // like this, so we use that.
  VkDeviceSize uniform_buffer_size = vulkan_global_uniform_buffer_size;
  if (!create_buffer(uniform_buffer_size, _uniform_buffer, _uniform_buffer_memory,
                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     (VkMemoryPropertyFlagBits)(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))) {
    // No?  Put it in GPU-accessible CPU memory, then.
    if (!create_buffer(uniform_buffer_size, _uniform_buffer, _uniform_buffer_memory,
                       VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
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

  // Fill in the features supported by this physical device.
  _is_hardware = (pipe->_gpu_properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_CPU);

  _max_vertices_per_array = std::max((uint32_t)0x7fffffff, limits.maxDrawIndexedIndexValue);
  _max_vertices_per_primitive = INT_MAX;

  _max_texture_stages = num_texture_stages;
  _max_texture_dimension = limits.maxImageDimension2D;
  _max_3d_texture_dimension = limits.maxImageDimension3D;
  _max_2d_texture_array_layers = limits.maxImageArrayLayers;
  _max_cube_map_dimension = limits.maxImageDimensionCube;
  _max_buffer_texture_size = limits.maxTexelBufferElements;

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
  _supports_timer_query = false;

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
    ShaderModule::C_basic_shader |
    ShaderModule::C_vertex_texture |
    ShaderModule::C_sampler_shadow |
    ShaderModule::C_invariant |
    ShaderModule::C_matrix_non_square |
    ShaderModule::C_integer |
    ShaderModule::C_texture_lod |
    ShaderModule::C_texture_fetch |
    ShaderModule::C_sampler_cube_shadow |
    ShaderModule::C_vertex_id |
    ShaderModule::C_round_even |
    ShaderModule::C_instance_id |
    ShaderModule::C_buffer_texture |
    ShaderModule::C_bit_encoding |
    ShaderModule::C_texture_gather |
    ShaderModule::C_extended_arithmetic |
    ShaderModule::C_texture_query_lod |
    ShaderModule::C_compute_shader |
    ShaderModule::C_texture_query_levels |
    ShaderModule::C_enhanced_layouts |
    ShaderModule::C_derivative_control |
    ShaderModule::C_texture_query_samples;

  if (features.geometryShader) {
    _supported_shader_caps |=
      ShaderModule::C_geometry_shader |
      ShaderModule::C_primitive_id;
  }

  if (features.tessellationShader) {
    _supported_shader_caps |= ShaderModule::C_tessellation_shader;
  }

  if (features.shaderFloat64) {
    _supported_shader_caps |= ShaderModule::C_double;
  }

  if (features.imageCubeArray) {
    _supported_shader_caps |= ShaderModule::C_cube_map_array;
  }

  if (features.sampleRateShading) {
    _supported_shader_caps |= ShaderModule::C_sample_variables;
  }

  if (features.vertexPipelineStoresAndAtomics && features.fragmentStoresAndAtomics) {
    _supported_shader_caps |= ShaderModule::C_image_load_store;
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
 *
 */
VulkanGraphicsStateGuardian::
~VulkanGraphicsStateGuardian() {
  if (_device == VK_NULL_HANDLE) {
    nassertv(_memory_pages.empty());
    return;
  }

  // And all the semaphores that were generated on this device.
  for (VkSemaphore semaphore : _semaphores) {
    vkDestroySemaphore(_device, semaphore, nullptr);
  }

  // Remove the things we created in the constructor, in reverse order.
  vkDestroyBuffer(_device, _uniform_buffer, nullptr);
  vkDestroyBuffer(_device, _color_vertex_buffer, nullptr);
  vkDestroyDescriptorSetLayout(_device, _lattr_descriptor_set_layout, nullptr);
  vkDestroyDescriptorSetLayout(_device, _tattr_descriptor_set_layout, nullptr);
  vkDestroyDescriptorPool(_device, _descriptor_pool, nullptr);
  vkDestroySampler(_device, _shadow_sampler, nullptr);
  vkDestroyPipelineCache(_device, _pipeline_cache, nullptr);
  vkDestroyCommandPool(_device, _cmd_pool, nullptr);

  for (FrameData &frame_data : _frame_data_pool) {
    vkDestroyFence(_device, frame_data._fence, nullptr);
  }

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
  nassertv(_frame_data == nullptr);

  // Make sure it's no longer doing anything before we destroy it.
  vkDeviceWaitIdle(_device);

  // Call finish_frame() on all frames.  We don't need to wait on the fences due
  // to the above call.
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
prepare_texture(Texture *texture, int view) {
  using std::swap;

  PStatTimer timer(_prepare_texture_pcollector);

  VulkanGraphicsPipe *vkpipe;
  DCAST_INTO_R(vkpipe, get_pipe(), nullptr);

  VkImageCreateFlags flags = 0;
  VkImageType type;
  VkExtent3D extent;
  extent.width = texture->get_x_size();
  extent.height = texture->get_y_size();
  extent.depth = 1;
  uint32_t num_layers = 1;
  uint32_t num_levels = 1;
  bool is_buffer = false;

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
    is_buffer = true;
    break;
  }
  const VkExtent3D orig_extent = extent;

  // Check if the format is actually supported.
  VkFormat format = get_image_format(texture);
  VkFormatProperties fmt_props;
  vkGetPhysicalDeviceFormatProperties(vkpipe->_gpu, format, &fmt_props);

  bool supported;
  if (is_buffer) {
    supported = (fmt_props.bufferFeatures & VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT) != 0;
  } else {
    supported = (fmt_props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) != 0;
  }

  bool pack_bgr8 = false;
  if (!supported) {
    // Not supported.  Can we convert it to a format that is supported?  The two
    // we pick have mandatory support in Vulkan, so no need to check things.
    switch (format) {
    case VK_FORMAT_B8G8R8_UNORM:
      format = VK_FORMAT_R8G8B8A8_UNORM;
      pack_bgr8 = true;
      break;

    case VK_FORMAT_B8G8R8_UINT:
      format = VK_FORMAT_R8G8B8A8_UINT;
      pack_bgr8 = true;
      break;

    default:
      vulkandisplay_cat.error()
        << "Texture format " << format << " not supported.\n";
      return nullptr;
    }

    // Update the properties for the new format.
    vkGetPhysicalDeviceFormatProperties(vkpipe->_gpu, format, &fmt_props);
  }

  VulkanTextureContext *tc;
  if (!is_buffer) {
    // Image texture.  Is the size supported for this format?
    VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (fmt_props.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT) {
      usage |= VK_IMAGE_USAGE_STORAGE_BIT;
    }

    VkImageFormatProperties img_props;
    vkGetPhysicalDeviceImageFormatProperties(vkpipe->_gpu, format, type,
                                             VK_IMAGE_TILING_OPTIMAL, usage,
                                             flags, &img_props);
    if (num_layers > img_props.maxArrayLayers) {
      //TODO: more elegant solution to reduce layer count.
      vulkandisplay_cat.error()
        << "Texture has too many layers, this format has a maximum of "
        << num_layers << "\n";
      return nullptr;
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

      if (!texture->has_ram_mipmap_image(mipmap_begin)) {
        // Ugh, and to do this, we have to generate mipmaps on the CPU.
        texture->generate_ram_mipmap_images();
      }
    }

    int mipmap_end = mipmap_begin + 1;
    if (texture->uses_mipmaps()) {
      mipmap_end = texture->get_expected_num_mipmap_levels();
      nassertr(mipmap_end > mipmap_begin, nullptr);
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
        texture->generate_ram_mipmap_images();

        // We now have as many levels as we're going to get.
        mipmap_end = texture->get_num_ram_mipmap_images();
        generate_mipmaps = false;
      } else {
        // We'll be generating mipmaps from it, so mark it as transfer source.
        usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
      }
    }

    num_levels = mipmap_end - mipmap_begin;
    if (num_levels > img_props.maxMipLevels) {
      mipmap_end -= num_levels - img_props.maxMipLevels;
      num_levels = img_props.maxMipLevels;
    }

    tc = create_image(type, format, extent, num_levels, num_layers,
                      VK_SAMPLE_COUNT_1_BIT, usage, flags);
    nassertr_always(tc != nullptr, nullptr);
    tc->set_texture(texture);
    tc->_mipmap_begin = mipmap_begin;
    tc->_mipmap_end = mipmap_end;
    tc->_generate_mipmaps = generate_mipmaps;
    tc->_pack_bgr8 = pack_bgr8;

    Texture::Format tex_format = texture->get_format();
    if (tex_format == Texture::F_depth_stencil ||
        tex_format == Texture::F_depth_component ||
        tex_format == Texture::F_depth_component16 ||
        tex_format == Texture::F_depth_component24 ||
        tex_format == Texture::F_depth_component32) {
      tc->_aspect_mask = VK_IMAGE_ASPECT_DEPTH_BIT;
    } else {
      tc->_aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT;
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
    view_info.subresourceRange.layerCount = num_layers;

    VkResult err;
    err = vkCreateImageView(_device, &view_info, nullptr, &tc->_image_view);
    if (err) {
      vulkan_error(err, "Failed to create image view for texture");
      vkDestroyImage(_device, tc->_image, nullptr);
      delete tc;
      return nullptr;
    }

    if (vulkandisplay_cat.is_debug()) {
      vulkandisplay_cat.debug()
        << "Created image " << tc->_image << " and view " << tc->_image_view
        << " for texture " << *texture << "\n";
    }
  }
  else {
    // Buffer texture.
    if (extent.width > (uint32_t)_max_buffer_texture_size) {
      vulkandisplay_cat.error()
        << "Buffer texture size " << extent.width << " is too large, maximum size is "
        << _max_buffer_texture_size << " texels\n";
      return nullptr;
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
    VkDeviceSize size = texture->get_expected_ram_image_size();
    if (pack_bgr8) {
      size = size / 3 * 4;
    }
    if (!create_buffer(size, buffer, block, usage,
                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
      return nullptr;
    }

    VkBufferViewCreateInfo view_info;
    view_info.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
    view_info.pNext = nullptr;
    view_info.flags = 0;
    view_info.buffer = buffer;
    view_info.format = format;
    view_info.offset = 0;
    view_info.range = VK_WHOLE_SIZE;

    VkBufferView buffer_view;
    VkResult err;
    err = vkCreateBufferView(_device, &view_info, nullptr, &buffer_view);
    if (err) {
      vulkan_error(err, "Failed to create buffer view for texture");
      return nullptr;
    }

    if (vulkandisplay_cat.is_debug()) {
      vulkandisplay_cat.debug()
        << "Created buffer " << buffer << " and view " << buffer_view
        << " for texture " << *texture << "\n";
    }

    tc = new VulkanTextureContext(get_prepared_objects(), texture, view);
    tc->_format = format;
    tc->_extent = extent;
    tc->_buffer = buffer;
    tc->_buffer_view = buffer_view;
    tc->_block = std::move(block);
    tc->_pack_bgr8 = pack_bgr8;
  }

  // We can't upload it at this point because the texture lock is currently
  // held, so accessing the RAM image will cause a deadlock.
  return tc;
}

/**
 * Uploads the texture data for the given texture.
 */
bool VulkanGraphicsStateGuardian::
upload_texture(VulkanTextureContext *tc) {
  nassertr(_frame_data != nullptr, false);

  Texture *texture = tc->get_texture();
  VkImage image = tc->_image;

  //TODO: check if the image is currently in use on a different queue, and if
  // so, use a semaphore to control order or create a new image and discard
  // the old one.

  VulkanGraphicsPipe *vkpipe;
  DCAST_INTO_R(vkpipe, get_pipe(), false);

  // Do we even have an image to upload?
  if (texture->get_ram_image().is_null()) {
    if (texture->has_clear_color()) {
      // No, but we have to clear it to a solid color.
      LColor col = texture->get_clear_color();
      if (tc->_buffer != VK_NULL_HANDLE) {
        // Vulkan can only clear buffers using a multiple of 4 bytes.  If it's
        // all zeroes or texels are 4 bytes, it's easy.
        if (col == LColor::zero()) {
          tc->clear_buffer(_frame_data->_transfer_cmd, 0);
          return true;
        }
        vector_uchar data = texture->get_clear_data();
        if (data.size() == sizeof(uint32_t)) {
          tc->clear_buffer(_frame_data->_transfer_cmd, *(uint32_t *)&data[0]);
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
        tc->clear_color_image(_frame_data->_transfer_cmd, value);
        return true;
      }
      else {
        VkClearDepthStencilValue value;
        value.depth = col[0];
        value.stencil = 0;
        tc->clear_depth_stencil_image(_frame_data->_transfer_cmd, value);
        return true;
      }
    } else {
      // No clear color means we get an uninitialized image.
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
  VulkanMemoryBlock block;
  if (!create_buffer(buffer_size, buffer, block,
                     VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
    vulkandisplay_cat.error()
      << "Failed to create staging buffer for texture "
      << texture->get_name() << std::endl;
    return false;
  }

  // Now fill in the data into the staging buffer.
  auto data = block.map();
  if (!data) {
    return false;
  }

  if (tc->_image != VK_NULL_HANDLE) {
    // Issue a command to transition the image into a layout optimal for
    // transferring into.
    tc->transition(_frame_data->_transfer_cmd, _graphics_queue_family_index,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT);

    // Schedule a copy from our staging buffer to the image.
    VkBufferImageCopy region = {};
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
          vkCmdPipelineBarrier(_frame_data->_transfer_cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
                               VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                               0, nullptr, 0, nullptr, 1, &barrier);
          levels_in_dst_optimal_layout.clear_bit(barrier.subresourceRange.baseMipLevel);

          blit.dstSubresource.mipLevel = region.imageSubresource.mipLevel;
          blit.dstOffsets[1].x = region.imageExtent.width;
          blit.dstOffsets[1].y = region.imageExtent.height;
          blit.dstOffsets[1].z = region.imageExtent.depth;
          vkCmdBlitImage(_frame_data->_transfer_cmd, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
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
        nassertr(region.bufferOffset + src_size <= block.get_size(), false);

        uint8_t *dest = (uint8_t *)data + region.bufferOffset;

        if (tc->_pack_bgr8) {
          // Pack RGB data into RGBA, since most cards don't support RGB8.
          const uint8_t *src_end = src + src_size;
          uint32_t *dest32 = (uint32_t *)dest;
          nassertr(((uintptr_t)dest32 & 0x3) == 0, false);

          for (; src < src_end; src += 3) {
            *dest32++ = 0xff000000 | (src[0] << 16) | (src[1] << 8) | src[2];
          }
          src_size = src_size / 3 * 4;
        } else {
          memcpy(dest, src, src_size);
        }

        // Schedule a copy from the staging buffer.
        vkCmdCopyBufferToImage(_frame_data->_transfer_cmd, buffer, image,
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

    data.unmap();

    // Now, ensure that all the mipmap levels are in the same layout.
    if (!levels_in_dst_optimal_layout.has_all_of(0, tc->_mip_levels)) {
      for (size_t ri = 0; ri < levels_in_dst_optimal_layout.get_num_subranges(); ++ri) {
        barrier.subresourceRange.baseMipLevel = levels_in_dst_optimal_layout.get_subrange_begin(ri);
        barrier.subresourceRange.levelCount = levels_in_dst_optimal_layout.get_subrange_end(ri)
                                            - levels_in_dst_optimal_layout.get_subrange_begin(ri);

        vkCmdPipelineBarrier(_frame_data->_transfer_cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
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
      const uint8_t *src_end = src + src_size;
      uint32_t *dest32 = (uint32_t *)data;
      nassertr(((uintptr_t)dest32 & 0x3) == 0, false);

      for (; src < src_end; src += 3) {
        *dest32++ = 0xff000000 | (src[0] << 16) | (src[1] << 8) | src[2];
      }
      src_size = src_size / 3 * 4;
    } else {
      memcpy(data, src, src_size);
    }
    data.unmap();

    VkBufferCopy region;
    region.srcOffset = 0;
    region.dstOffset = 0;
    region.size = src_size;
    vkCmdCopyBuffer(_frame_data->_transfer_cmd, buffer, tc->_buffer, 1, &region);
  }

  tc->mark_loaded();

  // Make sure that the staging memory is not deleted until the next fence.
  _frame_data->_pending_free.push_back(std::move(block));
  _frame_data->_pending_destroy_buffers.push_back(buffer);

  // Tell the GraphicsEngine that we uploaded the texture.  This may cause
  // it to unload the data from RAM at the end of this frame.
  GraphicsEngine *engine = get_engine();
  if (engine != nullptr) {
    engine->texture_uploaded(texture);
  }

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
update_texture(TextureContext *tc, bool force) {
  VulkanTextureContext *vtc;
  DCAST_INTO_R(vtc, tc, false);

  if (vtc->was_modified()) {
    Texture *tex = tc->get_texture();

    VkExtent3D extent;
    extent.width = tex->get_x_size();
    extent.height = tex->get_y_size();
    uint32_t arrayLayers;

    if (tex->get_texture_type() == Texture::TT_3d_texture) {
      extent.depth = tex->get_z_size();
      arrayLayers = 1;
    } else if (tex->get_texture_type() == Texture::TT_1d_texture_array) {
      extent.height = 1;
      extent.depth = 1;
      arrayLayers = tex->get_y_size();
    } else {
      extent.depth = 1;
      arrayLayers = tex->get_z_size();
    }

    //VkFormat format = get_image_format(tex);

    if (//format != vtc->_format ||
        extent.width != vtc->_extent.width ||
        extent.height != vtc->_extent.height ||
        extent.depth != vtc->_extent.depth ||
        arrayLayers != vtc->_array_layers) {
      // We need to recreate the image entirely. TODO!
      std::cerr << "have to recreate image\n";
      return false;
    }

    if (!upload_texture(vtc)) {
      return false;
    }

    vtc->mark_loaded();
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
  nassertv(_frame_data != nullptr);

  VulkanTextureContext *vtc;
  DCAST_INTO_V(vtc, tc);

  if (vtc->_image != VK_NULL_HANDLE && vulkandisplay_cat.is_debug()) {
    vulkandisplay_cat.debug()
      << "Deleting image " << vtc->_image << " and view " << vtc->_image_view << "\n";
  }

  if (vtc->_image_view != VK_NULL_HANDLE) {
    _frame_data->_pending_destroy_image_views.push_back(vtc->_image_view);
  }
  if (vtc->_image != VK_NULL_HANDLE) {
    _frame_data->_pending_destroy_images.push_back(vtc->_image);
  }

  if (vtc->_buffer != VK_NULL_HANDLE && vulkandisplay_cat.is_debug()) {
    vulkandisplay_cat.debug()
      << "Deleting buffer " << vtc->_buffer << " and view " << vtc->_buffer_view << "\n";
  }

  if (vtc->_buffer_view != VK_NULL_HANDLE) {
    _frame_data->_pending_destroy_buffer_views.push_back(vtc->_buffer_view);
  }
  if (vtc->_buffer != VK_NULL_HANDLE) {
    _frame_data->_pending_destroy_buffers.push_back(vtc->_buffer);
  }

  // Make sure that the memory remains untouched until the frame is over.
  _frame_data->_pending_free.push_back(std::move(vtc->_block));

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
  nassertr(_frame_data != nullptr, false);

  bool success = true;

  // If we wanted to optimize this use-case, we could allocate a single buffer
  // to hold all texture views and copy that in one go.
  int num_views = tex->get_num_views();
  for (int view = 0; view < num_views; ++view) {
    VulkanTextureContext *tc;
    DCAST_INTO_R(tc, tex->prepare_now(view, get_prepared_objects(), this), false);

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

  VkSamplerAddressMode wrap_map[] = {VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                                     VK_SAMPLER_ADDRESS_MODE_REPEAT,
                                     VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
                                     VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE,
                                     VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
                                     VK_SAMPLER_ADDRESS_MODE_REPEAT};

  //TODO: support shadow filter and border color.
  VkSamplerCreateInfo sampler_info;
  sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  sampler_info.pNext = nullptr;
  sampler_info.flags = 0;
  sampler_info.magFilter = (VkFilter)(sampler.get_effective_magfilter() & 1);
  sampler_info.minFilter = (VkFilter)(sampler.get_effective_minfilter() & 1);
  sampler_info.mipmapMode = (VkSamplerMipmapMode)
    (sampler.get_effective_minfilter() == SamplerState::FT_nearest_mipmap_linear ||
     sampler.get_effective_minfilter() == SamplerState::FT_linear_mipmap_linear);
  sampler_info.addressModeU = wrap_map[sampler.get_wrap_u()];
  sampler_info.addressModeV = wrap_map[sampler.get_wrap_v()];
  sampler_info.addressModeW = wrap_map[sampler.get_wrap_w()];
  sampler_info.mipLodBias = sampler.get_lod_bias();
  sampler_info.anisotropyEnable = (sampler.get_effective_anisotropic_degree() > 1);
  sampler_info.maxAnisotropy = sampler.get_effective_anisotropic_degree();
  sampler_info.compareEnable = VK_FALSE;
  sampler_info.compareOp = VK_COMPARE_OP_NEVER;
  sampler_info.minLod = sampler.get_min_lod();
  sampler_info.maxLod = sampler.get_max_lod();
  sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
  sampler_info.unnormalizedCoordinates = VK_FALSE;

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
  nassertv(_frame_data != nullptr);

  VulkanSamplerContext *vsc;
  DCAST_INTO_V(vsc, sc);

  nassertv(vsc->_sampler != VK_NULL_HANDLE);
  _frame_data->_pending_destroy_samplers.push_back(vsc->_sampler);
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
  ds_layouts[DS_texture_attrib] = _tattr_descriptor_set_layout;
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
    // Create a descriptor set for the UBOs.
    VkDescriptorSetAllocateInfo alloc_info;
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.pNext = nullptr;
    alloc_info.descriptorPool = _descriptor_pool;
    alloc_info.descriptorSetCount = 1;
    alloc_info.pSetLayouts = &ds_layouts[DS_dynamic_uniforms];
    err = vkAllocateDescriptorSets(_device, &alloc_info, &sc->_uniform_descriptor_set);
    if (err) {
      vulkan_error(err, "Failed to allocate descriptor set for dynamic uniforms");
      delete sc;
      return nullptr;
    }

    // We set the offsets to 0, since we use dynamic offsets.
    size_t count = 0;
    VkDescriptorBufferInfo buffer_info[2];
    if (sc->_mat_block_size > 0) {
      buffer_info[count].buffer = _uniform_buffer;
      buffer_info[count].offset = 0;
      buffer_info[count].range = sc->_mat_block_size;
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

    vkDestroyDescriptorSetLayout(_device, ds_layouts[DS_dynamic_uniforms], nullptr);
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
  for (size_t i = 0; i <= (size_t)Shader::Stage::compute; ++i) {
    if (sc->_modules[i] != VK_NULL_HANDLE) {
      vkDestroyShaderModule(_device, sc->_modules[i], nullptr);
      sc->_modules[i] = VK_NULL_HANDLE;
    }
  }

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

  vkDestroyDescriptorSetLayout(_device, sc->_sattr_descriptor_set_layout, nullptr);
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

  //TODO: don't use host-visible memory, but copy from a staging buffer.
  VkBuffer buffer;
  VulkanMemoryBlock block;
  if (!create_buffer(data_size, buffer, block,
                     VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
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
  vbc->set_active(true);

  if (vbc->was_modified(reader)) {
    VkDeviceSize num_bytes = reader->get_data_size_bytes();
    if (num_bytes != 0) {
      const unsigned char *client_pointer = reader->get_read_pointer(force);
      if (client_pointer == nullptr) {
        return false;
      }

      if (auto data = vbc->_block.map()) {
        memcpy(data, client_pointer, num_bytes);

        _data_transferred_pcollector.add_level(num_bytes);
      } else {
        vulkandisplay_cat.error()
          << "Failed to map vertex buffer memory.\n";
        return false;
      }
    }

    vbc->mark_loaded(reader);
  }
  vbc->enqueue_lru(&_prepared_objects->_graphics_memory_lru);

  return true;
}

/**
 * Frees the resources previously allocated via a call to prepare_data(),
 * including deleting the VertexBufferContext itself, if necessary.
 */
void VulkanGraphicsStateGuardian::
release_vertex_buffer(VertexBufferContext *context) {
  nassertv(_frame_data != nullptr);

  VulkanVertexBufferContext *vbc;
  DCAST_INTO_V(vbc, context);

  _frame_data->_pending_destroy_buffers.push_back(vbc->_buffer);
  _frame_data->_pending_free.push_back(std::move(vbc->_block));
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

  } else if (index_type != GeomEnums::NT_uint16 &&
             index_type != GeomEnums::NT_uint32) {
    vulkandisplay_cat.error()
      << "Unsupported index type: " << index_type;
    return nullptr;
  }

  //TODO: don't use host-visible memory, but copy from a staging buffer.
  VkBuffer buffer;
  VulkanMemoryBlock block;
  if (!create_buffer(data_size, buffer, block,
                     VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
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
  ibc->set_active(true);

  if (ibc->was_modified(reader)) {
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

      } else if (index_type != GeomEnums::NT_uint16 &&
                 index_type != GeomEnums::NT_uint32) {
        vulkandisplay_cat.error()
          << "Unsupported index type: " << index_type;
        return false;
      }

      auto data = ibc->_block.map();
      if (!data) {
        vulkandisplay_cat.error()
          << "Failed to map index buffer memory.\n";
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

      _data_transferred_pcollector.add_level(num_bytes);
    }

    ibc->mark_loaded(reader);
  }
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
 * Dispatches a currently bound compute shader using the given work group
 * counts.
 */
void VulkanGraphicsStateGuardian::
dispatch_compute(int num_groups_x, int num_groups_y, int num_groups_z) {
  nassertv(_frame_data != nullptr);

  //TODO: must actually be outside render pass, and on a queue that supports
  // compute.  Should we have separate pool/queue/buffer for compute?
  VkPipeline pipeline = _current_shader->get_compute_pipeline(this);
  nassertv(pipeline != VK_NULL_HANDLE);
  vkCmdBindPipeline(_frame_data->_cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);

  vkCmdDispatch(_frame_data->_cmd, num_groups_x, num_groups_y, num_groups_z);
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
 *
 * Special case: if (state==NULL), then the target state is already stored in
 * _target.
 */
void VulkanGraphicsStateGuardian::
set_state_and_transform(const RenderState *state,
                        const TransformState *trans) {
  // This does not actually set the state just yet, because we can't make a
  // pipeline state without knowing the vertex format.
  _state_rs = state;
  _target_rs = state;
  _internal_transform = trans;

  determine_target_shader();

  VulkanShaderContext *sc = _default_sc;
  Shader *shader = (Shader *)_target_shader->get_shader();
  if (shader != nullptr) {
    DCAST_INTO_V(sc, shader->prepare_now(get_prepared_objects(), this));
  }

  _current_shader = sc;
  nassertv(sc != nullptr);

  // Put the modelview projection matrix and color scale in the push constants.
  if (sc->_projection_mat_stage_mask != 0) {
    CPT(TransformState) combined = _projection_mat->compose(trans);
    LMatrix4f matrix = LCAST(float, combined->get_mat());
    vkCmdPushConstants(_frame_data->_cmd, sc->_pipeline_layout, sc->_push_constant_stage_mask, 0, 64, matrix.get_data());
  }

  if (sc->_color_scale_stage_mask != 0) {
    const ColorScaleAttrib *color_scale_attrib;
    state->get_attrib_def(color_scale_attrib);
    LColorf color = LCAST(float, color_scale_attrib->get_scale());
    vkCmdPushConstants(_frame_data->_cmd, sc->_pipeline_layout, sc->_push_constant_stage_mask, 64, 16, color.get_data());
  }

  // Update and bind descriptor sets.
  VkDescriptorSet descriptor_sets[DS_SET_COUNT] = {};

  const LightAttrib *target_light;
  state->get_attrib_def(target_light);
  if (get_attrib_descriptor_set(descriptor_sets[DS_light_attrib],
                                _lattr_descriptor_set_layout,
                                target_light)) {
    // The first time this set is bound in this frame, we update it.  Once we
    // use it in a command buffer, we can't update it again anyway.
    update_lattr_descriptor_set(descriptor_sets[DS_light_attrib], target_light);
  }

  determine_target_texture();
  if (get_attrib_descriptor_set(descriptor_sets[DS_texture_attrib],
                                _tattr_descriptor_set_layout,
                                _target_texture)) {
    update_tattr_descriptor_set(descriptor_sets[DS_texture_attrib], _target_texture);
  }

  if (get_attrib_descriptor_set(descriptor_sets[DS_shader_attrib],
                                sc->_sattr_descriptor_set_layout,
                                _target_shader)) {
    update_sattr_descriptor_set(descriptor_sets[DS_shader_attrib], _target_shader);
  }

  descriptor_sets[DS_dynamic_uniforms] = sc->_uniform_descriptor_set;

  //TODO: properly compute altered field.
  uint32_t num_offsets = 0;
  uint32_t offset = 0;
  if (sc->_mat_block_size > 0) {
    offset = sc->update_dynamic_uniforms(this, ~0);
    num_offsets = 1;
  }

  bool is_compute = (sc->_modules[(size_t)Shader::Stage::compute] != VK_NULL_HANDLE);
  vkCmdBindDescriptorSets(_frame_data->_cmd, is_compute ? VK_PIPELINE_BIND_POINT_COMPUTE : VK_PIPELINE_BIND_POINT_GRAPHICS,
                          sc->_pipeline_layout, 0, DS_SET_COUNT, descriptor_sets,
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
  nassertv(_frame_data != nullptr);
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
    vkCmdClearAttachments(_frame_data->_cmd, ai, attachments, _viewports.size(), rects);
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

  vkCmdSetViewport(_frame_data->_cmd, 0, count, viewports);
  vkCmdSetScissor(_frame_data->_cmd, 0, count, &_viewports[0]);
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
  nassertr_always(!_closing_gsg, false);
  nassertr_always(_frame_data == nullptr, false);

  // First, finish old, finished frames.
  VkFence reset_fences[_frame_data_capacity];
  size_t num_reset_fences = 0;

  if (_frame_data_head != _frame_data_capacity) {
    while (true) {
      FrameData &frame_data = _frame_data_pool[_frame_data_tail];
      if (vkGetFenceStatus(_device, frame_data._fence) == VK_NOT_READY) {
        // This frame is not yet ready, so abort the loop here, since there's no
        // use checking frames that come after this one.
        break;
      }

      // This frame has completed execution.
      reset_fences[num_reset_fences++] = frame_data._fence;
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

  // If the frame queue is full, we must wait until a frame is done.
  if (_frame_data_tail == _frame_data_head) {
    FrameData &frame_data = _frame_data_pool[_frame_data_tail];
    VkResult err;
    err = vkWaitForFences(_device, 1, &frame_data._fence, VK_TRUE, 1000000000ULL);
    if (err == VK_TIMEOUT) {
      vulkandisplay_cat.error()
        << "Timed out waiting for previous frame to complete rendering.\n";
      return false;
    }
    else if (err) {
      vulkan_error(err, "Failure waiting for command buffer fence");
      return false;
    }

    // This frame has completed execution.
    reset_fences[num_reset_fences++] = frame_data._fence;
    finish_frame(frame_data);
    _frame_data_tail = (_frame_data_tail + 1) % _frame_data_capacity;
  }

  // Reset the used fences to unsignaled status.
  if (num_reset_fences > 0) {
    VkResult err = vkResetFences(_device, num_reset_fences, reset_fences);
    nassertr(!err, false);
  }

  _frame_data = &_frame_data_pool[_frame_data_head % _frame_data_capacity];

  // Begin the transfer command buffer, for preparing resources.
  VkCommandBufferBeginInfo begin_info;
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.pNext = nullptr;
  begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  begin_info.pInheritanceInfo = nullptr;

  VkResult err;
  err = vkBeginCommandBuffer(_frame_data->_transfer_cmd, &begin_info);
  if (err) {
    vulkan_error(err, "Can't begin transfer command buffer");
    _frame_data = nullptr;
    return false;
  }

  // Increase the frame counter, which we use to determine whether we've
  // updated any resources in this frame.
  _frame_data->_frame_index = ++_frame_counter;
  _frame_data_head = (_frame_data_head + 1) % _frame_data_capacity;

  // Make sure we have a white texture.
  if (_white_texture.is_null()) {
    _white_texture = new Texture();
    _white_texture->setup_2d_texture(1, 1, Texture::T_unsigned_byte, Texture::F_rgba8);
    _white_texture->set_clear_color(LColor(1, 1, 1, 1));
    _white_texture->prepare_now(0, get_prepared_objects(), this);
  }

  // Update the "null" vertex buffer.
  LVecBase4f data[2] = {LVecBase4(0, 0, 0, 1), LVecBase4(1, 1, 1, 1)};
  vkCmdUpdateBuffer(_frame_data->_transfer_cmd, _color_vertex_buffer, 0, 32, (const uint32_t *)data[0].get_data());

  // Call the GSG's begin_frame, which will cause any queued-up release() and
  // prepare() methods to be called.  Note that some of them may add to the
  // transfer command buffer, which is why we've begun it already.
  if (GraphicsStateGuardian::begin_frame(current_thread)) {
    // Now begin the main (ie. graphics) command buffer.
    err = vkBeginCommandBuffer(_frame_data->_cmd, &begin_info);
    if (!err) {
      return true;
    }
    vulkan_error(err, "Can't begin command buffer");
  }

  // We've already started putting stuff in the transfer command buffer, so now
  // we are obliged to submit it, even if we won't actually end up rendering
  // anything in this frame.
  vkEndCommandBuffer(_frame_data->_transfer_cmd);

  VkSubmitInfo submit_info;
  submit_info.pNext = nullptr;
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.waitSemaphoreCount = 0;
  submit_info.pWaitSemaphores = nullptr;
  submit_info.pWaitDstStageMask = nullptr;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &_frame_data->_transfer_cmd;
  submit_info.signalSemaphoreCount = 0;
  submit_info.pSignalSemaphores = nullptr;

  err = vkQueueSubmit(_queue, 1, &submit_info, _frame_data->_fence);
  if (err) {
    vulkan_error(err, "Error submitting queue");
  }
  _frame_data = nullptr;
  return false;
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
}

/**
 * Called after each frame is rendered, to allow the GSG a chance to do any
 * internal cleanup after rendering the frame, and before the window flips.
 */
void VulkanGraphicsStateGuardian::
end_frame(Thread *current_thread) {
  GraphicsStateGuardian::end_frame(current_thread);

  nassertv(_frame_data->_transfer_cmd != VK_NULL_HANDLE);
  vkEndCommandBuffer(_frame_data->_transfer_cmd);

  // Note down the current watermark of the uniform buffer.
  _frame_data->_uniform_buffer_head = _uniform_buffer_allocator.get_head();

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
  if (!_download_queue.empty()) {
    size_t num_downloads = _download_queue.size();
    VkBufferMemoryBarrier *barriers = (VkBufferMemoryBarrier *)
      alloca(sizeof(VkBufferMemoryBarrier) * num_downloads);

    for (size_t i = 0; i < num_downloads; ++i) {
      barriers[i].sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
      barriers[i].pNext = nullptr;
      barriers[i].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      barriers[i].dstAccessMask = VK_ACCESS_HOST_READ_BIT;
      barriers[i].srcQueueFamilyIndex = _graphics_queue_family_index;
      barriers[i].dstQueueFamilyIndex = _graphics_queue_family_index;
      barriers[i].buffer = _download_queue[i]._buffer;
      barriers[i].offset = 0;
      barriers[i].size = VK_WHOLE_SIZE;
    }

    vkCmdPipelineBarrier(_frame_data->_cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0,
                         0, nullptr, (uint32_t)num_downloads, barriers, 0, nullptr);
  }

  nassertv(_frame_data->_cmd != VK_NULL_HANDLE);
  vkEndCommandBuffer(_frame_data->_cmd);

  VkCommandBuffer cmdbufs[] = {_frame_data->_transfer_cmd, _frame_data->_cmd};

  // Submit the command buffers to the queue.
  VkSubmitInfo submit_info;
  submit_info.pNext = nullptr;
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.waitSemaphoreCount = 0;
  submit_info.pWaitSemaphores = nullptr;
  submit_info.pWaitDstStageMask = nullptr;
  submit_info.commandBufferCount = 2;
  submit_info.pCommandBuffers = cmdbufs;
  submit_info.signalSemaphoreCount = 0;
  submit_info.pSignalSemaphores = nullptr;

  if (_wait_semaphore != VK_NULL_HANDLE) {
    // We may need to wait until the attachments are available for writing.
    static const VkPipelineStageFlags flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &_wait_semaphore;
    submit_info.pWaitDstStageMask = &flags;
  }

  if (_signal_semaphore != VK_NULL_HANDLE) {
    // And we were asked to signal a semaphore when we are done rendering.
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &_signal_semaphore;
  }

  VkResult err;
  err = vkQueueSubmit(_queue, 1, &submit_info, _frame_data->_fence);
  if (err) {
    vulkan_error(err, "Error submitting queue");
    return;
  }
  _frame_data = nullptr;

  // We're done with these for now.
  _wait_semaphore = VK_NULL_HANDLE;
  _signal_semaphore = VK_NULL_HANDLE;

  // If we queued up texture downloads, wait for the queue to finish (slow!)
  // and then copy the data from Vulkan host memory to Panda memory.
  if (!_download_queue.empty()) {
    {
      PStatTimer timer(_flush_pcollector);
      err = vkWaitForFences(_device, 1, &_frame_data->_fence, VK_TRUE, ~0ULL);
    }
    if (err) {
      vulkan_error(err, "Failed to wait for command buffer execution");
    }

    for (QueuedDownload &down : _download_queue) {
      PTA_uchar target = down._texture->modify_ram_image();
      size_t view_size = down._texture->get_ram_view_size();

      if (auto data = down._block.map()) {
        memcpy(target.p() + view_size * down._view, data, view_size);
      } else {
        vulkandisplay_cat.error()
          << "Failed to map memory for RAM transfer.\n";
        vkDestroyBuffer(_device, down._buffer, nullptr);
        continue;
      }

      // We won't need this buffer any more.
      vkDestroyBuffer(_device, down._buffer, nullptr);
    }
    _download_queue.clear();
  }

  //TODO: delete command buffer, schedule for deletion, or recycle.
}

/**
 * Called after the frame has finished executing to clean up any resources.
 * All frames *must* be finished in order!  It is illegal to call this for a
 * frame when a preceding frame has not finished yet!
 */
void VulkanGraphicsStateGuardian::
finish_frame(FrameData &frame_data) {
  ++_last_finished_frame;
  nassertv(frame_data._frame_index == _last_finished_frame);

  for (VkBufferView buffer_view : frame_data._pending_destroy_buffer_views) {
    vkDestroyBufferView(_device, buffer_view, nullptr);
  }
  frame_data._pending_destroy_buffer_views.clear();

  for (VkBuffer buffer : frame_data._pending_destroy_buffers) {
    vkDestroyBuffer(_device, buffer, nullptr);
  }
  frame_data._pending_destroy_buffers.clear();

  for (VkImageView image_view : frame_data._pending_destroy_image_views) {
    vkDestroyImageView(_device, image_view, nullptr);
  }
  frame_data._pending_destroy_image_views.clear();

  for (VkImage image : frame_data._pending_destroy_images) {
    vkDestroyImage(_device, image, nullptr);
  }
  frame_data._pending_destroy_images.clear();

  for (VkSampler sampler : frame_data._pending_destroy_samplers) {
    vkDestroySampler(_device, sampler, nullptr);
  }
  frame_data._pending_destroy_samplers.clear();

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

  // Make the used uniform buffer space available.
  _uniform_buffer_allocator.set_tail(frame_data._uniform_buffer_head);
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
  if (_current_shader == nullptr) {
    return false;
  }

  _instance_count = (uint32_t)std::min(num_instances, (size_t)0xffffffff);

  // Prepare and bind the vertex buffers.
  size_t num_arrays = data_reader->get_num_arrays();
  VkBuffer *buffers = (VkBuffer *)alloca(sizeof(VkBuffer) * (num_arrays + 1));
  VkDeviceSize *offsets = (VkDeviceSize *)alloca(sizeof(VkDeviceSize) * (num_arrays + 1));

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
  buffers[i] = _color_vertex_buffer;
  offsets[i] = 0;

  vkCmdBindVertexBuffers(_frame_data->_cmd, 0, num_arrays + 1, buffers, offsets);

  _format = data_reader->get_format();
  return true;
}

/**
 * Draws a series of disconnected triangles.
 */
bool VulkanGraphicsStateGuardian::
draw_triangles(const GeomPrimitivePipelineReader *reader, bool force) {
  return do_draw_primitive(reader, force, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
}

/**
 * Draws a series of disconnected triangles with adjacency information.
 */
bool VulkanGraphicsStateGuardian::
draw_triangles_adj(const GeomPrimitivePipelineReader *reader, bool force) {
  return do_draw_primitive(reader, force, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY);
}

/**
 * Draws a series of triangle strips.
 */
bool VulkanGraphicsStateGuardian::
draw_tristrips(const GeomPrimitivePipelineReader *reader, bool force) {
  return do_draw_primitive(reader, force, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
}

/**
 * Draws a series of triangle strips with adjacency information.
 */
bool VulkanGraphicsStateGuardian::
draw_tristrips_adj(const GeomPrimitivePipelineReader *reader, bool force) {
  return do_draw_primitive(reader, force, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY);
}

/**
 * Draws a series of triangle fans.
 */
bool VulkanGraphicsStateGuardian::
draw_trifans(const GeomPrimitivePipelineReader *reader, bool force) {
  return do_draw_primitive(reader, force, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN);
}

/**
 * Draws a series of "patches", which can only be processed by a tessellation
 * shader.
 */
bool VulkanGraphicsStateGuardian::
draw_patches(const GeomPrimitivePipelineReader *reader, bool force) {
  return do_draw_primitive(reader, force, VK_PRIMITIVE_TOPOLOGY_PATCH_LIST);
}

/**
 * Draws a series of disconnected line segments.
 */
bool VulkanGraphicsStateGuardian::
draw_lines(const GeomPrimitivePipelineReader *reader, bool force) {
  return do_draw_primitive(reader, force, VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
}

/**
 * Draws a series of disconnected line segments with adjacency information.
 */
bool VulkanGraphicsStateGuardian::
draw_lines_adj(const GeomPrimitivePipelineReader *reader, bool force) {
  return do_draw_primitive(reader, force, VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY);
}

/**
 * Draws a series of line strips.
 */
bool VulkanGraphicsStateGuardian::
draw_linestrips(const GeomPrimitivePipelineReader *reader, bool force) {
  return do_draw_primitive(reader, force, VK_PRIMITIVE_TOPOLOGY_LINE_STRIP);
}

/**
 * Draws a series of line strips with adjacency information.
 */
bool VulkanGraphicsStateGuardian::
draw_linestrips_adj(const GeomPrimitivePipelineReader *reader, bool force) {
  return do_draw_primitive(reader, force, VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY);
}

/**
 * Draws a series of disconnected points.
 */
bool VulkanGraphicsStateGuardian::
draw_points(const GeomPrimitivePipelineReader *reader, bool force) {
  return do_draw_primitive(reader, force, VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
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
 * Resets all internal state as if the gsg were newly created.
 */
void VulkanGraphicsStateGuardian::
reset() {
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
  DCAST_INTO_R(tc, tex->prepare_now(view, get_prepared_objects(), this), false);

  nassertr(fbtc->_extent.width <= tc->_extent.width &&
           fbtc->_extent.height <= tc->_extent.height &&
           fbtc->_extent.depth <= tc->_extent.depth, false);
  nassertr(fbtc->_mip_levels == tc->_mip_levels, false);
  nassertr(fbtc->_array_layers == tc->_array_layers, false);
  nassertr(fbtc->_aspect_mask == tc->_aspect_mask, false);

  // Issue a command to transition the image into a layout optimal for
  // transferring from.
  fbtc->transition(_frame_data->_cmd, _graphics_queue_family_index,
    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_READ_BIT);

  tc->transition(_frame_data->_cmd, _graphics_queue_family_index,
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT);

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

    vkCmdCopyImage(_frame_data->_cmd, fbtc->_image, fbtc->_layout, tc->_image, tc->_layout, 1, &region);
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

    vkCmdBlitImage(_frame_data->_cmd, fbtc->_image, fbtc->_layout, tc->_image, tc->_layout, 1, &region, VK_FILTER_NEAREST);
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
framebuffer_copy_to_ram(Texture *tex, int view, int z,
                        const DisplayRegion *dr, const RenderBuffer &rb) {

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

  return do_extract_image(fbtc, tex, view);
}

/**
 * Internal method used by extract_texture_data and framebuffer_copy_to_ram.
 * Queues up a texture-to-RAM download.
 */
bool VulkanGraphicsStateGuardian::
do_extract_image(VulkanTextureContext *tc, Texture *tex, int view, int z) {
  VkDeviceSize buffer_size = tex->get_expected_ram_image_size();

  // Create a temporary buffer for transferring into.
  QueuedDownload down;
  if (!create_buffer(buffer_size, down._buffer, down._block,
                     VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
    vulkandisplay_cat.error()
      << "Failed to create staging buffer for framebuffer-to-RAM copy.\n";
    return false;
  }

  // We tack this onto the existing command buffer, for now.
  VkCommandBuffer cmd = _frame_data->_cmd;

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

    // Issue a command to transition the image into a layout optimal for
    // transferring from.
    tc->transition(cmd, _graphics_queue_family_index,
      VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
      VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_READ_BIT);

    vkCmdCopyImageToBuffer(cmd, tc->_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                           down._buffer, 1, &region);
  }
  else {
    VkBufferCopy region;
    region.srcOffset = 0;
    region.dstOffset = 0;
    region.size = buffer_size;
    vkCmdCopyBuffer(_frame_data->_transfer_cmd, tc->_buffer, down._buffer, 1, &region);
  }

  down._texture = tex;
  down._view = view;
  _download_queue.push_back(std::move(down));
  return true;
}

/**
 * Invoked by all the draw_xyz methods.
 */
bool VulkanGraphicsStateGuardian::
do_draw_primitive(const GeomPrimitivePipelineReader *reader, bool force,
                  VkPrimitiveTopology topology) {

  VkPipeline pipeline = _current_shader->get_pipeline(this, _state_rs, _format, topology, _fb_ms_count);
  nassertr(pipeline != VK_NULL_HANDLE, false);
  vkCmdBindPipeline(_frame_data->_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

  int num_vertices = reader->get_num_vertices();

  if (reader->is_indexed()) {
    // This is an indexed primitive.  Set up and bind the index buffer.
    VulkanIndexBufferContext *ibc;
    DCAST_INTO_R(ibc, reader->prepare_now(get_prepared_objects(), this), false);

    if (!update_index_buffer(ibc, reader, force)) {
      return false;
    }

    vkCmdBindIndexBuffer(_frame_data->_cmd, ibc->_buffer, 0, ibc->_index_type);
    vkCmdDrawIndexed(_frame_data->_cmd, num_vertices, _instance_count, 0, 0, 0);
  } else {
    // A non-indexed primitive.
    vkCmdDraw(_frame_data->_cmd, num_vertices, _instance_count, reader->get_first_vertex(), 0);
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
 * @return a VulkanTextureContext on success.
 */
VulkanTextureContext *VulkanGraphicsStateGuardian::
create_image(VkImageType type, VkFormat format, const VkExtent3D &extent,
             uint32_t levels, uint32_t layers, VkSampleCountFlagBits samples,
             VkImageUsageFlags usage, VkImageCreateFlags flags) {
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
    return nullptr;
  }

  // Get the memory requirements, and find an appropriate heap to alloc in.
  VkMemoryRequirements mem_reqs;
  vkGetImageMemoryRequirements(_device, image, &mem_reqs);

  VulkanMemoryBlock block;
  if (!allocate_memory(block, mem_reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, false)) {
    vulkandisplay_cat.error()
      << "Failed to allocate " << mem_reqs.size << " bytes for image.\n";
    vkDestroyImage(_device, image, nullptr);
    return nullptr;
  }

  // Bind memory to image.
  if (!block.bind_image(image)) {
    vulkan_error(err, "Failed to bind memory to multisample color image");
    vkDestroyImage(_device, image, nullptr);
    return nullptr;
  }

  VulkanTextureContext *tc = new VulkanTextureContext(get_prepared_objects(), image, format);
  tc->_format = format;
  tc->_extent = extent;
  tc->_mip_levels = levels;
  tc->_array_layers = layers;
  tc->_block = std::move(block);

  tc->set_resident(true);
  tc->update_data_size_bytes(mem_reqs.size);
  return tc;
}

/**
 * Creates a new semaphore on this device.
 */
VkSemaphore VulkanGraphicsStateGuardian::
create_semaphore() {
  VkSemaphoreCreateInfo semaphore_info = {};
  semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkSemaphore semaphore;
  VkResult
  err = vkCreateSemaphore(_device, &semaphore_info, nullptr, &semaphore);
  nassertr_always(err == VK_SUCCESS, VK_NULL_HANDLE);
  _semaphores.push_back(semaphore);
  return semaphore;
}

/**
 * Creates a VkPipeline for the given RenderState+GeomVertexFormat combination.
 */
VkPipeline VulkanGraphicsStateGuardian::
make_pipeline(VulkanShaderContext *sc, const RenderState *state,
              const GeomVertexFormat *format, VkPrimitiveTopology topology,
              VkSampleCountFlagBits multisamples) {
  if (vulkandisplay_cat.is_spam()) {
    vulkandisplay_cat.spam()
      << "Making pipeline for state " << *state << " and format " << *format << "\n";
  }

  VkPipelineShaderStageCreateInfo stages[(size_t)Shader::Stage::compute + 1];
  const VkShaderStageFlagBits stage_flags[(size_t)Shader::Stage::compute + 1] = {
    VK_SHADER_STAGE_VERTEX_BIT,
    VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
    VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
    VK_SHADER_STAGE_GEOMETRY_BIT,
    VK_SHADER_STAGE_FRAGMENT_BIT,
    VK_SHADER_STAGE_COMPUTE_BIT,
  };
  uint32_t num_stages = 0;

  for (size_t i = 0; i <= (size_t)Shader::Stage::compute; ++i) {
    if (sc->_modules[i] != VK_NULL_HANDLE) {
      VkPipelineShaderStageCreateInfo &stage = stages[num_stages++];
      stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      stage.pNext = nullptr;
      stage.flags = 0;
      stage.stage = stage_flags[i];
      stage.module = sc->_modules[i];
      stage.pName = "main";
      stage.pSpecializationInfo = nullptr;
    }
  }

  // Describe each vertex input binding (ie. GeomVertexArray).  Leave one
  // extra slot for the "dummy" binding, see below.
  int num_bindings = format->get_num_arrays();
  VkVertexInputBindingDescription *binding_desc = (VkVertexInputBindingDescription *)
    alloca(sizeof(VkVertexInputBindingDescription) * (num_bindings + 1));

  int i = 0;
  for (i = 0; i < num_bindings; ++i) {
    binding_desc[i].binding = i;
    binding_desc[i].stride = format->get_array(i)->get_stride();
    binding_desc[i].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  }

  // Prepare a "dummy" binding, in case we need it, which is bound to missing
  // vertex attributes.  It contains only a single value, set to stride=0.
  int dummy_binding = -1;
  binding_desc[i].binding = i;
  binding_desc[i].stride = 0;
  binding_desc[i].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  const ColorAttrib *color_attr;
  state->get_attrib_def(color_attr);

  // Now describe each vertex attribute (ie. GeomVertexColumn).
  const Shader *shader = sc->get_shader();
  nassertr(shader != nullptr, VK_NULL_HANDLE);
  pvector<Shader::ShaderVarSpec>::const_iterator it;

  VkVertexInputAttributeDescription *attrib_desc = (VkVertexInputAttributeDescription *)
    alloca(sizeof(VkVertexInputAttributeDescription) * shader->_var_spec.size());

  i = 0;
  for (it = shader->_var_spec.begin(); it != shader->_var_spec.end(); ++it) {
    const Shader::ShaderVarSpec &spec = *it;
    int array_index;
    const GeomVertexColumn *column;

    attrib_desc[i].location = spec._id._location;

    if (!format->get_array_info(spec._name, array_index, column) ||
        (spec._name == InternalName::get_color() &&
         color_attr->get_color_type() != ColorAttrib::T_vertex)) {
      // The shader references a non-existent vertex column.  To make this a
      // well-defined operation (as in OpenGL), we bind a "dummy" vertex buffer
      // containing a fixed value with a stride of 0.
      if (dummy_binding == -1) {
        dummy_binding = num_bindings++;
      }

      attrib_desc[i].binding = dummy_binding;
      if (spec._name == InternalName::get_color()) {
        // Look up the offset into the color palette.
        if (color_attr->get_color_type() != ColorAttrib::T_flat) {
          attrib_desc[i].offset = 16;
        } else {
          LColorf color = LCAST(float, color_attr->get_color());
          attrib_desc[i].offset = get_color_palette_offset(color);
        }
      } else {
        attrib_desc[i].offset = 0;
      }
      attrib_desc[i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
      ++i;
      continue;
    }

    attrib_desc[i].binding = array_index;
    attrib_desc[i].offset = column->get_start();

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
  vertex_info.pVertexBindingDescriptions = binding_desc;
  vertex_info.vertexAttributeDescriptionCount = i;
  vertex_info.pVertexAttributeDescriptions = attrib_desc;

  VkPipelineInputAssemblyStateCreateInfo assembly_info;
  assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  assembly_info.pNext = nullptr;
  assembly_info.flags = 0;
  assembly_info.topology = topology;
  assembly_info.primitiveRestartEnable = (
    topology == VK_PRIMITIVE_TOPOLOGY_LINE_STRIP ||
    topology == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP ||
    topology == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN ||
    topology == VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY ||
    topology == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY);

  VkPipelineViewportStateCreateInfo viewport_info;
  viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport_info.pNext = nullptr;
  viewport_info.flags = 0;
  viewport_info.viewportCount = 1;
  viewport_info.pViewports = nullptr;
  viewport_info.scissorCount = 1;
  viewport_info.pScissors = nullptr;

  const RenderModeAttrib *render_mode;
  state->get_attrib_def(render_mode);
  const CullFaceAttrib *cull_face;
  state->get_attrib_def(cull_face);

  VkPipelineRasterizationStateCreateInfo raster_info;
  raster_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  raster_info.pNext = nullptr;
  raster_info.flags = 0;
  raster_info.depthClampEnable = VK_FALSE;
  raster_info.rasterizerDiscardEnable = VK_FALSE;

  if (_supported_geom_rendering & Geom::GR_render_mode_wireframe) {
    switch (render_mode->get_mode()) {
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
  } else {
    // Not supported.  The geometry will have been changed at munge time.
    raster_info.polygonMode = VK_POLYGON_MODE_FILL;
  }

  raster_info.cullMode = (VkCullModeFlagBits)cull_face->get_effective_mode();
  raster_info.frontFace = VK_FRONT_FACE_CLOCKWISE; // Flipped
  raster_info.depthBiasEnable = VK_FALSE;
  raster_info.depthBiasConstantFactor = 0;
  raster_info.depthBiasClamp = 0;
  raster_info.depthBiasSlopeFactor = 0;
  raster_info.lineWidth = render_mode->get_thickness();

  VkPipelineMultisampleStateCreateInfo ms_info;
  ms_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  ms_info.pNext = nullptr;
  ms_info.flags = 0;
  ms_info.rasterizationSamples = multisamples;
  ms_info.sampleShadingEnable = VK_FALSE;
  ms_info.minSampleShading = 0.0;
  ms_info.pSampleMask = nullptr;
  ms_info.alphaToCoverageEnable = VK_FALSE;
  ms_info.alphaToOneEnable = VK_FALSE;

  const DepthWriteAttrib *depth_write;
  state->get_attrib_def(depth_write);
  const DepthTestAttrib *depth_test;
  state->get_attrib_def(depth_test);

  VkPipelineDepthStencilStateCreateInfo ds_info;
  ds_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  ds_info.pNext = nullptr;
  ds_info.flags = 0;
  ds_info.depthTestEnable = (depth_test->get_mode() != RenderAttrib::M_none);
  ds_info.depthWriteEnable = depth_write->get_mode();
  ds_info.depthCompareOp = (VkCompareOp)std::max(0, depth_test->get_mode() - 1);
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

  const ColorBlendAttrib *color_blend;
  state->get_attrib_def(color_blend);
  const ColorWriteAttrib *color_write;
  state->get_attrib_def(color_write);
  const LogicOpAttrib *logic_op;
  state->get_attrib_def(logic_op);

  VkPipelineColorBlendAttachmentState att_state[1];
  if (color_blend->get_mode() != ColorBlendAttrib::M_none) {
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
    const TransparencyAttrib *transp;
    state->get_attrib_def(transp);

    switch (transp->get_mode()) {
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
        << "invalid transparency mode " << (int)transp->get_mode() << std::endl;
      break;
    }
  }
  att_state[0].colorWriteMask = color_write->get_channels();

  VkPipelineColorBlendStateCreateInfo blend_info;
  blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  blend_info.pNext = nullptr;
  blend_info.flags = 0;

  if (logic_op->get_operation() != LogicOpAttrib::O_none) {
    blend_info.logicOpEnable = VK_TRUE;
    blend_info.logicOp = (VkLogicOp)(logic_op->get_operation() - 1);
  } else {
    blend_info.logicOpEnable = VK_FALSE;
    blend_info.logicOp = VK_LOGIC_OP_COPY;
  }

  blend_info.attachmentCount = 1;
  blend_info.pAttachments = att_state;

  LColor constant_color = color_blend->get_color();
  blend_info.blendConstants[0] = constant_color[0];
  blend_info.blendConstants[1] = constant_color[1];
  blend_info.blendConstants[2] = constant_color[2];
  blend_info.blendConstants[3] = constant_color[3];

  // Tell Vulkan that we'll be specifying the viewport and scissor separately.
  const VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

  VkPipelineDynamicStateCreateInfo dynamic_info;
  dynamic_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamic_info.pNext = nullptr;
  dynamic_info.flags = 0;
  dynamic_info.dynamicStateCount = 2;
  dynamic_info.pDynamicStates = dynamic_states;

  VkGraphicsPipelineCreateInfo pipeline_info;
  pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipeline_info.pNext = nullptr;
  pipeline_info.flags = 0;
  pipeline_info.stageCount = num_stages;
  pipeline_info.pStages = stages;
  pipeline_info.pVertexInputState = &vertex_info;
  pipeline_info.pInputAssemblyState = &assembly_info;
  pipeline_info.pTessellationState = nullptr;
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
  pipeline_info.stage.module = sc->_modules[(size_t)Shader::Stage::compute];
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
  // Look it up in the attribute map.
  auto it = _attrib_descriptor_set_map.find(attrib);
  if (it != _attrib_descriptor_set_map.end()) {
    // Found something.  Check that it's not just a different state that has
    // been allocated in the memory of a previous state.
    DescriptorSet &set = it->second;
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

  DescriptorSet set;

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

  _attrib_descriptor_set_map[attrib] = std::move(set);
  out = set._handle;
  return true;
}

/**
 * Updates the descriptor set containing all the light attributes.
 */
bool VulkanGraphicsStateGuardian::
update_lattr_descriptor_set(VkDescriptorSet ds, const LightAttrib *attr) {
  const size_t num_shadow_maps = 16;
  VkWriteDescriptorSet *writes = (VkWriteDescriptorSet *)alloca(num_shadow_maps * sizeof(VkWriteDescriptorSet));
  VkDescriptorImageInfo *image_infos = (VkDescriptorImageInfo *)alloca(num_shadow_maps * sizeof(VkDescriptorImageInfo));

  size_t num_lights = attr->get_num_non_ambient_lights();

  PT(Texture) dummy = get_dummy_shadow_map(Texture::TT_2d_texture);

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

    VulkanTextureContext *tc;
    DCAST_INTO_R(tc, texture->prepare_now(0, _prepared_objects, this), false);

    tc->set_active(true);
    update_texture(tc, true);

    // Transition the texture so that it can be read by the shader.  This has
    // to happen on the transfer command buffer, since it can't happen during
    // an active render pass.
    // We don't know at this point which stages is using them, and finding out
    // would require duplication of descriptor sets, so we flag all stages.
    VkPipelineStageFlags stage_flags = 0
      | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT
      | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
      | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
      ;
    if (_supported_shader_caps & ShaderModule::C_tessellation_shader) {
      //stage_flags |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;
      //stage_flags |= VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
    }
    if (_supported_shader_caps & ShaderModule::C_geometry_shader) {
      //stage_flags |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
    }

    tc->transition(_frame_data->_transfer_cmd, _graphics_queue_family_index,
                   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                   stage_flags, VK_ACCESS_SHADER_READ_BIT);

    VkDescriptorImageInfo &image_info = image_infos[i];
    image_info.sampler = _shadow_sampler;
    image_info.imageView = tc->_image_view;
    image_info.imageLayout = tc->_layout;

    VkWriteDescriptorSet &write = writes[i];
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.pNext = nullptr;
    write.dstSet = ds;
    write.dstBinding = i;
    write.dstArrayElement = 0;
    write.descriptorCount = 1;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.pImageInfo = &image_info;
    write.pBufferInfo = nullptr;
    write.pTexelBufferView = nullptr;
  }

  vkUpdateDescriptorSets(_device, num_shadow_maps, writes, 0, nullptr);
  return true;
}

/**
 * Updates the descriptor set containing all the texture attributes.
 */
bool VulkanGraphicsStateGuardian::
update_tattr_descriptor_set(VkDescriptorSet ds, const TextureAttrib *attr) {
  // To ensure compatibility with any shader, we pad this out with white
  // textures.
  size_t num_textures = _max_texture_stages;

  VkWriteDescriptorSet *writes = (VkWriteDescriptorSet *)alloca(num_textures * sizeof(VkWriteDescriptorSet));
  VkDescriptorImageInfo *image_infos = (VkDescriptorImageInfo *)alloca(num_textures * sizeof(VkDescriptorImageInfo));

  for (size_t i = 0; i < num_textures; ++i) {
    SamplerState sampler;
    int view = 0;

    Texture *texture;
    if (i < (size_t)attr->get_num_on_stages()) {
      TextureStage *stage = attr->get_on_stage(i);
      sampler = attr->get_on_sampler(stage);
      view += stage->get_tex_view_offset();
      texture = attr->get_on_texture(stage);

      // We can't support this easily, because texel buffers need a different
      // descriptor type, so we'd need a different layout as well, which ruins
      // the whole convenience of doing TextureAttrib this way.  However, it's
      // not a limitation that people are likely to run into anyway.
      if (texture->get_texture_type() == Texture::TT_buffer_texture) {
        vulkandisplay_cat.error()
          << "Buffer textures are not supported via texture stages.  Use shader inputs instead.\n";
        texture = _white_texture;
        view = 0;
      }
    } else {
      texture = _white_texture;
    }

    VulkanTextureContext *tc;
    DCAST_INTO_R(tc, texture->prepare_now(view, _prepared_objects, this), false);

    VulkanSamplerContext *sc;
    DCAST_INTO_R(sc, sampler.prepare_now(_prepared_objects, this), false);

    tc->set_active(true);
    update_texture(tc, true);

    // Transition the texture so that it can be read by the shader.  This has
    // to happen on the transfer command buffer, since it can't happen during
    // an active render pass.
    // We don't know at this point which stages is using them, and finding out
    // would require duplication of descriptor sets, so we flag all stages.
    VkPipelineStageFlags stage_flags = 0
      | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT
      | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
      | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
      ;
    if (_supported_shader_caps & ShaderModule::C_tessellation_shader) {
      //stage_flags |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;
      //stage_flags |= VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
    }
    if (_supported_shader_caps & ShaderModule::C_geometry_shader) {
      //stage_flags |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
    }

    tc->transition(_frame_data->_transfer_cmd, _graphics_queue_family_index,
                   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                   stage_flags, VK_ACCESS_SHADER_READ_BIT);

    VkDescriptorImageInfo &image_info = image_infos[i];
    image_info.sampler = sc->_sampler;
    image_info.imageView = tc->_image_view;
    image_info.imageLayout = tc->_layout;

    VkWriteDescriptorSet &write = writes[i];
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.pNext = nullptr;
    write.dstSet = ds;
    write.dstBinding = i;
    write.dstArrayElement = 0;
    write.descriptorCount = 1;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.pImageInfo = &image_info;
    write.pBufferInfo = nullptr;
    write.pTexelBufferView = nullptr;
  }

  vkUpdateDescriptorSets(_device, num_textures, writes, 0, nullptr);
  return true;
}

/**
 * Updates the descriptor set containing all the ShaderAttrib textures.
 */
bool VulkanGraphicsStateGuardian::
update_sattr_descriptor_set(VkDescriptorSet ds, const ShaderAttrib *attr) {
  Shader *shader = (Shader *)attr->get_shader();
  if (shader == nullptr) {
    // Nothing to do.
    return true;
  }

  VulkanShaderContext *sc;
  DCAST_INTO_R(sc, shader->prepare_now(get_prepared_objects(), this), false);

  // Allocate enough memory.
  size_t max_num_descriptors = 1 + shader->_tex_spec.size() + shader->_img_spec.size();
  VkWriteDescriptorSet *writes = (VkWriteDescriptorSet *)alloca(max_num_descriptors * sizeof(VkWriteDescriptorSet));
  VkDescriptorImageInfo *image_infos = (VkDescriptorImageInfo *)alloca(max_num_descriptors * sizeof(VkDescriptorImageInfo));

  // First the UBO, then the shader input textures.
  size_t i = 0;

  VkDescriptorBufferInfo buffer_info;
  if (sc->_ptr_block_size > 0) {
    buffer_info.buffer = _uniform_buffer;
    buffer_info.offset = sc->update_sattr_uniforms(this);
    buffer_info.range = sc->_ptr_block_size;

    VkWriteDescriptorSet &write = writes[i];
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.pNext = nullptr;
    write.dstSet = ds;
    write.dstBinding = i;
    write.dstArrayElement = 0;
    write.descriptorCount = 1;
    write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write.pImageInfo = nullptr;
    write.pBufferInfo = &buffer_info;
    write.pTexelBufferView = nullptr;

    ++i;
  }

  for (Shader::ShaderTexSpec &spec : shader->_tex_spec) {
    if (spec._part != Shader::STO_named_input || spec._id._location < 0) {
      continue;
    }

    int view = get_current_tex_view_offset();
    SamplerState sampler;

    PT(Texture) texture = fetch_specified_texture(spec, sampler, view);
    if (texture == nullptr) {
      return false;
    }

    if (texture->get_texture_type() != spec._desired_type) {
      vulkandisplay_cat.error()
        << "Sampler type of shader input '" << *spec._name << "' does not "
           "match type of texture " << *texture << ".\n";
    }

    VulkanTextureContext *tc;
    DCAST_INTO_R(tc, texture->prepare_now(view, _prepared_objects, this), false);

    VulkanSamplerContext *sc;
    DCAST_INTO_R(sc, sampler.prepare_now(_prepared_objects, this), false);

    tc->set_active(true);
    update_texture(tc, true);

    // Transition the texture so that it can be read by the shader.  This has
    // to happen on the transfer command buffer, since it can't happen during
    // an active render pass.
    VkPipelineStageFlags stage_flags = 0;
    if (spec._id._stage_mask & (1 << (int)Shader::Stage::vertex)) {
      stage_flags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
    }
    if (spec._id._stage_mask & (1 << (int)Shader::Stage::tess_control)) {
      stage_flags |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;
    }
    if (spec._id._stage_mask & (1 << (int)Shader::Stage::tess_evaluation)) {
      stage_flags |= VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
    }
    if (spec._id._stage_mask & (1 << (int)Shader::Stage::fragment)) {
      stage_flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    if (spec._id._stage_mask & (1 << (int)Shader::Stage::geometry)) {
      stage_flags |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
    }
    if (spec._id._stage_mask & (1 << (int)Shader::Stage::compute)) {
      stage_flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    }

    tc->transition(_frame_data->_transfer_cmd, _graphics_queue_family_index,
                   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                   stage_flags, VK_ACCESS_SHADER_READ_BIT);

    VkWriteDescriptorSet &write = writes[i];
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.pNext = nullptr;
    write.dstSet = ds;
    write.dstBinding = i;
    write.dstArrayElement = 0;
    write.descriptorCount = 1;
    write.pImageInfo = nullptr;
    write.pBufferInfo = nullptr;
    write.pTexelBufferView = nullptr;

    if (spec._desired_type != Texture::TT_buffer_texture) {
      write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      VkDescriptorImageInfo &image_info = image_infos[i];
      image_info.sampler = sc->_sampler;
      image_info.imageView = tc->_image_view;
      image_info.imageLayout = tc->_layout;
      write.pImageInfo = &image_info;
    } else {
      write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
      write.pTexelBufferView = &tc->_buffer_view;
    }

    ++i;
  }

  for (const Shader::ShaderImgSpec &spec : shader->_img_spec) {
    if (spec._id._location < 0) {
      continue;
    }

    const ParamTextureImage *param = nullptr;
    Texture *texture;
    VkAccessFlags access_mask = 0;

    const ShaderInput &sinp = attr->get_shader_input(spec._name);
    switch (sinp.get_value_type()) {
    case ShaderInput::M_texture_image:
      param = (const ParamTextureImage *)sinp.get_param();
      texture = param->get_texture();
      if (param->has_read_access()) {
        access_mask |= VK_ACCESS_SHADER_READ_BIT;
      }
      if (param->has_write_access()) {
        access_mask |= VK_ACCESS_SHADER_WRITE_BIT;
      }
      break;

    case ShaderInput::M_texture:
      // People find it convenient to be able to pass a texture without
      // further ado.
      texture = sinp.get_texture();
      access_mask = VK_ACCESS_SHADER_READ_BIT;
      break;

    case ShaderInput::M_invalid:
      vulkandisplay_cat.error()
        << "Missing texture image binding input " << *spec._name << "\n";
      return false;

    default:
      vulkandisplay_cat.error()
        << "Mismatching type for parameter " << *spec._name
        << ", expected texture image binding\n";
      return false;
    }

    if (texture->get_texture_type() != spec._desired_type) {
      vulkandisplay_cat.error()
        << "Sampler type of shader input '" << *spec._name << "' does not "
           "match type of texture " << *texture << ".\n";
    }

    VulkanTextureContext *tc;
    int view = get_current_tex_view_offset();
    DCAST_INTO_R(tc, texture->prepare_now(view, _prepared_objects, this), false);

    tc->set_active(true);
    update_texture(tc, true);

    // Transition the texture so that it can be read by the shader.  This has
    // to happen on the transfer command buffer, since it can't happen during
    // an active render pass.
    VkPipelineStageFlags stage_flags = 0;
    if (spec._id._stage_mask & (1 << (int)Shader::Stage::vertex)) {
      stage_flags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
    }
    if (spec._id._stage_mask & (1 << (int)Shader::Stage::tess_control)) {
      stage_flags |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;
    }
    if (spec._id._stage_mask & (1 << (int)Shader::Stage::tess_evaluation)) {
      stage_flags |= VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
    }
    if (spec._id._stage_mask & (1 << (int)Shader::Stage::fragment)) {
      stage_flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    if (spec._id._stage_mask & (1 << (int)Shader::Stage::geometry)) {
      stage_flags |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
    }
    if (spec._id._stage_mask & (1 << (int)Shader::Stage::compute)) {
      stage_flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    }

    // Load/store images need to be in the general layout.
    tc->transition(_frame_data->_transfer_cmd, _graphics_queue_family_index,
                   VK_IMAGE_LAYOUT_GENERAL, stage_flags, access_mask);

    VkWriteDescriptorSet &write = writes[i];
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.pNext = nullptr;
    write.dstSet = ds;
    write.dstBinding = i;
    write.dstArrayElement = 0;
    write.descriptorCount = 1;
    write.pImageInfo = nullptr;
    write.pBufferInfo = nullptr;
    write.pTexelBufferView = nullptr;

    if (spec._desired_type != Texture::TT_buffer_texture) {
      write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
      VkDescriptorImageInfo &image_info = image_infos[i];
      image_info.sampler = VK_NULL_HANDLE;
      image_info.imageView = tc->_image_view;
      image_info.imageLayout = tc->_layout;
      write.pImageInfo = &image_info;
    } else {
      write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
      write.pTexelBufferView = &tc->_buffer_view;
    }

    ++i;
  }

  vkUpdateDescriptorSets(_device, i, writes, 0, nullptr);
  return true;
}

/**
 * Returns a writable pointer to the dynamic uniform buffer.
 */
void *VulkanGraphicsStateGuardian::
alloc_dynamic_uniform_buffer(VkDeviceSize size, uint32_t &offset) {
  if (size == 0) {
    offset = 0;
    return nullptr;
  }

  ssize_t result = _uniform_buffer_allocator.alloc(size);
  if (result >= 0) {
    offset = (uint32_t)result;
    return (char *)_uniform_buffer_ptr + result;
  }

  TrueClock *clock = TrueClock::get_global_ptr();
  double start_time = clock->get_short_raw_time();

  VkFence reset_fences[_frame_data_capacity - 1];
  size_t num_reset_fences = 0;

  // Wait for the last frame to be done, that should free up some space.
  while (result < 0 && &_frame_data_pool[_frame_data_tail] != _frame_data) {
    FrameData &frame_data = _frame_data_pool[_frame_data_tail];
    VkResult err;
    err = vkWaitForFences(_device, 1, &frame_data._fence, VK_TRUE, 1000000000ULL);
    if (err == VK_TIMEOUT) {
      vulkandisplay_cat.error()
        << "Timed out waiting for previous frame to complete rendering.\n";
      break;
    }
    else if (err) {
      vulkan_error(err, "Failure waiting for command buffer fence");
      break;
    }

    // This frame has completed execution.
    reset_fences[num_reset_fences++] = frame_data._fence;
    finish_frame(frame_data);
    _frame_data_tail = (_frame_data_tail + 1) % _frame_data_capacity;

    // Try the allocation again.
    result = _uniform_buffer_allocator.alloc(size);
  }

  // Reset the used fences to unsignaled status.
  if (num_reset_fences > 0) {
    VkResult err = vkResetFences(_device, num_reset_fences, reset_fences);
    nassertr(!err, nullptr);
  }

  if (result >= 0) {
    double end_time = clock->get_short_raw_time();
    double stall_time = (end_time - start_time);
    vulkandisplay_cat.warning()
      << "Stalled for " << (stall_time * 1000) << " ms due to running out of "
         "global uniform buffer space trying to allocate " << size << " bytes."
         "  Increase vulkan-global-uniform-buffer-size for best performance"
      << " (current is " << _uniform_buffer_allocator.get_capacity() << ").\n";

    offset = (uint32_t)result;
    return (char *)_uniform_buffer_ptr + result;
  }

  //TODO: fail more gracefully.  Create a new buffer on the fly and manage
  // multiple buffers?  Or submit work and insert a fence, then replace the
  // buffer with a new one?
  vulkandisplay_cat.error()
    << "Used up entire global uniform buffer in a single frame, cannot "
       "recover.  Increase vulkan-global-uniform-buffer-size substantially"
    << " (current is " << _uniform_buffer_allocator.get_capacity() << ").\n";
  abort();
  return nullptr;
}

/**
 * Returns the offset of the given color in the color palette.
 */
uint32_t VulkanGraphicsStateGuardian::
get_color_palette_offset(const LColor &color) {
  ColorPaletteIndices::const_iterator it = _color_palette.find(color);
  if (it != _color_palette.end()) {
    return it->second * 16;
  }

  // Not yet in the palette.  Write an entry.
  if (_next_palette_index >= vulkan_color_palette_size) {
    vulkandisplay_cat.error()
      << "Ran out of color palette entries.  Increase "
         "vulkan-color-palette-size value in Config.prc.\n";
    return 1;
  }

  uint32_t offset = _next_palette_index * 16;
  _color_palette[color] = _next_palette_index++;
  vkCmdUpdateBuffer(_frame_data->_transfer_cmd, _color_vertex_buffer, offset, 16,
                    (const uint32_t *)color.get_data());
  return offset;
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
    return (VkFormat)(VK_FORMAT_B8G8R8_UINT + is_signed);
  case Texture::F_rgba8i:
    return (VkFormat)(VK_FORMAT_B8G8R8A8_UINT + is_signed);
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
