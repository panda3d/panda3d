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
  _cmd(from._cmd), _seq(from._seq), _wait_semaphore(from._wait_semaphore) {
  from._cmd = VK_NULL_HANDLE;
  from._seq = 0;
  from._wait_semaphore = VK_NULL_HANDLE;
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
  from._cmd = VK_NULL_HANDLE;
  from._seq = 0;
  from._wait_semaphore = VK_NULL_HANDLE;
  return *this;
}
