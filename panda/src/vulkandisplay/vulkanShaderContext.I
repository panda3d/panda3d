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

  _matrix_cache = pvector<LMatrix4>(shader->_matrix_cache_desc.size(), LMatrix4::ident_mat());
  _matrix_cache_deps = shader->_matrix_cache_deps;
}

/**
 *
 */
INLINE VulkanShaderContext::
~VulkanShaderContext()  {
}

/**
 * Returns the module for the given stage, or VK_NULL_HANDLE.
 */
INLINE VkShaderModule VulkanShaderContext::
get_module(Shader::Stage stage) const {
  return _modules[(size_t)stage];
}

/**
 * Returns true if these two PipelineKey objects are identical.
 */
INLINE bool VulkanShaderContext::PipelineKey::
operator ==(const PipelineKey &other) const {
  return _format == other._format
      && _topology == other._topology
      && _patch_control_points == other._patch_control_points
      && _multisamples == other._multisamples
      && _color_type == other._color_type
      && _render_mode_attrib == other._render_mode_attrib
      && _cull_face_mode == other._cull_face_mode
      && _depth_write_mode == other._depth_write_mode
      && _depth_test_mode == other._depth_test_mode
      && _color_write_mask == other._color_write_mask
      && _logic_op == other._logic_op
      && _color_blend_attrib == other._color_blend_attrib
      && _transparency_mode == other._transparency_mode
      && _alpha_test_attrib == other._alpha_test_attrib;
}

/**
 * Defines relative sorting order of PipelineKey objects.
 */
INLINE bool VulkanShaderContext::PipelineKey::
operator < (const PipelineKey &other) const {
  if (_format != other._format) {
    return _format < other._format;
  }
  if (_topology != other._topology) {
    return _topology < other._topology;
  }
  if (_patch_control_points != other._patch_control_points) {
    return _patch_control_points < other._patch_control_points;
  }
  if (_multisamples != other._multisamples) {
    return _multisamples < other._multisamples;
  }
  if (_color_type != other._color_type) {
    return _color_type < other._color_type;
  }
  if (_render_mode_attrib != other._render_mode_attrib) {
    return _render_mode_attrib < other._render_mode_attrib;
  }
  if (_cull_face_mode != other._cull_face_mode) {
    return _cull_face_mode < other._cull_face_mode;
  }
  if (_depth_write_mode != other._depth_write_mode) {
    return _depth_write_mode < other._depth_write_mode;
  }
  if (_depth_test_mode != other._depth_test_mode) {
    return _depth_test_mode < other._depth_test_mode;
  }
  if (_color_write_mask != other._color_write_mask) {
    return _color_write_mask < other._color_write_mask;
  }
  if (_logic_op != other._logic_op) {
    return _logic_op < other._logic_op;
  }
  if (_color_blend_attrib != other._color_blend_attrib) {
    return _color_blend_attrib < other._color_blend_attrib;
  }
  if (_transparency_mode != other._transparency_mode) {
    return _transparency_mode < other._transparency_mode;
  }
  if (_alpha_test_attrib != other._alpha_test_attrib) {
    return _alpha_test_attrib < other._alpha_test_attrib;
  }
  return 0;
}
