/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vulkanGraphicsBuffer.cxx
 * @author rdb
 * @date 2016-04-13
 */

#include "vulkanGraphicsBuffer.h"
#include "vulkanGraphicsStateGuardian.h"

TypeHandle VulkanGraphicsBuffer::_type_handle;

/**
 *
 */
VulkanGraphicsBuffer::
VulkanGraphicsBuffer(GraphicsEngine *engine, GraphicsPipe *pipe,
                     const std::string &name,
                     const FrameBufferProperties &fb_prop,
                     const WindowProperties &win_prop,
                     int flags,
                     GraphicsStateGuardian *gsg,
                     GraphicsOutput *host) :
  GraphicsBuffer(engine, pipe, name, fb_prop, win_prop, flags, gsg, host) {
}

/**
 *
 */
VulkanGraphicsBuffer::
~VulkanGraphicsBuffer() {
}

/**
 * Clears the entire framebuffer before rendering, according to the settings
 * of get_color_clear_active() and get_depth_clear_active() (inherited from
 * DrawableRegion).
 *
 * This function is called only within the draw thread.
 */
void GraphicsOutput::
clear(Thread *current_thread) {
  // We do the clear in begin_frame(), and the validation layers don't like it
  // if an extra clear is being done at the beginning of a frame.  That's why
  // this is empty for now.  Need a cleaner solution for this.
}

/**
 * This function will be called within the draw thread before beginning
 * rendering for a given frame.  It should do whatever setup is required, and
 * return true if the frame should be rendered, or false if it should be
 * skipped.
 */
bool VulkanGraphicsBuffer::
begin_frame(FrameMode mode, Thread *current_thread) {
  begin_frame_spam(mode);
  if (_gsg == nullptr) {
    return false;
  }

  if (mode != FM_render) {
    return true;
  }

  VulkanGraphicsStateGuardian *vkgsg;
  DCAST_INTO_R(vkgsg, _gsg, false);
  //vkgsg->reset_if_new();

  if (_current_clear_mask != _clear_mask) {
    // The clear flags have changed.  Recreate the render pass.  Note that the
    // clear flags don't factor into render pass compatibility, so we don't
    // need to recreate the framebuffer.
    vkQueueWaitIdle(vkgsg->_queue);
    setup_render_pass();
  }

  if (_framebuffer_size != _size) {
    // Uh-oh, the window must have resized.  Recreate the framebuffer.
    // Before destroying the old, make sure the queue is no longer rendering
    // anything to it.
    vkQueueWaitIdle(vkgsg->_queue);
    destroy_framebuffer();
    if (!create_framebuffer()) {
      return false;
    }
  }

  // Instruct the GSG that we are commencing a new frame.  This will cause it
  // to create a command buffer.
  vkgsg->set_current_properties(&get_fb_properties());
  if (!vkgsg->begin_frame(current_thread)) {
    return false;
  }

  /*if (mode == FM_render) {
    clear_cube_map_selection();
  }*/

  // Now that we have a command buffer, start our render pass.  First
  // transition the swapchain images into the valid state for rendering into.
  VkCommandBuffer cmd = vkgsg->_cmd;

  VkClearValue clears[2];
  LColor clear_color = get_clear_color();
  clears[0].color.float32[0] = clear_color[0];
  clears[0].color.float32[1] = clear_color[1];
  clears[0].color.float32[2] = clear_color[2];
  clears[0].color.float32[3] = clear_color[3];

  VkRenderPassBeginInfo begin_info;
  begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  begin_info.pNext = nullptr;
  begin_info.renderPass = _render_pass;
  begin_info.framebuffer = _framebuffer;
  begin_info.renderArea.offset.x = 0;
  begin_info.renderArea.offset.y = 0;
  begin_info.renderArea.extent.width = _size[0];
  begin_info.renderArea.extent.height = _size[1];
  begin_info.clearValueCount = 1;
  begin_info.pClearValues = clears;

  VkImageMemoryBarrier *barriers = (VkImageMemoryBarrier *)
    alloca(sizeof(VkImageMemoryBarrier) * _attachments.size());

  for (size_t i = 0; i < _attachments.size(); ++i) {
    const Attachment &attach = _attachments[i];
    barriers[i].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barriers[i].pNext = nullptr;
    barriers[i].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barriers[i].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barriers[i].oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barriers[i].newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    barriers[i].srcQueueFamilyIndex = vkgsg->_graphics_queue_family_index;
    barriers[i].dstQueueFamilyIndex = vkgsg->_graphics_queue_family_index;
    barriers[i].image = attach._image;
    barriers[i].subresourceRange.aspectMask = attach._aspect;
    barriers[i].subresourceRange.baseMipLevel = 0;
    barriers[i].subresourceRange.levelCount = 1;
    barriers[i].subresourceRange.baseArrayLayer = 0;
    barriers[i].subresourceRange.layerCount = 1;
  }

  vkCmdBeginRenderPass(cmd, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
  vkgsg->_render_pass = _render_pass;

  return true;
}

/**
 * This function will be called within the draw thread after rendering is
 * completed for a given frame.  It should do whatever finalization is
 * required.
 */
void VulkanGraphicsBuffer::
end_frame(FrameMode mode, Thread *current_thread) {
  end_frame_spam(mode);

  if (mode == FM_render) {
    VulkanGraphicsStateGuardian *vkgsg;
    DCAST_INTO_V(vkgsg, _gsg);
    VkCommandBuffer cmd = vkgsg->_cmd;
    nassertv(cmd != VK_NULL_HANDLE);

    vkCmdEndRenderPass(cmd);
    vkgsg->_render_pass = VK_NULL_HANDLE;

    if (mode == FM_render) {
      copy_to_textures();
    }

    // Note: this will close the command buffer.
    _gsg->end_frame(current_thread);

    trigger_flip();
    clear_cube_map_selection();
  }
}

/**
 * Closes the buffer right now.  Called from the window thread.
 */
void VulkanGraphicsBuffer::
close_buffer() {
  if (!_gsg.is_null()) {
    VulkanGraphicsStateGuardian *vkgsg;
    DCAST_INTO_V(vkgsg, _gsg);

    // Wait until the queue is done with any commands that might use the swap
    // chain, then destroy it.
    vkQueueWaitIdle(vkgsg->_queue);
    destroy_framebuffer();

    if (_render_pass != VK_NULL_HANDLE) {
      vkDestroyRenderPass(vkgsg->_device, _render_pass, nullptr);
      _render_pass = VK_NULL_HANDLE;
    }

    _gsg.clear();
  }
}

/**
 * Opens the buffer right now.  Called from the window thread.  Returns true
 * if the buffer is successfully opened, or false if there was a problem.
 */
bool VulkanGraphicsBuffer::
open_buffer() {
  VulkanGraphicsPipe *vkpipe;
  DCAST_INTO_R(vkpipe, _pipe, false);

  // Choose a suitable color format.  Sorted in order of preferability,
  // preferring lower bpps over higher bpps, and preferring formats that pack
  // bits in fewer channels (because if the user only requests red bits, they
  // probably would prefer to maximize the amount of red bits rather than to
  // have bits in other channels that they don't end up using)
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
    {24,  8,  8,  8,  8, false, VK_FORMAT_R8G8B8A8_UNORM},
    {32, 16, 16,  0,  0, false, VK_FORMAT_R16G16_SFLOAT},
    {30, 10, 10, 10,  2, false, VK_FORMAT_A2B10G10R10_UNORM_PACK32},
    {48, 16, 16, 16, 16,  true, VK_FORMAT_R16G16B16A16_SFLOAT},
    {32, 32,  0,  0,  0,  true, VK_FORMAT_R32_SFLOAT},
    {64, 32, 32,  0,  0,  true, VK_FORMAT_R32G32_SFLOAT},
    {96, 32, 32, 32, 32,  true, VK_FORMAT_R32G32B32A32_SFLOAT},
    {0}
  };

  if (_fb_properties.get_srgb_color()) {
    // This the only sRGB format.  Deal with it.
    _color_format = VK_FORMAT_R8G8B8A8_SRGB;
    _fb_properties.set_rgba_bits(8, 8, 8, 8);
    _fb_properties.set_float_color(false);

  } else {
    for (int i = 0; formats[i].r; ++i) {
      if (formats[i].r >= _fb_properties.get_red_bits() &&
          formats[i].g >= _fb_properties.get_green_bits() &&
          formats[i].b >= _fb_properties.get_blue_bits() &&
          formats[i].a >= _fb_properties.get_alpha_bits() &&
          formats[i].rgb >= _fb_properties.get_color_bits() &&
          formats[i].has_float >= _fb_properties.get_float_color()) {

        // This format meets the requirements.
        _color_format = formats[i].format;
        _fb_properties.set_rgba_bits(formats[i].r, formats[i].g,
                                     formats[i].b, formats[i].a);
        break;
      }
    }
  }

  // Choose a suitable depth/stencil format that satisfies the requirements.
  VkFormatProperties fmt_props;
  bool request_depth32 = _fb_properties.get_depth_bits() > 24 ||
                         _fb_properties.get_float_depth();

  if (_fb_properties.get_depth_bits() > 0 || _fb_properties.get_stencil_bits() > 0) {
    // Vulkan requires support for at least of one of these two formats.
    vkGetPhysicalDeviceFormatProperties(vkpipe->_gpu, VK_FORMAT_D32_SFLOAT_S8_UINT, &fmt_props);
    bool supports_depth32 = (fmt_props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0;
    vkGetPhysicalDeviceFormatProperties(vkpipe->_gpu, VK_FORMAT_D24_UNORM_S8_UINT, &fmt_props);
    bool supports_depth24 = (fmt_props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0;

    if ((supports_depth32 && request_depth32) || !supports_depth24) {
      _depth_stencil_format = VK_FORMAT_D32_SFLOAT_S8_UINT;
      _fb_properties.set_depth_bits(32);
    } else {
      _depth_stencil_format = VK_FORMAT_D24_UNORM_S8_UINT;
      _fb_properties.set_depth_bits(24);
    }
    _fb_properties.set_stencil_bits(8);

    _depth_stencil_aspect_mask |= VK_IMAGE_ASPECT_DEPTH_BIT |
                                  VK_IMAGE_ASPECT_STENCIL_BIT;

  } else if (_fb_properties.get_depth_bits() > 0) {
    // Vulkan requires support for at least of one of these two formats.
    vkGetPhysicalDeviceFormatProperties(vkpipe->_gpu, VK_FORMAT_D32_SFLOAT, &fmt_props);
    bool supports_depth32 = (fmt_props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0;
    vkGetPhysicalDeviceFormatProperties(vkpipe->_gpu, VK_FORMAT_X8_D24_UNORM_PACK32, &fmt_props);
    bool supports_depth24 = (fmt_props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0;

    if ((supports_depth32 && request_depth32) || !supports_depth24) {
      _depth_stencil_format = VK_FORMAT_D32_SFLOAT;
      _fb_properties.set_depth_bits(32);
    } else {
      _depth_stencil_format = VK_FORMAT_X8_D24_UNORM_PACK32;
      _fb_properties.set_depth_bits(24);
    }

    _depth_stencil_aspect_mask |= VK_IMAGE_ASPECT_DEPTH_BIT;

  } else {
    _depth_stencil_format = VK_FORMAT_UNDEFINED;
    _depth_stencil_aspect_mask |= 0;
  }

  return setup_render_pass() && create_framebuffer();
}

/**
 * Creates a render pass object for this buffer.  Call this whenever the
 * format or clear parameters change.  Note that all pipeline states become
 * invalid if the render pass is no longer compatible; however, we currently
 * call this only when the clear flags change, which does not affect pipeline
 * compatibility.
 */
bool VulkanGraphicsBuffer::
setup_render_pass() {
  VulkanGraphicsStateGuardian *vkgsg;
  DCAST_INTO_R(vkgsg, _gsg, false);

  if (vulkandisplay_cat.is_debug()) {
    vulkandisplay_cat.debug()
      << "Creating render pass for VulkanGraphicsBuffer " << this << "\n";
  }

  // Now we want to create a render pass, and for that we need to describe the
  // framebuffer attachments as well as any subpasses we'd like to use.
  VkAttachmentDescription attachments[2];
  attachments[0].flags = 0;
  attachments[0].format = _color_format;
  attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  attachments[1].flags = 0;
  attachments[1].format = _depth_stencil_format;
  attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  if (get_clear_color_active()) {
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  } else {
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
  }

  if (get_clear_depth_active()) {
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  }

  if (get_clear_stencil_active()) {
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  }

  VkAttachmentReference color_reference;
  color_reference.attachment = 0;
  color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depth_reference;
  depth_reference.attachment = 1;
  depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass;
  subpass.flags = 0;
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.inputAttachmentCount = 0;
  subpass.pInputAttachments = nullptr;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &color_reference;
  subpass.pResolveAttachments = nullptr;
  subpass.pDepthStencilAttachment = _depth_stencil_format ? &depth_reference : nullptr;
  subpass.preserveAttachmentCount = 0;
  subpass.pPreserveAttachments = nullptr;

  VkRenderPassCreateInfo pass_info;
  pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  pass_info.pNext = nullptr;
  pass_info.flags = 0;
  pass_info.attachmentCount = _depth_stencil_format ? 2 : 1;
  pass_info.pAttachments = attachments;
  pass_info.subpassCount = 1;
  pass_info.pSubpasses = &subpass;
  pass_info.dependencyCount = 0;
  pass_info.pDependencies = nullptr;

  VkRenderPass pass;
  VkResult
  err = vkCreateRenderPass(vkgsg->_device, &pass_info, nullptr, &pass);
  if (err) {
    vulkan_error(err, "Failed to create render pass");
    return false;
  }

  // Destroy the previous render pass object.
  if (_render_pass != VK_NULL_HANDLE) {
    // Actually, we can't destroy it, since we may now have pipeline states
    // that reference it.  Destroying it now would also require destroying the
    // framebuffer and clearing all of the prepared states from the GSG.
    // Maybe we need to start reference counting render passes?
    vulkandisplay_cat.warning() << "Leaking VkRenderPass.\n";
    //vkDestroyRenderPass(vkgsg->_device, _render_pass, nullptr);
    //_render_pass = VK_NULL_HANDLE;
  }

  _render_pass = pass;
  _current_clear_mask = _clear_mask;
  return true;
}

/**
 * Destroys an existing swapchain.  Before calling this, make sure that no
 * commands are executing on any queue that uses this swapchain.
 */
void VulkanGraphicsBuffer::
destroy_framebuffer() {
  VulkanGraphicsStateGuardian *vkgsg;
  DCAST_INTO_V(vkgsg, _gsg);
  VkDevice device = vkgsg->_device;

  // Make sure that the GSG's command buffer releases its resources.
  if (vkgsg->_cmd != VK_NULL_HANDLE) {
    vkResetCommandBuffer(vkgsg->_cmd, 0);
  }

  /*if (!_present_cmds.empty()) {
    vkFreeCommandBuffers(device, vkgsg->_cmd_pool, _present_cmds.size(), &_present_cmds[0]);
    _present_cmds.clear();
  }*/

  // Destroy the resources held for each attachment.
  Attachments::iterator it;
  for (it = _attachments.begin(); it != _attachments.end(); ++it) {
    Attachment &attach = *it;

    vkDestroyImageView(device, attach._image_view, nullptr);
    vkFreeMemory(device, attach._memory, nullptr);
  }
  _attachments.clear();

  if (_framebuffer != VK_NULL_HANDLE) {
    vkDestroyFramebuffer(device, _framebuffer, nullptr);
    _framebuffer = VK_NULL_HANDLE;
  }
}

/**
 * Creates or recreates the framebuffer.
 */
bool VulkanGraphicsBuffer::
create_framebuffer() {
  VulkanGraphicsPipe *vkpipe;
  VulkanGraphicsStateGuardian *vkgsg;
  DCAST_INTO_R(vkpipe, _pipe, false);
  DCAST_INTO_R(vkgsg, _gsg, false);
  VkDevice device = vkgsg->_device;
  VkResult err;

  if (!create_attachment(_color_format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_ASPECT_COLOR_BIT) ||
      !create_attachment(_depth_stencil_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT)) {
    return false;
  }

  int num_attachments = (int)_attachments.size();
  VkImageView *attach_views = (VkImageView *)alloca(sizeof(VkImageView) * num_attachments);

  for (int i = 0; i < num_attachments; ++i) {
    attach_views[i] = _attachments[i]._image_view;
  }

  VkFramebufferCreateInfo fb_info;
  fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  fb_info.pNext = nullptr;
  fb_info.flags = 0;
  fb_info.renderPass = _render_pass;
  fb_info.attachmentCount = num_attachments;
  fb_info.pAttachments = attach_views;
  fb_info.width = _size[0];
  fb_info.height = _size[1];
  fb_info.layers = 1;

  err = vkCreateFramebuffer(device, &fb_info, nullptr, &_framebuffer);
  if (err) {
    vulkan_error(err, "Failed to create framebuffer");
    return false;
  }

  _framebuffer_size = _size;
  return true;
}

/**
 * Adds a new attachment to the framebuffer.
 * @return Returns true on success.
 */
bool VulkanGraphicsBuffer::
create_attachment(VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspect) {
  VulkanGraphicsPipe *vkpipe;
  DCAST_INTO_R(vkpipe, _pipe, false);

  VulkanGraphicsStateGuardian *vkgsg;
  DCAST_INTO_R(vkgsg, _gsg, false);
  VkDevice device = vkgsg->_device;

  Attachment attach;
  attach._format = format;
  attach._usage = usage;
  attach._aspect = aspect;

  VkImageCreateInfo img_info;
  img_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  img_info.pNext = nullptr;
  img_info.flags = 0;
  img_info.imageType = VK_IMAGE_TYPE_2D;
  img_info.format = format;
  img_info.extent.width = _size[0];
  img_info.extent.height = _size[1];
  img_info.extent.depth = 1;
  img_info.mipLevels = 1;
  img_info.arrayLayers = 1;
  img_info.samples = VK_SAMPLE_COUNT_1_BIT;
  img_info.tiling = VK_IMAGE_TILING_OPTIMAL;
  img_info.usage = usage;
  img_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  img_info.queueFamilyIndexCount = 0;
  img_info.pQueueFamilyIndices = nullptr;
  img_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

  VkResult
  err = vkCreateImage(device, &img_info, nullptr, &attach._image);
  if (err) {
    vulkan_error(err, "Failed to create image");
    return false;
  }

  // Get the memory requirements, and find an appropriate heap to alloc in.
  VkMemoryRequirements mem_reqs;
  vkGetImageMemoryRequirements(device, attach._image, &mem_reqs);

  VkMemoryAllocateInfo alloc_info;
  alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  alloc_info.pNext = nullptr;
  alloc_info.allocationSize = mem_reqs.size;

  if (!vkpipe->find_memory_type(alloc_info.memoryTypeIndex, mem_reqs.memoryTypeBits, 0)) {
    vulkan_error(err, "Failed to find memory heap to allocate depth buffer");
    return false;
  }

  VkDeviceMemory memory;
  err = vkAllocateMemory(device, &alloc_info, nullptr, &memory);
  if (err) {
    vulkan_error(err, "Failed to allocate memory for depth image");
    return false;
  }

  // Bind memory to image.
  err = vkBindImageMemory(device, attach._image, memory, 0);
  if (err) {
    vulkan_error(err, "Failed to bind memory to depth image");
    return false;
  }

  VkImageViewCreateInfo view_info;
  view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  view_info.pNext = nullptr;
  view_info.flags = 0;
  view_info.image = attach._image;
  view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
  view_info.format = format;
  view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
  view_info.subresourceRange.aspectMask = aspect;
  view_info.subresourceRange.baseMipLevel = 0;
  view_info.subresourceRange.levelCount = 1;
  view_info.subresourceRange.baseArrayLayer = 0;
  view_info.subresourceRange.layerCount = 1;

  err = vkCreateImageView(device, &view_info, nullptr, &attach._image_view);
  if (err) {
    vulkan_error(err, "Failed to create image view for attachment");
    return false;
  }

  _attachments.push_back(attach);
  return true;
}
