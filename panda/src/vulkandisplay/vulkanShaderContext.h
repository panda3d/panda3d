/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vulkanShaderContext.h
 * @author rdb
 * @date 2016-02-18
 */

#ifndef VULKANSHADERCONTEXT_H
#define VULKANSHADERCONTEXT_H

#include "config_vulkandisplay.h"

/**
 * Manages a set of Vulkan shader modules.
 */
class EXPCL_VULKANDISPLAY VulkanShaderContext : public ShaderContext {
public:
  INLINE VulkanShaderContext(Shader *shader);
  ~VulkanShaderContext() {};

  ALLOC_DELETED_CHAIN(VulkanShaderContext);

public:
  VkShaderModule _modules[2];

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ShaderContext::init_type();
    register_type(_type_handle, "VulkanShaderContext",
                  ShaderContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "vulkanShaderContext.I"

#endif  // VULKANSHADERCONTEXT_H
