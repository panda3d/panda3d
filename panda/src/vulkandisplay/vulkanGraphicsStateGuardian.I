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
 * Copy constructor for PipelineKey.
 */
INLINE VulkanGraphicsStateGuardian::PipelineKey::
PipelineKey(const PipelineKey &copy) :
  _state(copy._state),
  _format(copy._format),
  _topology(copy._topology) {
}

/**
 * Copy assignment operator for PipelineKey.
 */
INLINE void VulkanGraphicsStateGuardian::PipelineKey::
operator = (const PipelineKey &copy) {
  _state = copy._state;
  _format = copy._format;
  _topology = copy._topology;
}

#ifdef USE_MOVE_SEMANTICS
/**
 * Move constructor for PipelineKey, defined for efficiency (so that we don't
 * have to increase and decrease the reference count when moving this object).
 */
INLINE VulkanGraphicsStateGuardian::PipelineKey::
PipelineKey(PipelineKey &&from) NOEXCEPT :
  _state(move(from._state)),
  _format(move(from._format)),
  _topology(from._topology) {
}

/**
 * Move assignment operator for PipelineKey.
 */
INLINE void VulkanGraphicsStateGuardian::PipelineKey::
operator = (PipelineKey &&from) NOEXCEPT {
  _state = move(from._state);
  _format = move(from._format);
  _topology = from._topology;
}
#endif  // USE_MOVE_SEMANTICS

/**
 * Returns true if these two PipelineKey objects are identical.
 */
INLINE bool VulkanGraphicsStateGuardian::PipelineKey::
operator ==(const PipelineKey &other) const {
  return _state == other._state
      && _format == other._format
      && _topology == other._topology;
}

/**
 * Defines relative sorting order of PipelineKey objects.
 */
INLINE bool VulkanGraphicsStateGuardian::PipelineKey::
operator < (const PipelineKey &other) const {
  if (_state != other._state) {
    return _state < other._state;
  }
  if (_format != other._format) {
    return _format < other._format;
  }
  return _topology < other._topology;
}

/**
 * Copy constructor for DescriptorSetKey.
 */
INLINE VulkanGraphicsStateGuardian::DescriptorSetKey::
DescriptorSetKey(const DescriptorSetKey &copy) :
  _tex_attrib(copy._tex_attrib),
  _shader_attrib(copy._shader_attrib) {
}

/**
 * Copy assignment operator for DescriptorSetKey.
 */
INLINE void VulkanGraphicsStateGuardian::DescriptorSetKey::
operator = (const DescriptorSetKey &copy) {
  _tex_attrib = copy._tex_attrib;
  _shader_attrib = copy._shader_attrib;
}

#ifdef USE_MOVE_SEMANTICS
/**
 * Move constructor for DescriptorSetKey, defined for efficiency (so that we
 * don't have to increase and decrease the reference count when moving this
 * object).
 */
INLINE VulkanGraphicsStateGuardian::DescriptorSetKey::
DescriptorSetKey(DescriptorSetKey &&from) NOEXCEPT :
  _tex_attrib(move(from._tex_attrib)),
  _shader_attrib(move(from._shader_attrib)) {
}

/**
 * Move assignment operator for DescriptorSetKey.
 */
INLINE void VulkanGraphicsStateGuardian::DescriptorSetKey::
operator = (DescriptorSetKey &&from) NOEXCEPT {
  _tex_attrib = move(from._tex_attrib);
  _shader_attrib = move(from._shader_attrib);
}
#endif  // USE_MOVE_SEMANTICS

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
