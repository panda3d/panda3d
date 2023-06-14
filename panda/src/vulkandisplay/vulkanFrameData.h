/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vulkanFrameData.h
 * @author rdb
 * @date 2023-06-14
 */

#ifndef VULKANFRAMEDATA_H
#define VULKANFRAMEDATA_H

#include "config_vulkandisplay.h"
#include "vulkanMemoryPage.h"
#include "screenshotRequest.h"

/**
 * Stores all the data that has been collected between a begin_frame/end_frame
 * pair, until the frame has finished rendering on the GPU.
 */
class VulkanFrameData {
public:
  void finish_downloads(VkDevice device);

public:
  uint64_t _frame_index = 0;
  VkFence _fence = VK_NULL_HANDLE;
  VkCommandBuffer _cmd = VK_NULL_HANDLE;
  VkCommandBuffer _transfer_cmd = VK_NULL_HANDLE;

  // Keep track of resources that should be deleted after this frame is done.
  pvector<VulkanMemoryBlock> _pending_free;
  pvector<VkBuffer> _pending_destroy_buffers;
  pvector<VkBufferView> _pending_destroy_buffer_views;
  pvector<VkFramebuffer> _pending_destroy_framebuffers;
  pvector<VkImage> _pending_destroy_images;
  pvector<VkImageView> _pending_destroy_image_views;
  pvector<VkRenderPass> _pending_destroy_render_passes;
  pvector<VkSampler> _pending_destroy_samplers;
  pvector<VkDescriptorSet> _pending_free_descriptor_sets;

  VkDeviceSize _uniform_buffer_head = 0;
  VkDeviceSize _staging_buffer_head = 0;

  // Queued buffer-to-RAM transfer.
  struct QueuedDownload {
    VkBuffer _buffer;
    VulkanMemoryBlock _block;
    PT(Texture) _texture;
    int _view;
    PT(ScreenshotRequest) _request;
  };
  typedef pvector<QueuedDownload> DownloadQueue;
  DownloadQueue _download_queue;

  bool _wait_for_finish = false;
};

#endif
