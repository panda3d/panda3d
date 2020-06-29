/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vulkanShaderContext.I
 * @author rdb
 * @date 2016-02-18
 */


/**
 * Constructs a shader context.  Follow this up with calls to fill in the
 * module array.
 */
INLINE VulkanShaderContext::
VulkanShaderContext(Shader *shader) :
  ShaderContext(shader),
  _modules{VK_NULL_HANDLE} {

  _mat_part_cache = new LMatrix4[_shader->cp_get_mat_cache_size()];
}

/**
 *
 */
INLINE VulkanShaderContext::
~VulkanShaderContext()  {
  delete[] _mat_part_cache;
}

/**
 * Returns true if these two PipelineKey objects are identical.
 */
INLINE bool VulkanShaderContext::PipelineKey::
operator ==(const PipelineKey &other) const {
  return _state == other._state
      && _format == other._format
      && _topology == other._topology
      && _multisamples == other._multisamples;
}

/**
 * Defines relative sorting order of PipelineKey objects.
 */
INLINE bool VulkanShaderContext::PipelineKey::
operator < (const PipelineKey &other) const {
  if (_state != other._state) {
    return _state < other._state;
  }
  if (_format != other._format) {
    return _format < other._format;
  }
  if (_topology != other._topology) {
    return _topology < other._topology;
  }
  return _multisamples < other._multisamples;
}
