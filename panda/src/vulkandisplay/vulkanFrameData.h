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

class VulkanTextureContext;
class VulkanBufferContext;

/**
 * Stores all the resources that have been used during a frame until it has
 * finished rendering on the GPU, at which point they may be released back to
 * their respective pools (and other CPU-related tasks depending on the data
 * may be performed).
 *
 * The definition of a "frame" is a bit vague here, as the frame may be ended
 * arbitrarily early, however it lasts at least as long as a
 * begin_frame()/end_frame() pair and at most as long as a clock frame.
 */
class VulkanFrameData {
public:
  void finish_downloads(VkDevice device);

  void replace_timer_query_pool(VkQueryPool new_pool, size_t new_size);

public:
  uint64_t _frame_index = 0;
  int _clock_frame_number = 0;
  VkFence _fence = VK_NULL_HANDLE;

  // Keep track of resources that should be deleted after this frame is done.
  pvector<VkCommandBuffer> _pending_command_buffers;
  pvector<VulkanMemoryBlock> _pending_free;
  pvector<VkBuffer> _pending_destroy_buffers;
  pvector<VkBufferView> _pending_destroy_buffer_views;
  pvector<VkFramebuffer> _pending_destroy_framebuffers;
  pvector<VkImage> _pending_destroy_images;
  pvector<VkImageView> _pending_destroy_image_views;
  pvector<VkRenderPass> _pending_destroy_render_passes;
  pvector<VkSampler> _pending_destroy_samplers;
  pvector<VkSemaphore> _pending_destroy_semaphores;
  pvector<VkDescriptorSet> _pending_free_descriptor_sets;

  VkDeviceSize _uniform_buffer_head = 0;
  VkDeviceSize _staging_buffer_head = 0;

  // Defines a range in a timer query pool that is used in this frame.
  // The _prev pointer is a linked list of old pools that were replaced during
  // this frame.
  struct TimerQueryPool {
    VkQueryPool _pool = VK_NULL_HANDLE;
    size_t _pool_size = 0;
    uint32_t _offset = 0;
    pvector<uint16_t> _pstats_indices;
    TimerQueryPool *_prev = nullptr;
  };
  TimerQueryPool _timer_query_pool;

  // PStats timestamp taken in begin_frame().
  double _start_time;

  // PStats timestamp taken right before submission, so we can compare how
  // long it takes for the GPU to start the work.
  double _submit_time;

  // PStats timestamp taken after waiting for the frame on the GPU.
  double _finish_time = 0;

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
