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
