/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animControl.cxx
 * @author drose
 * @date 1999-02-19
 */

#include "animControl.h"
#include "animChannelBase.h"
#include "partBundle.h"
#include "config_chan.h"
#include "dcast.h"
#include "mutexHolder.h"
#include "throw_event.h"

TypeHandle AnimControl::_type_handle;

/**
 * This constructor is used to create a temporarily uninitialized AnimControl
 * that will serve as a placeholder for an animation while the animation is
 * being loaded during an asynchronous load-and-bind operation.
 */
AnimControl::
AnimControl(const std::string &name, PartBundle *part,
            double frame_rate, int num_frames) :
  Namable(name),
  _pending_lock(name),
  _pending_cvar(_pending_lock),
  _bound_joints(BitArray::all_on()),
  _part(part)
{
#ifdef DO_MEMORY_USAGE
  MemoryUsage::update_type(this, get_class_type());
#endif

  _pending = true;
  _anim = nullptr;
  _channel_index = -1;
  set_frame_rate(frame_rate);
  set_num_frames(num_frames);

  _marked_frame = -1;
}

/**
 * This can only be called once for a given AnimControl.  It is used to supply
 * the AnimBundle and related information.
 */
void AnimControl::
setup_anim(PartBundle *part, AnimBundle *anim, int channel_index,
           const BitArray &bound_joints) {
  MutexHolder holder(_pending_lock);
  nassertv(_pending && part == _part);
  nassertv(_anim == nullptr);
  _anim = anim;
  _channel_index = channel_index;
  _bound_joints = bound_joints;
  set_frame_rate(_anim->get_base_frame_rate());
  set_num_frames(_anim->get_num_frames());

  // Now the AnimControl is fully set up.
  _marked_frame = -1;
  _pending = false;
  _pending_cvar.notify_all();
  if (!_pending_done_event.empty()) {
    throw_event(_pending_done_event);
  }
}

/**
 * Called to initialize the AnimControl with its array of bound_joints, before
 * setup_anim() has completed.
 */
void AnimControl::
set_bound_joints(const BitArray &bound_joints) {
  MutexHolder holder(_pending_lock);
  _bound_joints = bound_joints;
}

/**
 * This can only be called once for a given AnimControl.  It indicates the
 * attempt to bind it asynchronously has failed.
 */
void AnimControl::
fail_anim(PartBundle *part) {
  MutexHolder holder(_pending_lock);
  nassertv(_pending && part == _part);
  _pending = false;
  _pending_cvar.notify_all();
  if (!_pending_done_event.empty()) {
    throw_event(_pending_done_event);
  }
}

/**
 *
 */
AnimControl::
~AnimControl() {
  get_part()->control_removed(this);
}

/**
 * Blocks the current thread until the AnimControl has finished loading and is
 * fully bound.
 */
void AnimControl::
wait_pending() {
  MutexHolder holder(_pending_lock);
  if (_pending) {
    // TODO: we should elevate the priority of the associated BindAnimRequest
    // while we're waiting for it, so it will jump to the front of the queue.
    chan_cat.info()
      << "Blocking " << *Thread::get_current_thread()
      << " until " << get_name() << " is bound\n";
    while (_pending) {
      _pending_cvar.wait();
    }
  }
}

/**
 * Specifies an event name that will be thrown when the AnimControl is
 * finished binding asynchronously.  If the AnimControl has already finished
 * binding, the event will be thrown immediately.
 */
void AnimControl::
set_pending_done_event(const std::string &done_event) {
  MutexHolder holder(_pending_lock);
  _pending_done_event = done_event;
  if (!_pending) {
    throw_event(_pending_done_event);
  }
}

/**
 * Returns the event name that will be thrown when the AnimControl is finished
 * binding asynchronously.
 */
std::string AnimControl::
get_pending_done_event() const {
  MutexHolder holder(_pending_lock);
  return _pending_done_event;
}

/**
 * Returns the PartBundle bound in with this AnimControl.
 */
PartBundle *AnimControl::
get_part() const {
  return DCAST(PartBundle, _part);
}

/**
 *
 */
void AnimControl::
output(std::ostream &out) const {
  out << "AnimControl(" << get_name() << ", " << get_part()->get_name()
      << ": ";
  AnimInterface::output(out);
  out << ")";

  if (is_pending()) {
    out << " (pending bind)";
  } else if (!has_anim()) {
    out << " (failed bind)";
  }
}

/**
 * Returns true if the indicated channel value has changed since the last call
 * to mark_channels().
 */
bool AnimControl::
channel_has_changed(AnimChannelBase *channel, bool frame_blend_flag) const {
  if (_marked_frame < 0) {
    return true;
  }

  int this_frame = get_frame();
  double this_frac = 0.0;
  if (frame_blend_flag) {
    this_frac = get_frac();
  }
  return channel->has_changed(_marked_frame, _marked_frac,
                              this_frame, this_frac);
}

/**
 * Marks this point as the point of reference for the next call to
 * channel_has_changed().
 */
void AnimControl::
mark_channels(bool frame_blend_flag) {
  _marked_frame = get_frame();
  _marked_frac = 0.0;
  if (frame_blend_flag) {
    _marked_frac = get_frac();
  }
}

/**
 * This is provided as a callback method for when the user calls one of the
 * play/loop/pose type methods to start the animation playing.
 */
void AnimControl::
animation_activated() {
  get_part()->control_activated(this);
}
