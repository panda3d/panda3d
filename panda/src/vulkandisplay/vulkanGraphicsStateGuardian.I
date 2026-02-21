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
  if (_frame_data == nullptr) {
    _frame_data = &get_next_frame_data();
  }
  return *_frame_data;
}

/**
 * Returns an existing allocation in the transform buffer for the given table.
 * Returns true if it needs to be updated, false if not.  Either way, offset
 * will indicate what part of the buffer can be written to.  If there is no
 * room in the buffer, the offset will be past the end of the buffer, so you
 * must check this if it returns true.
 */
INLINE bool VulkanGraphicsStateGuardian::
get_transform_buffer(const TransformTable *table, uint32_t &offset) {
  return table != nullptr &&
    get_transform_buffer(table, table->get_num_transforms(), table->get_modified(), offset);
}

/**
 * Returns an existing allocation in the transform buffer for the given table.
 * Returns true if it needs to be updated, false if not.  Either way, offset
 * will indicate what part of the buffer can be written to.  If there is no
 * room in the buffer, the offset will be past the end of the buffer, so you
 * must check this if it returns true.
 */
INLINE bool VulkanGraphicsStateGuardian::
get_transform_buffer(const InstanceList *instances, uint32_t &offset) {
  return instances != nullptr &&
    get_transform_buffer(instances, instances->size(), instances->get_modified(), offset);
}

/**
 *
 */
INLINE void VulkanGraphicsStateGuardian::
set_object_name(VkObjectType type, void *handle, const char *name) {
#ifndef NDEBUG
  if (_vkSetDebugUtilsObjectName != nullptr) {
    VkDebugUtilsObjectNameInfoEXT info = {};
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    info.objectType = type;
    info.objectHandle = (uint64_t)handle;
    info.pObjectName = name;
    _vkSetDebugUtilsObjectName(_device, &info);
  }
#endif
}

/**
 *
 */
INLINE void VulkanGraphicsStateGuardian::
set_object_name(VkBuffer object, const std::string &name) {
  set_object_name(VK_OBJECT_TYPE_BUFFER, object, name.c_str());
}

/**
 *
 */
INLINE void VulkanGraphicsStateGuardian::
set_object_name(VkBufferView object, const std::string &name) {
  set_object_name(VK_OBJECT_TYPE_BUFFER_VIEW, object, name.c_str());
}

/**
 *
 */
INLINE void VulkanGraphicsStateGuardian::
set_object_name(VkDescriptorSet object, const std::string &name) {
  set_object_name(VK_OBJECT_TYPE_DESCRIPTOR_SET, object, name.c_str());
}

/**
 *
 */
INLINE void VulkanGraphicsStateGuardian::
set_object_name(VkImage object, const std::string &name) {
  set_object_name(VK_OBJECT_TYPE_IMAGE, object, name.c_str());
}

/**
 *
 */
INLINE void VulkanGraphicsStateGuardian::
set_object_name(VkImageView object, const std::string &name) {
  set_object_name(VK_OBJECT_TYPE_IMAGE_VIEW, object, name.c_str());
}

/**
 *
 */
INLINE void VulkanGraphicsStateGuardian::
set_object_name(VkSampler object, const std::string &name) {
  set_object_name(VK_OBJECT_TYPE_SAMPLER, object, name.c_str());
}
