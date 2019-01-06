/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animBundle.I
 * @author drose
 * @date 1999-02-21
 */

/**
 *
 */
INLINE AnimBundle::
AnimBundle(const std::string &name, PN_stdfloat fps, int num_frames) : AnimGroup(name) {
  _fps = fps;
  _num_frames = num_frames;
  _root = this;
}

/**
 *
 */
INLINE AnimBundle::
AnimBundle() {
}

/**
 * Returns the ideal number of frames per second of the animation, when it is
 * running at normal speed.  This may not be the same as the actual playing
 * frame rate, as it might have been adjusted through set_play_rate() on the
 * AnimControl object.  See AnimControl::get_effective_frame_rate().
 */
INLINE double AnimBundle::
get_base_frame_rate() const {
  return _fps;
}

/**
 * Returns the number of frames of animation, or 0 if the animation has no
 * fixed number of frames.
 */
INLINE int AnimBundle::
get_num_frames() const {
  return _num_frames;
}
