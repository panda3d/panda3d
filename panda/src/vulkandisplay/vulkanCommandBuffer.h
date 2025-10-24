/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vulkanCommandBuffer.h
 * @author rdb
 * @date 2025-10-23
 */

#ifndef VULKANCOMMANDBUFFER_H
#define VULKANCOMMANDBUFFER_H

#include "config_vulkandisplay.h"
#include "vulkanMemoryPage.h"
#include "screenshotRequest.h"

class VulkanTextureContext;
class VulkanBufferContext;

/**
 * Keeps track of a single command buffer as well as the resources.
 *
 * Each command buffer has a unique sequence index which indicates the order in
 * which it is intended to be submitted.
 */
class VulkanCommandBuffer {
public:
  VulkanCommandBuffer() = default;
  INLINE VulkanCommandBuffer(VulkanCommandBuffer &&from) noexcept;
  INLINE VulkanCommandBuffer(VkCommandBuffer cmd, uint64_t seq,
                             VkSemaphore wait_for = VK_NULL_HANDLE);

  INLINE VulkanCommandBuffer &operator = (VulkanCommandBuffer &&from) noexcept;

  operator VkCommandBuffer() const {
    return _cmd;
  }

  void add_barrier(VulkanTextureContext *tc, VkImageLayout layout,
                   VkPipelineStageFlags2 stage_mask,
                   VkAccessFlags2 access_mask = 0);
  void add_barrier(VulkanBufferContext *bc,
                   VkPipelineStageFlags2 stage_mask,
                   VkAccessFlags2 access_mask = 0);

  INLINE void add_barrier(VkImageMemoryBarrier2 barrier);
  INLINE void add_barrier(VkBufferMemoryBarrier2 barrier);

  INLINE void flush_barriers();
  void do_flush_barriers();

public:
  VkCommandBuffer _cmd = VK_NULL_HANDLE;
  uint64_t _seq = 0;

  // Semaphore to wait on before these commands may be executed.  Note that
  // there is no _signal_done_semaphore; this is passed as an argument into
  // GSG::end_command_buffer().
  VkSemaphore _wait_semaphore = VK_NULL_HANDLE;

  // These barriers need to be issued BEFORE the command buffer (usually the
  // barrier is added to the previous command buffer).
  pvector<VkImageMemoryBarrier2> _image_barriers;
  pvector<VkBufferMemoryBarrier2> _buffer_barriers;

  // These barriers are issued during or after the command stream.
  pvector<VkImageMemoryBarrier2> _pending_image_barriers;
  pvector<VkBufferMemoryBarrier2> _pending_buffer_barriers;
};

#include "vulkanCommandBuffer.I"

#endif
