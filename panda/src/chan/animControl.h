// Filename: animControl.h
// Created by:  drose (19Feb99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef ANIMCONTROL_H
#define ANIMCONTROL_H

#include "pandabase.h"

#include "animBundle.h"
#include "partGroup.h"

#include "clockObject.h"
#include "typedef.h"
#include "referenceCount.h"
#include "event.h"
#include "pt_Event.h"
#include "cmath.h"
#include <string>
#include "pmap.h"

class PartBundle;
class AnimChannelBase;

////////////////////////////////////////////////////////////////////
//       Class : AnimControl
// Description : Controls the timing of a character animation.  An
//               AnimControl object is created for each
//               character/bundle binding and manages the state of the
//               animation: whether started, stopped, or looping, and
//               the current frame number and play rate.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA AnimControl : public ReferenceCount {
public:
  AnimControl(PartBundle *part, AnimBundle *anim, int channel_index);

PUBLISHED:
  ~AnimControl();
  void play(const CPT_Event &stop_event = NULL);
  void play(int from, int to, const CPT_Event &stop_event = NULL);
  void loop(bool restart);
  void loop(bool restart, int from, int to);
  void pingpong(bool restart, int from, int to);
  void stop();
  void pose(int frame);

  void add_event(int frame, const CPT_Event &event);
  int remove_event(const string &event_name);
  void remove_all_events();

  INLINE void set_play_rate(double play_rate);
  INLINE double get_play_rate() const;
  INLINE double get_frame_rate() const;

  INLINE int get_frame() const;
  INLINE int get_num_frames() const;

  INLINE bool is_playing() const;

  PartBundle *get_part() const;
  INLINE AnimBundle *get_anim() const;

  void output(ostream &out) const;

public:
  // The following functions aren't really part of the public
  // interface; they're just public so we don't have to declare a
  // bunch of friends.

  void advance_time(double time);

  bool channel_has_changed(AnimChannelBase *channel) const;
  void mark_channels();

  INLINE int get_channel_index() const;

private:

  enum ActionType {
    AT_event,
    AT_stop,
    AT_jump,
    AT_forward,
    AT_backward,
  };

#ifdef WIN32_VC
public:
#endif

  class EXPCL_PANDA Action {
  public:
    ActionType _type;
    CPT_Event _event;
    int _jump_to;

    void output(ostream &out) const;
  };

private:
  typedef pmultimap<int, Action> Actions;

  static void insert_event_action(Actions &actions, int frame, const CPT_Event &event);
  static void insert_stop_action(Actions &actions, int frame);
  static void insert_jump_action(Actions &actions, int frame, int jump_to);
  static void insert_forward_action(Actions &actions, int frame);
  static void insert_backward_action(Actions &actions, int frame);

  void set_frame(double frame);

  bool do_actions_forward(int from, int to);
  bool do_actions_backward(int from, int to);

  void do_action(int frame, const Action &action,
                 int &sequence_frame, const Action *&sequence_action);
  void do_sequence_action(int frame, const Action &action);

  Actions _actions;
  Actions _user_actions;

  // This is a PT(PartGroup) instead of a PT(PartBundle), just because
  // we can't include partBundle.h for circular reasons.  But it
  // actually keeps a pointer to a PartBundle.
  PT(PartGroup) _part;
  PT(AnimBundle) _anim;
  int _channel_index;

  double _play_rate;

  // We keep a floating-point frame number.  This increments fluidly,
  // and records honest "between-frame" increments.  However, whenever
  // we report the current frame number, we just report the integer
  // portion.
  double _frame;
  double _as_of_time;

  bool _playing;

  // This is the frame number as of the last call to mark_channels().
  int _marked_frame;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ReferenceCount::init_type();
    register_type(_type_handle, "AnimControl",
                  ReferenceCount::get_class_type());
  }

private:
  static TypeHandle _type_handle;

friend inline ostream &operator << (ostream &, const AnimControl::Action &);
friend class AnimControl::Action;
};

inline ostream &operator << (ostream &out, const AnimControl &ac) {
  ac.output(out);
  return out;
}

inline ostream &operator << (ostream &out, const AnimControl::Action &action) {
  action.output(out);
  return out;
}

#include "animControl.I"

#endif
