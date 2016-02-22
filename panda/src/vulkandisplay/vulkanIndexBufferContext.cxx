/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vulkanIndexBufferContext.cxx
 * @author rdb
 * @date 2016-02-22
 */

#include "vulkanIndexBufferContext.h"

TypeHandle VulkanIndexBufferContext::_type_handle;

/**
 * Evicts the page from the LRU.  Called internally when the LRU determines
 * that it is full.  May also be called externally when necessary to
 * explicitly evict the page.
 *
 * It is legal for this method to either evict the page as requested, do
 * nothing (in which case the eviction will be requested again at the next
 * epoch), or requeue itself on the tail of the queue (in which case the
 * eviction will be requested again much later).
 */
void VulkanIndexBufferContext::
evict_lru() {
  /*dequeue_lru();

  TODO: manage freeing when not currently bound

  update_data_size_bytes(0);
  mark_unloaded();*/
}
