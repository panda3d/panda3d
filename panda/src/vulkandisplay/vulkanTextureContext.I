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
  _image_view(VK_NULL_HANDLE) {
}
