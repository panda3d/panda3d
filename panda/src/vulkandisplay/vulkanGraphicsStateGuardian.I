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
 * Returns true if these two DescriptorSetKey objects are identical.
 */
INLINE bool VulkanGraphicsStateGuardian::DescriptorSetKey::
operator ==(const DescriptorSetKey &other) const {
  return _tex_attrib == other._tex_attrib &&
         _shader_attrib == other._shader_attrib;
}

/**
 * Defines relative sorting order of DescriptorSetKey objects.
 */
INLINE bool VulkanGraphicsStateGuardian::DescriptorSetKey::
operator < (const DescriptorSetKey &other) const {
  if (_tex_attrib != other._tex_attrib) {
    return _tex_attrib < other._tex_attrib;
  }
  return _shader_attrib < other._shader_attrib;
}
