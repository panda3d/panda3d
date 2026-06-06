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

#ifdef HAVE_COCOA
#include "config_cocoadisplay.h"

#include <objc/objc.h>
#include <objc/message.h>
#endif

TypeHandle VulkanGraphicsWindow::_type_handle;

/**
 *
 */
VulkanGraphicsWindow::
VulkanGraphicsWindow(GraphicsEngine *engine, GraphicsPipe *pipe,
                     std::string name,
                     const FrameBufferProperties &fb_prop,
                     const WindowProperties &win_prop,
                     int flags,
                     GraphicsStateGuardian *gsg,
                     GraphicsOutput *host) :
  BaseGraphicsWindow(engine, pipe, std::move(name), fb_prop, win_prop, flags, gsg, host)
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
  if (_awaiting_configure_since != -1) {
    return false;
  }
#endif

  if (vulkandisplay_cat.is_spam()) {
    vulkandisplay_cat.spam()
      << "Drawing " << this << ": exposed.\n";
  }

  /*if (mode == FM_refresh) {
    return true;
  }*/

  VulkanGraphicsStateGuardian *vkgsg;
  DCAST_INTO_R(vkgsg, _gsg, false);

  if (vkgsg->needs_reset()) {
    vkQueueWaitIdle(vkgsg->_queue);
    if (_image_available != VK_NULL_HANDLE) {
      vkDestroySemaphore(vkgsg->_device, _image_available, nullptr);
      _image_available = VK_NULL_HANDLE;
    }
    destroy_swapchain();
    vkgsg->reset_if_new();
  }

  if (!vkgsg->is_valid()) {
    return false;
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
  if (!vkgsg->begin_frame(current_thread, _image_available)) {
    return false;
  }

  // Ownership of this was transferred to the VulkanFrameData.
  _image_available = VK_NULL_HANDLE;

  copy_async_screenshot();

  if (mode == FM_refresh) {
    return true;
  }

  nassertr(_image_index < _swap_buffers.size(), false);
  SwapBuffer &buffer = _swap_buffers[_image_index];

  nassertr(buffer._tc->_read_seq < vkgsg->_render_cmd._seq, false);
  buffer._tc->set_active(true);

  /*if (mode == FM_render) {
    clear_cube_map_selection();
  }*/

  // Reset this to reflect getting the swapchain image fresh from the present
  // engine.  (The multisample color image, if any, is a stable resource owned
  // by the window and keeps its synchronization state across frames.)
  buffer._tc->_write_stage_mask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
  buffer._tc->_write_access_mask = 0;
  buffer._tc->_read_stage_mask = 0;

  //NB. We can't let add_barrier pool these barriers because they would be
  // issued before the wait on the semaphore is complete.  Therefore we
  // twiddle the _write_seq as well to force it to issue it in the middle of
  // the command stream.
  if (vkgsg->_render_cmd._wait_semaphore) {
    buffer._tc->_read_seq = vkgsg->_render_cmd._seq;
    buffer._tc->_write_seq = vkgsg->_render_cmd._seq;
  }

  // If we have multisamples, we render to a separate multisample image and
  // resolve into the swapchain image acquired for this frame; otherwise we
  // render directly into the swapchain image.  Either way, only the per-frame
  // swapchain image changes, so we repoint attachment 0 here.
  if (_ms_color_tc != nullptr) {
    _framebuffer.set_attachment(0, _ms_color_tc, VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                nullptr, buffer._tc);
  } else {
    _framebuffer.set_attachment(0, buffer._tc);
  }
  return _framebuffer.begin_rendering(vkgsg, this);
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

  nassertv((VkCommandBuffer)vkgsg->_render_cmd != VK_NULL_HANDLE);
  SwapBuffer &buffer = _swap_buffers[_image_index];

  VkSemaphore signal_done = VK_NULL_HANDLE;
  if (mode != FM_refresh) {
    _framebuffer.end_rendering(vkgsg);

    // Now we can do copy-to-texture, now that the render pass has ended.
    copy_to_textures();

    signal_done = buffer._render_complete;

    // If we copied the textures, transition it back to the present state.
    //if (buffer._tc->_layout != VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
    vkgsg->_render_cmd.add_barrier(buffer._tc,
                                   VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                                   VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
                                   0);
    //}
  }

  // Note: this will close the command buffer, and unsignal the previous
  // frame's semaphore.
  vkgsg->end_frame(current_thread, signal_done);

  if (mode == FM_render) {
    nassertv(!_flip_ready);
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

  SwapBuffer &buffer = _swap_buffers[_image_index];

  VkResult results[1] = {VK_SUCCESS};
  VkPresentInfoKHR present;
  present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  present.pNext = nullptr;
  present.waitSemaphoreCount = 1;
  present.pWaitSemaphores = &buffer._render_complete;
  present.swapchainCount = 1;
  present.pSwapchains = &_swapchain;
  present.pImageIndices = &_image_index;
  present.pResults = results;

  err = vkQueuePresentKHR(queue, &present);
  if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
    // It's out of date.  We need to recreate the swap chain.
    if (vulkandisplay_cat.is_debug()) {
      vulkandisplay_cat.debug()
        << "Swap chain "
        << ((err == VK_SUBOPTIMAL_KHR) ? "suboptimal" : "out of date")
        << " for VulkanGraphicsWindow " << this << "\n";
    }
    _swapchain_size.set(-1, -1);
    _flip_ready = false;
    return;
  }
  else if (err != VK_SUCCESS) {
    vulkan_error(err, "Error presenting queue");

    if (err == VK_ERROR_DEVICE_LOST) {
      vkgsg->mark_new();
      _flip_ready = false;
    }

    if (err == VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT || err == VK_ERROR_SURFACE_LOST_KHR) {
      _flip_ready = false;
    }
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

  nassertv(_image_available == VK_NULL_HANDLE);
  _image_available = vkgsg->create_semaphore();
  nassertv(_image_available != VK_NULL_HANDLE);

  // Get a new image for rendering into.  This may wait until a new image is
  // available.
  VkResult
  err = vkAcquireNextImageKHR(vkgsg->_device, _swapchain, UINT64_MAX,
                              _image_available, VK_NULL_HANDLE, &_image_index);
  if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
    // It's out of date.  We need to recreate the swap chain.
    if (vulkandisplay_cat.is_debug()) {
      vulkandisplay_cat.debug()
        << "Swap chain "
        << ((err == VK_SUBOPTIMAL_KHR) ? "suboptimal" : "out of date")
        << " for VulkanGraphicsWindow " << this << "\n";
    }
    _swapchain_size.set(-1, -1);
    _flip_ready = false;
    return;
  }
  if (err == VK_ERROR_DEVICE_LOST) {
    vkgsg->mark_new();
    _flip_ready = false;
    return;
  }

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

    nassertv(vkgsg->_device != VK_NULL_HANDLE);

    // Submit any pending work first, while the swapchain is still alive.  This
    // matters when the window is abandoned right after opening (e.g. because the
    // requested FrameBufferProperties couldn't be satisfied): the certification
    // frame leaves an unsubmitted command buffer that waits on the swapchain's
    // image-acquired semaphore, and submitting that after the swapchain has been
    // destroyed is invalid.
    vkgsg->flush();

    // Wait until the queue is done with any commands that might use the swap
    // chain, then destroy it.
    vkQueueWaitIdle(vkgsg->_queue);
    destroy_swapchain();

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

#elif defined(HAVE_COCOA)
  CGFloat contents_scale = 1;
  if (dpi_aware) {
    contents_scale = get_backing_scale_factor();
  }

  CAMetalLayer *layer;
  layer = ((CAMetalLayer *(*)(objc_class *cls, SEL sel))objc_msgSend)(objc_getClass("CAMetalLayer"), sel_registerName("layer"));
  ((void (*)(id self, SEL sel, id delegate))objc_msgSend)((id)layer, sel_registerName("setDelegate:"), _view);
  ((void (*)(id self, SEL sel, CGFloat))objc_msgSend)((id)layer, sel_registerName("setContentsScale:"), contents_scale);
  ((void (*)(id self, SEL sel, BOOL))objc_msgSend)((id)layer, sel_registerName("setDisplaySyncEnabled:"), sync_video);
  ((void (*)(id self, SEL sel, BOOL arg))objc_msgSend)(_view, sel_registerName("setWantsLayer:"), TRUE);
  ((void (*)(id self, SEL sel, id arg))objc_msgSend)(_view, sel_registerName("setLayer:"), (id)layer);

  VkMetalSurfaceCreateInfoEXT surface_info;
  surface_info.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
  surface_info.pNext = nullptr;
  surface_info.flags = 0;
  surface_info.pLayer = layer;

  err = vkCreateMetalSurfaceEXT(vkpipe->_instance, &surface_info, nullptr, &_surface);

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

  vkgsg->reset_if_new();
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
  VkSurfaceFormatKHR *formats;
  err = vkGetPhysicalDeviceSurfaceFormatsKHR(vkpipe->_gpu, _surface,
                                             &num_formats, nullptr);
  if (!err) {
    formats = (VkSurfaceFormatKHR *)alloca(sizeof(VkSurfaceFormatKHR) * num_formats);
    err = vkGetPhysicalDeviceSurfaceFormatsKHR(vkpipe->_gpu, _surface,
                                               &num_formats, formats);
  }
  if (err) {
    vulkan_error(err, "Failed to get physical device surface formats");
    return false;
  }

  // If the format list includes just one entry of VK_FORMAT_UNDEFINED, the
  // surface has no preferred format.  Otherwise, at least one supported
  // format will be returned.
  if (num_formats == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
    _surface_format.format = VK_FORMAT_UNDEFINED;
    _surface_format.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
  }
  else {
    // Find a format that meets the requirements.
    nassertr(num_formats > 0, false);
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
  _framebuffer._config_id = vkgsg->choose_fb_config(_framebuffer._config, _fb_properties,
                                                    _surface_format.format);

  if (_fb_properties.get_stencil_bits() > 0) {
    _depth_stencil_aspect_mask = VK_IMAGE_ASPECT_DEPTH_BIT |
                                 VK_IMAGE_ASPECT_STENCIL_BIT;
  }
  else if (_fb_properties.get_depth_bits() > 0) {
    _depth_stencil_aspect_mask = VK_IMAGE_ASPECT_DEPTH_BIT;
  }
  else {
    _depth_stencil_aspect_mask = 0;
  }

  // Don't create the swapchain yet if we haven't yet gotten the configure
  // notify event, since we don't know the final size yet.
#ifdef HAVE_X11
  _swapchain_size.set(-1, -1);
  return _awaiting_configure_since != -1 || create_swapchain();
#else
  return create_swapchain();
#endif
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
  //if (vkgsg->_cmd != VK_NULL_HANDLE) {
  //  vkResetCommandBuffer(vkgsg->_cmd, 0);
  //}

  // Drop the borrowed attachment pointers before we free the images.
  _framebuffer.clear_attachments();

  // Destroy the resources held for each link in the swap chain.
  for (SwapBuffer &buffer : _swap_buffers) {
    buffer._tc->_image = VK_NULL_HANDLE;
    buffer._tc->destroy_now(device);
    delete buffer._tc;
    vkDestroySemaphore(device, buffer._render_complete, nullptr);
  }
  _swap_buffers.clear();

  if (_ms_color_tc != nullptr) {
    _ms_color_tc->destroy_now(device);
    delete _ms_color_tc;
    _ms_color_tc = nullptr;
  }

  if (_depth_stencil_tc != nullptr) {
    _depth_stencil_tc->destroy_now(device);
    delete _depth_stencil_tc;
    _depth_stencil_tc = nullptr;
  }

  // Destroy the previous swapchain.  This also destroys the swapchain images.
  if (_swapchain != VK_NULL_HANDLE) {
    vkDestroySwapchainKHR(device, _swapchain, nullptr);
    _swapchain = VK_NULL_HANDLE;
  }

  _swapchain_size.set(-1, -1);
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
      << "Creating swap chain for VulkanGraphicsWindow " << this << "\n";
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
    std::clamp((uint32_t)_size[0], surf_caps.minImageExtent.width, surf_caps.maxImageExtent.width),
    std::clamp((uint32_t)_size[1], surf_caps.minImageExtent.height, surf_caps.maxImageExtent.height));
  _framebuffer._size = _swapchain_size;

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
    if (err == VK_ERROR_DEVICE_LOST) {
      vkgsg->mark_new();
    }
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

  VkExtent3D extent = {swapchain_info.imageExtent.width,
                       swapchain_info.imageExtent.height,
                       1};

  PreparedGraphicsObjects *pgo = vkgsg->get_prepared_objects();
  // Now create an image view for each image.
  for (uint32_t i = 0; i < num_images; ++i) {
    SwapBuffer &buffer = _swap_buffers[i];

#ifdef NDEBUG
    const std::string debug_name;
#else
    std::string debug_name = get_name() + ":color#" + format_string(i);
    vkgsg->set_object_name(images[i], debug_name);
#endif

    buffer._tc = new VulkanTextureContext(pgo);
    buffer._tc->_image = images[i];
    buffer._tc->_format = swapchain_info.imageFormat;
    buffer._tc->_aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT;
    buffer._tc->_extent = extent;
    buffer._tc->_mip_levels = 1;
    buffer._tc->_array_layers = 1;
    buffer._tc->_swapchain_index = (int)i;

    if (!vkgsg->create_image_view(buffer._tc, debug_name)) {
      return false;
    }

    // Create a semaphore that is signalled when we are finished rendering,
    // to indicate that it is safe to present the image.
    buffer._render_complete = vkgsg->create_semaphore();
    nassertr(buffer._render_complete != VK_NULL_HANDLE, false);
  }

  // Now create a depth image.
  _depth_stencil_tc = nullptr;
  if (_framebuffer._config._depth_format != VK_FORMAT_UNDEFINED) {
    _depth_stencil_tc = vkgsg->create_attachment(
      _framebuffer._config._depth_format, extent, _depth_stencil_aspect_mask,
      _framebuffer._config._sample_count,
      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, get_name() + ":depth-stencil");
    if (_depth_stencil_tc == nullptr) {
      return false;
    }
  }

  // Create a multisample color image.
  _ms_color_tc = nullptr;
  if (_framebuffer._config._sample_count != VK_SAMPLE_COUNT_1_BIT) {
    _ms_color_tc = vkgsg->create_attachment(
      swapchain_info.imageFormat, extent, VK_IMAGE_ASPECT_COLOR_BIT,
      _framebuffer._config._sample_count,
      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, get_name() + ":ms-color");
    if (_ms_color_tc == nullptr) {
      return false;
    }
  }

  // Set up the framebuffer attachment list.  The multisample-color (or, with no
  // multisampling, the swapchain image) and depth attachments are stable across
  // the lifetime of the swapchain; only the color attachment is repointed at the
  // freshly acquired swapchain image each frame, in begin_frame().  Note that
  // the swap chain depth buffer is never read back, so it doesn't need storing.
  _framebuffer.clear_attachments();
  _framebuffer.add_attachment(_ms_color_tc != nullptr ? _ms_color_tc
                                                      : _swap_buffers[0]._tc,
                              VK_ATTACHMENT_STORE_OP_STORE);
  if (_depth_stencil_tc != nullptr) {
    _framebuffer.add_attachment(_depth_stencil_tc, VK_ATTACHMENT_STORE_OP_DONT_CARE);
  }

  // Create a semaphore for signalling the availability of an image.
  _image_available = vkgsg->create_semaphore();
  nassertr(_image_available != VK_NULL_HANDLE, false);

  // We need to acquire an image before we continue rendering.
  _image_index = 0;
  _flip_ready = false;
  err = vkAcquireNextImageKHR(vkgsg->_device, _swapchain, UINT64_MAX,
                              _image_available, VK_NULL_HANDLE, &_image_index);
  if (err == VK_SUBOPTIMAL_KHR) {
    vulkandisplay_cat.error()
      << "Swap chain is suboptimal for VulkanGraphicsWindow " << this << "\n";
  }
  else if (err) {
    vulkan_error(err, "Failed to acquire swapchain image");
    if (err == VK_ERROR_DEVICE_LOST) {
      vkgsg->mark_new();
    }
    return false;
  }

  return true;
}
