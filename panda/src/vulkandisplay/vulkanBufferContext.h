/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vulkanBufferContext.h
 * @author rdb
 * @date 2024-10-16
 */

#ifndef VULKANBUFFERCONTEXT_H
#define VULKANBUFFERCONTEXT_H

#include "config_vulkandisplay.h"
#include "bufferContext.h"
#include "deletedChain.h"

/**
 * Generic buffer (used for shader buffers).
 */
class EXPCL_VULKANDISPLAY VulkanBufferContext : public BufferContext {
public:
  INLINE VulkanBufferContext(PreparedGraphicsObjects *pgo,
                             TypedWritableReferenceCount *object);
  ALLOC_DELETED_CHAIN(VulkanBufferContext);

  virtual void evict_lru();

  INLINE void mark_read(uint64_t seq);

public:
  VkBuffer _buffer;
  VulkanMemoryBlock _block;
  bool _host_visible = false;

  // Used for shader buffers.
  VkAccessFlags2 _write_access_mask = 0;
  VkPipelineStageFlags2 _write_stage_mask = 0;
  VkPipelineStageFlags2 _read_stage_mask = 0;

  // Sequence number of the last command buffer in which this was used.
  uint64_t _read_seq = 0;
  uint64_t _write_seq = 0;

  // Index of the barrier into the list of barriers of the _read_seq CB.
  bool _hoisted_barrier_exists = false;
  size_t _buffer_barrier_index = 0;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    BufferContext::init_type();
    register_type(_type_handle, "VulkanBufferContext",
                  BufferContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "vulkanBufferContext.I"

#endif
