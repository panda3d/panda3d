/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lightLensNode.I
 * @author drose
 * @date 2002-03-26
 */

/**
 * Returns true if this light defines a specular color, false if the specular
 * color is derived automatically from the light color.
 */
INLINE bool LightLensNode::
has_specular_color() const {
  return _has_specular_color;
}

/**
 * Returns whether this light is configured to cast shadows or not.
 */
INLINE bool LightLensNode::
is_shadow_caster() const {
  return _shadow_caster;
}

/**
 * Returns the sort of the shadow buffer to be created for this light source.
 */
INLINE int LightLensNode::
get_shadow_buffer_sort() const {
  return _sb_sort;
}

/**
 * Returns the size of the shadow buffer to be created for this light source.
 */
INLINE LVecBase2i LightLensNode::
get_shadow_buffer_size() const {
  return _sb_size;
}

/**
 * Sets the size of the shadow buffer to be created for this light source.
 */
INLINE void LightLensNode::
set_shadow_buffer_size(const LVecBase2i &size) {
  if (size != _sb_size) {
    clear_shadow_buffers();
    _sb_size = size;
    setup_shadow_map();
  }
}

/**
 * Returns the buffer that has been constructed for a given GSG, or NULL if no
 * such buffer has (yet) been constructed.  This should be used for debugging
 * only, you will not need to call this normally.
 */
INLINE GraphicsOutputBase *LightLensNode::
get_shadow_buffer(GraphicsStateGuardianBase *gsg) {
  ShadowBuffers::iterator it = _sbuffers.find(gsg);
  if (it == _sbuffers.end()) {
    return nullptr;
  } else {
    return (*it).second;
  }
}

/**
 * Marks this light as having been used by the auto shader.
 */
INLINE void LightLensNode::
mark_used_by_auto_shader() const {
  _used_by_auto_shader = true;
}
