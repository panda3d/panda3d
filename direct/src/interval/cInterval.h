// Filename: cInterval.h
// Created by:  drose (27Aug02)
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

#ifndef CINTERVAL_H
#define CINTERVAL_H

#include "directbase.h"
#include "typedReferenceCount.h"
#include "pvector.h"

////////////////////////////////////////////////////////////////////
//       Class : CInterval
// Description : The base class for timeline components.  A CInterval
//               represents a single action, event, or collection of
//               nested intervals that will be performed at some
//               specific time or over a period of time.
//
//               This is essentially similar to the Python "Interval"
//               class, but it is implemented in C++ (hence the name).
//               Intervals that may be implemented in C++ will inherit
//               from this class; Intervals that must be implemented
//               in Python will inherit from the similar Python class.
////////////////////////////////////////////////////////////////////
class EXPCL_DIRECT CInterval : public TypedReferenceCount {
public:
  CInterval(const string &name, double duration, bool open_ended);

PUBLISHED:
  INLINE const string &get_name() const;
  INLINE double get_duration() const;
  INLINE bool get_open_ended() const;

  enum EventType {
    ET_initialize,
    ET_instant,
    ET_step,
    ET_finalize,
    ET_reverse_initialize,
    ET_reverse_instant,
    ET_reverse_finalize,
    ET_interrupt
  };

  INLINE void set_t(double t, EventType event = ET_step);
  INLINE double get_t() const;

  void setup_play(double start_time, double end_time, double play_rate);
  int step_play();

  // These functions control the actual playback of the interval.
  virtual void initialize(double t);
  virtual void instant();
  virtual void step(double t);
  virtual void finalize();
  virtual void reverse_initialize(double t);
  virtual void reverse_instant();
  virtual void reverse_finalize();
  virtual void interrupt();

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level) const;

public:
  void mark_dirty();

protected:
  INLINE void recompute() const;
  virtual void do_recompute();

  double _curr_t;
  string _name;
  double _duration;

  // For setup_play() and step_play().
  double _clock_start;
  double _start_t;
  double _end_t;
  bool _end_t_at_end;
  bool _start_t_at_start;
  double _play_rate;
  int _loop_count;
  bool _restart;

private:
  bool _open_ended;
  bool _dirty;

  // We keep a record of the "parent" intervals (that is, any
  // CMetaInterval objects that keep a pointer to this one) strictly
  // so we can mark all of our parents dirty when this interval gets
  // dirty.
  typedef pvector<CInterval *> Parents;
  Parents _parents;
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "CInterval",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class CMetaInterval;
};

INLINE ostream &operator << (ostream &out, const CInterval &ival);

#include "cInterval.I"

#endif



