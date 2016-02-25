/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vulkanGraphicsWindow.cxx
 * @author rdb
 * @date 2016-02-16
 */

#include "vulkanGraphicsWindow.h"
#include "vulkanGraphicsStateGuardian.h"

TypeHandle VulkanGraphicsWindow::_type_handle;

/**
 *
 */
VulkanGraphicsWindow::
VulkanGraphicsWindow(GraphicsEngine *engine, GraphicsPipe *pipe,
                  const string &name,
                  const FrameBufferProperties &fb_prop,
                  const WindowProperties &win_prop,
                  int flags,
                  GraphicsStateGuardian *gsg,
                  GraphicsOutput *host) :
  BaseGraphicsWindow(engine, pipe, name, fb_prop, win_prop, flags, gsg, host),
  _surface(VK_NULL_HANDLE),
  _swapchain(VK_NULL_HANDLE),
  _render_pass(VK_NULL_HANDLE),
  _present_complete(VK_NULL_HANDLE),
  _image_index(0)
{
}

/**
 *
 */
VulkanGraphicsWindow::
~VulkanGraphicsWindow() {
}

/**
 * This function will be called within the draw thread before beginning
 * rendering for a given frame.  It should do whatever setup is required, and
 * return true if the frame should be rendered, or false if it should be
 * skipped.
 */
bool VulkanGraphicsWindow::
begin_frame(FrameMode mode, Thread *current_thread) {
  begin_frame_spam(mode);
  if (_gsg == (GraphicsStateGuardian *)NULL) {
    return false;
  }

  if (!get_unexposed_draw() && !_got_expose_event) {
    if (vulkandisplay_cat.is_spam()) {
      vulkandisplay_cat.spam()
        << "Not drawing " << this << ": unexposed.\n";
    }
    return false;
  }

  if (vulkandisplay_cat.is_spam()) {
    vulkandisplay_cat.spam()
      << "Drawing " << this << ": exposed.\n";
  }

  if (mode != FM_render) {
    return true;
  }

  VulkanGraphicsStateGuardian *vkgsg;
  DCAST_INTO_R(vkgsg, _gsg, false);
  //vkgsg->reset_if_new();

  // Instruct the GSG that we are commencing a new frame.  This will cause it
  // to create a command buffer.
  vkgsg->set_current_properties(&get_fb_properties());
  if (!vkgsg->begin_frame(current_thread)) {
    return false;
  }

  VkSemaphoreCreateInfo semaphore_info;
  semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  semaphore_info.pNext = NULL;
  semaphore_info.flags = 0;

  VkResult err;
  err = vkCreateSemaphore(vkgsg->_device, &semaphore_info,
                          NULL, &_present_complete);
  nassertr(err == 0, false);

  err = vkAcquireNextImageKHR(vkgsg->_device, _swapchain, UINT64_MAX,
                              _present_complete, (VkFence)0, &_image_index);

  /*if (mode == FM_render) {
    clear_cube_map_selection();
  }*/

  // Now that we have a command buffer, start our render pass.
  VkCommandBuffer cmd = vkgsg->_cmd;

  VkClearValue clears[2];
  LColor clear_color = get_clear_color();
  clears[0].color.float32[0] = clear_color[0];
  clears[0].color.float32[1] = clear_color[1];
  clears[0].color.float32[2] = clear_color[2];
  clears[0].color.float32[3] = clear_color[3];
  clears[1].depthStencil.depth = get_clear_depth();
  clears[1].depthStencil.stencil = get_clear_stencil();

  VkRenderPassBeginInfo begin_info;
  begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  begin_info.pNext = NULL;
  begin_info.renderPass = _render_pass;
  begin_info.framebuffer = _framebuffers[_image_index];
  begin_info.renderArea.offset.x = 0;
  begin_info.renderArea.offset.y = 0;
  begin_info.renderArea.extent.width = _size[0];
  begin_info.renderArea.extent.height = _size[1];
  begin_info.clearValueCount = 2;
  begin_info.pClearValues = clears;

  vkCmdBeginRenderPass(cmd, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
  vkgsg->_render_pass = _render_pass;

  return true;
}

/**
 * This function will be called within the draw thread after rendering is
 * completed for a given frame.  It should do whatever finalization is
 * required.
 */
void VulkanGraphicsWindow::
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
 * This function will be called within the draw thread after end_frame() has
 * been called on all windows, to initiate the exchange of the front and back
 * buffers.
 *
 * This should instruct the window to prepare for the flip at the next video
 * sync, but it should not wait.
 *
 * We have the two separate functions, begin_flip() and end_flip(), to make it
 * easier to flip all of the windows at the same time.
 */
void VulkanGraphicsWindow::
begin_flip() {
}

/**
 * This function will be called within the draw thread after end_frame() has
 * been called on all windows, to initiate the exchange of the front and back
 * buffers.
 *
 * This should instruct the window to prepare for the flip when command, but
 * will not actually flip
 *
 * We have the two separate functions, begin_flip() and end_flip(), to make it
 * easier to flip all of the windows at the same time.
 */
void VulkanGraphicsWindow::
ready_flip() {
}

/**
 * This function will be called within the draw thread after begin_flip() has
 * been called on all windows, to finish the exchange of the front and back
 * buffers.
 *
 * This should cause the window to wait for the flip, if necessary.
 */
void VulkanGraphicsWindow::
end_flip() {
  VulkanGraphicsStateGuardian *vkgsg;
  DCAST_INTO_V(vkgsg, _gsg);
  VkDevice device = vkgsg->_device;
  VkQueue queue = vkgsg->_queue;
  VkFence fence = vkgsg->_fence;

  nassertv(_present_complete != VK_NULL_HANDLE);

  // Submit the GSG's command buffers to the queue.
  VkPipelineStageFlags stage_flags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  VkSubmitInfo submit_info;
  submit_info.pNext = NULL;
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.waitSemaphoreCount = 1;
  submit_info.pWaitSemaphores = &_present_complete;
  submit_info.pWaitDstStageMask = &stage_flags;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &vkgsg->_cmd;
  submit_info.signalSemaphoreCount = 0;
  submit_info.pSignalSemaphores = NULL;

  VkResult err;
  err = vkQueueSubmit(queue, 1, &submit_info, fence);
  if (err) {
    vulkan_error(err, "Error submitting queue");
    return;
  }

  VkResult results[1];
  VkPresentInfoKHR present;
  present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  present.pNext = NULL;
  present.waitSemaphoreCount = 1;
  present.pWaitSemaphores = &_present_complete;
  present.swapchainCount = 1;
  present.pSwapchains = &_swapchain;
  present.pImageIndices = &_image_index;
  present.pResults = results;

  err = vkQueuePresentKHR(queue, &present);
  if (err == VK_ERROR_OUT_OF_DATE_KHR) {
    cerr << "out of date.\n";

  } else if (err == VK_SUBOPTIMAL_KHR) {
    cerr << "suboptimal.\n";

  } else if (err != VK_SUCCESS) {
    vulkan_error(err, "Error presenting queue");
    return;
  }

  nassertv(vkQueueWaitIdle(queue) == VK_SUCCESS);
  assert(err == VK_SUCCESS);

  vkDestroySemaphore(vkgsg->_device, _present_complete, NULL);
  _present_complete = VK_NULL_HANDLE;
}

/**
 * Closes the window right now.  Called from the window thread.
 */
void VulkanGraphicsWindow::
close_window() {
  _gsg.clear();
  BaseGraphicsWindow::close_window();
}

/**
 * Opens the window right now.  Called from the window thread.  Returns true
 * if the window is successfully opened, or false if there was a problem.
 */
bool VulkanGraphicsWindow::
open_window() {
  VulkanGraphicsPipe *vkpipe;
  DCAST_INTO_R(vkpipe, _pipe, false);

  if (!BaseGraphicsWindow::open_window()) {
    return false;
  }

  // Create a surface using the WSI extension.
  VkWin32SurfaceCreateInfoKHR surface_info;
  surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  surface_info.pNext = NULL;
  surface_info.flags = 0;
  surface_info.hinstance = GetModuleHandle(NULL);
  surface_info.hwnd = _hWnd;

  VkResult err = vkCreateWin32SurfaceKHR(vkpipe->_instance, &surface_info, NULL, &_surface);
  if (err) {
    vulkan_error(err, "Failed to create Win32 surface");
    return false;
  }

  // Make sure we have a GSG, which manages a VkDevice.
  VulkanGraphicsStateGuardian *vkgsg;
  if (_gsg == NULL) {
    // Find a queue suitable both for graphics and for presenting to our
    // surface.  TODO: fall back to separate graphics/present queues?
    uint32_t queue_family_index;
    if (!vkpipe->find_queue_family_for_surface(queue_family_index, _surface, VK_QUEUE_GRAPHICS_BIT)) {
      vulkan_error(err, "Failed to find graphics queue that can present to surface");
      return false;
    }

    // There is no old gsg.  Create a new one.
    vkgsg = new VulkanGraphicsStateGuardian(_engine, vkpipe, NULL, queue_family_index);
    _gsg = vkgsg;
  } else {
    //TODO: check that the GSG's queue can present to our surface.
  }

  _fb_properties.set_force_hardware(vkgsg->is_hardware());
  _fb_properties.set_force_software(!vkgsg->is_hardware());

  VkDevice device = vkgsg->_device;

  // Query the preferred image formats for this surface.
  uint32_t num_formats;
  err = vkGetPhysicalDeviceSurfaceFormatsKHR(vkpipe->_gpu, _surface,
                                             &num_formats, NULL);
  nassertr(!err, false);
  VkSurfaceFormatKHR *formats =
      (VkSurfaceFormatKHR *)alloca(sizeof(VkSurfaceFormatKHR) * num_formats);
  err = vkGetPhysicalDeviceSurfaceFormatsKHR(vkpipe->_gpu, _surface,
                                             &num_formats, formats);
  nassertr(!err, false);

  // Get the surface capabilities to make sure we make a compatible swapchain.
  VkSurfaceCapabilitiesKHR surf_caps;
  err = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkpipe->_gpu, _surface, &surf_caps);
  if (err) {
    vulkan_error(err, "Failed to get surface capabilities");
    return false;
  }

  uint32_t num_images = (uint32_t)(1 + _fb_properties.get_back_buffers());
  num_images = min(surf_caps.maxImageCount, num_images);
  num_images = max(surf_caps.minImageCount, num_images);

  // Get the supported presentation modes for this surface.
  uint32_t num_present_modes = 0;
  err = vkGetPhysicalDeviceSurfacePresentModesKHR(vkpipe->_gpu, _surface, &num_present_modes, NULL);
  VkPresentModeKHR *present_modes = (VkPresentModeKHR *)
    alloca(sizeof(VkPresentModeKHR) * num_present_modes);
  err = vkGetPhysicalDeviceSurfacePresentModesKHR(vkpipe->_gpu, _surface, &num_present_modes, present_modes);

  VkSwapchainCreateInfoKHR swapchain_info;
  swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapchain_info.pNext = NULL;
  swapchain_info.surface = _surface;
  swapchain_info.minImageCount = num_images;
  swapchain_info.imageExtent.width = _size[0];
  swapchain_info.imageExtent.height = _size[1];
  swapchain_info.imageArrayLayers = 1;
  swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  swapchain_info.queueFamilyIndexCount = 0;
  swapchain_info.pQueueFamilyIndices = NULL;
  swapchain_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
  swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swapchain_info.clipped = true;
  swapchain_info.oldSwapchain = NULL;

  // If the format list includes just one entry of VK_FORMAT_UNDEFINED, the
  // surface has no preferred format.  Otherwise, at least one supported
  // format will be returned.
  if (num_formats == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
    swapchain_info.imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
  } else {
    nassertr(num_formats > 1, false);
    swapchain_info.imageFormat = formats[0].format;
  }
  swapchain_info.imageColorSpace = formats[0].colorSpace;

  // Choose a present mode.  Use FIFO mode as fallback, which is always
  // available.  TODO: respect sync_video when choosing a mode.
  swapchain_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
  for (size_t i = 0; i < num_present_modes; ++i) {
    if (present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
      // This is the lowest-latency non-tearing mode, so we'll take this.
      swapchain_info.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
      break;
    }
    if (present_modes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR) {
      // This is the fastest present mode, though it tears, so we'll use this
      // if mailbox mode isn't available.
      swapchain_info.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    }
  }

  err = vkCreateSwapchainKHR(device, &swapchain_info, NULL, &_swapchain);
  if (err) {
    vulkan_error(err, "Failed to create swap chain");
    return false;
  }

  // Get the images in the swap chain, which may be more than requested.
  vkGetSwapchainImagesKHR(device, _swapchain, &num_images, NULL);
  _image_views.resize(num_images);
  _framebuffers.resize(num_images);
  _fb_properties.set_back_buffers(num_images - 1);

  VkImage *images = (VkImage *)alloca(sizeof(VkImage) * num_images);
  vkGetSwapchainImagesKHR(device, _swapchain, &num_images, images);

  // Now create an image view for each image.
  for (uint32_t i = 0; i < num_images; ++i) {
    VkImageViewCreateInfo view_info;
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.pNext = NULL;
    view_info.flags = 0;
    view_info.image = images[i];
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = swapchain_info.imageFormat;
    view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    err = vkCreateImageView(device, &view_info, NULL, &_image_views[i]);
    if (err) {
      vulkan_error(err, "Failed to create image view for swapchain");
      return false;
    }
  }

  // Now create a depth image.
  VkImageCreateInfo depth_img_info;
  depth_img_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  depth_img_info.pNext = NULL;
  depth_img_info.flags = 0;
  depth_img_info.imageType = VK_IMAGE_TYPE_2D;
  depth_img_info.extent.width = _size[0];
  depth_img_info.extent.height = _size[1];
  depth_img_info.extent.depth = 1;
  depth_img_info.mipLevels = 1;
  depth_img_info.arrayLayers = 1;
  depth_img_info.samples = VK_SAMPLE_COUNT_1_BIT;
  depth_img_info.tiling = VK_IMAGE_TILING_OPTIMAL;
  depth_img_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  depth_img_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  depth_img_info.queueFamilyIndexCount = 0;
  depth_img_info.pQueueFamilyIndices = NULL;
  depth_img_info.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  // Choose a suitable format that satisfies the requirements.
  if (_fb_properties.get_depth_bits() > 24 ||
      _fb_properties.get_float_depth()) {
    _fb_properties.set_depth_bits(32);
    if (_fb_properties.get_stencil_bits() > 0) {
      depth_img_info.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
      _fb_properties.set_stencil_bits(8);
    } else {
      depth_img_info.format = VK_FORMAT_D32_SFLOAT;
    }
  } else {
    depth_img_info.format = VK_FORMAT_D24_UNORM_S8_UINT;
    _fb_properties.set_depth_bits(24);
    _fb_properties.set_stencil_bits(8);
  }

  VkImage depth_image;
  err = vkCreateImage(device, &depth_img_info, NULL, &depth_image);
  if (err) {
    vulkan_error(err, "Failed to create depth image");
    return false;
  }

  // Get the memory requirements, and find an appropriate heap to alloc in.
  VkMemoryRequirements mem_reqs;
  vkGetImageMemoryRequirements(device, depth_image, &mem_reqs);

  VkMemoryAllocateInfo alloc_info;
  alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  alloc_info.pNext = NULL;
  alloc_info.allocationSize = mem_reqs.size;

  if (!vkpipe->find_memory_type(alloc_info.memoryTypeIndex, mem_reqs.memoryTypeBits, 0)) {
    vulkan_error(err, "Failed to find memory heap to allocate depth buffer");
    return false;
  }

  VkDeviceMemory depth_mem;
  err = vkAllocateMemory(device, &alloc_info, NULL, &depth_mem);
  if (err) {
    vulkan_error(err, "Failed to allocate memory for depth image");
    return false;
  }

  // Bind memory to image.
  err = vkBindImageMemory(device, depth_image, depth_mem, 0);
  if (err) {
    vulkan_error(err, "Failed to bind memory to depth image");
    return false;
  }

  VkImageViewCreateInfo view_info;
  view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  view_info.pNext = NULL;
  view_info.flags = 0;
  view_info.image = depth_image;
  view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
  view_info.format = depth_img_info.format;
  view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
  view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
  view_info.subresourceRange.baseMipLevel = 0;
  view_info.subresourceRange.levelCount = 1;
  view_info.subresourceRange.baseArrayLayer = 0;
  view_info.subresourceRange.layerCount = 1;

  if (_fb_properties.get_stencil_bits()) {
    view_info.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
  }

  err = vkCreateImageView(device, &view_info, NULL, &_depth_stencil_view);
  if (err) {
    vulkan_error(err, "Failed to create image view for depth/stencil");
    return false;
  }

  // Now we want to create a render pass, and for that we need to describe the
  // framebuffer attachments as well as any subpasses we'd like to use.
  VkAttachmentDescription attachments[2];
  attachments[0].flags = 0;
  attachments[0].format = swapchain_info.imageFormat;
  attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  attachments[1].flags = 0;
  attachments[1].format = depth_img_info.format;
  attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

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
  subpass.pInputAttachments = NULL;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &color_reference;
  subpass.pResolveAttachments = NULL;
  subpass.pDepthStencilAttachment = &depth_reference;
  subpass.preserveAttachmentCount = 0;
  subpass.pPreserveAttachments = NULL;

  VkRenderPassCreateInfo pass_info;
  pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  pass_info.pNext = NULL;
  pass_info.flags = 0;
  pass_info.attachmentCount = 2;
  pass_info.pAttachments = attachments;
  pass_info.subpassCount = 1;
  pass_info.pSubpasses = &subpass;
  pass_info.dependencyCount = 0;
  pass_info.pDependencies = NULL;

  err = vkCreateRenderPass(device, &pass_info, NULL, &_render_pass);
  if (err) {
    vulkan_error(err, "Failed to create render pass");
    return false;
  }

  // Now finally create a framebuffer for each link in the swap chain.
  VkImageView attach_views[2];
  attach_views[1] = _depth_stencil_view;

  VkFramebufferCreateInfo fb_info;
  fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  fb_info.pNext = NULL;
  fb_info.flags = 0;
  fb_info.renderPass = _render_pass;
  fb_info.attachmentCount = 2;
  fb_info.pAttachments = attach_views;
  fb_info.width = _size[0];
  fb_info.height = _size[1];
  fb_info.layers = 1;

  for (uint32_t i = 0; i < num_images; ++i) {
    attach_views[0] = _image_views[i];
    err = vkCreateFramebuffer(device, &fb_info, NULL, &_framebuffers[i]);
    if (err) {
      vulkan_error(err, "Failed to create framebuffer");
      return false;
    }
  }

  return true;
}
