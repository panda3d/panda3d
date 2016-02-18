/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vulkanGraphicsStateGuardian.h
 * @author rdb
 * @date 2016-02-16
 */

#ifndef VULKANGRAPHICSSTATEGUARDIAN_H
#define VULKANGRAPHICSSTATEGUARDIAN_H

#include "config_vulkandisplay.h"

#include <vulkan/vulkan.h>

/**
 * Manages a Vulkan device, and manages sending render commands to this
 * device.
 */
class VulkanGraphicsStateGuardian : public GraphicsStateGuardian {
public:
  VulkanGraphicsStateGuardian(GraphicsEngine *engine, VulkanGraphicsPipe *pipe,
                              VulkanGraphicsStateGuardian *share_with);
  virtual ~VulkanGraphicsStateGuardian();

  virtual PT(GeomMunger) make_geom_munger(const RenderState *state,
                                          Thread *current_thread);

  virtual void clear(DrawableRegion *clearable);
  virtual void prepare_display_region(DisplayRegionPipelineReader *dr);

  virtual bool begin_frame(Thread *current_thread);
  virtual bool begin_scene();
  virtual void end_scene();
  virtual void end_frame(Thread *current_thread);

  virtual void reset();

private:
  VkDevice _device;
  VkQueue _queue;
  VkCommandPool _cmd_pool;
  VkCommandBuffer _cmd;
  pvector<VkRect2D> _viewports;

  friend class VulkanGraphicsWindow;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsStateGuardian::init_type();
    register_type(_type_handle, "VulkanGraphicsStateGuardian",
                  GraphicsStateGuardian::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class wglGraphicsBuffer;
};

#include "vulkanGraphicsStateGuardian.I"

#endif
