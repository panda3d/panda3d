/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cInterval.h
 * @author drose
 * @date 2002-08-27
 */

#ifndef CINTERVAL_H
#define CINTERVAL_H

#include "directbase.h"
#include "typedReferenceCount.h"
#include "pvector.h"
#include "config_interval.h"
#include "pStatCollector.h"

class CIntervalManager;

/**
 * The base class for timeline components.  A CInterval represents a single
 * action, event, or collection of nested intervals that will be performed at
 * some specific time or over a period of time.
 *
 * This is essentially similar to the Python "Interval" class, but it is
 * implemented in C++ (hence the name). Intervals that may be implemented in
 * C++ will inherit from this class; Intervals that must be implemented in
 * Python will inherit from the similar Python class.
 */
class EXPCL_DIRECT_INTERVAL CInterval : public TypedReferenceCount {
public:
  CInterval(const std::string &name, double duration, bool open_ended);
  virtual ~CInterval();

PUBLISHED:
  INLINE const std::string &get_name() const;
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

  enum State {
    S_initial,
    S_started,
    S_paused,
    S_final
  };

  INLINE State get_state() const;
  INLINE bool is_stopped() const;

  INLINE void set_done_event(const std::string &event);
  INLINE const std::string &get_done_event() const;

  void set_t(double t);
  INLINE double get_t() const;

  INLINE void set_auto_pause(bool auto_pause);
  INLINE bool get_auto_pause() const;
  INLINE void set_auto_finish(bool auto_finish);
  INLINE bool get_auto_finish() const;

  INLINE void set_wants_t_callback(bool wants_t_callback);
  INLINE bool get_wants_t_callback() const;

  INLINE void set_manager(CIntervalManager *manager);
  INLINE CIntervalManager *get_manager() const;

  void start(double start_t = 0.0, double end_t = -1.0, double play_rate = 1.0);
  void loop(double start_t = 0.0, double end_t = -1.0, double play_rate = 1.0);
  double pause();
  void resume();
  void resume(double start_t);
  void resume_until(double end_t);
  void finish();
  void clear_to_initial();
  bool is_playing() const;

  double get_play_rate() const;
  void set_play_rate(double play_rate);

  // These functions control the actual playback of the interval.  Don't call
  // them directly; they're intended to be called from a supervising object,
  // e.g.  the Python start() .. finish() interface.

  // These cannot be declared private because they must be accessible to
  // Python, but the method names are prefixed with priv_ to remind you that
  // you probably don't want to be using them directly.
  void priv_do_event(double t, EventType event);
  virtual void priv_initialize(double t);
  virtual void priv_instant();
  virtual void priv_step(double t);
  virtual void priv_finalize();
  virtual void priv_reverse_initialize(double t);
  virtual void priv_reverse_instant();
  virtual void priv_reverse_finalize();
  virtual void priv_interrupt();

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent_level) const;

  void setup_play(double start_time, double end_time, double play_rate,
                  bool do_loop);
  void setup_resume();
  void setup_resume_until(double end_t);
  bool step_play();

PUBLISHED:
  MAKE_PROPERTY(name, get_name);
  MAKE_PROPERTY(duration, get_duration);
  MAKE_PROPERTY(open_ended, get_open_ended);
  MAKE_PROPERTY(state, get_state);
  MAKE_PROPERTY(stopped, is_stopped);
  MAKE_PROPERTY(done_event, get_done_event, set_done_event);
  MAKE_PROPERTY(t, get_t, set_t);
  MAKE_PROPERTY(auto_pause, get_auto_pause, set_auto_pause);
  MAKE_PROPERTY(auto_finish, get_auto_finish, set_auto_finish);
  MAKE_PROPERTY(manager, get_manager, set_manager);
  MAKE_PROPERTY(play_rate, get_play_rate, set_play_rate);
  MAKE_PROPERTY(playing, is_playing);

public:
  void mark_dirty();
  INLINE bool check_t_callback();

protected:
  void interval_done();

  INLINE void recompute() const;
  virtual void do_recompute();
  INLINE void check_stopped(TypeHandle type, const char *method_name) const;
  INLINE void check_started(TypeHandle type, const char *method_name) const;

  State _state;
  double _curr_t;
  std::string _name;
  std::string _pname;
  std::string _done_event;
  double _duration;

  bool _auto_pause;
  bool _auto_finish;
  bool _wants_t_callback;
  double _last_t_callback;
  CIntervalManager *_manager;

  // For setup_play() and step_play().
  double _clock_start;
  double _start_t;
  double _end_t;
  bool _end_t_at_end;
  bool _start_t_at_start;
  double _play_rate;
  bool _do_loop;
  int _loop_count;

private:
  bool _open_ended;
  bool _dirty;

  // We keep a record of the "parent" intervals (that is, any CMetaInterval
  // objects that keep a pointer to this one) strictly so we can mark all of
  // our parents dirty when this interval gets dirty.
  typedef pvector<CInterval *> Parents;
  Parents _parents;

  static PStatCollector _root_pcollector;
  PStatCollector _ival_pcollector;

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

INLINE std::ostream &operator << (std::ostream &out, const CInterval &ival);
EXPCL_DIRECT_INTERVAL std::ostream &operator << (std::ostream &out, CInterval::State state);

#include "cInterval.I"

#endif
