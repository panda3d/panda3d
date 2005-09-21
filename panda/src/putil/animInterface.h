// Filename: animInterface.h
// Created by:  drose (20Sep05)
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

#ifndef ANIMINTERFACE_H
#define ANIMINTERFACE_H

#include "pandabase.h"
#include "typeHandle.h"
#include "register_type.h"

////////////////////////////////////////////////////////////////////
//       Class : AnimInterface
// Description : This is the fundamental interface for things that
//               have a play/loop/stop type interface for frame-based
//               animation, such as animated characters.  This is the
//               base class for AnimControl and other, similar
//               classes.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA AnimInterface {
protected:
  AnimInterface();
  AnimInterface(const AnimInterface &copy);

PUBLISHED:
  virtual ~AnimInterface();
  INLINE void play();
  void play(double from, double to);
  INLINE void loop(bool restart);
  void loop(bool restart, double from, double to);
  INLINE void pingpong(bool restart);
  void pingpong(bool restart, double from, double to);
  INLINE void stop();
  void pose(int frame);

  INLINE void set_play_rate(double play_rate);
  INLINE double get_play_rate() const;
  INLINE double get_frame_rate() const;
  INLINE int get_num_frames() const;

  INLINE int get_frame() const;
  INLINE double get_frac() const;
  int get_full_frame() const;
  double get_full_fframe() const;
  bool is_playing() const;

  virtual void output(ostream &out) const;

protected:
  INLINE void set_frame_rate(double frame_rate);
  INLINE void set_num_frames(int num_frames);
  virtual void animation_activated();

private:
  double get_f() const;
  void internal_set_rate(double frame_rate, double play_rate);

private:
  enum PlayMode {
    PM_pose,
    PM_play,
    PM_loop,
    PM_pingpong,
  };

  double _frame_rate;
  int _num_frames;

  PlayMode _play_mode;
  double _start_time;
  double _start_frame;
  double _play_frames;
  int _from_frame;
  int _to_frame;

  double _play_rate;
  double _effective_frame_rate;
  bool _paused;
  double _paused_f;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    register_type(_type_handle, "AnimInterface");
  }

private:
  static TypeHandle _type_handle;
};

INLINE ostream &operator << (ostream &out, const AnimInterface &ai);

#include "animInterface.I"

#endif
