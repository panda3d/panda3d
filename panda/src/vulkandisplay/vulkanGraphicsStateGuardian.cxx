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

#include "colorAttrib.h"
#include "colorBlendAttrib.h"
#include "colorScaleAttrib.h"
#include "colorWriteAttrib.h"
#include "cullFaceAttrib.h"
#include "depthTestAttrib.h"
#include "depthWriteAttrib.h"
#include "logicOpAttrib.h"
#include "renderModeAttrib.h"
#include "transparencyAttrib.h"

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
  _cmd(VK_NULL_HANDLE),
  _transfer_cmd(VK_NULL_HANDLE),
  _render_pass(VK_NULL_HANDLE),
  _wait_semaphore(VK_NULL_HANDLE),
  _signal_semaphore(VK_NULL_HANDLE),
  _pipeline_cache(VK_NULL_HANDLE),
  _default_sc(nullptr),
  _total_allocated(0)
{
  const char *const layers[] = {
    "VK_LAYER_LUNARG_standard_validation",
  };

  const char *extensions[] = {
    "VK_KHR_swapchain",
  };

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
  device_info.enabledExtensionCount = 1;
  device_info.ppEnabledExtensionNames = extensions;
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

  // Create a fence to signal when the command buffers have finished.
  VkFenceCreateInfo fence_info;
  fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fence_info.pNext = nullptr;
  fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
  err = vkCreateFence(_device, &fence_info, nullptr, &_fence);
  if (err) {
    vulkan_error(err, "Failed to create fence");
    return;
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

  // Create a descriptor set layout.
  VkDescriptorSetLayoutBinding layout_binding;
  layout_binding.binding = 0;
  layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  layout_binding.descriptorCount = 1;
  layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  layout_binding.pImmutableSamplers = nullptr;

  VkDescriptorSetLayoutCreateInfo set_info;
  set_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  set_info.pNext = nullptr;
  set_info.flags = 0;
  set_info.bindingCount = 1;
  set_info.pBindings = &layout_binding;

  err = vkCreateDescriptorSetLayout(_device, &set_info, nullptr, &_descriptor_set_layout);
  if (err) {
    vulkan_error(err, "Failed to create descriptor set layout");
  }

  VkDescriptorPoolSize pool_size;
  pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  pool_size.descriptorCount = 64;

  VkDescriptorPoolCreateInfo pool_info;
  pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_info.pNext = nullptr;
  pool_info.flags = 0;
  pool_info.maxSets = 64;
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

  // Load the default shader.  Temporary hack.
  static PT(Shader) default_shader;
  if (default_shader.is_null()) {
    default_shader = Shader::load(Shader::SL_SPIR_V, "vert.spv", "frag.spv");
    nassertv(default_shader);
  }
  if (_default_sc == nullptr) {
    ShaderContext *sc = default_shader->prepare_now(get_prepared_objects(), this);
    nassertv(sc);
    _default_sc = DCAST(VulkanShaderContext, sc);
  }

  // Fill in the features supported by this physical device.
  const VkPhysicalDeviceLimits &limits = pipe->_gpu_properties.limits;
  const VkPhysicalDeviceFeatures &features = pipe->_gpu_features;
  _is_hardware = (pipe->_gpu_properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_CPU);

  _max_vertices_per_array = std::max((uint32_t)0x7fffffff, limits.maxDrawIndexedIndexValue);
  _max_vertices_per_primitive = INT_MAX;

  _max_texture_dimension = limits.maxImageDimension2D;
  _max_3d_texture_dimension = limits.maxImageDimension3D;
  _max_2d_texture_array_layers = limits.maxImageArrayLayers;
  _max_cube_map_dimension = limits.maxImageDimensionCube;
  _max_buffer_texture_size = limits.maxTexelBufferElements;

  _supports_3d_texture = true;
  _supports_2d_texture_array = true;
  _supports_cube_map = true;
  _supports_buffer_texture = false; //TODO: add support.
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
  _supports_basic_shaders = false;
  _supports_geometry_shaders = (features.geometryShader != VK_FALSE);
  _supports_tessellation_shaders = (features.tessellationShader != VK_FALSE);
  _supports_compute_shaders = true;
  _supports_glsl = false;
  _supports_hlsl = false;
  _supports_framebuffer_multisample = true;
  _supports_framebuffer_blit = true;

  _supports_stencil = true;
  _supports_stencil_wrap = true;
  _supports_two_sided_stencil = true;
  _supports_geometry_instancing = true;
  _supports_indirect_draw = true;

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

  _is_valid = true;
  _needs_reset = false;
}

/**
 *
 */
VulkanGraphicsStateGuardian::
~VulkanGraphicsStateGuardian() {
  // And all the semaphores that were generated on this device.
  for (VkSemaphore semaphore : _semaphores) {
    vkDestroySemaphore(_device, semaphore, nullptr);
  }

  // Remove the things we created in the constructor, in reverse order.
  vkDestroyBuffer(_device, _color_vertex_buffer, nullptr);
  vkDestroyDescriptorPool(_device, _descriptor_pool, nullptr);
  vkDestroyDescriptorSetLayout(_device, _descriptor_set_layout, nullptr);
  vkDestroyPipelineCache(_device, _pipeline_cache, nullptr);
  vkDestroyCommandPool(_device, _cmd_pool, nullptr);
  vkDestroyFence(_device, _fence, nullptr);

  // Also free all the memory pages before destroying the device.
  _memory_pages.clear();

  vkDestroyDevice(_device, nullptr);
}

/**
 * This is called by the associated GraphicsWindow when close_window() is
 * called.  It should null out the _win pointer and possibly free any open
 * resources associated with the GSG.
 */
void VulkanGraphicsStateGuardian::
close_gsg() {
  // Make sure it's no longer doing anything before we destroy it.
  vkDeviceWaitIdle(_device);

  // We need to release all prepared resources, since the upcall to close_gsg
  // will cause the PreparedGraphicsObjects to be cleared out.
  {
    PT(PreparedGraphicsObjects) pgo = std::move(_prepared_objects);
    if (pgo != nullptr) {
      pgo->release_all_now(this);
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
  if (!vkpipe->find_memory_type(type_index, reqs.memoryTypeBits, required_flags)) {
    return false;
  }
  VkFlags flags = vkpipe->_memory_properties.memoryTypes[type_index].propertyFlags;

  VkMemoryAllocateInfo alloc_info;
  alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  alloc_info.pNext = nullptr;
  alloc_info.memoryTypeIndex = type_index;
  alloc_info.allocationSize = std::max((VkDeviceSize)vulkan_memory_page_size, reqs.size);

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

  VkResult err;
  VkImageCreateInfo image_info;
  image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  image_info.pNext = nullptr;
  image_info.flags = 0;

  int width = texture->get_x_size();
  int height = texture->get_y_size();
  int depth = 1;
  int num_layers = 1;

  switch (texture->get_texture_type()) {
  case Texture::TT_1d_texture:
  case Texture::TT_1d_texture_array:
    image_info.imageType = VK_IMAGE_TYPE_1D;
    swap(height, num_layers);
    break;

  case Texture::TT_cube_map:
  case Texture::TT_cube_map_array:
    image_info.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    // Fall through
  case Texture::TT_2d_texture:
  case Texture::TT_2d_texture_array:
    image_info.imageType = VK_IMAGE_TYPE_2D;
    num_layers = texture->get_z_size();
    break;

  case Texture::TT_3d_texture:
    image_info.imageType = VK_IMAGE_TYPE_3D;
    depth = texture->get_z_size();
    break;

  case Texture::TT_buffer_texture:
    // Not yet supported.
    return nullptr;
  }

  image_info.format = get_image_format(texture);
  image_info.extent.width = width;
  image_info.extent.height = height;
  image_info.extent.depth = depth;
  image_info.mipLevels = 1;
  image_info.arrayLayers = num_layers;
  image_info.samples = VK_SAMPLE_COUNT_1_BIT;
  image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
  image_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  image_info.queueFamilyIndexCount = 0;
  image_info.pQueueFamilyIndices = nullptr;
  image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

  // Check if the format is actually supported.
  VkFormatProperties fmt_props;
  vkGetPhysicalDeviceFormatProperties(vkpipe->_gpu, image_info.format, &fmt_props);

  bool pack_bgr8 = false;
  if ((fmt_props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) == 0) {
    // Not supported.  Can we convert it to a format that is supported?
    switch (image_info.format) {
    case VK_FORMAT_B8G8R8_UNORM:
      image_info.format = VK_FORMAT_A8B8G8R8_UNORM_PACK32;
      pack_bgr8 = true;
      break;

    case VK_FORMAT_B8G8R8_UINT:
      image_info.format = VK_FORMAT_A8B8G8R8_UINT_PACK32;
      pack_bgr8 = true;
      break;

    default:
      vulkandisplay_cat.error()
        << "Texture format " << image_info.format << " not supported.\n";
      return nullptr;
    }

    // Update the properties for the new format.
    vkGetPhysicalDeviceFormatProperties(vkpipe->_gpu, image_info.format, &fmt_props);
  }

  // Check that the size is actually supported.
  int mipmap_begin = 0;
  VkImageFormatProperties img_props;
  vkGetPhysicalDeviceImageFormatProperties(vkpipe->_gpu, image_info.format,
                                           image_info.imageType, image_info.tiling,
                                           image_info.usage, image_info.flags,
                                           &img_props);
  if (image_info.arrayLayers > img_props.maxArrayLayers) {
    //TODO: more elegant solution to reduce layer count.
    vulkandisplay_cat.error()
      << "Texture has too many layers, this format has a maximum of "
      << img_props.maxArrayLayers << "\n";
    return nullptr;
  }
  while (image_info.extent.width > img_props.maxExtent.width ||
         image_info.extent.height > img_props.maxExtent.height ||
         image_info.extent.depth > img_props.maxExtent.depth) {
    // Reduce the size by bumping the first mipmap level uploaded.
    image_info.extent.width = std::max(1U, image_info.extent.width >> 1);
    image_info.extent.height = std::max(1U, image_info.extent.height >> 1);
    image_info.extent.depth = std::max(1U, image_info.extent.depth >> 1);
    ++mipmap_begin;
  }

  if (mipmap_begin != 0) {
    vulkandisplay_cat.info()
      << "Reducing image " << texture->get_name() << " from "
      << width << " x " << height << " x " << depth << " to "
      << image_info.extent.width << " x " << image_info.extent.height << " x "
      << image_info.extent.depth << "\n";

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
      image_info.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
  }

  image_info.mipLevels = mipmap_end - mipmap_begin;
  if (image_info.mipLevels > img_props.maxMipLevels) {
    mipmap_end -= image_info.mipLevels - img_props.maxMipLevels;
    image_info.mipLevels = img_props.maxMipLevels;
  }

  VkImage image;
  err = vkCreateImage(_device, &image_info, nullptr, &image);
  if (err) {
    vulkan_error(err, "Failed to create texture image");
    return nullptr;
  }

  // Create a texture context to manage the image's lifetime.
  VulkanTextureContext *tc = new VulkanTextureContext(get_prepared_objects(), texture, view);
  nassertr_always(tc != nullptr, nullptr);
  tc->_format = image_info.format;
  tc->_extent = image_info.extent;
  tc->_mipmap_begin = mipmap_begin;
  tc->_mipmap_end = mipmap_end;
  tc->_mip_levels = image_info.mipLevels;
  tc->_array_layers = image_info.arrayLayers;
  tc->_generate_mipmaps = generate_mipmaps;
  tc->_pack_bgr8 = pack_bgr8;

  Texture::Format format = texture->get_format();
  if (format == Texture::F_depth_stencil ||
      format == Texture::F_depth_component ||
      format == Texture::F_depth_component16 ||
      format == Texture::F_depth_component24 ||
      format == Texture::F_depth_component32) {
    tc->_aspect_mask = VK_IMAGE_ASPECT_DEPTH_BIT;
  } else {
    tc->_aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT;
  }

  tc->_image = image;

  // Get the memory requirements, and find an appropriate heap to alloc in.
  // The texture will be stored in device-local memory, since we can't write
  // to OPTIMAL-tiled images anyway.
  VkMemoryRequirements mem_reqs;
  vkGetImageMemoryRequirements(_device, image, &mem_reqs);

  if (!allocate_memory(tc->_block, mem_reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, false)) {
    vulkandisplay_cat.error() << "Failed to allocate texture memory.\n";
    vkDestroyImage(_device, image, nullptr);
    delete tc;
    return nullptr;
  }


  // Bind memory to image.
  if (!tc->_block.bind_image(image)) {
    vkDestroyImage(_device, image, nullptr);
    delete tc;
    return nullptr;
  }

  // Now we'll create an image view that describes how we interpret the image.
  VkImageViewCreateInfo view_info;
  view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  view_info.pNext = nullptr;
  view_info.flags = 0;
  view_info.image = image;

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

  view_info.format = image_info.format;

  // We use the swizzle mask to emulate deprecated formats.
  switch (format) {
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
  view_info.subresourceRange.levelCount = image_info.mipLevels;
  view_info.subresourceRange.baseArrayLayer = 0;
  view_info.subresourceRange.layerCount = num_layers;

  err = vkCreateImageView(_device, &view_info, nullptr, &tc->_image_view);
  if (err) {
    vulkan_error(err, "Failed to create image view for texture");
    vkDestroyImage(_device, image, nullptr);
    delete tc;
    return nullptr;
  }

  if (vulkandisplay_cat.is_debug()) {
    vulkandisplay_cat.debug()
      << "Created image " << image << " and view " << tc->_image_view
      << " for texture " << *texture << "\n";
  }

  // Update the BufferResidencyTracker to keep track of the allocated memory.
  tc->set_resident(true);
  tc->update_data_size_bytes(mem_reqs.size);

  // We can't upload it at this point because the texture lock is currently
  // held, so accessing the RAM image will cause a deadlock.
  return tc;
}

/**
 * Uploads the texture data for the given texture.
 */
bool VulkanGraphicsStateGuardian::
upload_texture(VulkanTextureContext *tc) {
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
      if (tc->_aspect_mask == VK_IMAGE_ASPECT_COLOR_BIT) {
        VkClearColorValue value; //TODO: handle integer clears?
        value.float32[0] = col[0];
        value.float32[1] = col[1];
        value.float32[2] = col[2];
        value.float32[3] = col[3];
        tc->clear_color_image(_transfer_cmd, value);
      } else {
        VkClearDepthStencilValue value;
        value.depth = col[0];
        value.stencil = 0;
        tc->clear_depth_stencil_image(_transfer_cmd, value);
      }
    }

    return true;
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

  // Issue a command to transition the image into a layout optimal for
  // transferring into.
  tc->transition(_transfer_cmd, _graphics_queue_family_index,
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

  data.unmap();

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

  tc->mark_loaded();

  // Make sure that the staging memory is not deleted until the next fence.
  _pending_free.push_back(std::move(block));
  _pending_delete_buffers.push_back(buffer);

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

  if (vtc->was_image_modified()) {
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
  // This is called during begin_frame, after the fence has been waited upon,
  // so we know that any command buffers using this have finished executing.
  VulkanTextureContext *vtc;
  DCAST_INTO_V(vtc, tc);

  if (vulkandisplay_cat.is_debug()) {
    vulkandisplay_cat.debug()
      << "Deleting image " << vtc->_image << " and view " << vtc->_image_view << "\n";
  }

  if (vtc->_image_view != VK_NULL_HANDLE) {
    vkDestroyImageView(_device, vtc->_image_view, nullptr);
    vtc->_image_view = VK_NULL_HANDLE;
  }
  if (vtc->_image != VK_NULL_HANDLE) {
    vkDestroyImage(_device, vtc->_image, nullptr);
    vtc->_image = VK_NULL_HANDLE;
  }

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
  // This is called during begin_frame, after the fence has been waited upon,
  // so we know that any command buffers using this have finished executing.
  VulkanSamplerContext *vsc;
  DCAST_INTO_V(vsc, sc);

  if (vsc->_sampler != VK_NULL_HANDLE) {
    vkDestroySampler(_device, vsc->_sampler, nullptr);
    vsc->_sampler = VK_NULL_HANDLE;
  }

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
  if (shader->get_language() != Shader::SL_SPIR_V) {
    vulkandisplay_cat.error()
      << "Vulkan can only consume SPIR-V shaders.\n";
    return nullptr;
  }

  PStatTimer timer(_prepare_shader_pcollector);

  VkResult err;
  const Shader::ShaderType shader_types[] = {Shader::ST_vertex, Shader::ST_fragment};
  VulkanShaderContext *sc = new VulkanShaderContext(shader);

  for (int i = 0; i < 2; ++i) {
    std::string code = shader->get_text(shader_types[i]);

    VkShaderModuleCreateInfo module_info;
    module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    module_info.pNext = nullptr;
    module_info.flags = 0;
    module_info.codeSize = code.size();
    module_info.pCode = (const uint32_t *)code.data();

    err = vkCreateShaderModule(_device, &module_info, nullptr, &sc->_modules[i]);
    if (err) {
      vulkan_error(err, "Failed to load shader module");
      delete sc;
      return nullptr;
    }
  }

  if (!sc->make_pipeline_layout(_device)) {
    delete sc;
    return nullptr;
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
  if (sc->_modules[0] != VK_NULL_HANDLE) {
    vkDestroyShaderModule(_device, sc->_modules[0], nullptr);
    sc->_modules[0] = VK_NULL_HANDLE;
  }
  if (sc->_modules[1] != VK_NULL_HANDLE) {
    vkDestroyShaderModule(_device, sc->_modules[1], nullptr);
    sc->_modules[1] = VK_NULL_HANDLE;
  }

  // Destroy the pipeline states that use these modules.
  //TODO: is this safe?
  for (const auto &item : sc->_pipeline_map) {
    vkDestroyPipeline(_device, item.second, nullptr);
  }
  sc->_pipeline_map.clear();

  vkDestroyPipelineLayout(_device, sc->_pipeline_layout, nullptr);
  vkDestroyDescriptorSetLayout(_device, sc->_descriptor_set_layout, nullptr);

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
  // This is called during begin_frame, after the fence has been waited upon,
  // so we know that any command buffers using this have finished executing.
  VulkanVertexBufferContext *vbc;
  DCAST_INTO_V(vbc, context);

  if (vbc->_buffer) {
    vkDestroyBuffer(_device, vbc->_buffer, nullptr);
    vbc->_buffer = nullptr;
  }

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
  // This is called during begin_frame, after the fence has been waited upon,
  // so we know that any command buffers using this have finished executing.
  VulkanIndexBufferContext *ibc;
  DCAST_INTO_V(ibc, context);

  if (ibc->_buffer) {
    vkDestroyBuffer(_device, ibc->_buffer, nullptr);
    ibc->_buffer = nullptr;
  }

  delete ibc;
}

/**
 * Dispatches a currently bound compute shader using the given work group
 * counts.
 */
void VulkanGraphicsStateGuardian::
dispatch_compute(int num_groups_x, int num_groups_y, int num_groups_z) {
  //TODO: must actually be outside render pass, and on a queue that supports
  // compute.  Should we have separate pool/queue/buffer for compute?
  vkCmdDispatch(_cmd, num_groups_x, num_groups_y, num_groups_z);
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
  // This does nothing, because we can't make a pipeline state without knowing
  // the vertex format.
  _state_rs = state;

  VulkanShaderContext *sc = _default_sc;
  _current_shader = sc;
  nassertv(sc != nullptr);

  // Put the modelview projection matrix in the push constants.
  CPT(TransformState) combined = _projection_mat->compose(trans);
  LMatrix4f matrix = LCAST(float, combined->get_mat());
  vkCmdPushConstants(_cmd, sc->_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, 64, matrix.get_data());

  const ColorScaleAttrib *color_scale_attrib;
  state->get_attrib_def(color_scale_attrib);
  LColorf color = LCAST(float, color_scale_attrib->get_scale());
  vkCmdPushConstants(_cmd, sc->_pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 64, 16, color.get_data());

  const TextureAttrib *tex_attrib;
  state->get_attrib_def(tex_attrib);

  Texture *texture;
  if (tex_attrib->has_on_stage(TextureStage::get_default())) {
    texture = tex_attrib->get_on_texture(TextureStage::get_default());
    VulkanTextureContext *tc;
    DCAST_INTO_V(tc, texture->prepare_now(0, get_prepared_objects(), this));
    tc->set_active(true);
    update_texture(tc, true);

    // Transition the texture so that it can be read by the shader.  This has
    // to happen on the transfer command buffer, since it can't happen during
    // an active render pass.
    tc->transition(_transfer_cmd, _graphics_queue_family_index,
                   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                   VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                   VK_ACCESS_SHADER_READ_BIT);
  }

  VkDescriptorSet ds = get_descriptor_set(state);
  vkCmdBindDescriptorSets(_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          sc->_pipeline_layout, 0, 1, &ds, 0, nullptr);
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
  nassertv(_cmd != VK_NULL_HANDLE);
  nassertv(clearable->is_any_clear_active());

  VkClearAttachment attachments[2];
  int ai = 0;

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
    if (clearable->get_clear_depth_active()) {
      attachment.aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    if (clearable->get_clear_stencil_active()) {
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

  vkCmdClearAttachments(_cmd, ai, attachments, _viewports.size(), rects);
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

  vkCmdSetViewport(_cmd, 0, count, viewports);
  vkCmdSetScissor(_cmd, 0, count, &_viewports[0]);
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

  VkResult err;

  {
    PStatTimer timer(_flush_pcollector);

    // Make sure that the previous command buffer is done executing, so that
    // we don't update or delete resources while they're still being used.
    // We should probably come up with a better mechanism for this.
    //NB: if we remove this, we also need to change the code in release_xxx in
    // order not to delete resources that are still being used.
    err = vkWaitForFences(_device, 1, &_fence, VK_TRUE, 1000000000ULL);
    if (err == VK_TIMEOUT) {
      vulkandisplay_cat.error()
        << "Timed out waiting for previous frame to complete rendering.\n";
      return false;
    } else if (err) {
      vulkan_error(err, "Failure waiting for command buffer fence");
      return false;
    }
  }

  // Now we can return any memory that was pending deletion.
  for (VkBuffer buffer : _pending_delete_buffers) {
    vkDestroyBuffer(_device, buffer, nullptr);
  }
  _pending_delete_buffers.clear();
  _pending_free.clear();

  // Reset the fence to unsignaled status.
  err = vkResetFences(_device, 1, &_fence);
  nassertr(!err, false);

  if (_cmd == VK_NULL_HANDLE) {
    // Create a command buffer.
    VkCommandBufferAllocateInfo alloc_info;
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.pNext = nullptr;
    alloc_info.commandPool = _cmd_pool;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = 2;

    VkCommandBuffer buffers[2];
    err = vkAllocateCommandBuffers(_device, &alloc_info, buffers);
    nassertr(!err, false);
    _cmd = buffers[0];
    _transfer_cmd = buffers[1];
    nassertr(_cmd != VK_NULL_HANDLE, false);
  }

  // Begin the transfer command buffer, for preparing resources.
  VkCommandBufferBeginInfo begin_info;
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.pNext = nullptr;
  begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  begin_info.pInheritanceInfo = nullptr;

  err = vkBeginCommandBuffer(_transfer_cmd, &begin_info);
  if (err) {
    vulkan_error(err, "Can't begin command buffer");
    return false;
  }

  // Make sure we have a white texture.
  if (_white_texture.is_null()) {
    _white_texture = new Texture();
    _white_texture->setup_2d_texture(1, 1, Texture::T_unsigned_byte, Texture::F_rgba8);
    _white_texture->set_clear_color(LColor(1, 1, 1, 1));
    _white_texture->prepare_now(0, get_prepared_objects(), this);
  }

  // Update the "null" vertex buffer.
  LVecBase4f data[2] = {LVecBase4(0, 0, 0, 1), LVecBase4(1, 1, 1, 1)};
  vkCmdUpdateBuffer(_transfer_cmd, _color_vertex_buffer, 0, 32, (const uint32_t *)data[0].get_data());

  // Call the GSG's begin_frame, which will cause any queued-up release() and
  // prepare() methods to be called.  Note that some of them may add to the
  // command buffer, which is why we've begun it already.
  if (!GraphicsStateGuardian::begin_frame(current_thread)) {
    return false;
  }

  // Let's submit our preparation calls, so the GPU has something to munch on.
  //vkEndCommandBuffer(_transfer_cmd);

  /*VkSubmitInfo submit_info;
  submit_info.pNext = nullptr;
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.waitSemaphoreCount = 0;
  submit_info.pWaitSemaphores = nullptr;
  submit_info.pWaitDstStageMask = nullptr;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &_transfer_cmd;
  submit_info.signalSemaphoreCount = 0;
  submit_info.pSignalSemaphores = nullptr;
  err = vkQueueSubmit(_queue, 1, &submit_info, VK_NULL_HANDLE);
  if (err) {
    vulkan_error(err, "Failed to submit preparation command buffer");
    return false;
  }*/

  // Now begin the main (ie. graphics) command buffer.
  err = vkBeginCommandBuffer(_cmd, &begin_info);
  if (err) {
    vulkan_error(err, "Can't begin command buffer");
    return false;
  }

  return true;
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

  nassertv(_transfer_cmd != VK_NULL_HANDLE);
  vkEndCommandBuffer(_transfer_cmd);

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

    vkCmdPipelineBarrier(_cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0,
                         0, nullptr, (uint32_t)num_downloads, barriers, 0, nullptr);
  }

  nassertv(_cmd != VK_NULL_HANDLE);
  vkEndCommandBuffer(_cmd);

  VkCommandBuffer cmdbufs[] = {_transfer_cmd, _cmd};

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
  err = vkQueueSubmit(_queue, 1, &submit_info, _fence);
  if (err) {
    vulkan_error(err, "Error submitting queue");
    return;
  }

  // We're done with these for now.
  _wait_semaphore = VK_NULL_HANDLE;
  _signal_semaphore = VK_NULL_HANDLE;

  // If we queued up texture downloads, wait for the queue to finish (slow!)
  // and then copy the data from Vulkan host memory to Panda memory.
  if (!_download_queue.empty()) {
    {
      PStatTimer timer(_flush_pcollector);
      err = vkWaitForFences(_device, 1, &_fence, VK_TRUE, ~0ULL);
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
 * Called before a sequence of draw_primitive() functions are called, this
 * should prepare the vertex data for rendering.  It returns true if the
 * vertices are ok, false to abort this group of primitives.
 */
bool VulkanGraphicsStateGuardian::
begin_draw_primitives(const GeomPipelineReader *geom_reader,
                      const GeomVertexDataPipelineReader *data_reader,
                      bool force) {
  if (!GraphicsStateGuardian::begin_draw_primitives(geom_reader, data_reader, force)) {
    return false;
  }

  // We need to have a valid shader to be able to render anything.
  if (_current_shader == nullptr) {
    return false;
  }

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

  vkCmdBindVertexBuffers(_cmd, 0, num_arrays + 1, buffers, offsets);

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

  nassertr(_fb_color_tc != nullptr, false);
  VulkanTextureContext *fbtc = _fb_color_tc;

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
  fbtc->transition(_cmd, _graphics_queue_family_index,
    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_READ_BIT);

  tc->transition(_cmd, _graphics_queue_family_index,
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

    vkCmdCopyImage(_cmd, fbtc->_image, fbtc->_layout, tc->_image, tc->_layout, 1, &region);
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

    vkCmdBlitImage(_cmd, fbtc->_image, fbtc->_layout, tc->_image, tc->_layout, 1, &region, VK_FILTER_NEAREST);
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
  VkDeviceSize buffer_size = tc->_extent.width * tc->_extent.height * 4;

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
  VkCommandBuffer cmd = _cmd;

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

  VkPipeline pipeline = _current_shader->get_pipeline(this, _state_rs, _format, topology);
  nassertr(pipeline != VK_NULL_HANDLE, false);
  vkCmdBindPipeline(_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

  int num_vertices = reader->get_num_vertices();

  if (reader->is_indexed()) {
    // This is an indexed primitive.  Set up and bind the index buffer.
    VulkanIndexBufferContext *ibc;
    DCAST_INTO_R(ibc, reader->prepare_now(get_prepared_objects(), this), false);

    if (!update_index_buffer(ibc, reader, force)) {
      return false;
    }

    vkCmdBindIndexBuffer(_cmd, ibc->_buffer, 0, ibc->_index_type);
    vkCmdDrawIndexed(_cmd, num_vertices, 1, 0, 0, 0);
  } else {
    // A non-indexed primitive.
    vkCmdDraw(_cmd, num_vertices, 1, reader->get_first_vertex(), 0);
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
              const GeomVertexFormat *format, VkPrimitiveTopology topology) {
  if (vulkandisplay_cat.is_debug()) {
    vulkandisplay_cat.debug()
      << "Making pipeline for state " << *state << " and format " << *format << "\n";
  }

  VkPipelineShaderStageCreateInfo stages[2];
  stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  stages[0].pNext = nullptr;
  stages[0].flags = 0;
  stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
  stages[0].module = sc->_modules[0];
  stages[0].pName = "main";
  stages[0].pSpecializationInfo = nullptr;

  stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  stages[1].pNext = nullptr;
  stages[1].flags = 0;
  stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  stages[1].module = sc->_modules[1];
  stages[1].pName = "main";
  stages[1].pSpecializationInfo = nullptr;

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

  // Now describe each vertex attribute (ie. GeomVertexColumn).
  const Shader *shader = _default_sc->get_shader();
  pvector<Shader::ShaderVarSpec>::const_iterator it;

  VkVertexInputAttributeDescription *attrib_desc = (VkVertexInputAttributeDescription *)
    alloca(sizeof(VkVertexInputAttributeDescription) * shader->_var_spec.size());

  i = 0;
  for (it = shader->_var_spec.begin(); it != shader->_var_spec.end(); ++it) {
    const Shader::ShaderVarSpec &spec = *it;
    int array_index;
    const GeomVertexColumn *column;

    attrib_desc[i].location = spec._id._seqno;

    if (!format->get_array_info(spec._name, array_index, column)) {
      // The shader references a non-existent vertex column.  To make this a
      // well-defined operation (as in OpenGL), we bind a "dummy" vertex buffer
      // containing a fixed value with a stride of 0.
      if (dummy_binding == -1) {
        dummy_binding = num_bindings++;
      }

      attrib_desc[i].binding = dummy_binding;
      if (spec._name == InternalName::get_color()) {
        // Look up the offset into the color palette.
        const ColorAttrib *color_attr;
        state->get_attrib_def(color_attr);
        LColorf color = LCAST(float, color_attr->get_color());

        ColorPaletteIndices::const_iterator it = _color_palette.find(color);
        if (it != _color_palette.end()) {
          attrib_desc[i].offset = it->second * 16;
        } else {
          // Not yet in the palette.  Write an entry.
          if (_next_palette_index >= vulkan_color_palette_size) {
            attrib_desc[i].offset = 1;
            vulkandisplay_cat.error()
              << "Ran out of color palette entries.  Increase "
                 "vulkan-color-palette-size value in Config.prc.\n";
          } else {
            uint32_t offset = _next_palette_index * 16;
            attrib_desc[i].offset = offset;
            _color_palette[color] = _next_palette_index++;
            vkCmdUpdateBuffer(_transfer_cmd, _color_vertex_buffer, offset, 16,
                              (const uint32_t *)color.get_data());
          }
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
        attrib_desc[i].format = VK_FORMAT_B8G8R8A8_UNORM;
      } else {
        attrib_desc[i].format = VK_FORMAT_B8G8R8A8_UINT;
      }
      break;
    case GeomEnums::NT_packed_dabc:
      if (normalized) {
        attrib_desc[i].format = VK_FORMAT_A8B8G8R8_UNORM_PACK32;
      } else {
        attrib_desc[i].format = VK_FORMAT_A8B8G8R8_UINT_PACK32;
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
  ms_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
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
  pipeline_info.stageCount = 2;
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
 * Returns a VkDescriptorSet for the resources of the given render state.
 */
VkDescriptorSet VulkanGraphicsStateGuardian::
get_descriptor_set(const RenderState *state) {
  DescriptorSetKey key;
  key._tex_attrib =
    (const TextureAttrib *)state->get_attrib_def(TextureAttrib::get_class_slot());
  key._shader_attrib =
    (const ShaderAttrib *)state->get_attrib_def(ShaderAttrib::get_class_slot());

  DescriptorSetMap::const_iterator it;
  it = _descriptor_set_map.find(key);
  if (it == _descriptor_set_map.end()) {
    VkDescriptorSet ds = make_descriptor_set(state);
    _descriptor_set_map[std::move(key)] = ds;
    return ds;
  } else {
    return it->second;
  }
}

/**
 * Creates a VkDescriptorSet for the resources of the given render state.
 */
VkDescriptorSet VulkanGraphicsStateGuardian::
make_descriptor_set(const RenderState *state) {
  if (vulkandisplay_cat.is_debug()) {
    vulkandisplay_cat.debug()
      << "Making descriptor set for state " << *state << "\n";
  }

  VkResult err;
  VkDescriptorSet ds;

  //TODO: layout creation.
  VkDescriptorSetAllocateInfo alloc_info;
  alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  alloc_info.pNext = nullptr;
  alloc_info.descriptorPool = _descriptor_pool;
  alloc_info.descriptorSetCount = 1;
  alloc_info.pSetLayouts = &_descriptor_set_layout;
  err = vkAllocateDescriptorSets(_device, &alloc_info, &ds);
  if (err) {
    vulkan_error(err, "Failed to allocate descriptor set");
    return VK_NULL_HANDLE;
  }

  // Now fill in the descriptor set with bindings.
  const TextureAttrib *tex_attrib;
  state->get_attrib_def(tex_attrib);

  Texture *texture;
  SamplerState sampler;

  if (tex_attrib->has_on_stage(TextureStage::get_default())) {
    texture = tex_attrib->get_on_texture(TextureStage::get_default());
    sampler = tex_attrib->get_on_sampler(TextureStage::get_default());
    nassertr(texture != nullptr, VK_NULL_HANDLE);
  } else {
    // Just use a white texture.
    texture = _white_texture;
  }

  VulkanTextureContext *tc;
  DCAST_INTO_R(tc, texture->prepare_now(0, get_prepared_objects(), this), VK_NULL_HANDLE);

  VulkanSamplerContext *sc;
  DCAST_INTO_R(sc, sampler.prepare_now(get_prepared_objects(), this), VK_NULL_HANDLE);

  VkDescriptorImageInfo image_info;
  image_info.sampler = sc->_sampler;
  image_info.imageView = tc->_image_view;
  image_info.imageLayout = tc->_layout;

  VkWriteDescriptorSet write;
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.pNext = nullptr;
  write.dstSet = ds;
  write.dstBinding = 0;
  write.dstArrayElement = 0;
  write.descriptorCount = 1;
  write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  write.pImageInfo = &image_info;
  write.pBufferInfo = nullptr;
  write.pTexelBufferView = nullptr;

  vkUpdateDescriptorSets(_device, 1, &write, 0, nullptr);

  return ds;
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
  case VK_FORMAT_R8G8B8A8_UNORM:
  case VK_FORMAT_B8G8R8A8_UNORM:
    type = Texture::T_unsigned_byte;
    format = Texture::F_rgba8;
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
