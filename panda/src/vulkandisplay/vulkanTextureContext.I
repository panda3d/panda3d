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
VulkanTextureContext(PreparedGraphicsObjects *pgo, Texture *texture, int view) :
  TextureContext(pgo, texture, view),
  _image(VK_NULL_HANDLE),
  _memory(VK_NULL_HANDLE),
  _image_view(VK_NULL_HANDLE),
  _pack_bgr8(false),
  _layout(VK_IMAGE_LAYOUT_UNDEFINED),
  _access_mask(0),
    //TODO: it is not clear to me what we should set srcStageMask to here.
  _stage_mask(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT) {
}

/**
 * Constructs a Texture context that's not associated with a particular
 * texture, but tracks an already existing VkImage object.
 */
INLINE VulkanTextureContext::
VulkanTextureContext(PreparedGraphicsObjects *pgo, VkImage image, VkFormat format) :
  TextureContext(pgo, nullptr, 0),
  _format(format),
  _image(image),
  _memory(VK_NULL_HANDLE),
  _image_view(VK_NULL_HANDLE),
  _pack_bgr8(false),
  _layout(VK_IMAGE_LAYOUT_UNDEFINED),
  _access_mask(0),
  //TODO: it is not clear to me what we should set srcStageMask to here.
  _stage_mask(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT) {
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
