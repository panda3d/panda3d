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
#include "standardMunger.h"

#include "colorBlendAttrib.h"
#include "colorWriteAttrib.h"
#include "cullFaceAttrib.h"
#include "depthTestAttrib.h"
#include "depthWriteAttrib.h"
#include "renderModeAttrib.h"

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
  _cmd_pool(VK_NULL_HANDLE),
  _cmd(VK_NULL_HANDLE),
  _render_pass(VK_NULL_HANDLE),
  _pipeline_cache(VK_NULL_HANDLE),
  _pipeline_layout(VK_NULL_HANDLE),
  _default_sc(NULL)
{
  const char *const layers[] = {
    "VK_LAYER_LUNARG_standard_validation",
  };

  const char *extensions[] = {
    "VK_KHR_swapchain",
  };

  // Create a queue in the given queue family.
  const float queue_priorities[1] = {0.0f};
  VkDeviceQueueCreateInfo queue_info;
  queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_info.pNext = NULL;
  queue_info.flags = 0;
  queue_info.queueFamilyIndex = queue_family_index;
  queue_info.queueCount = 1;
  queue_info.pQueuePriorities = queue_priorities;

  VkDeviceCreateInfo device_info;
  device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  device_info.pNext = NULL;
  device_info.flags = 0;
  device_info.queueCreateInfoCount = 1;
  device_info.pQueueCreateInfos = &queue_info;
  device_info.enabledLayerCount = 1;
  device_info.ppEnabledLayerNames = layers;
  device_info.enabledExtensionCount = 1;
  device_info.ppEnabledExtensionNames = extensions;
  device_info.pEnabledFeatures = NULL;

  VkResult
  err = vkCreateDevice(pipe->_gpu, &device_info, NULL, &_device);
  if (err) {
    vulkan_error(err, "Failed to create device");
    return;
  }

  vkGetDeviceQueue(_device, queue_family_index, 0, &_queue);

  // Create a command pool to allocate command buffers from.
  VkCommandPoolCreateInfo pool_info;
  pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  pool_info.pNext = NULL;
  pool_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT |
                    VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  pool_info.queueFamilyIndex = queue_family_index;

  err = vkCreateCommandPool(_device, &pool_info, NULL, &_cmd_pool);
  if (err) {
    vulkan_error(err, "Failed to create command pool");
    return;
  }

  // Create a pipeline cache, which may help with performance.
  VkPipelineCacheCreateInfo cache_info;
  cache_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
  cache_info.pNext = NULL;
  cache_info.flags = 0;
  cache_info.initialDataSize = 0;
  cache_info.pInitialData = NULL;

  err = vkCreatePipelineCache(_device, &cache_info, NULL, &_pipeline_cache);
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
  layout_binding.pImmutableSamplers = NULL;

  VkDescriptorSetLayoutCreateInfo set_info;
  set_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  set_info.pNext = NULL;
  set_info.flags = 0;
  set_info.bindingCount = 0;
  set_info.pBindings = &layout_binding;

  VkDescriptorSetLayout desc_set_layout;
  err = vkCreateDescriptorSetLayout(_device, &set_info, NULL, &desc_set_layout);
  if (err) {
    vulkan_error(err, "Failed to create descriptor set layout");
  }

  // Create a pipeline layout.  We'll do that here for now.
  VkPushConstantRange ranges[2];
  ranges[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  ranges[0].offset = 0;
  ranges[0].size = 64;
  ranges[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  ranges[1].offset = 64;
  ranges[1].size = 16;

  VkPipelineLayoutCreateInfo layout_info;
  layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  layout_info.pNext = NULL;
  layout_info.flags = 0;
  layout_info.setLayoutCount = 1;
  layout_info.pSetLayouts = &desc_set_layout;
  layout_info.pushConstantRangeCount = 2;
  layout_info.pPushConstantRanges = ranges;

  err = vkCreatePipelineLayout(_device, &layout_info, NULL, &_pipeline_layout);
  if (err) {
    vulkan_error(err, "Failed to create pipeline layout");
    return;
  }

  // Fill in the features supported by this physical device.
  const VkPhysicalDeviceLimits &limits = pipe->_gpu_properties.limits;
  const VkPhysicalDeviceFeatures &features = pipe->_gpu_features;
  _is_hardware = (pipe->_gpu_properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_CPU);

  _max_vertices_per_array = max((uint32_t)0x7fffffff, limits.maxDrawIndexedIndexValue);
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
  _supports_compressed_texture = (features.textureCompressionBC != VK_FALSE);

  if (features.textureCompressionBC) {
    _compressed_texture_formats.set_bit(Texture::CM_dxt1);
    _compressed_texture_formats.set_bit(Texture::CM_dxt3);
    _compressed_texture_formats.set_bit(Texture::CM_dxt5);
    _compressed_texture_formats.set_bit(Texture::CM_rgtc);
  }

  // Assume no limits on number of lights or clip planes.
  _max_lights = -1;
  _max_clip_planes = -1;

  _supports_occlusion_query = false;
  _supports_timer_query = false;

  // Initially, we set this to false; a GSG that knows it has this property
  // should set it to true.
  _copy_texture_inverted = false;

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

  _supported_geom_rendering =
    Geom::GR_indexed_point |
    Geom::GR_point |
    Geom::GR_indexed_other |
    Geom::GR_triangle_strip | Geom::GR_triangle_fan |
    Geom::GR_line_strip;
  //TODO: designate provoking vertex used for flat shading
  //TODO: add flags indicating support for render modes

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
}

/**
 * Returns the vendor of the video card driver
 */
string VulkanGraphicsStateGuardian::
get_driver_vendor() {
  VulkanGraphicsPipe *vkpipe;
  DCAST_INTO_R(vkpipe, get_pipe(), NULL);

  const char *vendor = vkpipe->get_vendor_name();
  if (vendor != NULL) {
    return string(vendor);
  } else {
    char vendor[24];
    sprintf(vendor, "Unknown vendor 0x%04X", vkpipe->_gpu_properties.vendorID);
    return string(vendor);
  }
}

/**
 * Returns GL_Renderer
 */
string VulkanGraphicsStateGuardian::
get_driver_renderer() {
  VulkanGraphicsPipe *vkpipe;
  DCAST_INTO_R(vkpipe, get_pipe(), NULL);
  return string(vkpipe->_gpu_properties.deviceName);
}

/**
 * Returns driver version This has an implementation-defined meaning, and may
 * be "" if the particular graphics implementation does not provide a way to
 * query this information.
 */
string VulkanGraphicsStateGuardian::
get_driver_version() {
  return string();
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
  VulkanGraphicsPipe *vkpipe;
  DCAST_INTO_R(vkpipe, get_pipe(), NULL);

  VkResult err;
  VkImageCreateInfo image_info;
  image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  image_info.pNext = NULL;
  image_info.flags = 0;

  int depth = 1;
  int num_layers = 1;

  switch (texture->get_texture_type()) {
  case Texture::TT_1d_texture:
    image_info.imageType = VK_IMAGE_TYPE_1D;
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
    return (TextureContext *)NULL;
  }

  //TODO: compressed formats.
  //TODO: check format support and adjust as appropriate.  In particular,
  // rgb8 does not seem to be supported on most drivers.
  Texture::Format format = texture->get_format();
  bool is_signed = !Texture::is_unsigned(texture->get_component_type());
  switch (format) {
  case Texture::F_depth_stencil:
    image_info.format = VK_FORMAT_D24_UNORM_S8_UINT;
    break;
  case Texture::F_color_index:
    image_info.format = VK_FORMAT_R8_UINT;
    break;
  case Texture::F_red:
  case Texture::F_green:
  case Texture::F_blue:
    image_info.format = (VkFormat)(VK_FORMAT_R8_UNORM + is_signed);
    break;
  case Texture::F_rgb:
    image_info.format = (VkFormat)(VK_FORMAT_R8G8B8_UNORM + is_signed);
    break;
  case Texture::F_rgb5:
    image_info.format = VK_FORMAT_R5G6B5_UNORM_PACK16;
    break;
  case Texture::F_rgb8:
    image_info.format = (VkFormat)(VK_FORMAT_R8G8B8_UNORM + is_signed);
    break;
  case Texture::F_rgb12:
    image_info.format = (VkFormat)(VK_FORMAT_R16G16B16_UNORM + is_signed);
    break;
  case Texture::F_rgb332:
    image_info.format = VK_FORMAT_R5G6B5_UNORM_PACK16;
    break;
  case Texture::F_rgba:
    image_info.format = (VkFormat)(VK_FORMAT_R8G8B8A8_UNORM + is_signed);
    break;
  case Texture::F_rgbm:
    image_info.format = (VkFormat)(VK_FORMAT_R8G8B8A8_UNORM + is_signed);
    break;
  case Texture::F_rgba4:
    image_info.format = VK_FORMAT_R4G4B4A4_UNORM_PACK16;
    break;
  case Texture::F_rgba5:
    image_info.format = VK_FORMAT_R5G5B5A1_UNORM_PACK16;
    break;
  case Texture::F_rgba8:
    image_info.format = (VkFormat)(VK_FORMAT_R8G8B8A8_UNORM + is_signed);
    break;
  case Texture::F_rgba12:
    image_info.format = (VkFormat)(VK_FORMAT_R16G16B16A16_UNORM + is_signed);
    break;
  case Texture::F_luminance:
    image_info.format = (VkFormat)(VK_FORMAT_R8_UNORM + is_signed);
    break;
  case Texture::F_luminance_alpha:
    image_info.format = (VkFormat)(VK_FORMAT_R8G8_UNORM + is_signed);
    break;
  case Texture::F_luminance_alphamask:
    image_info.format = (VkFormat)(VK_FORMAT_R8G8_UNORM + is_signed);
    break;
  case Texture::F_rgba16:
    image_info.format = (VkFormat)(VK_FORMAT_R16G16B16A16_UNORM + is_signed);
    break;
  case Texture::F_rgba32:
    image_info.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    break;
  case Texture::F_depth_component:
    image_info.format = (VkFormat)(VK_FORMAT_D16_UNORM + is_signed);
    break;
  case Texture::F_depth_component16:
    image_info.format = (VkFormat)(VK_FORMAT_D16_UNORM + is_signed);
    break;
  case Texture::F_depth_component24:
    image_info.format = (VkFormat)(VK_FORMAT_X8_D24_UNORM_PACK32 + is_signed);
    break;
  case Texture::F_depth_component32:
    image_info.format = VK_FORMAT_D32_SFLOAT;
    break;
  case Texture::F_r16:
    image_info.format = (VkFormat)(VK_FORMAT_R16_UNORM + is_signed);
    break;
  case Texture::F_rg16:
    image_info.format = (VkFormat)(VK_FORMAT_R16G16_UNORM + is_signed);
    break;
  case Texture::F_rgb16:
    image_info.format = (VkFormat)(VK_FORMAT_R16G16B16_UNORM + is_signed);
    break;
  case Texture::F_srgb:
    image_info.format = VK_FORMAT_R8G8B8_SRGB;
    break;
  case Texture::F_srgb_alpha:
    image_info.format = VK_FORMAT_R8G8B8A8_SRGB;
    break;
  case Texture::F_sluminance:
    image_info.format = VK_FORMAT_R8_SRGB;
    break;
  case Texture::F_sluminance_alpha:
    image_info.format = VK_FORMAT_R8G8B8A8_SRGB;
    break;
  case Texture::F_r32i:
    image_info.format = (VkFormat)(VK_FORMAT_R32_UINT + is_signed);
    break;
  case Texture::F_r32:
    image_info.format = VK_FORMAT_R32_SFLOAT;
    break;
  case Texture::F_rg32:
    image_info.format = VK_FORMAT_R32G32_SFLOAT;
    break;
  case Texture::F_rgb32:
    image_info.format = VK_FORMAT_R32G32B32_SFLOAT;
    break;
  case Texture::F_r8i:
    image_info.format = (VkFormat)(VK_FORMAT_R8_UINT + is_signed);
    break;
  case Texture::F_rg8i:
    image_info.format = (VkFormat)(VK_FORMAT_R8G8_UINT + is_signed);
    break;
  case Texture::F_rgb8i:
    image_info.format = (VkFormat)(VK_FORMAT_R8G8B8_UINT + is_signed);
    break;
  case Texture::F_rgba8i:
    image_info.format = (VkFormat)(VK_FORMAT_R8G8B8A8_UINT + is_signed);
    break;
  case Texture::F_r11_g11_b10:
    image_info.format = VK_FORMAT_B10G11R11_UFLOAT_PACK32;
    break;
  case Texture::F_rgb9_e5:
    image_info.format = VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
    break;
  case Texture::F_rgb10_a2:
    image_info.format = (VkFormat)(VK_FORMAT_A2R10G10B10_UNORM_PACK32 + is_signed);
    break;
  case Texture::F_rg:
    image_info.format = (VkFormat)(VK_FORMAT_R8G8_UNORM + is_signed);
    break;
  }

  int num_levels = 1;
  if (texture->uses_mipmaps()) {
    num_levels = texture->get_expected_num_mipmap_levels();
  }

  //TODO: check that size is acceptable.
  image_info.extent.width = texture->get_x_size();
  image_info.extent.height = texture->get_y_size();
  image_info.extent.depth = depth;
  image_info.mipLevels = num_levels;
  image_info.arrayLayers = num_layers;
  image_info.samples = VK_SAMPLE_COUNT_1_BIT;
  image_info.tiling = VK_IMAGE_TILING_LINEAR;
  image_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
  image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  image_info.queueFamilyIndexCount = 0;
  image_info.pQueueFamilyIndices = NULL;
  image_info.initialLayout = VK_IMAGE_LAYOUT_GENERAL;

  VkImage image;
  err = vkCreateImage(_device, &image_info, NULL, &image);
  if (err) {
    vulkan_error(err, "Failed to create texture image");
    return (TextureContext *)NULL;
  }

  // Get the memory requirements, and find an appropriate heap to alloc in.
  VkMemoryRequirements mem_reqs;
  vkGetImageMemoryRequirements(_device, image, &mem_reqs);

  VkMemoryAllocateInfo alloc_info;
  alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  alloc_info.pNext = NULL;
  alloc_info.allocationSize = mem_reqs.size;

  if (!vkpipe->find_memory_type(alloc_info.memoryTypeIndex, mem_reqs.memoryTypeBits, 0)) {
    vulkan_error(err, "Failed to find memory heap to allocate texture memory");
    return (TextureContext *)NULL;
  }

  VkDeviceMemory memory;
  err = vkAllocateMemory(_device, &alloc_info, NULL, &memory);
  if (err) {
    vulkan_error(err, "Failed to allocate memory for texture image");
    return (TextureContext *)NULL;
  }

  // Bind memory to image.
  err = vkBindImageMemory(_device, image, memory, 0);
  if (err) {
    vulkan_error(err, "Failed to bind memory to texture image");
    return (TextureContext *)NULL;
  }

  // Now we'll create an image view that describes how we interpret the image.
  VkImageViewCreateInfo view_info;
  view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  view_info.pNext = NULL;
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
    view_info.components.r = VK_COMPONENT_SWIZZLE_ZERO;
    view_info.components.g = VK_COMPONENT_SWIZZLE_ZERO;
    view_info.components.b = VK_COMPONENT_SWIZZLE_ZERO;
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

  if (format == Texture::F_depth_stencil ||
      format == Texture::F_depth_component ||
      format == Texture::F_depth_component16 ||
      format == Texture::F_depth_component24 ||
      format == Texture::F_depth_component32) {
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
  } else {
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  }
  view_info.subresourceRange.baseMipLevel = 0;
  view_info.subresourceRange.levelCount = num_levels;
  view_info.subresourceRange.baseArrayLayer = 0;
  view_info.subresourceRange.layerCount = num_layers;

  VkImageView image_view;
  err = vkCreateImageView(_device, &view_info, NULL, &image_view);
  if (err) {
    vulkan_error(err, "Failed to create image view for texture");
    return (TextureContext *)NULL;
  }

  VulkanTextureContext *tc = new VulkanTextureContext(get_prepared_objects(), texture, view);
  tc->_image = image;
  tc->_memory = memory;
  tc->_image_view = image_view;
  tc->update_data_size_bytes(alloc_info.allocationSize);
  return tc;
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
update_texture(TextureContext *, bool force) {
  return true;
}

/**
 * Frees the resources previously allocated via a call to prepare_texture(),
 * including deleting the TextureContext itself, if it is non-NULL.
 */
void VulkanGraphicsStateGuardian::
release_texture(TextureContext *) {
}

/**
 * This method should only be called by the GraphicsEngine.  Do not call it
 * directly; call GraphicsEngine::extract_texture_data() instead.
 *
 * This method will be called in the draw thread to download the texture
 * memory's image into its ram_image value.  It returns true on success, false
 * otherwise.
 */
bool VulkanGraphicsStateGuardian::
extract_texture_data(Texture *) {
  return false;
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
  VkSamplerAddressMode wrap_map[] = {VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                                     VK_SAMPLER_ADDRESS_MODE_REPEAT,
                                     VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
                                     VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE,
                                     VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
                                     VK_SAMPLER_ADDRESS_MODE_REPEAT};

  //TODO: support shadow filter and border color.
  VkSamplerCreateInfo sampler_info;
  sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  sampler_info.pNext = NULL;
  sampler_info.flags = 0;
  sampler_info.magFilter = (VkFilter)(sampler.get_effective_magfilter() & 1);
  sampler_info.minFilter = (VkFilter)(sampler.get_effective_minfilter() & 1);
  sampler_info.mipmapMode = (VkSamplerMipmapMode)(sampler.get_effective_minfilter() >> 1);
  sampler_info.addressModeU = wrap_map[sampler.get_wrap_u()];
  sampler_info.addressModeV = wrap_map[sampler.get_wrap_v()];
  sampler_info.addressModeW = wrap_map[sampler.get_wrap_w()];
  sampler_info.mipLodBias = sampler.get_lod_bias();
  sampler_info.maxAnisotropy = sampler.get_anisotropic_degree();
  sampler_info.compareEnable = VK_FALSE;
  sampler_info.compareOp = VK_COMPARE_OP_NEVER;
  sampler_info.minLod = sampler.get_min_lod();
  sampler_info.maxLod = sampler.get_max_lod();
  sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
  sampler_info.unnormalizedCoordinates = VK_FALSE;

  VkResult err;
  VkSampler vk_sampler;
  err = vkCreateSampler(_device, &sampler_info, NULL, &vk_sampler);
  if (err) {
    return (SamplerContext *)NULL;
  }

  return new VulkanSamplerContext(sampler, vk_sampler);
}

/**
 * Frees the resources previously allocated via a call to prepare_sampler(),
 * including deleting the SamplerContext itself, if it is non-NULL.
 */
void VulkanGraphicsStateGuardian::
release_sampler(SamplerContext *) {
}

/**
 * Prepares the indicated Geom for retained-mode rendering, by creating
 * whatever structures are necessary in the GSG (for instance, vertex
 * buffers). Returns the newly-allocated GeomContext that can be used to
 * render the geom.
 */
GeomContext *VulkanGraphicsStateGuardian::
prepare_geom(Geom *) {
  return (GeomContext *)NULL;
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
    return (ShaderContext *)NULL;
  }

  VkResult err;
  const Shader::ShaderType shader_types[] = {Shader::ST_vertex, Shader::ST_fragment};
  VulkanShaderContext *sc = new VulkanShaderContext(shader);

  for (int i = 0; i < 2; ++i) {
    string code = shader->get_text(shader_types[i]);

    VkShaderModuleCreateInfo module_info;
    module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    module_info.pNext = NULL;
    module_info.flags = 0;
    module_info.codeSize = code.size();
    module_info.pCode = (const uint32_t *)code.data();

    err = vkCreateShaderModule(_device, &module_info, NULL, &sc->_modules[i]);
    if (err) {
      vulkan_error(err, "Failed to load shader module");
      delete sc;
      return (ShaderContext *)NULL;
    }
  }

  return sc;
}

/**
 * Releases the resources allocated by prepare_shader
 */
void VulkanGraphicsStateGuardian::
release_shader(ShaderContext *sc) {
}

/**
 * Prepares the indicated buffer for retained-mode rendering.
 */
VertexBufferContext *VulkanGraphicsStateGuardian::
prepare_vertex_buffer(GeomVertexArrayData *array_data) {
  VulkanGraphicsPipe *vkpipe;
  DCAST_INTO_R(vkpipe, get_pipe(), NULL);

  CPT(GeomVertexArrayDataHandle) handle = array_data->get_handle();
  size_t data_size = handle->get_data_size_bytes();

  VkBufferCreateInfo buf_info;
  buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buf_info.pNext = NULL;
  buf_info.flags = 0;
  buf_info.size = data_size;
  buf_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  buf_info.queueFamilyIndexCount = 0;
  buf_info.pQueueFamilyIndices = NULL;

  VkResult err;
  VkBuffer buffer;
  err = vkCreateBuffer(_device, &buf_info, NULL, &buffer);
  if (err)  {
    vulkan_error(err, "Failed to create vertex buffer");
    return NULL;
  }

  VkMemoryRequirements mem_reqs;
  vkGetBufferMemoryRequirements(_device, buffer, &mem_reqs);

  VkMemoryAllocateInfo alloc_info;
  alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  alloc_info.pNext = NULL;
  alloc_info.allocationSize = mem_reqs.size;

  // Find a host visible memory heap, since we're about to map it.
  if (!vkpipe->find_memory_type(alloc_info.memoryTypeIndex, mem_reqs.memoryTypeBits,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
    vulkan_error(err, "Failed to find memory heap to allocate vertex buffer");
    vkDestroyBuffer(_device, buffer, NULL);
    return NULL;
  }

  VkDeviceMemory memory;
  err = vkAllocateMemory(_device, &alloc_info, NULL, &memory);
  if (err) {
    vulkan_error(err, "Failed to allocate memory for vertex buffer");
    vkDestroyBuffer(_device, buffer, NULL);
    return NULL;
  }

  err = vkBindBufferMemory(_device, buffer, memory, 0);
  if (err) {
    vulkan_error(err, "Failed to bind memory to vertex buffer");
    vkDestroyBuffer(_device, buffer, NULL);
    vkFreeMemory(_device, memory, NULL);
    return NULL;
  }

  VulkanVertexBufferContext *vbc = new VulkanVertexBufferContext(_prepared_objects, array_data);
  vbc->_buffer = buffer;
  vbc->_memory = memory;
  vbc->update_data_size_bytes(alloc_info.allocationSize);

  void *data;
  err = vkMapMemory(_device, memory, 0, alloc_info.allocationSize, 0, &data);
  if (err || !data) {
    vulkan_error(err, "Failed to map vertex buffer memory");
    vkDestroyBuffer(_device, buffer, NULL);
    vkFreeMemory(_device, memory, NULL);
    return NULL;
  }

  const unsigned char *source_data = handle->get_read_pointer(true);
  memcpy(data, source_data, data_size);

  vkUnmapMemory(_device, memory);
  return vbc;
}

/**
 * Frees the resources previously allocated via a call to prepare_data(),
 * including deleting the VertexBufferContext itself, if necessary.
 */
void VulkanGraphicsStateGuardian::
release_vertex_buffer(VertexBufferContext *) {
}

/**
 * Prepares the indicated buffer for retained-mode rendering.
 */
IndexBufferContext *VulkanGraphicsStateGuardian::
prepare_index_buffer(GeomPrimitive *) {
  return (IndexBufferContext *)NULL;
}

/**
 * Frees the resources previously allocated via a call to prepare_data(),
 * including deleting the IndexBufferContext itself, if necessary.
 */
void VulkanGraphicsStateGuardian::
release_index_buffer(IndexBufferContext *) {
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

  // Put the modelview projection matrix in the push constants.
  CPT(TransformState) combined = _projection_mat->compose(trans);
  LMatrix4f matrix = LCAST(float, combined->get_mat());
  vkCmdPushConstants(_cmd, _pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, 64, matrix.get_data());

  const ColorAttrib *color_attrib;
  state->get_attrib_def(color_attrib);
  LColorf color = LCAST(float, color_attrib->get_color());
  vkCmdPushConstants(_cmd, _pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 64, 16, color.get_data());
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

  if (clearable->get_clear_color_active()) {
    LColor color = clearable->get_clear_color();
    VkClearAttachment &attachment = attachments[ai++];
    attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    attachment.colorAttachment = 0;
    attachment.clearValue.color.float32[0] = color[0];
    attachment.clearValue.color.float32[1] = color[1];
    attachment.clearValue.color.float32[2] = color[2];
    attachment.clearValue.color.float32[3] = color[3];
  }

  if (clearable->get_clear_depth_active() ||
      clearable->get_clear_stencil_active()) {
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
  nassertv(dr != (DisplayRegionPipelineReader *)NULL);
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
 * Makes the current lens (whichever lens was most recently specified with
 * set_scene()) active, so that it will transform future rendered geometry.
 * Normally this is only called from the draw process, and usually it is
 * called by set_scene().
 *
 * The return value is true if the lens is acceptable, false if it is not.
 */
bool VulkanGraphicsStateGuardian::
prepare_lens() {
  if (_scene_setup->get_inverted()) {
    // Vulkan uses an upside down coordinate system.
    _projection_mat = TransformState::make_mat(_current_lens->get_projection_mat());
  } else {
    _projection_mat = TransformState::make_mat(
      _current_lens->get_projection_mat() * LMatrix4::scale_mat(1.0f, -1.0f, 1.0f));
  }

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
  if (!GraphicsStateGuardian::begin_frame(current_thread)) {
    return false;
  }

  // Create a command buffer.
  VkCommandBufferAllocateInfo alloc_info;
  alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info.pNext = NULL;
  alloc_info.commandPool = _cmd_pool;
  alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  alloc_info.commandBufferCount = 1;

  VkResult err;
  if (_cmd == VK_NULL_HANDLE) {
    err = vkAllocateCommandBuffers(_device, &alloc_info, &_cmd);
    nassertr(!err, false);
    nassertr(_cmd != VK_NULL_HANDLE, false);
  }

  VkCommandBufferBeginInfo begin_info;
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.pNext = NULL;
  begin_info.flags = 0;
  begin_info.pInheritanceInfo = NULL;

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

  nassertv(_cmd != VK_NULL_HANDLE);
  vkEndCommandBuffer(_cmd);

  //TODO: delete command buffer, schedule for deletion, or recycle.
}

/**
 * Called before a sequence of draw_primitive() functions are called, this
 * should prepare the vertex data for rendering.  It returns true if the
 * vertices are ok, false to abort this group of primitives.
 */
bool VulkanGraphicsStateGuardian::
begin_draw_primitives(const GeomPipelineReader *geom_reader,
                      const GeomMunger *munger,
                      const GeomVertexDataPipelineReader *data_reader,
                      bool force) {
  if (!GraphicsStateGuardian::begin_draw_primitives(geom_reader, munger, data_reader, force)) {
    return false;
  }

  // Prepare and bind the vertex buffers.
  int num_arrays = data_reader->get_num_arrays();
  VkBuffer *buffers = (VkBuffer *)alloca(sizeof(VkBuffer) * num_arrays);
  VkDeviceSize *offsets = (VkDeviceSize *)alloca(sizeof(VkDeviceSize) * num_arrays);

  for (int i = 0; i < num_arrays; ++i) {
    CPT(GeomVertexArrayDataHandle) handle = data_reader->get_array_reader(i);
    VulkanVertexBufferContext *vbc;
    DCAST_INTO_R(vbc, handle->prepare_now(get_prepared_objects(), this), false);
    buffers[i] = vbc->_buffer;
    offsets[i] = 0;
  }

  vkCmdBindVertexBuffers(_cmd, 0, num_arrays, buffers, offsets);

  _format = data_reader->get_format();
  return true;
}

/**
 * Draws a series of disconnected triangles.
 */
bool VulkanGraphicsStateGuardian::
draw_triangles(const GeomPrimitivePipelineReader *reader, bool force) {
  VkPipeline pipeline = get_pipeline(_state_rs, _format,
                                     VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
  nassertr(pipeline != VK_NULL_HANDLE, false);
  vkCmdBindPipeline(_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

  int num_vertices = reader->get_num_vertices();
  _vertices_tri_pcollector.add_level(num_vertices);
  _primitive_batches_tri_pcollector.add_level(1);

  if (reader->is_indexed()) {
    // Not yet supported.
    vkCmdDrawIndexed(_cmd, num_vertices, 1, 0, 0, 0);
  } else {
    vkCmdDraw(_cmd, num_vertices, 1, reader->get_first_vertex(), 0);
  }
  return true;
}

/**
 * Draws a series of triangle strips.
 */
bool VulkanGraphicsStateGuardian::
draw_tristrips(const GeomPrimitivePipelineReader *, bool) {
  return false;
}

/**
 * Draws a series of triangle fans.
 */
bool VulkanGraphicsStateGuardian::
draw_trifans(const GeomPrimitivePipelineReader *, bool) {
  return false;
}

/**
 * Draws a series of "patches", which can only be processed by a tessellation
 * shader.
 */
bool VulkanGraphicsStateGuardian::
draw_patches(const GeomPrimitivePipelineReader *, bool) {
  return false;
}

/**
 * Draws a series of disconnected line segments.
 */
bool VulkanGraphicsStateGuardian::
draw_lines(const GeomPrimitivePipelineReader *, bool) {
  return false;
}

/**
 * Draws a series of line strips.
 */
bool VulkanGraphicsStateGuardian::
draw_linestrips(const GeomPrimitivePipelineReader *, bool) {
  return false;
}

/**
 * Draws a series of disconnected points.
 */
bool VulkanGraphicsStateGuardian::
draw_points(const GeomPrimitivePipelineReader *, bool) {
  return false;
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
 * Returns a VkPipeline for the given RenderState+GeomVertexFormat combination.
 */
VkPipeline VulkanGraphicsStateGuardian::
get_pipeline(const RenderState *state, const GeomVertexFormat *format,
             VkPrimitiveTopology topology) {
  PipelineKey key;
  key._state = state;
  key._format = format;
  key._topology = topology;

  PipelineMap::const_iterator it;
  it = _pipeline_map.find(key);
  if (it == _pipeline_map.end()) {
    VkPipeline pipeline = make_pipeline(state, format, topology);
    _pipeline_map[MOVE(key)] = pipeline;
    return pipeline;
  } else {
    return it->second;
  }
}

/**
 * Creates a VkPipeline for the given RenderState+GeomVertexFormat combination.
 */
VkPipeline VulkanGraphicsStateGuardian::
make_pipeline(const RenderState *state, const GeomVertexFormat *format,
              VkPrimitiveTopology topology) {
  if (vulkandisplay_cat.is_debug()) {
    vulkandisplay_cat.debug()
      << "Making pipeline for state " << *state << " and format " << *format << "\n";
  }

  // Load the default shader.  Temporary hack.
  static PT(Shader) default_shader;
  if (default_shader.is_null()) {
    default_shader = Shader::load(Shader::SL_SPIR_V, "vert.spv", "frag.spv");
    nassertr(default_shader, NULL);

    ShaderContext *sc = default_shader->prepare_now(get_prepared_objects(), this);
    nassertr(sc, NULL);
    _default_sc = DCAST(VulkanShaderContext, sc);
  }

  VkPipelineShaderStageCreateInfo stages[2];
  stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  stages[0].pNext = NULL;
  stages[0].flags = 0;
  stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
  stages[0].module = _default_sc->_modules[0];
  stages[0].pName = "main";
  stages[0].pSpecializationInfo = NULL;

  stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  stages[1].pNext = NULL;
  stages[1].flags = 0;
  stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  stages[1].module = _default_sc->_modules[1];
  stages[1].pName = "main";
  stages[1].pSpecializationInfo = NULL;

  // Describe each vertex input binding (ie. GeomVertexArray).
  VkVertexInputBindingDescription *binding_desc = (VkVertexInputBindingDescription *)
    alloca(sizeof(VkVertexInputBindingDescription) * format->get_num_arrays());

  for (size_t i = 0; i < format->get_num_arrays(); ++i) {
    binding_desc[i].binding = i;
    binding_desc[i].stride = format->get_array(i)->get_stride();
    binding_desc[i].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  }

  // Now describe each vertex attribute (ie. GeomVertexColumn).
  VkVertexInputAttributeDescription *attrib_desc = (VkVertexInputAttributeDescription *)
    alloca(sizeof(VkVertexInputAttributeDescription) * format->get_num_columns());

  for (size_t i = 0; i < format->get_num_columns(); ++i) {
    const GeomVertexColumn *column = format->get_column(i);
    attrib_desc[i].location = i;
    attrib_desc[i].binding = format->get_array_with(i);
    attrib_desc[i].offset = column->get_start();

    // Determine which Vulkan format to map this column to.  The formats are
    // laid out somewhat (though not entirely) consistently, so we can use a
    // trick to jump to the format for the right number of components.
    int fmt_jump = column->get_num_components() - 1;
    switch (column->get_numeric_type()) {
    case GeomEnums::NT_uint8:
      if (fmt_jump < 3) {
        attrib_desc[i].format = (VkFormat)(VK_FORMAT_R8_UINT + 7 * fmt_jump);
      } else {
        attrib_desc[i].format = VK_FORMAT_R8G8B8A8_UINT;
      }
      break;
    case GeomEnums::NT_uint16:
      attrib_desc[i].format = (VkFormat)(VK_FORMAT_R16_UINT + 7 * fmt_jump);
      break;
    case GeomEnums::NT_uint32:
      attrib_desc[i].format = (VkFormat)(VK_FORMAT_R32_UINT + 3 * fmt_jump);
      break;
    case GeomEnums::NT_packed_dcba:
      attrib_desc[i].format = VK_FORMAT_B8G8R8A8_UINT;
      break;
    case GeomEnums::NT_packed_dabc:
      attrib_desc[i].format = VK_FORMAT_A8B8G8R8_UINT_PACK32;
      break;
    case GeomEnums::NT_float32:
      attrib_desc[i].format = (VkFormat)(VK_FORMAT_R32_SFLOAT + 3 * fmt_jump);
      break;
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
  }

  VkPipelineVertexInputStateCreateInfo vertex_info;
  vertex_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertex_info.pNext = NULL;
  vertex_info.flags = 0;
  vertex_info.vertexBindingDescriptionCount = format->get_num_arrays();
  vertex_info.pVertexBindingDescriptions = binding_desc;
  vertex_info.vertexAttributeDescriptionCount = format->get_num_columns();
  vertex_info.pVertexAttributeDescriptions = attrib_desc;

  VkPipelineInputAssemblyStateCreateInfo assembly_info;
  assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  assembly_info.pNext = NULL;
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
  viewport_info.pNext = NULL;
  viewport_info.flags = 0;
  viewport_info.viewportCount = 1;
  viewport_info.pViewports = NULL;
  viewport_info.scissorCount = 1;
  viewport_info.pScissors = NULL;

  const RenderModeAttrib *render_mode;
  state->get_attrib_def(render_mode);
  const CullFaceAttrib *cull_face;
  state->get_attrib_def(cull_face);

  VkPipelineRasterizationStateCreateInfo raster_info;
  raster_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  raster_info.pNext = NULL;
  raster_info.flags = 0;
  raster_info.depthClampEnable = VK_TRUE;
  raster_info.rasterizerDiscardEnable = VK_FALSE;

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

  raster_info.cullMode = (VkCullModeFlagBits)cull_face->get_effective_mode();
  raster_info.frontFace = VK_FRONT_FACE_CLOCKWISE; // Flipped
  raster_info.depthBiasEnable = VK_FALSE;
  raster_info.depthBiasConstantFactor = 0;
  raster_info.depthBiasClamp = 0;
  raster_info.depthBiasSlopeFactor = 0;
  raster_info.lineWidth = render_mode->get_thickness();

  VkPipelineMultisampleStateCreateInfo ms_info;
  ms_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  ms_info.pNext = NULL;
  ms_info.flags = 0;
  ms_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  ms_info.sampleShadingEnable = VK_FALSE;
  ms_info.minSampleShading = 0.0;
  ms_info.pSampleMask = NULL;
  ms_info.alphaToCoverageEnable = VK_FALSE;
  ms_info.alphaToOneEnable = VK_FALSE;

  const DepthWriteAttrib *depth_write;
  state->get_attrib_def(depth_write);
  const DepthTestAttrib *depth_test;
  state->get_attrib_def(depth_test);

  VkPipelineDepthStencilStateCreateInfo ds_info;
  ds_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  ds_info.pNext = NULL;
  ds_info.flags = 0;
  ds_info.depthTestEnable = (depth_test->get_mode() != RenderAttrib::M_none);
  ds_info.depthWriteEnable = depth_write->get_mode();
  ds_info.depthCompareOp = (VkCompareOp)(depth_test->get_mode() - 1);
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

  VkPipelineColorBlendAttachmentState att_state[1];
  att_state[0].blendEnable = (color_blend->get_mode() != ColorBlendAttrib::M_none);
  att_state[0].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
  att_state[0].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
  att_state[0].colorBlendOp = (VkBlendOp)(color_blend->get_mode() - 1);
  att_state[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  att_state[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  att_state[0].alphaBlendOp = (VkBlendOp)(color_blend->get_mode() - 1);
  att_state[0].colorWriteMask = color_write->get_channels();

  VkPipelineColorBlendStateCreateInfo blend_info;
  blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  blend_info.pNext = NULL;
  blend_info.flags = 0;
  blend_info.logicOpEnable = VK_FALSE;
  blend_info.logicOp = VK_LOGIC_OP_NO_OP;
  blend_info.attachmentCount = 1;
  blend_info.pAttachments = att_state;
  blend_info.blendConstants[0] = 1.0f;
  blend_info.blendConstants[1] = 1.0f;
  blend_info.blendConstants[2] = 1.0f;
  blend_info.blendConstants[3] = 1.0f;

  // Tell Vulkan that we'll be specifying the viewport and scissor separately.
  const VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

  VkPipelineDynamicStateCreateInfo dynamic_info;
  dynamic_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamic_info.pNext = NULL;
  dynamic_info.flags = 0;
  dynamic_info.dynamicStateCount = 2;
  dynamic_info.pDynamicStates = dynamic_states;

  VkGraphicsPipelineCreateInfo pipeline_info;
  pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipeline_info.pNext = NULL;
  pipeline_info.flags = 0;
  pipeline_info.stageCount = 2;
  pipeline_info.pStages = stages;
  pipeline_info.pVertexInputState = &vertex_info;
  pipeline_info.pInputAssemblyState = &assembly_info;
  pipeline_info.pTessellationState = NULL;
  pipeline_info.pViewportState = &viewport_info;
  pipeline_info.pRasterizationState = &raster_info;
  pipeline_info.pMultisampleState = &ms_info;
  pipeline_info.pDepthStencilState = &ds_info;
  pipeline_info.pColorBlendState = &blend_info;
  pipeline_info.pDynamicState = &dynamic_info;
  pipeline_info.layout = _pipeline_layout;
  pipeline_info.renderPass = _render_pass;
  pipeline_info.subpass = 0;
  pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
  pipeline_info.basePipelineIndex = 0;

  VkResult err;
  VkPipeline pipeline;
  err = vkCreateGraphicsPipelines(_device, _pipeline_cache, 1, &pipeline_info, NULL, &pipeline);
  if (err) {
    vulkan_error(err, "Failed to create graphics pipeline");
    return VK_NULL_HANDLE;
  }

  return pipeline;
}
