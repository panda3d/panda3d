/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vulkanTextureContext.h
 * @author rdb
 * @date 2016-02-19
 */

#ifndef VULKANTEXTURECONTEXT_H
#define VULKANTEXTURECONTEXT_H

#include "config_vulkandisplay.h"
#include "textureContext.h"
#include "small_vector.h"

class VulkanCommandBuffer;

/**
 * Manages a Vulkan image and device memory.
 */
class EXPCL_VULKANDISPLAY VulkanTextureContext : public TextureContext {
public:
  INLINE VulkanTextureContext(PreparedGraphicsObjects *pgo, Texture *texture = nullptr);
  ~VulkanTextureContext() {};

  ALLOC_DELETED_CHAIN(VulkanTextureContext);

  bool needs_recreation() const;

  void release(VulkanFrameData &frame_data);
  void destroy_now(VkDevice device);

  INLINE VkImageView get_image_view(int view) const;
  INLINE VkBufferView get_buffer_view(int view) const;

  INLINE void discard();

  void clear_color_image(VulkanCommandBuffer &cmd, const VkClearColorValue &value);
  void clear_depth_stencil_image(VulkanCommandBuffer &cmd, const VkClearDepthStencilValue &value);
  void clear_buffer(VulkanCommandBuffer &cmd, uint32_t fill);

public:
  VkFormat _format = VK_FORMAT_UNDEFINED;
  VkExtent3D _extent;
  int _mipmap_begin = 0, _mipmap_end = 1;
  uint32_t _mip_levels = 1;
  uint32_t _array_layers = 1;
  VkImageAspectFlags _aspect_mask = 0;
  bool _generate_mipmaps = false;
  bool _pack_bgr8 = false;
  bool _swap_bgra8 = false;
  bool _supports_render_to_texture = false;

  // Just for debugging.  It's -1 if it's not a swapchain image.
  int _swapchain_index = -1;

  // Depending on whether it's a buffer texture or image texture, either the
  // image and image view or buffer and buffer view will be set.
  VkImage _image = VK_NULL_HANDLE;
  small_vector<VkImageView> _image_views;
  VkBuffer _buffer = VK_NULL_HANDLE;
  small_vector<VkBufferView> _buffer_views;
  VulkanMemoryBlock _block;

  // Sequence number of the last command buffer in which this was used.
  uint64_t _read_seq = 0;
  uint64_t _write_seq = 0;

  // Index of the barrier into the list of barriers of the _read_seq CB.
  size_t _image_barrier_index = 0;
  size_t _buffer_barrier_index = 0;

  // The "current" layout and details of the last write
  VkImageLayout _layout = VK_IMAGE_LAYOUT_UNDEFINED;
  VkAccessFlags _write_access_mask = 0;
  VkPipelineStageFlags _write_stage_mask = 0;

  // Which stages we've already synchronized with the last write.
  VkPipelineStageFlags _read_stage_mask = 0;

  // If you're wondering why there is no _read_access_mask, read this:
  // https://github.com/KhronosGroup/Vulkan-Docs/issues/131

  VkBuffer _async_staging_buffer = VK_NULL_HANDLE;
  void *_async_staging_ptr = nullptr;
  size_t _async_buffer_size = 0;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TextureContext::init_type();
    register_type(_type_handle, "VulkanTextureContext",
                  TextureContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "vulkanTextureContext.I"

#endif  // VULKANTEXTURECONTEXT_H
