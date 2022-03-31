/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animControlCollection.h
 * @author drose
 * @date 2000-02-22
 */

#ifndef ANIMCONTROLCOLLECTION_H
#define ANIMCONTROLCOLLECTION_H

#include "pandabase.h"

#include "animControl.h"

#include "event.h"
#include "pt_Event.h"

#include "pmap.h"

/**
 * This is a named collection of AnimControl pointers.  An AnimControl may be
 * added to the collection by name.  While an AnimControl is associated, its
 * reference count is maintained; associating a new AnimControl with the same
 * name will decrement the previous control's reference count (and possibly
 * delete it, unbinding its animation).
 */
class EXPCL_PANDA_CHAN AnimControlCollection {
PUBLISHED:
  AnimControlCollection();
  ~AnimControlCollection();

  void store_anim(AnimControl *control, const std::string &name);
  AnimControl *find_anim(const std::string &name) const;
  bool unbind_anim(const std::string &name);

  int get_num_anims() const;
  AnimControl *get_anim(int n) const;
  std::string get_anim_name(int n) const;
  MAKE_SEQ(get_anims, get_num_anims, get_anim);
  MAKE_SEQ(get_anim_names, get_num_anims, get_anim_name);

  void clear_anims();

  // The following functions are convenience functions that vector directly
  // into the AnimControl's functionality by anim name.

  INLINE bool play(const std::string &anim_name);
  INLINE bool play(const std::string &anim_name, double from, double to);
  INLINE bool loop(const std::string &anim_name, bool restart);
  INLINE bool loop(const std::string &anim_name, bool restart, double from, double to);
  INLINE bool stop(const std::string &anim_name);
  INLINE bool pose(const std::string &anim_name, double frame);

  // These functions operate on all anims at once.
  void play_all();
  void play_all(double from, double to);
  void loop_all(bool restart);
  void loop_all(bool restart, double from, double to);
  bool stop_all();
  void pose_all(double frame);

  INLINE int get_frame(const std::string &anim_name) const;
  INLINE int get_frame() const;

  INLINE int get_num_frames(const std::string &anim_name) const;
  INLINE int get_num_frames() const;

  INLINE bool is_playing(const std::string &anim_name) const;
  INLINE bool is_playing() const;

  std::string which_anim_playing() const;

  void output(std::ostream &out) const;
  void write(std::ostream &out) const;

private:
  class ControlDef {
  public:
    std::string _name;
    PT(AnimControl) _control;
  };
  typedef pvector<ControlDef> Controls;
  Controls _controls;

  typedef pmap<std::string, size_t> ControlsByName;
  ControlsByName _controls_by_name;

  AnimControl *_last_started_control;
};

INLINE std::ostream &operator << (std::ostream &out, const AnimControlCollection &collection);

#include "animControlCollection.I"

#endif
