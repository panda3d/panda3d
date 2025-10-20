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

/**
 * Stores all the data that has been collected between a begin_frame/end_frame
 * pair, until the frame has finished rendering on the GPU.
 *
 * At the moment, the frame is divided up into two command buffers, one
 * collecting all the actions needed to prepare and upload the texture data
 * (_transfer_cmd) and one containing the actual rendering (_cmd).  At the end
 * of the transfer cmd we issue a barrier for preparing all the resources for
 * their first use.  Both command buffers are submitted in gsg->end_frame().
 */
class VulkanFrameData {
public:
  bool add_initial_barrier(VulkanTextureContext *tc, VkImageLayout layout,
                           VkPipelineStageFlags stage_mask,
                           VkAccessFlags access_mask = 0);

  bool begin_transfer_cmd();
  void end_transfer_cmd();

  bool begin_render_cmd();
  void end_render_cmd();

  void finish_downloads(VkDevice device);

  void replace_timer_query_pool(VkQueryPool new_pool, size_t new_size);

public:
  uint64_t _frame_index = 0;
  int _clock_frame_number = 0;
  VkFence _fence = VK_NULL_HANDLE;
  VkCommandBuffer _cmd = VK_NULL_HANDLE;
  VkCommandBuffer _transfer_cmd = VK_NULL_HANDLE;

  // The frame data takes ownership of this semaphore, which indicates when the
  // frame is allowed to start rendering (the image is available).
  VkSemaphore _wait_semaphore = VK_NULL_HANDLE;

  // Barriers that are aggregated for the beginning of the frame, put at the
  // end of the transfer command buffer.
  pvector<VulkanTextureContext *> _initial_barrier_textures;
  VkPipelineStageFlags _initial_barrier_src_stage_mask = 0;
  VkPipelineStageFlags _initial_barrier_dst_stage_mask = 0;
  size_t _initial_barrier_image_count = 0;
  size_t _initial_barrier_buffer_count = 0;

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
