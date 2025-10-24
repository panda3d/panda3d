/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vulkanCommandBuffer.I
 * @author rdb
 * @date 2025-10-23
 */

/**
 *
 */
INLINE VulkanCommandBuffer::
VulkanCommandBuffer(VulkanCommandBuffer &&from) noexcept :
  _cmd(from._cmd),
  _seq(from._seq),
  _wait_semaphore(from._wait_semaphore),
  _image_barriers(std::move(from._image_barriers)),
  _buffer_barriers(std::move(from._buffer_barriers)),
  _pending_image_barriers(std::move(from._pending_image_barriers)),
  _pending_buffer_barriers(std::move(from._pending_buffer_barriers)) {
  from._cmd = VK_NULL_HANDLE;
  from._seq = 0;
  from._wait_semaphore = VK_NULL_HANDLE;
  from._image_barriers.clear();
  from._buffer_barriers.clear();
  from._pending_image_barriers.clear();
  from._pending_buffer_barriers.clear();
}

/**
 *
 */
INLINE VulkanCommandBuffer::
VulkanCommandBuffer(VkCommandBuffer cmd, uint64_t seq, VkSemaphore wait_for) :
  _cmd(cmd), _seq(seq), _wait_semaphore(wait_for) {
}

/**
 *
 */
INLINE VulkanCommandBuffer &VulkanCommandBuffer::
operator = (VulkanCommandBuffer &&from) noexcept {
  nassertr(_cmd == VK_NULL_HANDLE, *this);
  _cmd = from._cmd;
  _seq = from._seq;
  _wait_semaphore = from._wait_semaphore;
  _image_barriers = std::move(from._image_barriers);
  _buffer_barriers = std::move(from._buffer_barriers);
  _pending_image_barriers = std::move(from._pending_image_barriers);
  _pending_buffer_barriers = std::move(from._pending_buffer_barriers);
  from._cmd = VK_NULL_HANDLE;
  from._seq = 0;
  from._wait_semaphore = VK_NULL_HANDLE;
  from._image_barriers.clear();
  from._buffer_barriers.clear();
  from._pending_image_barriers.clear();
  from._pending_buffer_barriers.clear();
  return *this;
}

/**
 * Queues a low-level barrier.  Only use for images not managed by a
 * VulkanTextureContext, otherwise see the other overload.
 */
INLINE void VulkanCommandBuffer::
add_barrier(VkImageMemoryBarrier2 barrier) {
  _pending_image_barriers.push_back(std::move(barrier));
}

/**
 * Queues a low-level barrier.  Only use for images not managed by a
 * VulkanBufferContext, otherwise see the other overload.
 */
INLINE void VulkanCommandBuffer::
add_barrier(VkBufferMemoryBarrier2 barrier) {
  _pending_buffer_barriers.push_back(std::move(barrier));
}

/**
 * Call this sometime between calling add_barrier() and issuing the command
 * that actually performs the access in order to issue the command for the
 * barriers.  This allows coalescing multiple barriers into one for efficiency.
 */
INLINE void VulkanCommandBuffer::
flush_barriers() {
  if (!_pending_buffer_barriers.empty() || !_pending_image_barriers.empty()) {
    do_flush_barriers();
  }
}
