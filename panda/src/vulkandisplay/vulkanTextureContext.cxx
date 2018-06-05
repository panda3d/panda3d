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

TypeHandle VulkanTextureContext::_type_handle;

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
 * Issues a command to transition the image to a new layout or queue family.
 * Does not (yet) do inter-queue synchronization.
 */
void VulkanTextureContext::
transition(VkCommandBuffer cmd, uint32_t queue_family, VkImageLayout layout,
           VkPipelineStageFlags dst_stage_mask, VkAccessFlags dst_access_mask) {

  nassertv(_image != VK_NULL_HANDLE);
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
