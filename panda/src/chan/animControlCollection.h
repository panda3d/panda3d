// Filename: animControlCollection.h
// Created by:  drose (22Feb00)
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

#ifndef ANIMCONTROLCOLLECTION_H
#define ANIMCONTROLCOLLECTION_H

#include "pandabase.h"

#include "animControl.h"

#include "event.h"
#include "pt_Event.h"

#include "pmap.h"

////////////////////////////////////////////////////////////////////
//       Class : AnimControlCollection
// Description : This is a named collection of AnimControl pointers.
//               An AnimControl may be added to the collection by
//               name.  While an AnimControl is associated, its
//               reference count is maintained; associating a new
//               AnimControl with the same name will decrement the
//               previous control's reference count (and possibly
//               delete it, unbinding its animation).
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA AnimControlCollection {
PUBLISHED:
  AnimControlCollection();
  ~AnimControlCollection();

  void store_anim(AnimControl *control, const string &name);
  AnimControl *find_anim(const string &name) const;
  bool unbind_anim(const string &name);

  int get_num_anims() const;
  void clear_anims();

  INLINE void set_stop_event(const CPT_Event &stop_event);
  INLINE void clear_stop_event();
  INLINE bool has_stop_event() const;
  INLINE CPT_Event get_stop_event() const;

  // The following functions are convenience functions that vector
  // directly into the AnimControl's functionality by anim name.

  INLINE bool play(const string &anim_name);
  INLINE bool play(const string &anim_name, int from, int to);
  INLINE bool loop(const string &anim_name, bool restart);
  INLINE bool loop(const string &anim_name, bool restart, int from, int to);
  INLINE bool stop(const string &anim_name);
  INLINE bool pose(const string &anim_name, int frame);

  // These functions operate on all anims at once.
  void play_all();
  void play_all(int from, int to);
  void loop_all(bool restart);
  void loop_all(bool restart, int from, int to);
  bool stop_all();
  void pose_all(int frame);

  INLINE int get_frame(const string &anim_name) const;
  INLINE int get_frame() const;

  INLINE int get_num_frames(const string &anim_name) const;

  INLINE bool is_playing(const string &anim_name) const;
  INLINE bool is_playing() const;

  string which_anim_playing() const;

  void output(ostream &out) const;
  void write(ostream &out) const;

private:
  typedef pmap<string,  PT(AnimControl) > Controls;
  Controls _controls;
  CPT_Event _stop_event;
  AnimControl *_last_started_control;
};

INLINE ostream &operator << (ostream &out, const AnimControlCollection &collection);

#include "animControlCollection.I"

#endif
