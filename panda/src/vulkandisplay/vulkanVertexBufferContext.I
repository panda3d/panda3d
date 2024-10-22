/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vulkanVertexBufferContext.I
 * @author rdb
 * @date 2016-02-18
 */

/**
 *
 */
INLINE VulkanVertexBufferContext::
VulkanVertexBufferContext(PreparedGraphicsObjects *pgo,
                          GeomVertexArrayData *data) :
  VertexBufferContext(pgo, data),
  _buffer(VK_NULL_HANDLE) {
}

/**
 * Returns true if the buffer has been used this frame.
 */
INLINE bool VulkanVertexBufferContext::
is_used_this_frame(VulkanFrameData &frame_data) const {
  return frame_data._frame_index == _last_use_frame;
}

/**
 * Marks the buffer as having been used this frame.
 */
INLINE void VulkanVertexBufferContext::
mark_used_this_frame(VulkanFrameData &frame_data) {
  if (frame_data._frame_index != _last_use_frame) {
    _last_use_frame = frame_data._frame_index;

    // Good opportunity to call this.
    set_active(true);
  }
}
