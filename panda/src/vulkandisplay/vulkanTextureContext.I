/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vulkanTextureContext.I
 * @author rdb
 * @date 2016-02-19
 */

/**
 * Constructs a Texture context.  Follow this up with calls to fill in the
 * image, allocate the memory and create an image view.
 */
INLINE VulkanTextureContext::
VulkanTextureContext(PreparedGraphicsObjects *pgo, Texture *texture) :
  TextureContext(pgo, texture) {
}

/**
 * Returns the VkImageView handle for the given view of the texture.
 */
INLINE VkImageView VulkanTextureContext::
get_image_view(int view) const {
  if (!_image_views.empty()) {
    return _image_views[std::min(std::max(view, 0), (int)_image_views.size() - 1)];
  } else {
    return VK_NULL_HANDLE;
  }
}

/**
 * Returns the VkBufferView handle for the given view of the texture.
 */
INLINE VkBufferView VulkanTextureContext::
get_buffer_view(int view) const {
  if (!_buffer_views.empty()) {
    return _buffer_views[std::min(std::max(view, 0), (int)_buffer_views.size() - 1)];
  } else {
    return VK_NULL_HANDLE;
  }
}

/**
 * Indicates that the texture has been read by the command buffer with the
 * given sequence number.
 */
INLINE void VulkanTextureContext::
mark_read(uint64_t seq) {
  if (seq != _read_seq) {
    _read_seq = seq;
    _image_barrier_index = 0;
    _buffer_barrier_index = 0;
    _hoisted_barrier_exists = false;
  }
}

/**
 * Indicates that future commands aren't really interested in seeing the
 * results of writes made before it to this image.
 */
INLINE void VulkanTextureContext::
discard() {
  // We don't clear _write_stage_mask or _read_stage_mask, because we still
  // want the execution dependency; we don't want the next write to stomp over
  // the texture data while it's still being read.
  _layout = VK_IMAGE_LAYOUT_UNDEFINED;
  //_write_access_mask = 0;
}
