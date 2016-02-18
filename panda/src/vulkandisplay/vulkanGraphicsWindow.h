/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vulkanGraphicsWindow.h
 * @author rdb
 * @date 2016-02-16
 */

#ifndef VULKANGRAPHICSWINDOW_H
#define VULKANGRAPHICSWINDOW_H

#include "config_vulkandisplay.h"

#ifdef _WIN32
#include "winGraphicsWindow.h"
typedef WinGraphicsWindow BaseGraphicsWindow;
#else
#include "graphicsWindow.h"
typedef GraphicsWindow BaseGraphicsWindow;
#endif

/**
 * A single graphics window for rendering to using the Vulkan API.  It manages
 * a VkSurface for the OS-specific base window implementation, as well as a
 * swapchain.
 */
class EXPCL_VULKANDISPLAY VulkanGraphicsWindow : public BaseGraphicsWindow {
public:
  VulkanGraphicsWindow(GraphicsEngine *engine, GraphicsPipe *pipe,
                       const string &name,
                       const FrameBufferProperties &fb_prop,
                       const WindowProperties &win_prop,
                       int flags,
                       GraphicsStateGuardian *gsg,
                       GraphicsOutput *host);
  virtual ~VulkanGraphicsWindow();

  virtual bool begin_frame(FrameMode mode, Thread *current_thread);
  virtual void end_frame(FrameMode mode, Thread *current_thread);

  virtual void begin_flip();
  virtual void ready_flip();
  virtual void end_flip();

protected:
  virtual void close_window();
  virtual bool open_window();

private:
  VkSurfaceKHR _surface;
  VkSwapchainKHR _swapchain;
  VkRenderPass _render_pass;
  VkSemaphore _present_complete;

  pvector<VkImageView> _image_views;
  VkImageView _depth_stencil_view;
  pvector<VkFramebuffer> _framebuffers;
  uint32_t _image_index;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    BaseGraphicsWindow::init_type();
    register_type(_type_handle, "VulkanGraphicsWindow",
                  BaseGraphicsWindow::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "vulkanGraphicsWindow.I"

#endif
