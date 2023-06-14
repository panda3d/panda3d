/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vulkanTextureContext.cxx
 * @author rdb
 * @date 2016-02-19
 */

#include "vulkanTextureContext.h"
#include "vulkanFrameData.h"

TypeHandle VulkanTextureContext::_type_handle;

/**
 * Schedules the deletion of the image resources for the end of the frame.
 */
void VulkanTextureContext::
release(VulkanFrameData &frame_data) {
  if (_image != VK_NULL_HANDLE) {
    frame_data._pending_destroy_images.push_back(_image);

    if (vulkandisplay_cat.is_debug()) {
      std::ostream &out = vulkandisplay_cat.debug()
        << "Scheduling image " << _image;

      if (!_image_views.empty()) {
        out << " with views";
        for (VkImageView image_view : _image_views) {
          out << " " << image_view;
        }
      }

      out << " for deletion\n";
    }

    _image = VK_NULL_HANDLE;
  }

  if (!_image_views.empty()) {
    frame_data._pending_destroy_image_views.insert(
      frame_data._pending_destroy_image_views.end(),
      _image_views.begin(), _image_views.end());

    _image_views.clear();
  }

  if (_buffer != VK_NULL_HANDLE) {
    frame_data._pending_destroy_buffers.push_back(_buffer);

    if (vulkandisplay_cat.is_debug()) {
      std::ostream &out = vulkandisplay_cat.debug()
        << "Scheduling buffer " << _buffer;

      if (!_buffer_views.empty()) {
        out << " with views";
        for (VkBufferView buffer_view : _buffer_views) {
          out << " " << buffer_view;
        }
      }

      out << " for deletion\n";
    }

    _buffer = VK_NULL_HANDLE;
  }

  if (!_buffer_views.empty()) {
    frame_data._pending_destroy_buffer_views.insert(
      frame_data._pending_destroy_buffer_views.end(),
      _buffer_views.begin(), _buffer_views.end());

    _buffer_views.clear();
  }

  // Make sure that the memory remains untouched until the frame is over.
  frame_data._pending_free.push_back(std::move(_block));

  // The memory isn't free yet, but it can be reclaimed by the memory allocator
  // if really necessary by waiting until the frame queue is empty.
  update_data_size_bytes(0);

  _format = VK_FORMAT_UNDEFINED;
  _layout = VK_IMAGE_LAYOUT_UNDEFINED;
}

/**
 * Destroys the handles associated with this context immediately.
 */
void VulkanTextureContext::
destroy_now(VkDevice device) {
  for (VkImageView image_view : _image_views) {
    vkDestroyImageView(device, image_view, nullptr);
  }
  _image_views.clear();

  if (_image != VK_NULL_HANDLE) {
    vkDestroyImage(device, _image, nullptr);
    _image = VK_NULL_HANDLE;
  }

  for (VkBufferView buffer_view : _buffer_views) {
    vkDestroyBufferView(device, buffer_view, nullptr);
  }
  _buffer_views.clear();

  if (_buffer != VK_NULL_HANDLE) {
    vkDestroyBuffer(device, _buffer, nullptr);
    _buffer = VK_NULL_HANDLE;
  }

  update_data_size_bytes(0);

  _format = VK_FORMAT_UNDEFINED;
  _layout = VK_IMAGE_LAYOUT_UNDEFINED;
}

/**
 * Inserts commands to clear the image.
 */
void VulkanTextureContext::
clear_color_image(VkCommandBuffer cmd, const VkClearColorValue &value) {
  nassertv(_aspect_mask == VK_IMAGE_ASPECT_COLOR_BIT);
  nassertv(_image != VK_NULL_HANDLE);

  // We're not interested in whatever was in here before.
  discard();

  transition(cmd, 0,//vkgsg->_graphics_queue_family_index,
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT);

  VkImageSubresourceRange range;
  range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  range.baseMipLevel = 0;
  range.levelCount = _mip_levels;
  range.baseArrayLayer = 0;
  range.layerCount = _array_layers;
  vkCmdClearColorImage(cmd, _image, _layout, &value, 1, &range);
}

/**
 * Inserts commands to clear the image.
 */
void VulkanTextureContext::
clear_depth_stencil_image(VkCommandBuffer cmd, const VkClearDepthStencilValue &value) {
  nassertv(_aspect_mask != VK_IMAGE_ASPECT_COLOR_BIT);
  nassertv(_image != VK_NULL_HANDLE);

  // We're not interested in whatever was in here before.
  discard();

  transition(cmd, 0,//vkgsg->_graphics_queue_family_index,
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT);

  VkImageSubresourceRange range;
  range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
  range.baseMipLevel = 0;
  range.levelCount = _mip_levels;
  range.baseArrayLayer = 0;
  range.layerCount = _array_layers;
  vkCmdClearDepthStencilImage(cmd, _image, _layout, &value, 1, &range);
}

/**
 * Inserts commands to clear the image.
 */
void VulkanTextureContext::
clear_buffer(VkCommandBuffer cmd, uint32_t fill) {
  nassertv(_buffer != VK_NULL_HANDLE);

  vkCmdFillBuffer(cmd, _buffer, 0, VK_WHOLE_SIZE, fill);
}

/**
 * Issues a command to transition the image to a new layout or queue family.
 * Does not (yet) do inter-queue synchronization.
 */
void VulkanTextureContext::
transition(VkCommandBuffer cmd, uint32_t queue_family, VkImageLayout layout,
           VkPipelineStageFlags dst_stage_mask, VkAccessFlags dst_access_mask) {

  if (_image == VK_NULL_HANDLE) {
    return;
  }

  if (_layout == layout) {
    return;
  }

  VkImageMemoryBarrier barrier;
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.pNext = nullptr;
  barrier.srcAccessMask = _access_mask;
  barrier.dstAccessMask = dst_access_mask;
  barrier.oldLayout = _layout;
  barrier.newLayout = layout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;//_graphics_queue_family_index;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;//_graphics_queue_family_index;
  barrier.image = _image;
  barrier.subresourceRange.aspectMask = _aspect_mask;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = _mip_levels;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = _array_layers;

  vkCmdPipelineBarrier(cmd, _stage_mask, dst_stage_mask, 0,
                       0, nullptr, 0, nullptr, 1, &barrier);

  _layout = layout;
  _access_mask = dst_access_mask;
  _stage_mask = dst_stage_mask;
}
