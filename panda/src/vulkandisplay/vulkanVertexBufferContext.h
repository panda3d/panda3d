/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vulkanVertexBufferContext.h
 * @author rdb
 * @date 2016-02-18
 */

#ifndef VULKANVERTEXBUFFERCONTEXT_H
#define VULKANVERTEXBUFFERCONTEXT_H

#include "config_vulkandisplay.h"
#include "vertexBufferContext.h"
#include "deletedChain.h"

/**
 * Manages a buffer used for holding vertex data and its allocated GPU memory.
 */
class EXPCL_VULKANDISPLAY VulkanVertexBufferContext : public VertexBufferContext {
public:
  INLINE VulkanVertexBufferContext(PreparedGraphicsObjects *pgo,
                                   GeomVertexArrayData *data);
  ALLOC_DELETED_CHAIN(VulkanVertexBufferContext);

  virtual void evict_lru();

public:
  VkBuffer _buffer;
  VkDeviceMemory _memory;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    VertexBufferContext::init_type();
    register_type(_type_handle, "VulkanVertexBufferContext",
                  VertexBufferContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "vulkanVertexBufferContext.I"

#endif
