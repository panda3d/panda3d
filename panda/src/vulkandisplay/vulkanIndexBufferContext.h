/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vulkanIndexBufferContext.h
 * @author rdb
 * @date 2016-02-22
 */

#ifndef VULKANINDEXBUFFERCONTEXT_H
#define VULKANINDEXBUFFERCONTEXT_H

#include "config_vulkandisplay.h"
#include "indexBufferContext.h"
#include "deletedChain.h"

/**
 * Manages a buffer used for holding index data and its allocated GPU memory.
 */
class EXPCL_VULKANDISPLAY VulkanIndexBufferContext : public IndexBufferContext {
public:
  INLINE VulkanIndexBufferContext(PreparedGraphicsObjects *pgo,
                                  GeomPrimitive *data);
  ALLOC_DELETED_CHAIN(VulkanIndexBufferContext);

  virtual void evict_lru();

public:
  VkBuffer _buffer;
  VulkanMemoryBlock _block;
  VkIndexType _index_type;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    IndexBufferContext::init_type();
    register_type(_type_handle, "VulkanIndexBufferContext",
                  IndexBufferContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "vulkanIndexBufferContext.I"

#endif
