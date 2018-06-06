/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fadeLodNode.I
 * @author sshodhan
 * @date 2004-06-14
 */

/**
 * set the time taken to complete an LOD switch
 */
INLINE void FadeLODNode::
set_fade_time(PN_stdfloat t) {
  _fade_time = t;
}

/**
 * get the time taken to complete an LOD switch
 */
INLINE PN_stdfloat FadeLODNode::
get_fade_time() const {
  return _fade_time;
}


/**
 * Returns the cull bin that is assigned to the fading part of the geometry
 * during a transition.
 */
INLINE const std::string &FadeLODNode::
get_fade_bin_name() const {
  return _fade_bin_name;
}

/**
 * Returns the draw order that is assigned (along with the bin name) to the
 * fading part of the geometry during a transition.
 */
INLINE int FadeLODNode::
get_fade_bin_draw_order() const {
  return _fade_bin_draw_order;
}

/**
 * Returns the override value that is applied to the state changes necessary
 * to apply the fade effect.  This should be larger than any attrib overrides
 * on the fading geometry.
 */
INLINE int FadeLODNode::
get_fade_state_override() const {
  return _fade_state_override;
}
