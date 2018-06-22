/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animControlCollection.I
 * @author drose
 * @date 2000-02-22
 */

/**
 * Starts the named animation playing.
 */
INLINE bool AnimControlCollection::
play(const std::string &anim_name) {
  AnimControl *control = find_anim(anim_name);
  if (control == nullptr) {
    return false;
  }
  _last_started_control = control;
  control->play();
  return true;
}

/**
 * Starts the named animation playing.
 */
INLINE bool AnimControlCollection::
play(const std::string &anim_name, double from, double to) {
  AnimControl *control = find_anim(anim_name);
  if (control == nullptr) {
    return false;
  }
  _last_started_control = control;
  control->play(from, to);
  return true;
}

/**
 * Starts the named animation looping.
 */
INLINE bool AnimControlCollection::
loop(const std::string &anim_name, bool restart) {
  AnimControl *control = find_anim(anim_name);
  if (control == nullptr) {
    return false;
  }
  _last_started_control = control;
  control->loop(restart);
  return true;
}

/**
 * Starts the named animation looping.
 */
INLINE bool AnimControlCollection::
loop(const std::string &anim_name, bool restart, double from, double to) {
  AnimControl *control = find_anim(anim_name);
  if (control == nullptr) {
    return false;
  }
  _last_started_control = control;
  control->loop(restart, from, to);
  return true;
}

/**
 * Stops the named animation.
 */
INLINE bool AnimControlCollection::
stop(const std::string &anim_name) {
  AnimControl *control = find_anim(anim_name);
  if (control == nullptr) {
    return false;
  }
  control->stop();
  return true;
}


/**
 * Sets to a particular frame in the named animation.
 */
INLINE bool AnimControlCollection::
pose(const std::string &anim_name, double frame) {
  AnimControl *control = find_anim(anim_name);
  if (control == nullptr) {
    return false;
  }
  _last_started_control = control;
  control->pose(frame);
  return true;
}

/**
 * Returns the current frame in the named animation, or 0 if the animation is
 * not found.
 */
INLINE int AnimControlCollection::
get_frame(const std::string &anim_name) const {
  AnimControl *control = find_anim(anim_name);
  if (control == nullptr) {
    return 0;
  }
  return control->get_frame();
}

/**
 * Returns the current frame in the last-started animation.
 */
INLINE int AnimControlCollection::
get_frame() const {
  if (_last_started_control == nullptr) {
    return 0;
  }
  return _last_started_control->get_frame();
}

/**
 * Returns true if the named animation is currently playing, false otherwise.
 */
INLINE bool AnimControlCollection::
is_playing(const std::string &anim_name) const {
  AnimControl *control = find_anim(anim_name);
  if (control == nullptr) {
    return false;
  }
  return control->is_playing();
}

/**
 * Returns true if the last-started animation is currently playing, false
 * otherwise.
 */
INLINE bool AnimControlCollection::
is_playing() const {
  if (_last_started_control == nullptr) {
    return false;
  }
  return _last_started_control->is_playing();
}

/**
 * Returns the total number of frames in the named animation, or 0 if the
 * animation is not found.
 */
INLINE int AnimControlCollection::
get_num_frames(const std::string &anim_name) const {
  AnimControl *control = find_anim(anim_name);
  if (control == nullptr) {
    return 0;
  }
  return control->get_num_frames();
}

/**
 * Returns the total number of frames in the last-started animation.
 */
INLINE int AnimControlCollection::
get_num_frames() const {
  if (_last_started_control == nullptr) {
    return 0;
  }
  return _last_started_control->get_num_frames();
}

INLINE std::ostream &
operator << (std::ostream &out, const AnimControlCollection &collection) {
  collection.output(out);
  return out;
}
