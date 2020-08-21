/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animInterface.cxx
 * @author drose
 * @date 2005-09-20
 */

#include "animInterface.h"
#include "clockObject.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"

using std::max;
using std::min;

TypeHandle AnimInterface::_type_handle;

/**
 *
 */
AnimInterface::
AnimInterface() :
  _num_frames(0)
{
}

/**
 *
 */
AnimInterface::
AnimInterface(const AnimInterface &copy) :
  _num_frames(copy._num_frames),
  _cycler(copy._cycler)
{
}

/**
 *
 */
AnimInterface::
~AnimInterface() {
}

/**
 * Returns the number of frames in the animation.  This is a property of the
 * animation and may not be directly adjusted by the user (although it may
 * change without warning with certain kinds of animations, since this is a
 * virtual method that may be overridden).
 */
int AnimInterface::
get_num_frames() const {
  return _num_frames;
}

/**
 *
 */
void AnimInterface::
output(std::ostream &out) const {
  CDReader cdata(_cycler);
  cdata->output(out);
}

/**
 * This is provided as a callback method for when the user calls one of the
 * play/loop/pose type methods to start the animation playing.
 */
void AnimInterface::
animation_activated() {
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void AnimInterface::
write_datagram(BamWriter *manager, Datagram &dg) {
  dg.add_int32(_num_frames);
  manager->write_cdata(dg, _cycler);
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new AnimInterface.
 */
void AnimInterface::
fillin(DatagramIterator &scan, BamReader *manager) {
  _num_frames = scan.get_int32();
  manager->read_cdata(scan, _cycler);
}

/**
 *
 */
AnimInterface::CData::
CData() :
  _frame_rate(0.0),
  _play_mode(PM_pose),
  _start_time(0.0),
  _start_frame(0.0),
  _play_frames(0.0),
  _from_frame(0),
  _to_frame(0),
  _play_rate(1.0),
  _effective_frame_rate(0.0),
  _paused(true),
  _paused_f(0.0)
{
}

/**
 *
 */
AnimInterface::CData::
CData(const AnimInterface::CData &copy) :
  _frame_rate(copy._frame_rate),
  _play_mode(copy._play_mode),
  _start_time(copy._start_time),
  _start_frame(copy._start_frame),
  _play_frames(copy._play_frames),
  _from_frame(copy._from_frame),
  _to_frame(copy._to_frame),
  _play_rate(copy._play_rate),
  _effective_frame_rate(copy._effective_frame_rate),
  _paused(copy._paused),
  _paused_f(copy._paused_f)
{
}

/**
 *
 */
CycleData *AnimInterface::CData::
make_copy() const {
  return new CData(*this);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void AnimInterface::CData::
write_datagram(BamWriter *, Datagram &dg) const {
  dg.add_stdfloat(_frame_rate);
  dg.add_uint8(_play_mode);
  dg.add_stdfloat(_start_time);
  dg.add_stdfloat(_start_frame);
  dg.add_stdfloat(_play_frames);
  dg.add_int32(_from_frame);
  dg.add_int32(_to_frame);
  dg.add_stdfloat(_play_rate);
  dg.add_bool(_paused);
  dg.add_stdfloat(_paused_f);
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new AnimInterface.
 */
void AnimInterface::CData::
fillin(DatagramIterator &scan, BamReader *) {
  _frame_rate = scan.get_stdfloat();
  _play_mode = (PlayMode)scan.get_uint8();
  _start_time = scan.get_stdfloat();
  _start_frame = scan.get_stdfloat();
  _play_frames = scan.get_stdfloat();
  _from_frame = scan.get_int32();
  _to_frame = scan.get_int32();
  _play_rate = scan.get_stdfloat();
  _effective_frame_rate = _frame_rate * _play_rate;
  _paused = scan.get_bool();
  _paused_f = scan.get_stdfloat();
}

/**
 * Runs the animation from the frame "from" to and including the frame "to",
 * at which point the animation is stopped.  Both "from" and "to" frame
 * numbers may be outside the range (0, get_num_frames()) and the animation
 * will follow the range correctly, reporting numbers modulo get_num_frames().
 * For instance, play(0, get_num_frames() * 2) will play the animation twice
 * and then stop.
 */
void AnimInterface::CData::
play(double from, double to) {
  if (from >= to) {
    pose(from);
    return;
  }

  _play_mode = PM_play;
  _start_time = ClockObject::get_global_clock()->get_frame_time();
  _start_frame = from;
  _play_frames = to - from + 1.0;
  _from_frame = (int)floor(from);
  _to_frame = (int)floor(to);
  _paused_f = 0.0;

  if (_effective_frame_rate < 0.0) {
    // If we'll be playing backward, start at the end.
    _start_time -= _play_frames / _effective_frame_rate;
  }
}

/**
 * Loops the animation from the frame "from" to and including the frame "to",
 * indefinitely.  If restart is true, the animation is restarted from the
 * beginning; otherwise, it continues from the current frame.
 */
void AnimInterface::CData::
loop(bool restart, double from, double to) {
  if (from >= to) {
    pose(from);
    return;
  }

  double fframe = get_full_fframe();

  _play_mode = PM_loop;
  _start_time = ClockObject::get_global_clock()->get_frame_time();
  _start_frame = from;
  _play_frames = to - from + 1.0;
  _from_frame = (int)floor(from);
  _to_frame = (int)floor(to);
  _paused_f = 0.0;

  if (!restart) {
    fframe = min(max(fframe, from), to);
    if (_paused) {
      _paused_f = fframe - _start_frame;
    } else {
      _start_time -= (fframe - _start_frame) / _effective_frame_rate;
    }
  }
}

/**
 * Loops the animation from the frame "from" to and including the frame "to",
 * and then back in the opposite direction, indefinitely.
 */
void AnimInterface::CData::
pingpong(bool restart, double from, double to) {
  if (from >= to) {
    pose(from);
    return;
  }

  double fframe = get_full_fframe();

  _play_mode = PM_pingpong;
  _start_time = ClockObject::get_global_clock()->get_frame_time();
  _start_frame = from;
  _play_frames = to - from + 1.0;
  _from_frame = (int)floor(from);
  _to_frame = (int)floor(to);
  _paused_f = 0.0;

  if (!restart) {
    fframe = min(max(fframe, from), to);
    if (_paused) {
      _paused_f = fframe - _start_frame;
    } else {
      _start_time -= (fframe - _start_frame) / _effective_frame_rate;
    }
  }
}

/**
 * Sets the animation to the indicated frame and holds it there.
 */
void AnimInterface::CData::
pose(double frame) {
  _play_mode = PM_pose;
  _start_time = ClockObject::get_global_clock()->get_frame_time();
  _start_frame = frame;
  _play_frames = 0.0;
  _from_frame = (int)floor(frame);
  _to_frame = (int)floor(frame);
  _paused_f = 0.0;
}

/**
 * Returns the current integer frame number, plus the indicated increment.
 *
 * Unlike the value returned by get_frame(), this frame number may extend
 * beyond the range of get_num_frames() if the frame range passed to play(),
 * loop(), etc.  did.
 *
 * Unlike the value returned by get_full_fframe(), this return value will
 * never exceed the value passed to to_frame in the play() method.
 */
int AnimInterface::CData::
get_full_frame(int increment) const {
  int frame = (int)floor(get_full_fframe()) + increment;
  if (_play_mode == PM_play) {
    // In play mode, we never let the return value exceed (_from_frame,
    // _to_frame).
    frame = min(max(frame, _from_frame), _to_frame);
  }
  return frame;
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
double AnimInterface::CData::
get_full_fframe() const {
  switch (_play_mode) {
  case PM_pose:
    return _start_frame;

  case PM_play:
    return min(max(get_f(), 0.0), _play_frames) + _start_frame;

  case PM_loop:
    nassertr(_play_frames >= 0.0, 0.0);
    return cmod(get_f(), _play_frames) + _start_frame;

  case PM_pingpong:
    {
      nassertr(_play_frames >= 0.0, 0.0);
      double f = cmod(get_f(), _play_frames * 2.0);
      if (f > _play_frames) {
        return (_play_frames * 2.0 - f) + _start_frame;
      } else {
        return f + _start_frame;
      }
    }
  }

  return _start_frame;
}

/**
 * Returns true if the animation is currently playing, false if it is stopped
 * (e.g.  because stop() or pose() was called, or because it reached the end
 * of the animation after play() was called).
 */
bool AnimInterface::CData::
is_playing() const {
  switch (_play_mode) {
  case PM_pose:
    return false;

  case PM_play:
    if (_effective_frame_rate < 0.0) {
      // If we're playing backwards, check if we're at the beginning.
      return get_f() > 0;
   } else {
      return get_f() < _play_frames;
    }

  case PM_loop:
  case PM_pingpong:
    return true;
  }

  return false;
}

/**
 *
 */
void AnimInterface::CData::
output(std::ostream &out) const {
  switch (_play_mode) {
  case PM_pose:
    out << "pose, frame " << get_full_fframe();
    return;

  case PM_play:
    out << "play, frame " << get_full_fframe();
    return;

  case PM_loop:
    out << "loop, frame " << get_full_fframe();
    return;

  case PM_pingpong:
    out << "pingpong, frame " << get_full_fframe();
    return;
  }
}

/**
 * Called internally to adjust either or both of the frame_rate or play_rate
 * without changing the current frame number if the animation is already
 * playing.
 */
void AnimInterface::CData::
internal_set_rate(double frame_rate, double play_rate) {
  double f = get_f();

  _frame_rate = frame_rate;
  _play_rate = play_rate;
  _effective_frame_rate = frame_rate * play_rate;

  if (_effective_frame_rate == 0.0) {
    _paused_f = f;
    _paused = true;

  } else {
    // Compute a new _start_time that will keep f the same value with the new
    // play_rate.
    double new_elapsed = f / _effective_frame_rate;
    double now = ClockObject::get_global_clock()->get_frame_time();
    _start_time = now - new_elapsed;
    _paused = false;
  }
}

/**
 * Returns the current floating-point frame number, elapsed since
 * _start_frame.
 */
double AnimInterface::CData::
get_f() const {
  if (_paused) {
    return _paused_f;

  } else {
    double now = ClockObject::get_global_clock()->get_frame_time();
    double elapsed = now - _start_time;
    return (elapsed * _effective_frame_rate);
  }
}
