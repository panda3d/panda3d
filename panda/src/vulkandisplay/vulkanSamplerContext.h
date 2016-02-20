/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vulkanSamplerContext.h
 * @author rdb
 * @date 2016-02-19
 */

#ifndef VULKANSAMPLERCONTEXT_H
#define VULKANSAMPLERCONTEXT_H

#include "config_vulkandisplay.h"
#include "samplerContext.h"

/**
 * Manages a Vulkan sampler object.
 */
class EXPCL_VULKANDISPLAY VulkanSamplerContext : public SamplerContext {
public:
  INLINE VulkanSamplerContext(const SamplerState &sampler_state, VkSampler sampler);
  ~VulkanSamplerContext() {};

  ALLOC_DELETED_CHAIN(VulkanSamplerContext);

public:
  VkSampler _sampler;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    SamplerContext::init_type();
    register_type(_type_handle, "VulkanSamplerContext",
                  SamplerContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "vulkanSamplerContext.I"

#endif  // VULKANSAMPLERCONTEXT_H
