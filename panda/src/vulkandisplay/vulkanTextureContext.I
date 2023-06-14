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
  TextureContext(pgo, texture),
  //TODO: it is not clear to me what we should set srcStageMask to here.
  _stage_mask(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT) {
}

/**
 * Constructs a Texture context that's not associated with a particular
 * texture, but tracks an already existing VkImage object.
 */
INLINE VulkanTextureContext::
VulkanTextureContext(PreparedGraphicsObjects *pgo, VkImage image, VkFormat format) :
  TextureContext(pgo, nullptr),
  _format(format),
  _image(image),
  //TODO: it is not clear to me what we should set srcStageMask to here.
  _stage_mask(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT) {
}

/**
 * Sets the Texture object this context is for.
 */
INLINE void VulkanTextureContext::
set_texture(Texture *texture) {
  _object = texture;
}

/**
 * Returns the VkImageView handle for the given view of the texture.
 */
INLINE const VkImageView &VulkanTextureContext::
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
INLINE const VkBufferView &VulkanTextureContext::
get_buffer_view(int view) const {
  if (!_buffer_views.empty()) {
    return _buffer_views[std::min(std::max(view, 0), (int)_buffer_views.size() - 1)];
  } else {
    return VK_NULL_HANDLE;
  }
}

/**
 * Records a way that the image has been accessed.
 */
INLINE void VulkanTextureContext::
access(VkPipelineStageFlags stage, VkAccessFlags access_mask) {
  _stage_mask |= stage;
  _access_mask |= access_mask;
}

/**
 * Indicates that future commands aren't really interested in seeing the
 * results of writes made before it to this image.
 */
INLINE void VulkanTextureContext::
discard() {
  _access_mask = 0;
  _layout = VK_IMAGE_LAYOUT_UNDEFINED;
}
