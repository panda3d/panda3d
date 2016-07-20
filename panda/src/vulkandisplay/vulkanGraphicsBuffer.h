/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vulkanGraphicsBuffer.h
 * @author rdb
 * @date 2016-04-13
 */

#ifndef VULKANGRAPHICSBUFFER_H
#define VULKANGRAPHICSBUFFER_H

#include "config_vulkandisplay.h"
#include "graphicsBuffer.h"

/**
 * An offscreen buffer using the Vulkan API.
 */
class EXPCL_VULKANDISPLAY VulkanGraphicsBuffer : public GraphicsBuffer {
public:
  VulkanGraphicsBuffer(GraphicsEngine *engine, GraphicsPipe *pipe,
                       const string &name,
                       const FrameBufferProperties &fb_prop,
                       const WindowProperties &win_prop,
                       int flags,
                       GraphicsStateGuardian *gsg,
                       GraphicsOutput *host);
  virtual ~VulkanGraphicsBuffer();

  //virtual void clear(Thread *current_thread);
  virtual bool begin_frame(FrameMode mode, Thread *current_thread);
  virtual void end_frame(FrameMode mode, Thread *current_thread);

protected:
  virtual void close_buffer();
  virtual bool open_buffer();

  bool setup_render_pass();

  void destroy_framebuffer();
  bool create_framebuffer();

  bool create_attachment(VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspect);

private:
  VkRenderPass _render_pass;
  VkFramebuffer _framebuffer;
  LVecBase2i _framebuffer_size;
  int _current_clear_mask;

  VkFormat _color_format;
  VkFormat _depth_stencil_format;
  VkImageAspectFlags _depth_stencil_aspect_mask;

  struct Attachment {
    VkImage _image;
    VkImageView _image_view;
    VkDeviceMemory _memory;
    VkFormat _format;
    VkImageUsageFlags _usage;
    VkImageAspectFlags _aspect;
  };
  typedef pvector<Attachment> Attachments;
  Attachments _attachments;
  bool _layout_defined;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsBuffer::init_type();
    register_type(_type_handle, "VulkanGraphicsBuffer",
                  GraphicsBuffer::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "vulkanGraphicsBuffer.I"

#endif
