// Filename: animControl.cxx
// Created by:  drose (19Feb99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////


#include "animControl.h"
#include "animChannelBase.h"
#include "partBundle.h"
#include "config_chan.h"

#include <event.h>
#include <throw_event.h>


TypeHandle AnimControl::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: AnimControl::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
AnimControl::
AnimControl(PartBundle *part, AnimBundle *anim, int channel_index) {
#ifdef DO_MEMORY_USAGE
  MemoryUsage::update_type(this, get_class_type());
#endif

  _part = part;
  _anim = anim;
  _channel_index = channel_index;

  _play_rate = 1.0f;
  _frame = 0.0f;
  _as_of_time = 0.0f;
  _playing = false;

  _marked_frame = -1;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControl::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
AnimControl::
~AnimControl() {
  get_part()->set_control_effect(this, 0.0f);
}


////////////////////////////////////////////////////////////////////
//     Function: AnimControl::play
//       Access: Published
//  Description: Runs the entire animation from beginning to end and
//               stops, throwing the stop event (if it is non-NULL).
////////////////////////////////////////////////////////////////////
void AnimControl::
play(const CPT_Event &stop_event) {
  nassertv(get_num_frames() > 0);

  if (get_play_rate() < 0.0f) {
    play(get_num_frames()-1, 0, stop_event);
  } else {
    play(0, get_num_frames()-1, stop_event);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControl::play
//       Access: Published
//  Description: Runs the animation from the frame "from" to and
//               including the frame "to", at which point the
//               animation is stopped and the indicated stop event is
//               thrown (if it is non-NULL).  If the to frame is less
//               than the from frame (unless play_rate is negative),
//               the animation will wrap around the end and begins
//               again at the beginning before the animation stops.
////////////////////////////////////////////////////////////////////
void AnimControl::
play(int from, int to, const CPT_Event &stop_event) {
  nassertv(get_num_frames() > 0);

  nassertv(from >= 0 && from < get_num_frames());
  nassertv(to >= 0 && to < get_num_frames());
  _as_of_time = ClockObject::get_global_clock()->get_frame_time();
  _playing = true;

  _actions = _user_actions;
  if (stop_event != (Event*)0L) {
    insert_event_action(_actions, to, stop_event);
  }
  insert_stop_action(_actions, to);

  get_part()->control_activated(this);
  set_frame(from);
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControl::loop
//       Access: Published
//  Description: Starts the entire animation looping.  If restart is
//               true, the animation is restarted from the beginning;
//               otherwise, it continues from the current frame.
////////////////////////////////////////////////////////////////////
void AnimControl::
loop(bool restart) {
  nassertv(get_num_frames() > 0);

  _as_of_time = ClockObject::get_global_clock()->get_frame_time();
  _playing = true;

  _actions = _user_actions;
  get_part()->control_activated(this);

  if (restart) {
    if (get_play_rate() < 0.0f) {
      set_frame(get_num_frames() - 1);
    } else {
      set_frame(0);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControl::loop
//       Access: Published
//  Description: Loops the animation from the frame "from" to and
//               including the frame "to", indefinitely.  If restart
//               is true, the animation is restarted from the
//               beginning; otherwise, it continues from the current
//               frame.
////////////////////////////////////////////////////////////////////
void AnimControl::
loop(bool restart, int from, int to) {
  nassertv(get_num_frames() > 0);

  nassertv(from >= 0 && from < get_num_frames());
  nassertv(to >= 0 && to < get_num_frames());
  _as_of_time = ClockObject::get_global_clock()->get_frame_time();
  _playing = true;

  _actions = _user_actions;

  if (get_play_rate() < 0.0f) {
    // If we're playing backward, we need to set up the loop a little
    // differently.

    if ((to == 0 && from == get_num_frames()-1) ||
        (to == from-1)) {

      // In this case, the user has specified to loop over the whole
      // range of animation.  We don't need a special jump action to
      // handle this.

    } else {
      // Otherwise, set up a jump action to effect the loop.  This isn't
      // completely accurate, since it will always jump exactly to the
      // first frame of the loop, no matter how many frames past the end
      // we'd gotten to, but it should be reasonably close, especially
      // if the number of frames in the loop is large enough and/or the
      // frame rate is high enough.
      insert_jump_action(_actions, (to-1+get_num_frames())%get_num_frames(),
                         from);
    }
  } else {

    if ((from == 0 && to == get_num_frames()-1) ||
        (from == to-1)) {

      // In this case, the user has specified to loop over the whole
      // range of animation.  We don't need a special jump action to
      // handle this.

    } else {
      // Otherwise, set up a jump action to effect the loop.  This isn't
      // completely accurate, since it will always jump exactly to the
      // first frame of the loop, no matter how many frames past the end
      // we'd gotten to, but it should be reasonably close, especially
      // if the number of frames in the loop is large enough and/or the
      // frame rate is high enough.
      insert_jump_action(_actions, (to+1)%get_num_frames(), from);
    }
  }

  get_part()->control_activated(this);
  if (restart) {
    set_frame(from);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControl::pingpong
//       Access: Published
//  Description: Loops the animation from the frame "from" to and
//               including the frame "to", and then back in the
//               opposite direction, indefinitely.
////////////////////////////////////////////////////////////////////
void AnimControl::
pingpong(bool restart, int from, int to) {
  if (from == to) {
    pose(from);
    return;
  }

  nassertv(get_num_frames() > 0);

  nassertv(from >= 0 && from < get_num_frames());
  nassertv(to >= 0 && to < get_num_frames());
  _as_of_time = ClockObject::get_global_clock()->get_frame_time();
  _playing = true;

  _actions = _user_actions;

  insert_forward_action(_actions, from);
  insert_backward_action(_actions, to);

  get_part()->control_activated(this);
  if (restart) {
    set_frame(from);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControl::stop
//       Access: Published
//  Description: Stops a currently playing or looping animation right
//               where it is.  The animation remains posed at the
//               current frame, and no event is thrown.
////////////////////////////////////////////////////////////////////
void AnimControl::
stop() {
  _playing = false;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControl::pose
//       Access: Published
//  Description: Sets the animation to the indicated frame and holds
//               it there.
////////////////////////////////////////////////////////////////////
void AnimControl::
pose(int frame) {
  int num_frames = get_num_frames();
  nassertv(num_frames > 0);

  // Modulo the number of frames.
  frame = (int)(frame - cfloor(frame / num_frames) * num_frames);
  nassertv(frame >= 0 && frame < num_frames);
  _as_of_time = ClockObject::get_global_clock()->get_frame_time();
  _playing = false;

  _actions = _user_actions;

  get_part()->control_activated(this);
  set_frame(frame);
}


////////////////////////////////////////////////////////////////////
//     Function: AnimControl::add_event
//       Access: Published
//  Description: Adds the indicated event to the list of events that
//               will be called whenever the animation reaches the
//               indicated frame number.  Once added, the event will
//               persist until it is removed via remove_event() or
//               remove_all_events().
////////////////////////////////////////////////////////////////////
void AnimControl::
add_event(int frame, const CPT_Event &event) {
  insert_event_action(_user_actions, frame, event);
  if (_playing) {
    insert_event_action(_actions, frame, event);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControl::remove_event
//       Access: Published
//  Description: Removes all events found that match the indicated
//               event name, and returns the number of events
//               removed.
////////////////////////////////////////////////////////////////////
int AnimControl::
remove_event(const string &event_name) {
  Actions new_actions;

  int removed = 0;

  Actions::const_iterator ai;
  for (ai = _user_actions.begin(); ai != _user_actions.end(); ++ai) {
    const Action &action = (*ai).second;
    if (action._type == AT_event &&
        action._event->get_name() == event_name) {
      // Remove this event by not copying it to new_actions.
      removed++;
    } else {
      // Preserve this event.
      new_actions.insert(*ai);
    }
  }

  if (removed != 0) {
    _user_actions.swap(new_actions);

    if (_playing) {
      new_actions.clear();
      int p_removed = 0;
      for (ai = _actions.begin(); ai != _actions.end(); ++ai) {
        const Action &action = (*ai).second;
        if (action._type == AT_event &&
            action._event->get_name() == event_name) {
          // Remove this event by not copying it to new_actions.
          p_removed++;
        } else {
          // Preserve this event.
          new_actions.insert(*ai);
        }
      }
      nassertr(p_removed == removed, removed);
      _actions.swap(new_actions);
    }
  }

  return removed;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControl::remove_all_events
//       Access: Published
//  Description: Removes all user-defined event messages.  However, if
//               called while an animation is running, this will not
//               take effect until the animation is stopped and
//               restarted.
////////////////////////////////////////////////////////////////////
void AnimControl::
remove_all_events() {
  _user_actions.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControl::get_part
//       Access: Published
//  Description: Returns the PartBundle bound in with this
//               AnimControl.
////////////////////////////////////////////////////////////////////
PartBundle *AnimControl::
get_part() const {
  return DCAST(PartBundle, _part);
}


////////////////////////////////////////////////////////////////////
//     Function: AnimControl::advance_time
//       Access: Public
//  Description: Tells the AnimControl what time it is.  This
//               recomputes the frame number according to the amount
//               of time elapsed since last time.  Until this function
//               is called, the frame number will not increment.  The
//               time passed to this function must be nondecreasing;
//               it is an error to call it with a value less than a
//               previously-passed value.
////////////////////////////////////////////////////////////////////
void AnimControl::
advance_time(double time) {
  double elapsed_time = time - _as_of_time;
  double elapsed_frames = elapsed_time * get_frame_rate();
  _as_of_time = time;

  if (_playing && elapsed_frames != 0.0f) {
    int orig_frame = get_frame();

    _frame += elapsed_frames;
    double num_frames = (double)get_num_frames();

    int new_frame = get_frame();

    // Now call all the actions.
    if (elapsed_frames < 0.0f) {
      // If we're playing the animation backward, we have to check the
      // actions in reverse order.

      if (new_frame >= 0) {
        do_actions_backward(orig_frame-1, new_frame);
      } else {
        if (do_actions_backward(orig_frame-1, 0)) {
          // floor() is correct here, instead of simply an integer
          // cast, because we are using floating-point arithmetic
          // anyway (no need to convert to integer format and back
          // again), and because we need correct behavior when the
          // frame number is negative.
          _frame = _frame - cfloor(_frame / num_frames) * num_frames;
          new_frame = get_frame();
          do_actions_backward(get_num_frames(), new_frame);
        }
      }
    } else {
      // Normally, we'll be playing the animation forward.

      if (new_frame < get_num_frames()) {
        do_actions_forward(orig_frame+1, new_frame);
      } else {
        if (do_actions_forward(orig_frame+1, get_num_frames()-1)) {
          // floor() is correct here, instead of simply an integer
          // cast, because we are using floating-point arithmetic
          // anyway (no need to convert to integer format and back
          // again), and because we need correct behavior when the
          // frame number is negative.
          _frame = _frame - cfloor(_frame / num_frames) * num_frames;
          new_frame = get_frame();
          do_actions_forward(0, new_frame);
        }
      }
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: AnimControl::channel_has_changed
//       Access: Public
//  Description: Returns true if the indicated channel value has
//               changed since the last call to mark_channels().
////////////////////////////////////////////////////////////////////
bool AnimControl::
channel_has_changed(AnimChannelBase *channel) const {
  if (_marked_frame < 0) {
    return true;
  }

  int this_frame = get_frame();
  return channel->has_changed(_marked_frame, this_frame);
}


////////////////////////////////////////////////////////////////////
//     Function: AnimControl::mark_channels
//       Access: Public
//  Description: Marks this point as the point of reference for the
//               next call to channel_has_changed().
////////////////////////////////////////////////////////////////////
void AnimControl::
mark_channels() {
  _marked_frame = get_frame();
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControl::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void AnimControl::
output(ostream &out) const {
  out << "AnimControl(" << get_part()->get_name()
      << ", " << get_anim()->get_name() << ")";
}



////////////////////////////////////////////////////////////////////
//     Function: AnimControl::insert_event_action
//       Access: Private, Static
//  Description: Inserts an "event" action at the indicated frame
//               number.  The event will be thrown as the animation
//               reaches the indicated frame number.
////////////////////////////////////////////////////////////////////
void AnimControl::
insert_event_action(Actions &actions, int frame, const CPT_Event &event) {
  Action new_action;
  new_action._type = AT_event;
  new_action._event = event;
  actions.insert(pair<int, Action>(frame, new_action));
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControl::insert_stop_action
//       Access: Private, Static
//  Description: Inserts a "stop" action at the indicated frame
//               number.  The animation will stop as soon as it
//               reaches the indicated frame number, traveling from
//               either direction.
////////////////////////////////////////////////////////////////////
void AnimControl::
insert_stop_action(Actions &actions, int frame) {
  Action new_action;
  new_action._type = AT_stop;
  actions.insert(pair<int, Action>(frame, new_action));
}


////////////////////////////////////////////////////////////////////
//     Function: AnimControl::insert_jump_action
//       Access: Private, Static
//  Description: Inserts a "jump" action at the indicated frame
//               number.  When the animation reaches the indicated
//               frame number, it will jump to the indicated jump_to
//               frame.  It will not seem to spend any time at the
//               reached frame number.
////////////////////////////////////////////////////////////////////
void AnimControl::
insert_jump_action(Actions &actions, int frame, int jump_to) {
  Action new_action;
  new_action._type = AT_jump;
  new_action._jump_to = jump_to;
  actions.insert(pair<int, Action>(frame, new_action));
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControl::insert_forward_action
//       Access: Private, Static
//  Description: Inserts a "forward" action at the indicated frame
//               number.  When the animation hits this action while
//               playing in a backward direction, it will stop and
//               play in a forward direction instead.
////////////////////////////////////////////////////////////////////
void AnimControl::
insert_forward_action(Actions &actions, int frame) {
  Action new_action;
  new_action._type = AT_forward;
  actions.insert(pair<int, Action>(frame, new_action));
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControl::insert_backward_action
//       Access: Private, Static
//  Description: Inserts a "backward" action at the indicated frame
//               number.  When the animation hits this action while
//               playing in a forward direction, it will stop and
//               play in a backward direction instead.
////////////////////////////////////////////////////////////////////
void AnimControl::
insert_backward_action(Actions &actions, int frame) {
  Action new_action;
  new_action._type = AT_backward;
  actions.insert(pair<int, Action>(frame, new_action));
}


////////////////////////////////////////////////////////////////////
//     Function: AnimControl::set_frame
//       Access: Private
//  Description: Sets the current frame number to the indicated frame,
//               and performs any actions on that frame.
////////////////////////////////////////////////////////////////////
void AnimControl::
set_frame(double frame) {
  _frame = frame;
  int iframe = get_frame();
  do_actions_forward(iframe, iframe);
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControl::do_actions_forward
//       Access: Private
//  Description: Calls each of the actions, in turn, on the timeline
//               between the indicated "from" frame and the indicated
//               "to" frame, inclusive.  If any of the actions
//               specifies to stop the animation, the animation is
//               stopped, the current frame number is set to the point
//               of stopping, no further actions are called, and false
//               is returned.  Otherwise, true is returned.
////////////////////////////////////////////////////////////////////
bool AnimControl::
do_actions_forward(int from, int to) {
  if (to >= from) {
    Actions::const_iterator lower = _actions.lower_bound(from);
    Actions::const_iterator upper = _actions.lower_bound(to+1);

    int sequence_frame = -1;
    const Action *sequence_action;

    Actions::const_iterator ai;
    for (ai = lower; ai != upper; ++ai) {
      int frame = (*ai).first;
      const Action &action = (*ai).second;

      if (sequence_frame != -1 && frame > sequence_frame) {
        // We encountered an action that resequenced our frame numbers.
        // Now that we've finished evaluating all the other actions that
        // occurred in the same frame, evaluate this one action and
        // exit.
        do_sequence_action(sequence_frame, *sequence_action);
        return false;
      }

      do_action(frame, action, sequence_frame, sequence_action);
    }

    if (sequence_frame != -1) {
      do_sequence_action(sequence_frame, *sequence_action);
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControl::do_actions_backward
//       Access: Private
//  Description: Calls each of the actions, in turn, on the timeline
//               between the indicated "from" frame and the indicated
//               "to" frame, in reverse order.  If any of the actions
//               specifies to stop the animation, the animation is
//               stopped, the current frame number is set to the point
//               of stopping, no further actions are called, and false
//               is returned.  Otherwise, true is returned.
////////////////////////////////////////////////////////////////////
bool AnimControl::
do_actions_backward(int from, int to) {
  if (from >= to) {
#if defined(WIN32_VC) && !defined(NO_PCH)
    typedef Actions::const_reverse_iterator Action_reverse_iterator;
#else
    typedef reverse_iterator<Actions::const_iterator> Action_reverse_iterator;
#endif
    Action_reverse_iterator lower(_actions.upper_bound(from));
    Action_reverse_iterator upper(_actions.upper_bound(to-1));

    int sequence_frame = -1;
    const Action *sequence_action;

    Action_reverse_iterator ai;
    for (ai = lower; ai != upper; ++ai) {
      int frame = (*ai).first;
      const Action &action = (*ai).second;

      if (sequence_frame != -1 && frame < sequence_frame) {
        // We encountered an action that resequenced our frame numbers.
        // Now that we've finished evaluating all the other actions that
        // occurred in the same frame, evaluate this one action and
        // exit.
        do_sequence_action(sequence_frame, *sequence_action);
        return false;
      }

      do_action(frame, action, sequence_frame, sequence_action);
    }

    if (sequence_frame != -1) {
      do_sequence_action(sequence_frame, *sequence_action);
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControl::do_action
//       Access: Private
//  Description: Performs a single action.  If the action involves
//               some resequencing behavior--stopping, or jumping
//               around to a new frame or something--does nothing
//               immediately, but instead sets sequence_frame and
//               sequence_action to the current frame number and
//               action, so they may be executed later (after all the
//               other actions this frame have been executed).
////////////////////////////////////////////////////////////////////
void AnimControl::
do_action(int frame, const Action &action,
          int &sequence_frame, const Action *&sequence_action) {
  switch (action._type) {
  case AT_stop:
  case AT_jump:
    sequence_frame = frame;
    sequence_action = &action;
    break;

  case AT_forward:
    // We only see "forward" actions if we're currently playing
    // backwards.
    if (_play_rate < 0.0f) {
      sequence_frame = frame;
      sequence_action = &action;
    }
    break;

  case AT_backward:
    // We only see "backward" actions if we're currently playing
    // forwards.
    if (_play_rate > 0.0f) {
      sequence_frame = frame;
      sequence_action = &action;
    }
    break;

  case AT_event:
    throw_event(action._event);
    break;

  default:
    chan_cat.error() << "Invalid action!\n";
    abort();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControl::do_sequence_action
//       Access: Private
//  Description: Performs an action that was saved from do_action(),
//               above.  This action presumably does some mucking
//               around with the frame number or something, so we
//               needed to do all the other actions associated with
//               that frame first.
////////////////////////////////////////////////////////////////////
void AnimControl::
do_sequence_action(int frame, const Action &action) {
  switch (action._type) {
  case AT_stop:
    stop();
    break;

  case AT_jump:
    set_frame(action._jump_to);
    break;

  case AT_forward:
  case AT_backward:
    _play_rate = -_play_rate;
    set_frame(frame);
    break;

  case AT_event:
  default:
    chan_cat.error() << "Invalid sequence action!\n";
    abort();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControl::Action::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void AnimControl::Action::
output(ostream &out) const {
  switch (_type) {
  case AT_event:
    out << "event " << *_event;
    break;

  case AT_stop:
    out << "stop";
    break;

  case AT_jump:
    out << "jump to " << _jump_to;
    break;

  case AT_forward:
    out << "forward";
    break;

  case AT_backward:
    out << "backward";
    break;

  default:
    out << "**error**";
  }
}
