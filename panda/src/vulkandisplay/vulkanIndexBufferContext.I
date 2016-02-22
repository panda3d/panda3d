/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vulkanIndexBufferContext.I
 * @author rdb
 * @date 2016-02-22
 */

/**
 *
 */
INLINE VulkanIndexBufferContext::
VulkanIndexBufferContext(PreparedGraphicsObjects *pgo,
                          GeomPrimitive *data) :
  IndexBufferContext(pgo, data),
  _buffer(VK_NULL_HANDLE),
  _memory(VK_NULL_HANDLE) {
}
