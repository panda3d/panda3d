/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animInterface.I
 * @author drose
 * @date 2005-09-20
 */

/**
 * Runs the entire animation from beginning to end and stops.
 */
INLINE void AnimInterface::
play() {
  play(0, get_num_frames() - 1);
}

/**
 * Runs the animation from the frame "from" to and including the frame "to",
 * at which point the animation is stopped.  Both "from" and "to" frame
 * numbers may be outside the range (0, get_num_frames()) and the animation
 * will follow the range correctly, reporting numbers modulo get_num_frames().
 * For instance, play(0, get_num_frames() * 2) will play the animation twice
 * and then stop.
 */
INLINE void AnimInterface::
play(double from, double to) {
  {
    CDWriter cdata(_cycler);
    cdata->play(from, to);
  }
  animation_activated();
}

/**
 * Starts the entire animation looping.  If restart is true, the animation is
 * restarted from the beginning; otherwise, it continues from the current
 * frame.
 */
INLINE void AnimInterface::
loop(bool restart) {
  loop(restart, 0, get_num_frames() - 1);
}

/**
 * Loops the animation from the frame "from" to and including the frame "to",
 * indefinitely.  If restart is true, the animation is restarted from the
 * beginning; otherwise, it continues from the current frame.
 */
INLINE void AnimInterface::
loop(bool restart, double from, double to) {
  {
    CDWriter cdata(_cycler);
    cdata->loop(restart, from, to);
  }
  animation_activated();
}

/**
 * Starts the entire animation bouncing back and forth between its first frame
 * and last frame.  If restart is true, the animation is restarted from the
 * beginning; otherwise, it continues from the current frame.
 */
INLINE void AnimInterface::
pingpong(bool restart) {
  pingpong(restart, 0, get_num_frames() - 1);
}

/**
 * Loops the animation from the frame "from" to and including the frame "to",
 * and then back in the opposite direction, indefinitely.
 */
INLINE void AnimInterface::
pingpong(bool restart, double from, double to) {
  {
    CDWriter cdata(_cycler);
    cdata->pingpong(restart, from, to);
  }
  animation_activated();
}

/**
 * Stops a currently playing or looping animation right where it is.  The
 * animation remains posed at the current frame.
 */
INLINE void AnimInterface::
stop() {
  CDWriter cdata(_cycler);
  cdata->pose(cdata->get_full_fframe());

  // Don't call animation_activated() here; stopping an animation should not
  // activate it.
}

/**
 * Sets the animation to the indicated frame and holds it there.
 */
INLINE void AnimInterface::
pose(double frame) {
  {
    CDWriter cdata(_cycler);
    cdata->pose(frame);
  }
  animation_activated();
}

/**
 * Changes the rate at which the animation plays.  1.0 is the normal speed,
 * 2.0 is twice normal speed, and 0.5 is half normal speed.  0.0 is legal to
 * pause the animation, and a negative value will play the animation
 * backwards.
 */
INLINE void AnimInterface::
set_play_rate(double play_rate) {
  CDWriter cdata(_cycler);
  cdata->internal_set_rate(cdata->_frame_rate, play_rate);
}

/**
 * Returns the rate at which the animation plays.  See set_play_rate().
 */
INLINE double AnimInterface::
get_play_rate() const {
  CDReader cdata(_cycler);
  return cdata->_play_rate;
}

/**
 * Returns the native frame rate of the animation.  This is the number of
 * frames per second that will elapse when the play_rate is set to 1.0.  It is
 * a fixed property of the animation and may not be adjusted by the user.
 */
INLINE double AnimInterface::
get_frame_rate() const {
  CDReader cdata(_cycler);
  return cdata->_frame_rate;
}

/**
 * Returns the current integer frame number.  This number will be in the range
 * 0 <= f < get_num_frames().
 */
INLINE int AnimInterface::
get_frame() const {
  int num_frames = get_num_frames();
  if (num_frames <= 0) {
    return 0;
  }
  CDReader cdata(_cycler);
  return cmod(cdata->get_full_frame(0), num_frames);
}

/**
 * Returns the current integer frame number + 1, constrained to the range 0 <=
 * f < get_num_frames().
 *
 * If the play mode is PM_play, this will clamp to the same value as
 * get_frame() at the end of the animation.  If the play mode is any other
 * value, this will wrap around to frame 0 at the end of the animation.
 */
INLINE int AnimInterface::
get_next_frame() const {
  int num_frames = get_num_frames();
  if (num_frames <= 0) {
    return 0;
  }
  CDReader cdata(_cycler);
  return cmod(cdata->get_full_frame(1), num_frames);
}

/**
 * Returns the fractional part of the current frame.  Normally, this is in the
 * range 0.0 <= f < 1.0, but in the one special case of an animation playing
 * to its end frame and stopping, it might exactly equal 1.0.
 *
 * It will always be true that get_full_frame() + get_frac() ==
 * get_full_fframe().
 */
INLINE double AnimInterface::
get_frac() const {
  CDReader cdata(_cycler);
  return cdata->get_frac();
}

/**
 * Returns the current integer frame number.
 *
 * Unlike the value returned by get_frame(), this frame number may extend
 * beyond the range of get_num_frames() if the frame range passed to play(),
 * loop(), etc.  did.
 *
 * Unlike the value returned by get_full_fframe(), this return value will
 * never exceed the value passed to to_frame in the play() method.
 */
INLINE int AnimInterface::
get_full_frame() const {
  CDReader cdata(_cycler);
  return cdata->get_full_frame(0);
}

/**
 * Returns the current floating-point frame number.
 *
 * Unlike the value returned by get_frame(), this frame number may extend
 * beyond the range of get_num_frames() if the frame range passed to play(),
 * loop(), etc.  did.
 *
 * Unlike the value returned by get_full_frame(), this return value may equal
 * (to_frame + 1.0), when the animation has played to its natural end.
 * However, in this case the return value of get_full_frame() will be
 * to_frame, not (to_frame + 1).
 */
INLINE double AnimInterface::
get_full_fframe() const {
  CDReader cdata(_cycler);
  return cdata->get_full_fframe();
}

/**
 * Returns true if the animation is currently playing, false if it is stopped
 * (e.g.  because stop() or pose() was called, or because it reached the end
 * of the animation after play() was called).
 */
INLINE bool AnimInterface::
is_playing() const {
  CDReader cdata(_cycler);
  return cdata->is_playing();
}

/**
 * Should be called by a derived class to specify the native frame rate of the
 * animation.  It is legal to call this after the animation has already
 * started.
 */
INLINE void AnimInterface::
set_frame_rate(double frame_rate) {
  CDWriter cdata(_cycler);
  cdata->internal_set_rate(frame_rate, cdata->_play_rate);
}

/**
 * Should be called by a derived class to specify the number of frames of the
 * animation.  It is legal to call this after the animation has already
 * started, but doing so may suddenly change the apparent current frame
 * number.
 */
INLINE void AnimInterface::
set_num_frames(int num_frames) {
  _num_frames = num_frames;
}

/**
 * Returns the fractional part of the current frame.  Normally, this is in the
 * range 0.0 <= f < 1.0, but in the one special case of an animation playing
 * to its end frame and stopping, it might exactly equal 1.0.
 *
 * It will always be true that get_full_frame() + get_frac() ==
 * get_full_fframe().
 */
INLINE double AnimInterface::CData::
get_frac() const {
  return get_full_fframe() - (double)get_full_frame(0);
}

INLINE std::ostream &
operator << (std::ostream &out, const AnimInterface &ai) {
  ai.output(out);
  return out;
}
