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
 *
 */
INLINE VulkanShaderContext::
~VulkanShaderContext() {
  clear_pipeline_cache();
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
  return _packed_state == other._packed_state
      && _fb_config == other._fb_config
      && _format == other._format
      && _render_mode_attrib == other._render_mode_attrib
      && _depth_test_mode == other._depth_test_mode
      && _color_blend_attrib == other._color_blend_attrib
      && _alpha_test_attrib == other._alpha_test_attrib
      && _depth_bias_attrib == other._depth_bias_attrib;
}

/**
 * Defines relative sorting order of PipelineKey objects.
 */
INLINE bool VulkanShaderContext::PipelineKey::
operator < (const PipelineKey &other) const {
  if (_packed_state != other._packed_state) {
    return _packed_state < other._packed_state;
  }
  if (_fb_config != other._fb_config) {
    return _fb_config < other._fb_config;
  }
  if (_format != other._format) {
    return _format < other._format;
  }
  if (_render_mode_attrib != other._render_mode_attrib) {
    return _render_mode_attrib < other._render_mode_attrib;
  }
  if (_depth_test_mode != other._depth_test_mode) {
    return _depth_test_mode < other._depth_test_mode;
  }
  if (_color_blend_attrib != other._color_blend_attrib) {
    return _color_blend_attrib < other._color_blend_attrib;
  }
  if (_alpha_test_attrib != other._alpha_test_attrib) {
    return _alpha_test_attrib < other._alpha_test_attrib;
  }
  if (_depth_bias_attrib != other._depth_bias_attrib) {
    return _depth_bias_attrib < other._depth_bias_attrib;
  }
  return 0;
}

/**
 *
 */
ALWAYS_INLINE VkPrimitiveTopology VulkanShaderContext::PipelineKey::
get_topology() const {
  return (VkPrimitiveTopology)((_packed_state & PS_topology_mask) >> PS_topology_shift);
}

/**
 *
 */
ALWAYS_INLINE ColorAttrib::Type VulkanShaderContext::PipelineKey::
get_color_type() const {
  return (ColorAttrib::Type)((_packed_state & PS_color_type_mask) >> PS_color_type_shift);
}

/**
 *
 */
ALWAYS_INLINE unsigned int VulkanShaderContext::PipelineKey::
get_color_write_channels() const {
  return (unsigned int)((_packed_state & PS_color_write_channels_mask) >> PS_color_write_channels_shift);
}

/**
 *
 */
ALWAYS_INLINE TransparencyAttrib::Mode VulkanShaderContext::PipelineKey::
get_transparency_mode() const {
  return (TransparencyAttrib::Mode)((_packed_state & PS_transparency_mode_mask) >> PS_transparency_mode_shift);
}

/**
 *
 */
ALWAYS_INLINE DepthWriteAttrib::Mode VulkanShaderContext::PipelineKey::
get_depth_write_mode() const {
  return (DepthWriteAttrib::Mode)((_packed_state & PS_depth_write_mask) >> PS_depth_write_shift);
}

/**
 *
 */
ALWAYS_INLINE CullFaceAttrib::Mode VulkanShaderContext::PipelineKey::
get_cull_face_mode() const {
  return (CullFaceAttrib::Mode)((_packed_state & PS_cull_face_mode_mask) >> PS_cull_face_mode_shift);
}

/**
 *
 */
ALWAYS_INLINE LogicOpAttrib::Operation VulkanShaderContext::PipelineKey::
get_logic_op() const {
  return (LogicOpAttrib::Operation)((_packed_state & PS_logic_op_mask) >> PS_logic_op_shift);
}

/**
 *
 */
ALWAYS_INLINE uint32_t VulkanShaderContext::PipelineKey::
get_patch_control_points() const {
  return ((_packed_state & PS_patch_control_points_mask) >> PS_patch_control_points_shift) + 1u;
}

/**
 *
 */
ALWAYS_INLINE bool VulkanShaderContext::PipelineKey::
is_instanced() const {
  return (_packed_state & PS_instanced_mask) != 0u;
}
