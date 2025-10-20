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
#include <objc/objc.h>
#include <objc/message.h>
#endif

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
  BaseGraphicsWindow(engine, pipe, name, fb_prop, win_prop, flags, gsg, host)
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
    /*if (_render_pass != VK_NULL_HANDLE) {
      vkDestroyRenderPass(vkgsg->_device, _render_pass, nullptr);
      _render_pass = VK_NULL_HANDLE;
    }*/
    vkgsg->reset_if_new();
  }

  if (!vkgsg->is_valid()) {
    return false;
  }

  /*if (_current_clear_mask != _clear_mask || _render_pass == VK_NULL_HANDLE) {
    // The clear flags have changed.  Recreate the render pass.  Note that the
    // clear flags don't factor into render pass compatibility, so we don't
    // need to recreate the framebuffer.
    if (!setup_render_pass()) {
      return false;
    }
  }*/

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

  VulkanFrameData &frame_data = vkgsg->get_frame_data();

  VulkanTextureContext *color_tc;
  nassertr(!buffer._tc->is_used_this_frame(frame_data), false);
  buffer._tc->mark_used_this_frame(frame_data);

  // If we have multisamples, we render to a different image, which we then
  // resolve into the swap chain image.
  if (_ms_color_tc != nullptr) {
    nassertr(!_ms_color_tc->is_used_this_frame(frame_data), false);
    _ms_color_tc->mark_used_this_frame(frame_data);
    color_tc = _ms_color_tc;
  } else {
    color_tc = buffer._tc;
  }

  /*if (mode == FM_render) {
    clear_cube_map_selection();
  }*/

  // Now that we have a command buffer, start our render pass.  First
  // transition the swapchain images into the valid state for rendering into.
  VkCommandBuffer cmd = frame_data._cmd;

  VkRenderingInfo render_info = {VK_STRUCTURE_TYPE_RENDERING_INFO};
  render_info.layerCount = 1;
  render_info.renderArea.extent.width = _swapchain_size[0];
  render_info.renderArea.extent.height = _swapchain_size[1];
  render_info.colorAttachmentCount = 1;

  VkRenderingAttachmentInfo color_attachment = {VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
  color_attachment.imageView = color_tc->get_image_view(0);
  color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  render_info.pColorAttachments = &color_attachment;

  if (get_clear_color_active()) {
    LColor clear_color = get_clear_color();
    color_attachment.clearValue.color.float32[0] = clear_color[0];
    color_attachment.clearValue.color.float32[1] = clear_color[1];
    color_attachment.clearValue.color.float32[2] = clear_color[2];
    color_attachment.clearValue.color.float32[3] = clear_color[3];
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  } else {
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  }
  color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

  if (color_tc->_layout != color_attachment.imageLayout ||
      (color_tc->_write_stage_mask & ~VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT) != 0 ||
      (color_tc->_read_stage_mask & ~VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT) != 0) {
    frame_data.add_initial_barrier(color_tc,
      color_attachment.imageLayout,
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
  }

  VkRenderingAttachmentInfo depth_attachment = {VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
  VkRenderingAttachmentInfo stencil_attachment = {VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
  if (_depth_stencil_tc != nullptr) {
    nassertr(!_depth_stencil_tc->is_used_this_frame(frame_data), false);
    _depth_stencil_tc->mark_used_this_frame(frame_data);

    if (_depth_stencil_aspect_mask & VK_IMAGE_ASPECT_DEPTH_BIT) {
      render_info.pDepthAttachment = &depth_attachment;

      if (get_clear_depth_active()) {
        depth_attachment.clearValue.depthStencil.depth = get_clear_depth();
        depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
      } else {
        depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      }
      depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      depth_attachment.imageView = _depth_stencil_tc->get_image_view(0);
      depth_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    if (_depth_stencil_aspect_mask & VK_IMAGE_ASPECT_STENCIL_BIT) {
      render_info.pStencilAttachment = &stencil_attachment;

      if (get_clear_stencil_active()) {
        stencil_attachment.clearValue.depthStencil.stencil = get_clear_stencil();
        stencil_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
      } else {
        stencil_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      }
      stencil_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      stencil_attachment.imageView = _depth_stencil_tc->get_image_view(0);
      stencil_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    if (_depth_stencil_tc->_layout != VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
        (_depth_stencil_tc->_write_stage_mask & ~VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT) != 0 ||
        (_depth_stencil_tc->_read_stage_mask & ~VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT) != 0) {
      frame_data.add_initial_barrier(_depth_stencil_tc,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
    }
  }

  vkgsg->_vkCmdBeginRendering(cmd, &render_info);
  vkgsg->_fb_color_tc = color_tc;
  vkgsg->_fb_depth_tc = _depth_stencil_tc;
  vkgsg->_fb_config = _fb_config_id;
  return true;

  /*VkClearValue clears[2];

  VkRenderPassBeginInfo begin_info;
  begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  begin_info.pNext = nullptr;
  begin_info.renderPass = _render_pass;
  begin_info.framebuffer = buffer._framebuffer;
  begin_info.renderArea.offset.x = 0;
  begin_info.renderArea.offset.y = 0;
  begin_info.renderArea.extent.width = _swapchain_size[0];
  begin_info.renderArea.extent.height = _swapchain_size[1];
  begin_info.clearValueCount = 0;
  begin_info.pClearValues = clears;

  if (!get_clear_color_active()) {
    // If we aren't clearing (which is a bad idea - please clear the window)
    // then we need to transition it to a consistent state..
    if (color_tc->_layout == VK_IMAGE_LAYOUT_UNDEFINED) {
      // If the attachment is set to LOAD, we need to clear it for the first
      // time if we don't want the validation layer to yell at us.
      // We clear it to an arbitrary arbitrary color.  We'll just pick the
      // color returned by get_clear_color(), even if it is meaningless.

      // Actually, doing this causes the validation layer to yell even louder
      // because the image has no VK_IMAGE_USAGE_TRANSFER_DST_BIT.
      //color_tc->clear_color_image(cmd, clears[0].color);
    }

    if (color_tc->_layout != VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL ||
        (color_tc->_write_stage_mask & ~VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT) != 0 ||
        (color_tc->_read_stage_mask & ~VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT) != 0) {
      frame_data.add_initial_barrier(color_tc,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
    }
  }
  else {
    // This transition will be made when the render pass ends.
    color_tc->_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_tc->_read_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    color_tc->_write_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    color_tc->_write_access_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    LColor clear_color = get_clear_color();
    clears[0].color.float32[0] = clear_color[0];
    clears[0].color.float32[1] = clear_color[1];
    clears[0].color.float32[2] = clear_color[2];
    clears[0].color.float32[3] = clear_color[3];
    begin_info.clearValueCount = 1;
  }

  if (_depth_stencil_tc != nullptr) {
    nassertr(!_depth_stencil_tc->is_used_this_frame(frame_data), false);
    _depth_stencil_tc->mark_used_this_frame(frame_data);

    // Transition the depth-stencil image to a consistent state.
    if (!get_clear_depth_active() || !get_clear_stencil_active()) {
      if (_depth_stencil_tc->_layout != VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
          (_depth_stencil_tc->_write_stage_mask & ~VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT) != 0 ||
          (_depth_stencil_tc->_read_stage_mask & ~VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT) != 0) {
        frame_data.add_initial_barrier(_depth_stencil_tc,
          VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
          VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
          VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
      }
    }
    else {
      // This transition will be made when the first subpass is started.
      _depth_stencil_tc->_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
      _depth_stencil_tc->_write_access_mask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
      _depth_stencil_tc->_write_stage_mask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
      _depth_stencil_tc->_read_stage_mask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }

    if (get_clear_depth_active() || get_clear_stencil_active()) {
      clears[1].depthStencil.depth = get_clear_depth();
      clears[1].depthStencil.stencil = get_clear_stencil();
      begin_info.clearValueCount = 2;
    }
  }

  vkCmdBeginRenderPass(cmd, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
  vkgsg->_render_pass = _render_pass;
  vkgsg->_fb_color_tc = color_tc;
  vkgsg->_fb_depth_tc = _depth_stencil_tc;
  vkgsg->_fb_config = _fb_config_id;
  return true;*/
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

  VkCommandBuffer cmd = vkgsg->_frame_data->_cmd;
  nassertv(cmd != VK_NULL_HANDLE);
  SwapBuffer &buffer = _swap_buffers[_image_index];

  VkSemaphore signal_done = VK_NULL_HANDLE;
  if (mode != FM_refresh) {
    vkgsg->_vkCmdEndRendering(cmd);
/*    vkCmdEndRenderPass(cmd);
    vkgsg->_render_pass = VK_NULL_HANDLE;

    // The driver implicitly transitioned this to the final layout.
    buffer._tc->_layout = _final_layout;*/
    buffer._tc->mark_written(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                             VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);

    if (_depth_stencil_tc != nullptr) {
      //_depth_stencil_tc->_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
      _depth_stencil_tc->mark_written(
        VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
    }

    // Now we can do copy-to-texture, now that the render pass has ended.
    copy_to_textures();

    signal_done = buffer._render_complete;

    // If we copied the textures, transition it back to the present state.
    if (buffer._tc->_layout != VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
      buffer._tc->transition(cmd, vkgsg->_graphics_queue_family_index,
                             VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                             VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                             VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
    }
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

    // Wait until the queue is done with any commands that might use the swap
    // chain, then destroy it.
    nassertv(vkgsg->_device != VK_NULL_HANDLE);
    vkQueueWaitIdle(vkgsg->_queue);
    destroy_swapchain();

    /*if (_render_pass != VK_NULL_HANDLE) {
      vkDestroyRenderPass(vkgsg->_device, _render_pass, nullptr);
      _render_pass = VK_NULL_HANDLE;
    }*/

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
  CAMetalLayer *layer;
  layer = ((CAMetalLayer *(*)(objc_class *cls, SEL sel))objc_msgSend)(objc_getClass("CAMetalLayer"), sel_registerName("layer"));
  ((void (*)(id self, SEL sel, id delegate))objc_msgSend)((id)layer, sel_registerName("setDelegate:"), _view);
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
  _fb_config_id = vkgsg->choose_fb_config(_fb_config, _fb_properties,
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
  return setup_render_pass() && (_awaiting_configure_since != -1 || create_swapchain());
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
  return true;

  VulkanGraphicsStateGuardian *vkgsg;
  DCAST_INTO_R(vkgsg, _gsg, false);

  if (vulkandisplay_cat.is_debug()) {
    vulkandisplay_cat.debug()
      << "Creating render pass for VulkanGraphicsWindow " << this << "\n";
  }

  nassertr(vkgsg->_frame_data == nullptr, false);

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

  VkAttachmentReference color_reference, depth_reference, resolve_reference;

  VkSubpassDescription subpass;
  subpass.flags = 0;
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.inputAttachmentCount = 0;
  subpass.pInputAttachments = nullptr;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &color_reference;
  subpass.pResolveAttachments = nullptr;
  subpass.pDepthStencilAttachment = nullptr;
  subpass.preserveAttachmentCount = 0;
  subpass.pPreserveAttachments = nullptr;

  // Now we want to create a render pass, and for that we need to describe the
  // framebuffer attachments as well as any subpasses we'd like to use.
  VkAttachmentDescription attachments[3];
  attachments[0].flags = 0;
  attachments[0].format = _surface_format.format;
  attachments[0].samples = _fb_config._sample_count;
  attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  attachments[0].finalLayout = _final_layout;

  if (_fb_config._sample_count > VK_SAMPLE_COUNT_1_BIT) {
    // If multisampling, we don't present this, but resolve it.
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  }

  if (get_clear_color_active()) {
    // We don't care about the current contents.
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  } else {
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
  }

  color_reference.attachment = 0;
  color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDependency dependency;
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
  dependency.dstStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT
                           | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  dependency.dependencyFlags = 0;

  size_t i = 1;
  if (_fb_config._depth_format) {
    attachments[i].flags = 0;
    attachments[i].format = _fb_config._depth_format;
    attachments[i].samples = _fb_config._sample_count;
    attachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[i].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[i].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    attachments[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    if (get_clear_depth_active()) {
      attachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    }

    if (get_clear_stencil_active()) {
      attachments[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    }

    if (get_clear_depth_active() && get_clear_stencil_active()) {
      // We don't care about the current contents.
      attachments[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    }

    depth_reference.attachment = i;
    depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    subpass.pDepthStencilAttachment = &depth_reference;
    ++i;

    dependency.srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependency.dstStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependency.dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
  }

  // Also create an attachment reference for the resolve target.
  if (_fb_config._sample_count > VK_SAMPLE_COUNT_1_BIT) {
    attachments[i].flags = 0;
    attachments[i].format = _surface_format.format;
    attachments[i].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[i].finalLayout = _final_layout;

    resolve_reference.attachment = i;
    resolve_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    subpass.pResolveAttachments = &resolve_reference;
    ++i;
  }

  VkRenderPassCreateInfo pass_info;
  pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  pass_info.pNext = nullptr;
  pass_info.flags = 0;
  pass_info.attachmentCount = i;
  pass_info.pAttachments = attachments;
  pass_info.subpassCount = 1;
  pass_info.pSubpasses = &subpass;
  pass_info.dependencyCount = 1;
  pass_info.pDependencies = &dependency;

  VkRenderPass pass;
  VkResult
  err = vkCreateRenderPass(vkgsg->_device, &pass_info, nullptr, &pass);
  if (err) {
    vulkan_error(err, "Failed to create render pass");
    return false;
  }

  // Destroy the previous render pass object.
  if (_render_pass != VK_NULL_HANDLE) {
    if (vkgsg->_last_frame_data != nullptr) {
      vkgsg->_last_frame_data->_pending_destroy_render_passes.push_back(_render_pass);
    } else {
      vkDestroyRenderPass(vkgsg->_device, _render_pass, nullptr);
    }
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
  //if (vkgsg->_cmd != VK_NULL_HANDLE) {
  //  vkResetCommandBuffer(vkgsg->_cmd, 0);
  //}

  // Destroy the resources held for each link in the swap chain.
  for (SwapBuffer &buffer : _swap_buffers) {
    // Destroy the framebuffers that use the swapchain images.
    //vkDestroyFramebuffer(device, buffer._framebuffer, nullptr);
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
    buffer._tc = new VulkanTextureContext(pgo);
    buffer._tc->_image = images[i];
    buffer._tc->_format = swapchain_info.imageFormat;
    buffer._tc->_aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT;
    buffer._tc->_extent = extent;
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

    VkImageView image_view;
    err = vkCreateImageView(device, &view_info, nullptr, &image_view);
    if (err) {
      vulkan_error(err, "Failed to create image view for swapchain");
      return false;
    }

    buffer._tc->_image_views.push_back(image_view);

    // Create a semaphore that is signalled when we are finished rendering,
    // to indicate that it is safe to present the image.
    buffer._render_complete = vkgsg->create_semaphore();
    nassertr(buffer._render_complete != VK_NULL_HANDLE, false);
  }

  // Now create a depth image.
  _depth_stencil_tc = nullptr;
  VkImageView depth_stencil_view = VK_NULL_HANDLE;
  if (_fb_config._depth_format != VK_FORMAT_UNDEFINED) {
    _depth_stencil_tc = new VulkanTextureContext(pgo);
    _depth_stencil_tc->_aspect_mask = _depth_stencil_aspect_mask;

    if (!vkgsg->create_image(_depth_stencil_tc, VK_IMAGE_TYPE_2D,
                             _fb_config._depth_format, extent, 1, 1, _fb_config._sample_count,
                             VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
      delete _depth_stencil_tc;
      _depth_stencil_tc = nullptr;
      return false;
    }

    VkImageViewCreateInfo view_info;
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.pNext = nullptr;
    view_info.flags = 0;
    view_info.image = _depth_stencil_tc->_image;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = _fb_config._depth_format;
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

    err = vkCreateImageView(device, &view_info, nullptr, &depth_stencil_view);
    if (err) {
      vulkan_error(err, "Failed to create image view for depth/stencil");
      return false;
    }

    _depth_stencil_tc->_image_views.push_back(depth_stencil_view);
  }

  // Create a multisample color image.
  _ms_color_tc = nullptr;
  VkImageView ms_color_view = VK_NULL_HANDLE;
  if (_fb_config._sample_count != VK_SAMPLE_COUNT_1_BIT) {
    _ms_color_tc = new VulkanTextureContext(pgo);
    if (!vkgsg->create_image(_ms_color_tc, VK_IMAGE_TYPE_2D,
                             swapchain_info.imageFormat, extent, 1, 1,
                             _fb_config._sample_count, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)) {
      delete _ms_color_tc;
      _ms_color_tc = nullptr;
      return false;
    }
    _ms_color_tc->_aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT;

    VkImageViewCreateInfo view_info;
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.pNext = nullptr;
    view_info.flags = 0;
    view_info.image = _ms_color_tc->_image;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = _ms_color_tc->_format;
    view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    err = vkCreateImageView(device, &view_info, nullptr, &ms_color_view);
    if (err) {
      vulkan_error(err, "Failed to create image view for multisample color");
      return false;
    }

    _ms_color_tc->_image_views.push_back(ms_color_view);
  }

  // Now finally create a framebuffer for each link in the swap chain.
  /*VkImageView attach_views[3];
  uint32_t num_views = 1;

  if (ms_color_view != VK_NULL_HANDLE) {
    attach_views[0] = ms_color_view;
    ++num_views;
  }
  if (depth_stencil_view != nullptr) {
    attach_views[1] = depth_stencil_view;
    ++num_views;
  }

  VkFramebufferCreateInfo fb_info;
  fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  fb_info.pNext = nullptr;
  fb_info.flags = 0;
  fb_info.renderPass = _render_pass;
  fb_info.attachmentCount = num_views;
  fb_info.pAttachments = attach_views;
  fb_info.width = swapchain_info.imageExtent.width;
  fb_info.height = swapchain_info.imageExtent.height;
  fb_info.layers = 1;*/

  for (uint32_t i = 0; i < num_images; ++i) {
    SwapBuffer &buffer = _swap_buffers[i];
    /*if (_ms_color_tc != nullptr) {
      attach_views[num_views - 1] = buffer._tc->get_image_view(0);
    } else {
      attach_views[0] = buffer._tc->get_image_view(0);
    }
    err = vkCreateFramebuffer(device, &fb_info, nullptr, &buffer._framebuffer);
    if (err) {
      vulkan_error(err, "Failed to create framebuffer");
      return false;
    }*/

    // Don't start rendering until the image has been acquired.
    buffer._tc->mark_written(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0);
  }

  // Create a semaphore for signalling the availability of an image.
  _image_available = vkgsg->create_semaphore();
  nassertr(_image_available != VK_NULL_HANDLE, false);

  // We need to acquire an image before we continue rendering.
  _image_index = 0;
  _flip_ready = false;
  err = vkAcquireNextImageKHR(vkgsg->_device, _swapchain, UINT64_MAX,
                              _image_available, VK_NULL_HANDLE, &_image_index);
  if (err) {
    vulkan_error(err, "Failed to acquire swapchain image");
    if (err == VK_ERROR_DEVICE_LOST) {
      vkgsg->mark_new();
    }
    return false;
  }

  return true;
}
