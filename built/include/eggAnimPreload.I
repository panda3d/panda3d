/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggAnimPreload.I
 * @author drose
 * @date 2008-08-06
 */

/**
 *
 */
INLINE EggAnimPreload::
EggAnimPreload(const std::string &name) : EggNode(name) {
  _has_fps = false;
  _has_num_frames = false;
}


/**
 *
 */
INLINE EggAnimPreload::
EggAnimPreload(const EggAnimPreload &copy) :
  EggNode(copy),
  _fps(copy._fps),
  _has_fps(copy._has_fps),
  _num_frames(copy._num_frames),
  _has_num_frames(copy._has_num_frames)
{
}


/**
 *
 */
INLINE EggAnimPreload &EggAnimPreload::
operator = (const EggAnimPreload &copy) {
  EggNode::operator = (copy);
  _fps = copy._fps;
  _has_fps = copy._has_fps;
  _num_frames = copy._num_frames;
  _has_num_frames = copy._has_num_frames;

  return *this;
}


/**
 *
 */
INLINE void EggAnimPreload::
set_fps(double fps) {
  _fps = fps;
  _has_fps = true;
}


/**
 *
 */
INLINE void EggAnimPreload::
clear_fps() {
  _has_fps = false;
}

/**
 *
 */
INLINE bool EggAnimPreload::
has_fps() const {
  return _has_fps;
}

/**
 * This is only valid if has_fps() returns true.
 */
INLINE double EggAnimPreload::
get_fps() const {
  nassertr(has_fps(), 0.0);
  return _fps;
}

/**
 *
 */
INLINE void EggAnimPreload::
set_num_frames(int num_frames) {
  _num_frames = num_frames;
  _has_num_frames = true;
}


/**
 *
 */
INLINE void EggAnimPreload::
clear_num_frames() {
  _has_num_frames = false;
}

/**
 *
 */
INLINE bool EggAnimPreload::
has_num_frames() const {
  return _has_num_frames;
}

/**
 * This is only valid if has_num_frames() returns true.
 */
INLINE int EggAnimPreload::
get_num_frames() const {
  nassertr(has_num_frames(), 0);
  return _num_frames;
}
