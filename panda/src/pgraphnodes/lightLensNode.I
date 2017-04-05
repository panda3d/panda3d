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
 * Returns whether this light is configured to cast shadows or not.
 */
INLINE bool LightLensNode::
is_shadow_caster() const {
  return _shadow_caster;
}

/**
 * Sets the flag indicating whether this light should cast shadows or not.
 * This is the variant without buffer size, meaning that the current buffer
 * size will be kept (512x512 is the default). Note that enabling shadows will
 * require the shader generator to be enabled on the scene.
 */
INLINE void LightLensNode::
set_shadow_caster(bool caster) {
  if (_shadow_caster && !caster) {
    clear_shadow_buffers();
  }
  _shadow_caster = caster;
  set_active(caster);
}

/**
 * Sets the flag indicating whether this light should cast shadows or not.
 * The xsize and ysize parameters specify the size of the shadow buffer that
 * will be set up, the sort parameter specifies the sort.  Note that enabling
 * shadows will require the shader generator to be enabled on the scene.
 */
INLINE void LightLensNode::
set_shadow_caster(bool caster, int buffer_xsize, int buffer_ysize, int buffer_sort) {
  if ((_shadow_caster && !caster) || buffer_xsize != _sb_size[0] || buffer_ysize != _sb_size[1]) {
    clear_shadow_buffers();
  }
  _shadow_caster = caster;
  _sb_size.set(buffer_xsize, buffer_ysize);

  if (buffer_sort != _sb_sort) {
    ShadowBuffers::iterator it;
    for(it = _sbuffers.begin(); it != _sbuffers.end(); ++it) {
      (*it).second->set_sort(buffer_sort);
    }
    _sb_sort = buffer_sort;
  }
  set_active(caster);
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
  }
  _sb_size = size;
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
    return NULL;
  } else {
    return (*it).second;
  }
}
