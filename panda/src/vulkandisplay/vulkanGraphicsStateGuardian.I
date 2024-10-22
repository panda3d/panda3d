/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vulkanGraphicsStateGuardian.I
 * @author rdb
 * @date 2016-02-16
 */

/**
 * Returns the current frame data object.
 * May only be called between begin_frame() and end_frame().
 */
INLINE VulkanFrameData &VulkanGraphicsStateGuardian::
get_frame_data() {
  nassertr(_frame_data != nullptr, *_last_frame_data);
  return *_frame_data;
}
