/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vulkanFrameData.cxx
 * @author rdb
 * @date 2023-06-14
 */

#include "vulkanFrameData.h"
#include "vulkanTextureContext.h"

/**
 * Ensures a pipeline barrier is created for an initial transition of the given
 * texture, after the transfer commands have completed, but before the frist
 * write.  These are pooled together.
 *
 * For now, may only be called for read barriers, not write, so access_mask
 * may not contain any write bits.
 *
 * The _initial_src_layout, etc. members should have already been initialized
 * in this frame.  Any transfers on the transfer queue must have already been
 * performed for this texture.
 *
 * Returns false if this is actually not possible, due to a layout mismatch,
 * in which case there can't be a pooled transition.
 */
bool VulkanFrameData::
add_initial_barrier(VulkanTextureContext *tc, VkImageLayout layout,
                    VkPipelineStageFlags stage_mask,
                    VkAccessFlags access_mask) {
  if (layout == tc->_layout && tc->_write_stage_mask == 0) {
    // No write to sync with.
    return true;
  }

  // These reads have already been synced.
  if (layout == tc->_layout) {
    stage_mask &= ~tc->_read_stage_mask;
    if (stage_mask == 0) {
      return true;
    }
  }

  if (tc->_initial_dst_layout == VK_IMAGE_LAYOUT_UNDEFINED) {
    // These are derived from the current stage of the texture, after any
    // transfer commands but before any writes have been done.
    tc->_initial_src_layout = tc->_layout;
    tc->_initial_src_access_mask |= tc->_write_access_mask;
    _initial_barrier_src_stage_mask |= tc->_write_stage_mask;

    if (layout != tc->_layout) {
      // If we change layout, wait for all reads to complete too.
      _initial_barrier_src_stage_mask |= tc->_read_stage_mask;
    }

    // And this is what we are transitioning to.
    tc->_initial_dst_layout = layout;
    tc->_layout = layout;

    _initial_barrier_textures.push_back(tc);
    _initial_barrier_image_count += (tc->_image != VK_NULL_HANDLE);
    _initial_barrier_buffer_count += (tc->_buffer != VK_NULL_HANDLE);
  }
  else if (tc->_initial_dst_layout != layout) {
    return false;
  }

  tc->mark_read(stage_mask);
  tc->_initial_dst_access_mask |= access_mask;
  _initial_barrier_dst_stage_mask |= stage_mask;
  return true;
}

/**
 *
 */
bool VulkanFrameData::
begin_transfer_cmd() {
  static const VkCommandBufferBeginInfo begin_info = {
    VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    nullptr,
    VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    nullptr,
  };

  VkResult err;
  err = vkBeginCommandBuffer(_transfer_cmd, &begin_info);
  if (err != VK_SUCCESS) {
    vulkan_error(err, "Can't begin transfer command buffer");
    return false;
  }
  return true;
}

/**
 * Issues a pipeline barrier to all the initial transitions, and closes the
 * transfer command buffer.
 */
void VulkanFrameData::
end_transfer_cmd() {
  nassertv(_transfer_cmd != VK_NULL_HANDLE);

  if (_initial_barrier_dst_stage_mask != 0) {
    VkImageMemoryBarrier *image_barriers = (VkImageMemoryBarrier *)alloca(sizeof(VkImageMemoryBarrier) * _initial_barrier_image_count);
    VkBufferMemoryBarrier *buffer_barriers = (VkBufferMemoryBarrier *)alloca(sizeof(VkBufferMemoryBarrier) * _initial_barrier_buffer_count);

    uint32_t ii = 0;
    uint32_t bi = 0;
    for (VulkanTextureContext *tc : _initial_barrier_textures) {
      if (tc->_image != VK_NULL_HANDLE) {
        VkImageMemoryBarrier &barrier = image_barriers[ii++];
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.pNext = nullptr;
        barrier.srcAccessMask = tc->_initial_src_access_mask;
        barrier.dstAccessMask = tc->_initial_dst_access_mask;
        barrier.oldLayout = tc->_initial_src_layout;
        barrier.newLayout = tc->_initial_dst_layout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = tc->_image;
        barrier.subresourceRange.aspectMask = tc->_aspect_mask;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = tc->_mip_levels;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = tc->_array_layers;
      }

      if (tc->_buffer != VK_NULL_HANDLE) {
        VkBufferMemoryBarrier &barrier = buffer_barriers[bi++];
        barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        barrier.pNext = nullptr;
        barrier.srcAccessMask = tc->_initial_src_access_mask;
        barrier.dstAccessMask = tc->_initial_dst_access_mask;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.buffer = tc->_buffer;
        barrier.offset = 0;
        barrier.size = VK_WHOLE_SIZE;
      }

      tc->_initial_src_access_mask = 0;
      tc->_initial_dst_access_mask = 0;
      tc->_initial_src_layout = VK_IMAGE_LAYOUT_UNDEFINED;
      tc->_initial_dst_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    }
    vkCmdPipelineBarrier(_transfer_cmd, _initial_barrier_src_stage_mask,
                         _initial_barrier_dst_stage_mask, 0,
                         0, nullptr, bi, buffer_barriers, ii, image_barriers);

    _initial_barrier_textures.clear();
    _initial_barrier_src_stage_mask = 0;
    _initial_barrier_dst_stage_mask = 0;
    _initial_barrier_image_count = 0;
    _initial_barrier_buffer_count = 0;
  }

  vkEndCommandBuffer(_transfer_cmd);
}

/**
 *
 */
bool VulkanFrameData::
begin_render_cmd() {
  static const VkCommandBufferBeginInfo begin_info = {
    VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    nullptr,
    VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    nullptr,
  };

  VkResult err;
  err = vkBeginCommandBuffer(_cmd, &begin_info);
  if (err != VK_SUCCESS) {
    vulkan_error(err, "Can't begin render command buffer");
    return false;
  }
  return true;
}

/**
 *
 */
void VulkanFrameData::
end_render_cmd() {
  nassertv(_cmd != VK_NULL_HANDLE);
  vkEndCommandBuffer(_cmd);
}

/**
 *
 */
void VulkanFrameData::
finish_downloads(VkDevice device) {
  for (QueuedDownload &down : _download_queue) {
    PTA_uchar target = down._texture->modify_ram_image();
    size_t view_size = down._texture->get_ram_view_size();

    if (auto data = down._block.map()) {
      // The texture is upside down, so invert it.
      size_t row_size = down._texture->get_x_size()
                      * down._texture->get_num_components()
                      * down._texture->get_component_width();
      unsigned char *dst = target.p() + view_size * (down._view + 1) - row_size;
      unsigned char *src = (unsigned char *)data;
      unsigned char *src_end = src + view_size;
      while (src < src_end) {
        memcpy(dst, src, row_size);
        src += row_size;
        dst -= row_size;
      }
    } else {
      vulkandisplay_cat.error()
        << "Failed to map memory for RAM transfer.\n";
    }

    // We won't need this buffer any more.
    vkDestroyBuffer(device, down._buffer, nullptr);

    if (down._request != nullptr) {
      down._request->finish();
    }
  }
  _download_queue.clear();
}

/**
 *
 */
void VulkanFrameData::
replace_timer_query_pool(VkQueryPool new_pool, size_t new_size) {
  if (_timer_query_pool._pool != VK_NULL_HANDLE) {
    TimerQueryPool *prev_pool = new TimerQueryPool(std::move(_timer_query_pool));
    _timer_query_pool._prev = prev_pool;
  }
  _timer_query_pool._pool = new_pool;
  _timer_query_pool._pool_size = new_size;
  _timer_query_pool._offset = 0;
  _timer_query_pool._pstats_indices.clear();
}
