/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vulkanCommandBuffer.cxx
 * @author rdb
 * @date 2025-10-23
 */

#include "vulkanCommandBuffer.h"
#include "vulkanTextureContext.h"
#include "shaderBuffer.h"

/**
 * Marks the given resource as being used by this command buffer, ensuring that
 * the appropriate pipeline barrier is added to the command buffer.
 *
 * Note that these barriers may be done BEFORE waiting on the semaphore.
 */
void VulkanCommandBuffer::
add_barrier(VulkanTextureContext *tc, VkImageLayout layout,
            VkPipelineStageFlags2 dst_stage_mask,
            VkAccessFlags2 dst_access_mask) {
  nassertv(_cmd != VK_NULL_HANDLE);

  // Are we writing to the texture?
  VkAccessFlags2 write_mask = (dst_access_mask &
    (VK_ACCESS_2_SHADER_WRITE_BIT |
     VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT |
     VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
     VK_ACCESS_2_TRANSFER_WRITE_BIT |
     VK_ACCESS_2_HOST_WRITE_BIT |
     VK_ACCESS_2_MEMORY_WRITE_BIT));

  nassertv(tc->_write_seq <= _seq);
  nassertv((write_mask == 0 || tc->_read_seq <= _seq));

  VkPipelineStageFlags2 src_stage_mask = tc->_write_stage_mask;
  VkAccessFlags2 src_access_mask = tc->_write_access_mask;

  bool is_write = (tc->_layout != layout || write_mask != 0);
  if (is_write) {
    // Before a layout transition or a write, all stages that previously read
    // this resource must have finished executing.
    src_stage_mask |= tc->_read_stage_mask;

    if (src_stage_mask == 0) {
      // Can't specify a source stage mask of zero.
      src_stage_mask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
    }
  }
  else if (src_stage_mask == 0) {
    // No write has been done, nothing to do here, except mark the read.
    tc->mark_read(_seq);
    tc->_read_stage_mask |= dst_stage_mask;
    return;
  }
  else {
    // We've already synchronized these reads since the last write.
    dst_stage_mask &= ~tc->_read_stage_mask;
    if (dst_stage_mask == 0) {
      // We could probably improve this by also early-outing if we've already
      // synchronized a *preceding* stage.
      tc->mark_read(_seq);
      return;
    }
  }

  VkImageMemoryBarrier2 img_barrier;
  if (tc->_image != VK_NULL_HANDLE) {
    img_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    img_barrier.pNext = nullptr;
    img_barrier.srcStageMask = src_stage_mask;
    img_barrier.srcAccessMask = src_access_mask;
    img_barrier.dstStageMask = dst_stage_mask;
    img_barrier.dstAccessMask = dst_access_mask;
    img_barrier.oldLayout = tc->_layout;
    img_barrier.newLayout = layout;
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.image = tc->_image;
    img_barrier.subresourceRange.aspectMask = tc->_aspect_mask;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.levelCount = tc->_mip_levels;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.layerCount = tc->_array_layers;
  }

  VkBufferMemoryBarrier2 buf_barrier;
  if (tc->_buffer != VK_NULL_HANDLE) {
    buf_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
    buf_barrier.pNext = nullptr;
    buf_barrier.srcStageMask = src_stage_mask;
    buf_barrier.srcAccessMask = src_access_mask;
    buf_barrier.dstStageMask = dst_stage_mask;
    buf_barrier.dstAccessMask = dst_access_mask;
    buf_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    buf_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    buf_barrier.buffer = tc->_buffer;
    buf_barrier.offset = 0;
    buf_barrier.size = VK_WHOLE_SIZE;
  }

  // We want to avoid adding lots of pipeline barriers to the command stream,
  // so we instead add this to the list of barriers to be issued at the
  // beginning of this CB, unless it has already been accessed in this CB.
  bool pool_possible =
    (tc->_write_seq < _seq && (tc->_read_seq < _seq || !is_write));

  if (vulkandisplay_cat.is_spam()) {
    const char dst_type = is_write ? 'W' : 'R';
    const char src_type = (src_access_mask != 0) ? 'W' : 'R';
    auto &out = vulkandisplay_cat.spam()
      << (pool_possible ? "Pooling " : "Issuing ")
      << dst_type << 'a' << src_type << " barrier for ";

    Texture *tex = tc->get_texture();
    if (tex != nullptr) {
      out << "texture " << *tex;
    } else if (tc->_swapchain_index >= 0) {
      out << "swapchain image " << tc->_swapchain_index;
    } else {
      out << "TC " << tc;
    }
    out << " on CB #" << _seq << " (last "
        << ((tc->_read_seq > tc->_write_seq) ? "read on #" : "write on #")
        << tc->_read_seq << ")\n";
  }

  if (pool_possible) {
    // First access in this CB, or a read in a CB without a write.
    if (tc->_read_seq == _seq && tc->_pooled_barrier_exists) {
      // Already exists, this barrier, just modify it.
      if (tc->_image != VK_NULL_HANDLE) {
        nassertv(tc->_image_barrier_index <= _image_barriers.size());
        VkImageMemoryBarrier2 &existing_barrier = _image_barriers[tc->_image_barrier_index];
        existing_barrier.srcStageMask |= img_barrier.srcStageMask;
        existing_barrier.srcAccessMask |= img_barrier.srcAccessMask;
        existing_barrier.dstStageMask |= img_barrier.dstStageMask;
        existing_barrier.dstAccessMask |= img_barrier.dstAccessMask;
      }
      if (tc->_buffer != VK_NULL_HANDLE) {
        nassertv(tc->_buffer_barrier_index <= _buffer_barriers.size());
        VkBufferMemoryBarrier2 &existing_barrier = _buffer_barriers[tc->_buffer_barrier_index];
        existing_barrier.srcStageMask |= img_barrier.srcStageMask;
        existing_barrier.srcAccessMask |= img_barrier.srcAccessMask;
        existing_barrier.dstStageMask |= img_barrier.dstStageMask;
        existing_barrier.dstAccessMask |= img_barrier.dstAccessMask;
      }
    } else {
      if (tc->_image != VK_NULL_HANDLE) {
        tc->_image_barrier_index = _image_barriers.size();
        _image_barriers.push_back(img_barrier);
      }
      if (tc->_buffer != VK_NULL_HANDLE) {
        tc->_buffer_barrier_index = _buffer_barriers.size();
        _buffer_barriers.push_back(buf_barrier);
      }
      tc->_pooled_barrier_exists = true;
    }
  }
  else {
    // We already have an access done in this CB, issue the barrier now.
    VkDependencyInfo info = {
      VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      nullptr, // pNext
      0, // dependencyFlags
      0, // memoryBarrierCount
      nullptr, // pMemoryBarriers
      tc->_buffer != VK_NULL_HANDLE,
      &buf_barrier,
      tc->_image != VK_NULL_HANDLE,
      &img_barrier,
    };
    vkCmdPipelineBarrier2(_cmd, &info);

    tc->_pooled_barrier_exists = false;
  }

  tc->_layout = layout;
  tc->_read_seq = _seq;

  if (write_mask != 0) {
    // Remember which stages wrote to it and how.
    tc->_write_stage_mask = dst_stage_mask;
    tc->_write_access_mask = write_mask;
    tc->_read_stage_mask = 0;
    tc->_write_seq = _seq;
  }
  else {
    // This is a read-after-write barrier.  It's possible that there will be
    // another read later from a different (earlier) stage, which is why we
    // don't zero out _write_stage_mask.  We can just check _read_stage_mask
    // the next time to see what we have already synchronized with the write.
    tc->_read_stage_mask |= dst_stage_mask & ~VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;

    if (dst_stage_mask & (VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT | VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT)) {
      // Actually, looks like we've synchronized all stages.  We still do need
      // to keep _read_stage_mask, since a subsequent write still needs to
      // wait for this read to complete.
      tc->_write_stage_mask = 0;
      tc->_write_access_mask = 0;
    }
  }
}

/**
 * Same as above, but for shader buffers.
 *
 * Note that these barriers may be done BEFORE waiting on the semaphore.
 */
void VulkanCommandBuffer::
add_barrier(VulkanBufferContext *bc, VkPipelineStageFlags2 dst_stage_mask,
            VkAccessFlags2 dst_access_mask) {
  nassertv(_cmd != VK_NULL_HANDLE);

  // Are we writing to the buffer?
  VkAccessFlags2 write_mask = (dst_access_mask &
    (VK_ACCESS_2_SHADER_WRITE_BIT |
     VK_ACCESS_2_TRANSFER_WRITE_BIT |
     VK_ACCESS_2_HOST_WRITE_BIT |
     VK_ACCESS_2_MEMORY_WRITE_BIT));

  nassertv(bc->_write_seq <= _seq);
  nassertv((write_mask == 0 || bc->_read_seq <= _seq));

  VkPipelineStageFlags2 src_stage_mask = bc->_write_stage_mask;
  VkAccessFlags2 src_access_mask = bc->_write_access_mask;

  if (write_mask != 0) {
    // Before a layout transition or a write, all stages that previously read
    // this resource must have finished executing.
    src_stage_mask |= bc->_read_stage_mask;

    if (src_stage_mask == 0) {
      // Can't specify a source stage mask of zero.
      src_stage_mask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
    }
  }
  else if (src_stage_mask == 0) {
    // No write has been done, nothing to do here, except mark the read.
    bc->mark_read(_seq);
    bc->_read_stage_mask |= dst_stage_mask;
    return;
  }
  else {
    // We've already synchronized these reads since the last write.
    dst_stage_mask &= ~bc->_read_stage_mask;
    if (dst_stage_mask == 0) {
      // We could probably improve this by also early-outing if we've already
      // synchronized a *preceding* stage.
      bc->mark_read(_seq);
      return;
    }
  }

  VkBufferMemoryBarrier2 buf_barrier;
  buf_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
  buf_barrier.pNext = nullptr;
  buf_barrier.srcStageMask = src_stage_mask;
  buf_barrier.srcAccessMask = src_access_mask;
  buf_barrier.dstStageMask = dst_stage_mask;
  buf_barrier.dstAccessMask = dst_access_mask;
  buf_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  buf_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  buf_barrier.buffer = bc->_buffer;
  buf_barrier.offset = 0;
  buf_barrier.size = VK_WHOLE_SIZE;

  // We want to avoid adding lots of pipeline barriers to the command stream,
  // so we instead add this to the list of barriers to be issued at the
  // beginning of this CB, unless it has already been accessed in this CB.
  bool pool_possible =
    (bc->_write_seq < _seq && (bc->_read_seq < _seq || write_mask == 0));

  if (vulkandisplay_cat.is_spam()) {
    const char dst_type = (write_mask != 0) ? 'W' : 'R';
    const char src_type = (src_access_mask != 0) ? 'W' : 'R';
    vulkandisplay_cat.spam()
      << (pool_possible ? "Pooling " : "Issuing ")
      << dst_type << 'a' << src_type << " barrier for "
      << "SSBO " << *(ShaderBuffer *)bc->get_object()
      << " on CB #" << _seq << " (last "
      << ((bc->_read_seq > bc->_write_seq) ? "read on #" : "write on #")
      << bc->_read_seq << ")\n";
  }

  if (pool_possible) {
    // First access in this CB, or a read in a CB without a write.
    if (bc->_read_seq == _seq && bc->_pooled_barrier_exists) {
      // Already exists, this barrier, just modify it.
      nassertv(bc->_buffer_barrier_index <= _buffer_barriers.size());
      VkBufferMemoryBarrier2 &existing_barrier = _buffer_barriers[bc->_buffer_barrier_index];
      existing_barrier.srcStageMask |= buf_barrier.srcStageMask;
      existing_barrier.srcAccessMask |= buf_barrier.srcAccessMask;
      existing_barrier.dstStageMask |= buf_barrier.dstStageMask;
      existing_barrier.dstAccessMask |= buf_barrier.dstAccessMask;
    } else {
      bc->_buffer_barrier_index = _buffer_barriers.size();
      _buffer_barriers.push_back(std::move(buf_barrier));
      bc->_pooled_barrier_exists = true;
    }
  }
  else {
    // We already have an access done in this CB, issue the barrier now.
    VkDependencyInfo info = {
      VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      nullptr, // pNext
      0, // dependencyFlags
      0, // memoryBarrierCount
      nullptr, // pMemoryBarriers
      1, // bufferMemoryBarrierCount
      &buf_barrier, // pBufferMemoryBarriers
      0, // imageMemoryBarrierCount
      nullptr, // pImageMemoryBarriers
    };
    vkCmdPipelineBarrier2(_cmd, &info);

    bc->_pooled_barrier_exists = false;
  }

  bc->_read_seq = _seq;

  if (write_mask != 0) {
    // Remember which stages wrote to it and how.
    bc->_write_stage_mask = dst_stage_mask;
    bc->_write_access_mask = write_mask;
    bc->_read_stage_mask = 0;
    bc->_write_seq = _seq;
  }
  else {
    // This is a read-after-write barrier.  It's possible that there will be
    // another read later from a different (earlier) stage, which is why we
    // don't zero out _write_stage_mask.  We can just check _read_stage_mask
    // the next time to see what we have already synchronized with the write.
    bc->_read_stage_mask |= dst_stage_mask & ~VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;

    if (dst_stage_mask & (VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT | VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT)) {
      // Actually, looks like we've synchronized all stages.  We still do need
      // to keep _read_stage_mask, since a subsequent write still needs to
      // wait for this read to complete.
      bc->_write_stage_mask = 0;
      bc->_write_access_mask = 0;
    }
  }
}
