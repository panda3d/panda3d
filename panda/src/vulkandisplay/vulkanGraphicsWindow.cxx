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
                     const std::string &name,
                     const FrameBufferProperties &fb_prop,
                     const WindowProperties &win_prop,
                     int flags,
                     GraphicsStateGuardian *gsg,
                     GraphicsOutput *host) :
  BaseGraphicsWindow(engine, pipe, name, fb_prop, win_prop, flags, gsg, host),
  _surface(VK_NULL_HANDLE),
  _swapchain(VK_NULL_HANDLE),
  _render_pass(VK_NULL_HANDLE),
  _image_available(VK_NULL_HANDLE),
  _render_complete(VK_NULL_HANDLE),
  _current_clear_mask(-1),
  _depth_stencil_tc(nullptr),
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
 * Clears the entire framebuffer before rendering, according to the settings
 * of get_color_clear_active() and get_depth_clear_active() (inherited from
 * DrawableRegion).
 *
 * This function is called only within the draw thread.
 */
void VulkanGraphicsWindow::
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
bool VulkanGraphicsWindow::
begin_frame(FrameMode mode, Thread *current_thread) {
  begin_frame_spam(mode);
  if (_gsg == nullptr) {
    return false;
  }

  if (!get_unexposed_draw() && !_got_expose_event) {
    if (vulkandisplay_cat.is_spam()) {
      vulkandisplay_cat.spam()
        << "Not drawing " << this << ": unexposed.\n";
    }
    return false;
  }

#ifdef HAVE_X11
  if (_awaiting_configure) {
    return false;
  }
#endif

  if (vulkandisplay_cat.is_spam()) {
    vulkandisplay_cat.spam()
      << "Drawing " << this << ": exposed.\n";
  }

  /*if (mode != FM_render) {
    return true;
  }*/

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

  if (_swapchain_size != _size) {
    // Uh-oh, the window must have resized.  Recreate the swapchain.
    // Before destroying the old, make sure the queue is no longer rendering
    // anything to it.
    vkQueueWaitIdle(vkgsg->_queue);
    destroy_swapchain();
    if (!create_swapchain()) {
      return false;
    }
  }

  // Instruct the GSG that we are commencing a new frame.  This will cause it
  // to create a command buffer.
  vkgsg->set_current_properties(&get_fb_properties());
  if (!vkgsg->begin_frame(current_thread)) {
    return false;
  }

  if (mode != FM_render) {
    return true;
  }

  nassertr(_image_index < _swap_buffers.size(), false);
  SwapBuffer &buffer = _swap_buffers[_image_index];

  buffer._tc->set_active(true);

  /*if (mode == FM_render) {
    clear_cube_map_selection();
  }*/

  // Now that we have a command buffer, start our render pass.  First
  // transition the swapchain images into the valid state for rendering into.
  VkCommandBuffer cmd = vkgsg->_cmd;

  VkClearValue clears[2];

  VkRenderPassBeginInfo begin_info;
  begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  begin_info.pNext = nullptr;
  begin_info.renderPass = _render_pass;
  begin_info.framebuffer = buffer._framebuffer;
  begin_info.renderArea.offset.x = 0;
  begin_info.renderArea.offset.y = 0;
  begin_info.renderArea.extent.width = _size[0];
  begin_info.renderArea.extent.height = _size[1];
  begin_info.clearValueCount = 0;
  begin_info.pClearValues = clears;

  if (!get_clear_color_active()) {
    // If we aren't clearing (which is a bad idea - please clear the window)
    // then we need to transition it to a consistent state..
    if (buffer._tc->_layout == VK_IMAGE_LAYOUT_UNDEFINED) {
      // If the attachment is set to LOAD, we need to clear it for the first
      // time if we don't want the validation layer to yell at us.
      // We clear it to an arbitrary arbitrary color.  We'll just pick the
      // color returned by get_clear_color(), even if it is meaningless.

      // Actually, doing this causes the validation layer to yell even louder
      // because the image has no VK_IMAGE_USAGE_TRANSFER_DST_BIT.
      //buffer._tc->clear_color_image(cmd, clears[0].color);
    }

    buffer._tc->transition(cmd, vkgsg->_graphics_queue_family_index,
                           VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
  } else {
    // This transition will be made when the first subpass is started.
    buffer._tc->_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    buffer._tc->_access_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    buffer._tc->_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    LColor clear_color = get_clear_color();
    clears[begin_info.clearValueCount].color.float32[0] = clear_color[0];
    clears[begin_info.clearValueCount].color.float32[1] = clear_color[1];
    clears[begin_info.clearValueCount].color.float32[2] = clear_color[2];
    clears[begin_info.clearValueCount].color.float32[3] = clear_color[3];
    ++begin_info.clearValueCount;
  }

  if (_depth_stencil_tc != nullptr) {
    _depth_stencil_tc->set_active(true);

    // Transition the depth-stencil image to a consistent state.
    if (!get_clear_depth_active() || !get_clear_stencil_active()) {
      _depth_stencil_tc->transition(cmd, vkgsg->_graphics_queue_family_index,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
    } else {
      // This transition will be made when the first subpass is started.
      _depth_stencil_tc->_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
      _depth_stencil_tc->_access_mask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
      _depth_stencil_tc->_stage_mask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                       VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    }

    if (get_clear_depth_active() || get_clear_stencil_active()) {
      clears[begin_info.clearValueCount].depthStencil.depth = get_clear_depth();
      clears[begin_info.clearValueCount].depthStencil.stencil = get_clear_stencil();
      ++begin_info.clearValueCount;
    }
  }

  vkCmdBeginRenderPass(cmd, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
  vkgsg->_render_pass = _render_pass;
  vkgsg->_fb_color_tc = buffer._tc;
  vkgsg->_fb_depth_tc = _depth_stencil_tc;
  vkgsg->_wait_semaphore = _image_available;
  vkgsg->_signal_semaphore = _render_complete;

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

  VulkanGraphicsStateGuardian *vkgsg;
  DCAST_INTO_V(vkgsg, _gsg);

  VkCommandBuffer cmd = vkgsg->_cmd;
  nassertv(cmd != VK_NULL_HANDLE);
  SwapBuffer &buffer = _swap_buffers[_image_index];

  if (mode == FM_render) {
    vkCmdEndRenderPass(cmd);
    vkgsg->_render_pass = VK_NULL_HANDLE;

    // The driver implicitly transitioned this to the final layout.
    buffer._tc->_layout = _final_layout;
    buffer._tc->_access_mask = VK_ACCESS_MEMORY_READ_BIT;

    // Now we can do copy-to-texture, now that the render pass has ended.
    copy_to_textures();
  }

  // If we copied the textures, transition it back to the present state.
  buffer._tc->transition(cmd, vkgsg->_graphics_queue_family_index,
                         VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                         VK_ACCESS_MEMORY_READ_BIT);

  // Note: this will close the command buffer, and unsignal the previous
  // frame's semaphore.
  vkgsg->end_frame(current_thread);

  if (mode == FM_render) {
    _flip_ready = true;
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
  VulkanGraphicsStateGuardian *vkgsg;
  DCAST_INTO_V(vkgsg, _gsg);
  VkQueue queue = vkgsg->_queue;
  VkResult err;

  VkResult results[1];
  VkPresentInfoKHR present;
  present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  present.pNext = nullptr;
  present.waitSemaphoreCount = 1;
  present.pWaitSemaphores = &_render_complete;
  present.swapchainCount = 1;
  present.pSwapchains = &_swapchain;
  present.pImageIndices = &_image_index;
  present.pResults = results;

  err = vkQueuePresentKHR(queue, &present);
  if (err == VK_ERROR_OUT_OF_DATE_KHR) {
    std::cerr << "out of date.\n";

  } else if (err == VK_SUBOPTIMAL_KHR) {
    std::cerr << "suboptimal.\n";

  } else if (err != VK_SUCCESS) {
    vulkan_error(err, "Error presenting queue");
    return;
  }
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

  // Get a new image for rendering into.  This may wait until a new image is
  // available.
  VkResult
  err = vkAcquireNextImageKHR(vkgsg->_device, _swapchain, UINT64_MAX,
                              _image_available, VK_NULL_HANDLE, &_image_index);
  nassertv(err == VK_SUCCESS);

  if (vulkandisplay_cat.is_spam()) {
    vulkandisplay_cat.spam()
      << "Acquired image " << _image_index << " from swapchain\n";
  }

  // Don't flip again until we've rendered another frame.
  _flip_ready = false;
}

/**
 * Closes the window right now.  Called from the window thread.
 */
void VulkanGraphicsWindow::
close_window() {
  // Destroy the previous swapchain first, if we had one.
  if (!_gsg.is_null()) {
    VulkanGraphicsStateGuardian *vkgsg;
    DCAST_INTO_V(vkgsg, _gsg);

    // Wait until the queue is done with any commands that might use the swap
    // chain, then destroy it.
    vkQueueWaitIdle(vkgsg->_queue);
    destroy_swapchain();

    if (_render_pass != VK_NULL_HANDLE) {
      vkDestroyRenderPass(vkgsg->_device, _render_pass, nullptr);
      _render_pass = VK_NULL_HANDLE;
    }

    _gsg.clear();
  }
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

#ifdef HAVE_X11
  int depth = DefaultDepth(_display, _screen);
  _visual_info = new XVisualInfo;
  XMatchVisualInfo(_display, _screen, depth, TrueColor, _visual_info);

  setup_colormap(_visual_info);
#endif

  if (!BaseGraphicsWindow::open_window()) {
    return false;
  }

  VkResult err;

  // Create a surface using the WSI extension.
#ifdef _WIN32
  VkWin32SurfaceCreateInfoKHR surface_info;
  surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  surface_info.pNext = nullptr;
  surface_info.flags = 0;
  surface_info.hinstance = GetModuleHandle(nullptr);
  surface_info.hwnd = _hWnd;

  err = vkCreateWin32SurfaceKHR(vkpipe->_instance, &surface_info, nullptr, &_surface);

#elif defined(HAVE_X11)
  VkXlibSurfaceCreateInfoKHR surface_info;
  surface_info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
  surface_info.pNext = nullptr;
  surface_info.flags = 0;
  surface_info.dpy = _display;
  surface_info.window = _xwindow;

  err = vkCreateXlibSurfaceKHR(vkpipe->_instance, &surface_info, nullptr, &_surface);
#endif

  if (err) {
    vulkan_error(err, "Failed to create surface");
    return false;
  }

  // Make sure we have a GSG, which manages a VkDevice.
  VulkanGraphicsStateGuardian *vkgsg;
  uint32_t queue_family_index = 0;
  if (_gsg == nullptr) {
    // Find a queue suitable both for graphics and for presenting to our
    // surface.  TODO: fall back to separate graphics/present queues?
    if (!vkpipe->find_queue_family_for_surface(queue_family_index, _surface, VK_QUEUE_GRAPHICS_BIT)) {
      vulkandisplay_cat.error()
        << "Failed to find graphics queue that can present to window surface.\n";
      return false;
    }

    // There is no old gsg.  Create a new one.
    vkgsg = new VulkanGraphicsStateGuardian(_engine, vkpipe, nullptr, queue_family_index);
    _gsg = vkgsg;
  } else {
    //TODO: check that the GSG's queue can present to our surface.
    DCAST_INTO_R(vkgsg, _gsg.p(), false);
  }

  if (!vkgsg->is_valid()) {
    _gsg.clear();
    vulkandisplay_cat.error()
      << "VulkanGraphicsStateGuardian is not valid.\n";
    return false;
  }

  _fb_properties.set_force_hardware(vkgsg->is_hardware());
  _fb_properties.set_force_software(!vkgsg->is_hardware());

  // Query the preferred image formats for this surface.
  uint32_t num_formats;
  err = vkGetPhysicalDeviceSurfaceFormatsKHR(vkpipe->_gpu, _surface,
                                             &num_formats, nullptr);
  nassertr(!err, false);
  VkSurfaceFormatKHR *formats =
      (VkSurfaceFormatKHR *)alloca(sizeof(VkSurfaceFormatKHR) * num_formats);
  err = vkGetPhysicalDeviceSurfaceFormatsKHR(vkpipe->_gpu, _surface,
                                             &num_formats, formats);
  nassertr(!err, false);

  // If the format list includes just one entry of VK_FORMAT_UNDEFINED, the
  // surface has no preferred format.  Otherwise, at least one supported
  // format will be returned.
  //TODO: add more logic for picking a suitable format.
  if (num_formats == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
    if (_fb_properties.get_srgb_color()) {
      _surface_format.format = VK_FORMAT_B8G8R8A8_SRGB;
    } else {
      _surface_format.format = VK_FORMAT_B8G8R8A8_UNORM;
    }
    _surface_format.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
  } else {
    // Find a format that meets the requirements.
    nassertr(num_formats > 1, false);
    _surface_format = formats[0];

    if (_fb_properties.get_srgb_color()) {
      for (uint32_t i = 0U; i < num_formats; ++i) {
        if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB ||
            formats[i].format == VK_FORMAT_R8G8B8A8_SRGB) {
          _surface_format = formats[i];
          _fb_properties.set_rgba_bits(8, 8, 8, 8);
          break;
        }
      }
    } else {
      for (uint32_t i = 0U; i < num_formats; ++i) {
        if (formats[i].format == VK_FORMAT_B8G8R8A8_UNORM ||
            formats[i].format == VK_FORMAT_R8G8B8A8_UNORM) {
          _surface_format = formats[i];
          _fb_properties.set_rgba_bits(8, 8, 8, 8);
          break;
        }
      }
    }
  }

  // Choose a suitable depth/stencil format that satisfies the requirements.
  VkFormatProperties fmt_props;
  bool request_depth32 = _fb_properties.get_depth_bits() > 24 ||
                         _fb_properties.get_float_depth();

  if (_fb_properties.get_depth_bits() > 0 && _fb_properties.get_stencil_bits() > 0) {
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

    _depth_stencil_aspect_mask = VK_IMAGE_ASPECT_DEPTH_BIT |
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

    _depth_stencil_aspect_mask = VK_IMAGE_ASPECT_DEPTH_BIT;

  } else {
    _depth_stencil_format = VK_FORMAT_UNDEFINED;
    _depth_stencil_aspect_mask = 0;
  }

  // Don't create the swapchain yet if we haven't yet gotten the configure
  // notify event, since we don't know the final size yet.
#ifdef HAVE_X11
  _swapchain_size.set(-1, -1);
  return setup_render_pass() && (_awaiting_configure || create_swapchain());
#else
  return setup_render_pass() && create_swapchain();
#endif
}

/**
 * Creates a render pass object for this window.  Call this whenever the
 * format or clear parameters change.  Note that all pipeline states become
 * invalid if the render pass is no longer compatible; however, we currently
 * call this only when the clear flags change, which does not affect pipeline
 * compatibility.
 */
bool VulkanGraphicsWindow::
setup_render_pass() {
  VulkanGraphicsStateGuardian *vkgsg;
  DCAST_INTO_R(vkgsg, _gsg, false);

  if (vulkandisplay_cat.is_debug()) {
    vulkandisplay_cat.debug()
      << "Creating render pass for VulkanGraphicsWindow " << this << "\n";
  }

  {
    // Do we intend to copy the framebuffer to a texture?
    _final_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    CDReader cdata(_cycler);
    RenderTextures::const_iterator ri;
    for (ri = cdata->_textures.begin(); ri != cdata->_textures.end(); ++ri) {
      if ((*ri)._plane == RTP_color) {
        RenderTextureMode mode = (*ri)._rtm_mode;
        if (mode == RTM_copy_texture || mode == RTM_copy_ram) {
          _final_layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
          break;
        }
      }
    }
  }

  // Now we want to create a render pass, and for that we need to describe the
  // framebuffer attachments as well as any subpasses we'd like to use.
  VkAttachmentDescription attachments[2];
  attachments[0].flags = 0;
  attachments[0].format = _surface_format.format;
  attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  attachments[0].finalLayout = _final_layout;

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
    // We don't care about the current contents.
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
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

  if (get_clear_depth_active() && get_clear_stencil_active()) {
    // We don't care about the current contents.
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
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
    vkDestroyRenderPass(vkgsg->_device, _render_pass, nullptr);
    _render_pass = VK_NULL_HANDLE;
  }

  _render_pass = pass;
  _current_clear_mask = _clear_mask;
  return true;
}

/**
 * Destroys an existing swapchain.  Before calling this, make sure that no
 * commands are executing on any queue that uses this swapchain.
 */
void VulkanGraphicsWindow::
destroy_swapchain() {
  VulkanGraphicsStateGuardian *vkgsg;
  DCAST_INTO_V(vkgsg, _gsg);
  VkDevice device = vkgsg->_device;

  // Make sure that the GSG's command buffer releases its resources.
  if (vkgsg->_cmd != VK_NULL_HANDLE) {
    vkResetCommandBuffer(vkgsg->_cmd, 0);
  }

  // Destroy the resources held for each link in the swap chain.
  SwapBuffers::iterator it;
  for (it = _swap_buffers.begin(); it != _swap_buffers.end(); ++it) {
    SwapBuffer &buffer = *it;

    // Destroy the framebuffers that use the swapchain images.
    vkDestroyFramebuffer(device, buffer._framebuffer, nullptr);
    vkDestroyImageView(device, buffer._tc->_image_view, nullptr);

    buffer._tc->update_data_size_bytes(0);
    delete buffer._tc;
  }
  _swap_buffers.clear();

  if (_depth_stencil_tc != nullptr) {
    if (_depth_stencil_tc->_image_view != VK_NULL_HANDLE) {
      vkDestroyImageView(device, _depth_stencil_tc->_image_view, nullptr);
      _depth_stencil_tc->_image_view = VK_NULL_HANDLE;
    }

    if (_depth_stencil_tc->_image != VK_NULL_HANDLE) {
      vkDestroyImage(device, _depth_stencil_tc->_image, nullptr);
      _depth_stencil_tc->_image = VK_NULL_HANDLE;
    }

    delete _depth_stencil_tc;
    _depth_stencil_tc = nullptr;
  }

  // Destroy the previous swapchain.  This also destroys the swapchain images.
  if (_swapchain != VK_NULL_HANDLE) {
    vkDestroySwapchainKHR(device, _swapchain, nullptr);
    _swapchain = VK_NULL_HANDLE;
  }

  _image_index = 0;
}

/**
 * Creates or recreates the swapchain and framebuffer.
 */
bool VulkanGraphicsWindow::
create_swapchain() {
  VulkanGraphicsPipe *vkpipe;
  VulkanGraphicsStateGuardian *vkgsg;
  DCAST_INTO_R(vkpipe, _pipe, false);
  DCAST_INTO_R(vkgsg, _gsg, false);
  VkDevice device = vkgsg->_device;
  VkResult err;

  if (vulkandisplay_cat.is_debug()) {
    vulkandisplay_cat.debug()
      << "Creating swap chain and framebuffers for VulkanGraphicsWindow " << this << "\n";
  }

  // Get the surface capabilities to make sure we make a compatible swapchain.
  VkSurfaceCapabilitiesKHR surf_caps;
  err = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkpipe->_gpu, _surface, &surf_caps);
  if (err) {
    vulkan_error(err, "Failed to get surface capabilities");
    return false;
  }

  uint32_t num_images = (uint32_t)(1 + _fb_properties.get_back_buffers());
  num_images = std::min(surf_caps.maxImageCount, num_images);
  num_images = std::max(surf_caps.minImageCount, num_images);

  _swapchain_size.set(
    std::max(std::min((uint32_t)_size[0], surf_caps.maxImageExtent.width), surf_caps.minImageExtent.width),
    std::max(std::min((uint32_t)_size[1], surf_caps.maxImageExtent.height), surf_caps.minImageExtent.height));

  // Get the supported presentation modes for this surface.
  uint32_t num_present_modes = 0;
  err = vkGetPhysicalDeviceSurfacePresentModesKHR(vkpipe->_gpu, _surface, &num_present_modes, nullptr);
  VkPresentModeKHR *present_modes = (VkPresentModeKHR *)
    alloca(sizeof(VkPresentModeKHR) * num_present_modes);
  err = vkGetPhysicalDeviceSurfacePresentModesKHR(vkpipe->_gpu, _surface, &num_present_modes, present_modes);

  // Note that we set the usage to include VK_IMAGE_USAGE_TRANSFER_SRC_BIT
  // since we can at any time be asked to copy the framebuffer to a texture.
  VkSwapchainCreateInfoKHR swapchain_info;
  swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapchain_info.pNext = nullptr;
  swapchain_info.flags = 0;
  swapchain_info.surface = _surface;
  swapchain_info.minImageCount = num_images;
  swapchain_info.imageFormat = _surface_format.format;
  swapchain_info.imageColorSpace = _surface_format.colorSpace;
  swapchain_info.imageExtent.width = _swapchain_size[0];
  swapchain_info.imageExtent.height = _swapchain_size[1];
  swapchain_info.imageArrayLayers = 1;
  swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  swapchain_info.queueFamilyIndexCount = 0;
  swapchain_info.pQueueFamilyIndices = nullptr;
  swapchain_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
  swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swapchain_info.clipped = true;
  swapchain_info.oldSwapchain = nullptr;

  // Choose a present mode.  Use FIFO mode as fallback, which is always
  // available.
  swapchain_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
  if (!sync_video) {
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
  }

  err = vkCreateSwapchainKHR(device, &swapchain_info, nullptr, &_swapchain);
  if (err) {
    vulkan_error(err, "Failed to create swap chain");
    return false;
  }

  // Get the images in the swap chain, which may be more than requested.
  vkGetSwapchainImagesKHR(device, _swapchain, &num_images, nullptr);
  _swap_buffers.resize(num_images);
  _fb_properties.set_back_buffers(num_images - 1);
  _image_index = 0;

  memset(&_swap_buffers[0], 0, sizeof(SwapBuffer) * num_images);

  VkImage *images = (VkImage *)alloca(sizeof(VkImage) * num_images);
  vkGetSwapchainImagesKHR(device, _swapchain, &num_images, images);

  PreparedGraphicsObjects *pgo = vkgsg->get_prepared_objects();
  // Now create an image view for each image.
  for (uint32_t i = 0; i < num_images; ++i) {
    SwapBuffer &buffer = _swap_buffers[i];
    buffer._tc = new VulkanTextureContext(pgo, images[i], swapchain_info.imageFormat);
    buffer._tc->_aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT;
    buffer._tc->_extent.width = swapchain_info.imageExtent.width;
    buffer._tc->_extent.height = swapchain_info.imageExtent.height;
    buffer._tc->_extent.depth = 1;
    buffer._tc->_mip_levels = 1;
    buffer._tc->_array_layers = 1;

    VkImageViewCreateInfo view_info;
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.pNext = nullptr;
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

    err = vkCreateImageView(device, &view_info, nullptr, &buffer._tc->_image_view);
    if (err) {
      vulkan_error(err, "Failed to create image view for swapchain");
      return false;
    }
  }

  // Now create a depth image.
  _depth_stencil_tc = nullptr;
  bool have_ds = (_depth_stencil_format != VK_FORMAT_UNDEFINED);

  if (have_ds) {
    VkImageCreateInfo depth_img_info;
    depth_img_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    depth_img_info.pNext = nullptr;
    depth_img_info.flags = 0;
    depth_img_info.imageType = VK_IMAGE_TYPE_2D;
    depth_img_info.format = _depth_stencil_format;
    depth_img_info.extent.width = swapchain_info.imageExtent.width;
    depth_img_info.extent.height = swapchain_info.imageExtent.height;
    depth_img_info.extent.depth = 1;
    depth_img_info.mipLevels = 1;
    depth_img_info.arrayLayers = 1;
    depth_img_info.samples = VK_SAMPLE_COUNT_1_BIT;
    depth_img_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    depth_img_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    depth_img_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    depth_img_info.queueFamilyIndexCount = 0;
    depth_img_info.pQueueFamilyIndices = nullptr;
    depth_img_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImage depth_stencil_image;
    err = vkCreateImage(device, &depth_img_info, nullptr, &depth_stencil_image);
    if (err) {
      vulkan_error(err, "Failed to create depth image");
      return false;
    }

    // Get the memory requirements, and find an appropriate heap to alloc in.
    VkMemoryRequirements mem_reqs;
    vkGetImageMemoryRequirements(device, depth_stencil_image, &mem_reqs);

    VulkanMemoryBlock block;
    if (!vkgsg->allocate_memory(block, mem_reqs, 0, false)) {
      vulkandisplay_cat.error() << "Failed to allocate texture memory for depth image.\n";
      return false;
    }

    // Bind memory to image.
    if (!block.bind_image(depth_stencil_image)) {
      vulkan_error(err, "Failed to bind memory to depth image");
      return false;
    }

    VkImageViewCreateInfo view_info;
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.pNext = nullptr;
    view_info.flags = 0;
    view_info.image = depth_stencil_image;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = depth_img_info.format;
    view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.subresourceRange.aspectMask = _depth_stencil_aspect_mask;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    if (_fb_properties.get_stencil_bits()) {
      view_info.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    VkImageView depth_stencil_view;
    err = vkCreateImageView(device, &view_info, nullptr, &depth_stencil_view);
    if (err) {
      vulkan_error(err, "Failed to create image view for depth/stencil");
      return false;
    }

    _depth_stencil_tc = new VulkanTextureContext(pgo, depth_stencil_image, view_info.format);
    _depth_stencil_tc->_extent = depth_img_info.extent;
    _depth_stencil_tc->_mip_levels = depth_img_info.mipLevels;
    _depth_stencil_tc->_array_layers = depth_img_info.arrayLayers;
    _depth_stencil_tc->_aspect_mask = _depth_stencil_aspect_mask;
    _depth_stencil_tc->_image_view = depth_stencil_view;
    _depth_stencil_tc->_block = std::move(block);
  }

  // Now finally create a framebuffer for each link in the swap chain.
  VkImageView attach_views[2];
  if (have_ds) {
    attach_views[1] = _depth_stencil_tc->_image_view;
  }

  VkFramebufferCreateInfo fb_info;
  fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  fb_info.pNext = nullptr;
  fb_info.flags = 0;
  fb_info.renderPass = _render_pass;
  fb_info.attachmentCount = 1 + (int)have_ds;
  fb_info.pAttachments = attach_views;
  fb_info.width = swapchain_info.imageExtent.width;
  fb_info.height = swapchain_info.imageExtent.height;
  fb_info.layers = 1;

  for (uint32_t i = 0; i < num_images; ++i) {
    SwapBuffer &buffer = _swap_buffers[i];
    attach_views[0] = buffer._tc->_image_view;
    err = vkCreateFramebuffer(device, &fb_info, nullptr, &buffer._framebuffer);
    if (err) {
      vulkan_error(err, "Failed to create framebuffer");
      return false;
    }
  }

  // Create a semaphore for signalling the availability of an image.
  // It will be signalled in end_flip() and waited upon before submitting the
  // command buffers that use that image for rendering to.
  _image_available = vkgsg->create_semaphore();
  nassertr(_image_available != VK_NULL_HANDLE, false);

  // Now create another one that is signalled when we are finished rendering,
  // to indicate that it is safe to present the image.
  _render_complete = vkgsg->create_semaphore();
  nassertr(_render_complete != VK_NULL_HANDLE, false);

  // We need to acquire an image before we continue rendering.
  _image_index = 0;
  _flip_ready = false;
  err = vkAcquireNextImageKHR(vkgsg->_device, _swapchain, UINT64_MAX,
                              _image_available, VK_NULL_HANDLE, &_image_index);
  if (err) {
    vulkan_error(err, "Failed to acquire swapchain image");
    return false;
  }

  return true;
}
