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
 * Returns true if the texture has been used this frame.  By "used" we mean
 * that it has been accessed by a shader, not updated from the CPU.
 */
INLINE bool VulkanTextureContext::
is_used_this_frame(VulkanFrameData &frame_data) const {
  return frame_data._frame_index == _last_use_frame;
}

/**
 * Marks the texture as having been used this frame.  By "used" we mean
 * that it has been accessed by a shader, not updated from the CPU.
 */
INLINE void VulkanTextureContext::
mark_used_this_frame(VulkanFrameData &frame_data) {
  if (frame_data._frame_index != _last_use_frame) {
    _last_use_frame = frame_data._frame_index;

    // Good opportunity to initialize these fields.
    _initial_src_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    _initial_dst_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    _initial_src_access_mask = 0;
    _initial_dst_access_mask = 0;

    // And to call this.
    set_active(true);
  }
}

/**
 * Indicates that the texture is being read from by the given stage.  Any
 * subsequent writes must wait until the given stage has passed the pipeline.
 *
 * Generally you should rely on gsg->use_texture() or tc->transition() to take
 * care of this.
 */
INLINE void VulkanTextureContext::
mark_read(VkPipelineStageFlags stage) {
  _read_stage_mask |= stage & ~VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
}

/**
 * Indicates that the texture is being written by the given stage.  Any
 * subsequent writes must wait until the given stage has passed the pipeline.
 *
 * Generally you should rely on gsg->use_texture() or tc->transition() to take
 * care of this.
 */
INLINE void VulkanTextureContext::
mark_written(VkPipelineStageFlags stage, VkAccessFlags access_mask) {
  _write_stage_mask |= stage;
  _write_access_mask |= access_mask;
  _read_stage_mask = 0;
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
  _write_access_mask = 0;
}
