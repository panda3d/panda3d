/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cMetaInterval.h
 * @author drose
 * @date 2002-08-27
 */

#ifndef CMETAINTERVAL_H
#define CMETAINTERVAL_H

#include "directbase.h"
#include "cInterval.h"
#include "pointerTo.h"

#include "pdeque.h"
#include "pvector.h"
#include "plist.h"
#include "pset.h"
#include <math.h>

/**
 * This interval contains a list of nested intervals, each of which has its
 * own begin and end times.  Some of them may overlap and some of them may
 * not.
 */
class EXPCL_DIRECT_INTERVAL CMetaInterval : public CInterval {
PUBLISHED:
  explicit CMetaInterval(const std::string &name);
  virtual ~CMetaInterval();

  enum RelativeStart {
    RS_previous_end,
    RS_previous_begin,
    RS_level_begin,
  };

  INLINE void set_precision(double precision);
  INLINE double get_precision() const;

  void clear_intervals();
  int push_level(const std::string &name,
                 double rel_time, RelativeStart rel_to);
  int add_c_interval(CInterval *c_interval,
                     double rel_time = 0.0f,
                     RelativeStart rel_to = RS_previous_end);
  int add_ext_index(int ext_index, const std::string &name,
                    double duration, bool open_ended,
                    double rel_time, RelativeStart rel_to);
  int pop_level(double duration = -1.0);

  bool set_interval_start_time(const std::string &name, double rel_time,
                               RelativeStart rel_to = RS_level_begin);
  double get_interval_start_time(const std::string &name) const;
  double get_interval_end_time(const std::string &name) const;

  enum DefType {
    DT_c_interval,
    DT_ext_index,
    DT_push_level,
    DT_pop_level
  };

  INLINE int get_num_defs() const;
  INLINE DefType get_def_type(int n) const;
  INLINE CInterval *get_c_interval(int n) const;
  INLINE int get_ext_index(int n) const;

  virtual void priv_initialize(double t);
  virtual void priv_instant();
  virtual void priv_step(double t);
  virtual void priv_finalize();
  virtual void priv_reverse_initialize(double t);
  virtual void priv_reverse_instant();
  virtual void priv_reverse_finalize();
  virtual void priv_interrupt();

  INLINE bool is_event_ready();
  INLINE int get_event_index() const;
  INLINE double get_event_t() const;
  INLINE EventType get_event_type() const;
  void pop_event();

  virtual void write(std::ostream &out, int indent_level) const;
  void timeline(std::ostream &out) const;

protected:
  virtual void do_recompute();

private:
  class IntervalDef {
  public:
    DefType _type;
    PT(CInterval) _c_interval;
    int _ext_index;
    std::string _ext_name;
    double _ext_duration;
    bool _ext_open_ended;
    double _rel_time;
    RelativeStart _rel_to;
    int _actual_begin_time;
  };

  enum PlaybackEventType {
    PET_begin,
    PET_end,
    PET_instant
  };

  class PlaybackEvent {
  public:
    INLINE PlaybackEvent(int time, int n, PlaybackEventType type);
    INLINE bool operator < (const PlaybackEvent &other) const;
    int _time;
    int _n;
    PlaybackEventType _type;
    PlaybackEvent *_begin_event;
  };

  class EventQueueEntry {
  public:
    INLINE EventQueueEntry(int n, EventType event_type, int time);
    int _n;
    EventType _event_type;
    int _time;
  };

  typedef pvector<IntervalDef> Defs;
  typedef pvector<PlaybackEvent *> PlaybackEvents;
  // ActiveEvents must be either a list or a vector--something that preserves
  // order--so we can call priv_step() on the currently active intervals in
  // the order they were encountered.
  typedef plist<PlaybackEvent *> ActiveEvents;
  typedef pdeque<EventQueueEntry> EventQueue;

  INLINE int double_to_int_time(double t) const;
  INLINE double int_to_double_time(int time) const;

  void clear_events();
  void do_event_forward(PlaybackEvent *event, ActiveEvents &new_active,
                        bool is_initial);
  void finish_events_forward(int now, ActiveEvents &new_active);
  void do_event_reverse(PlaybackEvent *event, ActiveEvents &new_active,
                        bool is_initial);
  void finish_events_reverse(int now, ActiveEvents &new_active);

  void enqueue_event(int n, CInterval::EventType event_type, bool is_initial,
                     int time = 0);
  void enqueue_self_event(CInterval::EventType event_type, double t = 0.0);
  void enqueue_done_event();
  bool service_event_queue();

  int recompute_level(int n, int level_begin, int &level_end);
  int get_begin_time(const IntervalDef &def, int level_begin,
                     int previous_begin, int previous_end);

  void write_event_desc(std::ostream &out, const IntervalDef &def,
                        int &extra_indent_level) const;


  double _precision;
  Defs _defs;
  int _current_nesting_level;

  PlaybackEvents _events;
  ActiveEvents _active;
  int _end_time;

  size_t _next_event_index;
  bool _processing_events;

  // This is the queue of events that have occurred due to a recent
  // priv_initialize(), priv_step(), etc., but have not yet been serviced, due
  // to an embedded external (e.g.  Python) interval that the scripting
  // language must service.  This queue should be considered precious, and
  // should never be arbitrarily flushed without servicing all of its events.
  EventQueue _event_queue;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CInterval::init_type();
    register_type(_type_handle, "CMetaInterval",
                  CInterval::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "cMetaInterval.I"

#endif
