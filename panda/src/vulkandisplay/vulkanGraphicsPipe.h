/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vulkanGraphicsPipe.h
 * @author rdb
 * @date 2016-02-16
 */

#ifndef VULKANGRAPHICSPIPE_H
#define VULKANGRAPHICSPIPE_H

#include "config_vulkandisplay.h"
#include "graphicsOutput.h"

#ifdef _WIN32
#include "winGraphicsPipe.h"
typedef WinGraphicsPipe BaseGraphicsPipe;
#elif defined(HAVE_X11)
#include "x11GraphicsPipe.h"
typedef x11GraphicsPipe BaseGraphicsPipe;
#else
#include "graphicsPipe.h"
typedef GraphicsPipe BaseGraphicsPipe;
#endif

/**
 * This graphics pipe represents the interface for creating Vulkan graphics
 * windows, and manages a single Vulkan instance and device (for now).
 */
class EXPCL_VULKANDISPLAY VulkanGraphicsPipe : public BaseGraphicsPipe {
public:
  VulkanGraphicsPipe();
  virtual ~VulkanGraphicsPipe();

  bool find_memory_type(uint32_t &type_index, uint32_t type_bits,
                        VkFlags required_flags) const;
  bool find_queue_family(uint32_t &queue_family_index,
                         VkFlags required_flags) const;
  bool find_queue_family_for_surface(uint32_t &queue_family_index,
                                     VkSurfaceKHR surface,
                                     VkFlags required_flags) const;

  const char *get_vendor_name() const;

  virtual string get_interface_name() const;
  static PT(GraphicsPipe) pipe_constructor();

protected:
  virtual PT(GraphicsOutput) make_output(const string &name,
                                         const FrameBufferProperties &fb_prop,
                                         const WindowProperties &win_prop,
                                         int flags,
                                         GraphicsEngine *engine,
                                         GraphicsStateGuardian *gsg,
                                         GraphicsOutput *host,
                                         int retry,
                                         bool &precertify);
  virtual PT(GraphicsStateGuardian) make_callback_gsg(GraphicsEngine *engine);

public:
  VkInstance _instance;
  VkPhysicalDevice _gpu;
  VkPhysicalDeviceFeatures _gpu_features;
  VkPhysicalDeviceProperties _gpu_properties;
  VkPhysicalDeviceMemoryProperties _memory_properties;
  pvector<VkQueueFamilyProperties> _queue_families;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    BaseGraphicsPipe::init_type();
    register_type(_type_handle, "VulkanGraphicsPipe",
                  BaseGraphicsPipe::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "vulkanGraphicsPipe.I"

#endif
