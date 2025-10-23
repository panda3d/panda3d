/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vulkanBufferContext.I
 * @author rdb
 * @date 2024-10-16
 */

/**
 *
 */
INLINE VulkanBufferContext::
VulkanBufferContext(PreparedGraphicsObjects *pgo,
                    TypedWritableReferenceCount *object) :
  BufferContext(&pgo->_sbuffer_residency, object),
  _buffer(VK_NULL_HANDLE) {
}

/**
 * Indicates that the buffer has been read by the command buffer with the
 * given sequence number.
 */
INLINE void VulkanBufferContext::
mark_read(uint64_t seq) {
  if (seq != _read_seq) {
    _read_seq = seq;
    _buffer_barrier_index = 0;
    _pooled_barrier_exists = false;
  }
}
