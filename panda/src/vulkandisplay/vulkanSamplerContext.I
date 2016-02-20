/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vulkanSamplerContext.I
 * @author rdb
 * @date 2016-02-18
 */


/**
 * Constructs a sampler context managing the given sampler object.
 */
INLINE VulkanSamplerContext::
VulkanSamplerContext(const SamplerState &sampler_state, VkSampler sampler) :
  SamplerContext(sampler_state),
  _sampler(sampler) {
}
