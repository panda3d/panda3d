/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vulkanBufferContext.I
 * @author rdb
 * @date 2024-10-16
 */

/**
 *
 */
INLINE VulkanBufferContext::
VulkanBufferContext(PreparedGraphicsObjects *pgo,
                    TypedWritableReferenceCount *object) :
  BufferContext(&pgo->_sbuffer_residency, object),
  _buffer(VK_NULL_HANDLE) {
}
