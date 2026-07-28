/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vulkanGraphicsPipe.I
 * @author rdb
 * @date 2016-02-16
 */

/**
 * Returns true if the given heap is a limited slice of device memory, like a
 * 256 MiB BAR aperture, rather than unified memory or a full-size heap.
 * Scarce heaps are reserved for allocations that get the most out of them.
 */
INLINE bool VulkanGraphicsPipe::
is_scarce_heap(uint32_t heap_index) const {
  return (_scarce_heaps & (1u << heap_index)) != 0;
}
