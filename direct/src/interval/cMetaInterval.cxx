/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cMetaInterval.cxx
 * @author drose
 * @date 2002-08-27
 */

#include "cMetaInterval.h"
#include "waitInterval.h"
#include "config_interval.h"
#include "indirectLess.h"
#include "indent.h"

#include <algorithm>
#include <math.h>   // for log10()
#include <stdio.h>  // for sprintf()

using std::string;

TypeHandle CMetaInterval::_type_handle;

/**
 *
 */
CMetaInterval::
CMetaInterval(const string &name) :
  CInterval(name, 0.0, true)
{
  _precision = interval_precision;
  _current_nesting_level = 0;
  _next_event_index = 0;
  _processing_events = false;
}

/**
 *
 */
CMetaInterval::
~CMetaInterval() {
  clear_intervals();
}

/**
 * Resets the list of intervals and prepares for receiving a new list.
 */
void CMetaInterval::
clear_intervals() {
  // Better not do this unless you have serviced all of the outstanding
  // events!
  bool lost_events = false;
  if (!_event_queue.empty()) {
    interval_cat.warning()
      << "Losing outstanding events for " << *this << "\n";
    _event_queue.clear();
    lost_events = true;
  }

  clear_events();

  // Go through all of our nested intervals and remove ourselves as their
  // parent.
  Defs::iterator di;
  for (di = _defs.begin(); di != _defs.end(); ++di) {
    IntervalDef &def = (*di);
    if (def._c_interval != nullptr) {
      CInterval::Parents::iterator pi =
        find(def._c_interval->_parents.begin(),
             def._c_interval->_parents.end(),
             this);
      nassertv(pi != def._c_interval->_parents.end());
      def._c_interval->_parents.erase(pi);
    }
  }
  _defs.clear();

  _current_nesting_level = 0;
  _next_event_index = 0;

#ifndef NDEBUG
  if (verify_intervals) {
    nassertv(!lost_events);
  }
#endif
}

/**
 * Marks the beginning of a nested level of child intervals.  Within the
 * nested level, a RelativeStart time of RS_level_begin refers to the start of
 * the level, and the first interval added within the level is always relative
 * to the start of the level.
 *
 * The return value is the index of the def entry created by this push.
 */
int CMetaInterval::
push_level(const string &name, double rel_time, RelativeStart rel_to) {
  nassertr(_event_queue.empty() && !_processing_events, -1);

  _defs.push_back(IntervalDef());
  IntervalDef &def = _defs.back();
  def._type = DT_push_level;
  def._ext_name = name;
  def._rel_time = rel_time;
  def._rel_to = rel_to;
  _current_nesting_level++;
  mark_dirty();

  return (int)_defs.size() - 1;
}

/**
 * Adds a new CInterval to the list.  The interval will be played when the
 * indicated time (relative to the given point) has been reached.
 *
 * The return value is the index of the def entry representing the new
 * interval.
 */
int CMetaInterval::
add_c_interval(CInterval *c_interval,
               double rel_time, RelativeStart rel_to) {
  nassertr(_event_queue.empty() && !_processing_events, -1);
  nassertr(c_interval != nullptr, -1);

  c_interval->_parents.push_back(this);
  c_interval->_ival_pcollector = PStatCollector(_ival_pcollector, c_interval->_pname);
  _defs.push_back(IntervalDef());
  IntervalDef &def = _defs.back();
  def._type = DT_c_interval;
  def._c_interval = c_interval;
  def._rel_time = rel_time;
  def._rel_to = rel_to;
  mark_dirty();

  return (int)_defs.size() - 1;
}

/**
 * Adds a new external interval to the list.  This represents some object in
 * the external scripting language that has properties similar to a CInterval
 * (for instance, a Python Interval object).
 *
 * The CMetaInterval object cannot play this external interval directly, but
 * it records a placeholder for it and will ask the scripting language to play
 * it when it is time, via is_event_ready() and related methods.
 *
 * The ext_index number itself is simply a handle that the scripting language
 * makes up and associates with its interval object somehow.  The
 * CMetaInterval object does not attempt to interpret this value.
 *
 * The return value is the index of the def entry representing the new
 * interval.
 */
int CMetaInterval::
add_ext_index(int ext_index, const string &name, double duration,
              bool open_ended,
              double rel_time, RelativeStart rel_to) {
  nassertr(_event_queue.empty() && !_processing_events, -1);

  _defs.push_back(IntervalDef());
  IntervalDef &def = _defs.back();
  def._type = DT_ext_index;
  def._ext_index = ext_index;
  def._ext_name = name;
  def._ext_duration = duration;
  def._ext_open_ended = open_ended;
  def._rel_time = rel_time;
  def._rel_to = rel_to;
  mark_dirty();

  return (int)_defs.size() - 1;
}

/**
 * Finishes a level marked by a previous call to push_level(), and returns to
 * the previous level.
 *
 * If the duration is not negative, it represents a phony duration to assign
 * to the level, for the purposes of sequencing later intervals.  Otherwise,
 * the level's duration is computed based on the intervals within the level.
 */
int CMetaInterval::
pop_level(double duration) {
  nassertr(_event_queue.empty() && !_processing_events, -1);
  nassertr(_current_nesting_level > 0, -1);

  _defs.push_back(IntervalDef());
  IntervalDef &def = _defs.back();
  def._type = DT_pop_level;
  def._ext_duration = duration;
  _current_nesting_level--;
  mark_dirty();

  return (int)_defs.size() - 1;
}

/**
 * Adjusts the start time of the child interval with the given name, if found.
 * This may be either a C++ interval added via add_c_interval(), or an
 * external interval added via add_ext_index(); the name must match exactly.
 *
 * If the interval is found, its start time is adjusted, and all subsequent
 * intervals are adjusting accordingly, and true is returned.  If a matching
 * interval is not found, nothing is changed and false is returned.
 */
bool CMetaInterval::
set_interval_start_time(const string &name, double rel_time,
                        CMetaInterval::RelativeStart rel_to) {
  nassertr(_event_queue.empty() && !_processing_events, false);
  Defs::iterator di;
  for (di = _defs.begin(); di != _defs.end(); ++di) {
    IntervalDef &def = (*di);

    bool match = false;
    switch (def._type) {
    case DT_c_interval:
      match = (def._c_interval->get_name() == name);
      break;

    case DT_ext_index:
      match = (def._ext_name == name);
      break;

    default:
      break;
    }
    if (match) {
      // Here's the interval.
      def._rel_time = rel_time;
      def._rel_to = rel_to;
      mark_dirty();
      return true;
    }
  }

  return false;
}

/**
 * Returns the actual start time, relative to the beginning of the interval,
 * of the child interval with the given name, if found, or -1 if the interval
 * is not found.
 */
double CMetaInterval::
get_interval_start_time(const string &name) const {
  recompute();
  Defs::const_iterator di;
  for (di = _defs.begin(); di != _defs.end(); ++di) {
    const IntervalDef &def = (*di);

    bool match = false;
    switch (def._type) {
    case DT_c_interval:
      match = (def._c_interval->get_name() == name);
      break;

    case DT_ext_index:
      match = (def._ext_name == name);
      break;

    default:
      break;
    }
    if (match) {
      // Here's the interval.
      return int_to_double_time(def._actual_begin_time);
    }
  }

  return -1.0;
}

/**
 * Returns the actual end time, relative to the beginning of the interval, of
 * the child interval with the given name, if found, or -1 if the interval is
 * not found.
 */
double CMetaInterval::
get_interval_end_time(const string &name) const {
  recompute();
  Defs::const_iterator di;
  for (di = _defs.begin(); di != _defs.end(); ++di) {
    const IntervalDef &def = (*di);

    bool match = false;
    double duration = 0.0;
    switch (def._type) {
    case DT_c_interval:
      duration = def._c_interval->get_duration();
      match = (def._c_interval->get_name() == name);
      break;

    case DT_ext_index:
      duration = def._ext_duration;
      match = (def._ext_name == name);
      break;

    default:
      break;
    }
    if (match) {
      // Here's the interval.
      return int_to_double_time(def._actual_begin_time) + duration;
    }
  }

  return -1.0;
}

/**
 * This replaces the first call to priv_step(), and indicates that the
 * interval has just begun.  This may be overridden by derived classes that
 * need to do some explicit initialization on the first call.
 */
void CMetaInterval::
priv_initialize(double t) {
  if (_processing_events) {
    enqueue_self_event(ET_initialize, t);
    return;
  }

  check_stopped(get_class_type(), "priv_initialize");
  // It may be tempting to flush the event_queue here, but don't do it.  Those
  // are events that must still be serviced from some previous interval
  // operation.  Throwing them away would be a mistake.

  recompute();
  _next_event_index = 0;
  _active.clear();

  int now = double_to_int_time(t);

  /*
  // One special case: if we step to t == 0.0, it really means to the very
  // beginning of the interval, *before* any events that occurred at time 0.
  // (Most of the time, stepping to a particular time means *after* any events
  // that occurred at that time.)
  if (t == 0.0) {
    now = -1;
  }
  */

  // Now look for events from the beginning up to the current time.
  _processing_events = true;
  ActiveEvents new_active;
  while (_next_event_index < _events.size() &&
         _events[_next_event_index]->_time <= now) {
    PlaybackEvent *event = _events[_next_event_index];
    _next_event_index++;

    // Do the indicated event.
    do_event_forward(event, new_active, true);
  }
  finish_events_forward(now, new_active);
  _processing_events = false;

  _curr_t = t;
  _state = S_started;
}

/**
 * This is called in lieu of priv_initialize() .. priv_step() ..
 * priv_finalize(), when everything is to happen within one frame.  The
 * interval should initialize itself, then leave itself in the final state.
 */
void CMetaInterval::
priv_instant() {
  if (_processing_events) {
    enqueue_self_event(ET_instant);
    return;
  }

  check_stopped(get_class_type(), "priv_instant");
  recompute();
  _active.clear();

  // Apply all of the events.  This just means we invoke "instant" for any end
  // or instant event, ignoring the begin events.
  _processing_events = true;
  PlaybackEvents::iterator ei;
  for (ei = _events.begin(); ei != _events.end(); ++ei) {
    PlaybackEvent *event = (*ei);
    if (event->_type != PET_begin) {
      enqueue_event(event->_n, ET_instant, true, 0);
    }
  }
  _processing_events = false;

  _next_event_index = _events.size();
  _curr_t = get_duration();
  _state = S_final;

  if (_event_queue.empty()) {
    interval_done();
  } else {
    enqueue_done_event();
  }
}

/**
 * Advances the time on the interval.  The time may either increase (the
 * normal case) or decrease (e.g.  if the interval is being played by a
 * slider).
 */
void CMetaInterval::
priv_step(double t) {
  if (_processing_events) {
    enqueue_self_event(ET_step, t);
    return;
  }

  check_started(get_class_type(), "priv_step");
  int now = double_to_int_time(t);

  /*
  // One special case: if we step to t == 0.0, it really means to the very
  // beginning of the interval, *before* any events that occurred at time 0.
  // (Most of the time, stepping to a particular time means *after* any events
  // that occurred at that time.)
  if (t == 0.0) {
    now = -1;
  }
  */

  // Now look for events between the last time we ran and the current time.

  _processing_events = true;
  if (_next_event_index < _events.size() &&
      _events[_next_event_index]->_time <= now) {
    // The normal case: time is increasing.
    ActiveEvents new_active;
    while (_next_event_index < _events.size() &&
           _events[_next_event_index]->_time <= now) {
      PlaybackEvent *event = _events[_next_event_index];
      _next_event_index++;

      // Do the indicated event.
      do_event_forward(event, new_active, false);
    }

    finish_events_forward(now, new_active);

  } else {
    // A less usual case: time is decreasing.
    ActiveEvents new_active;
    while (_next_event_index > 0 &&
           _events[_next_event_index - 1]->_time > now) {
      _next_event_index--;
      PlaybackEvent *event = _events[_next_event_index];

      do_event_reverse(event, new_active, false);
    }

    finish_events_reverse(now, new_active);
  }
  _processing_events = false;

  _curr_t = t;
  _state = S_started;
}

/**
 * This is called when an interval is interrupted.  It should advance the time
 * as if priv_step() were called, and also perform whatever cleanup might be
 * required.
 */
void CMetaInterval::
priv_finalize() {
  if (_processing_events) {
    enqueue_self_event(ET_finalize);
    return;
  }

  double duration = get_duration();
  if (_state == S_initial) {
    priv_initialize(duration);
  }

  // Do all remaining events.
  _processing_events = true;
  ActiveEvents new_active;
  while (_next_event_index < _events.size()) {
    PlaybackEvent *event = _events[_next_event_index];
    _next_event_index++;

    // Do the indicated event.
    do_event_forward(event, new_active, true);
  }
  finish_events_forward(double_to_int_time(duration), new_active);
  _processing_events = false;

  _curr_t = duration;
  _state = S_final;

  if (_event_queue.empty()) {
    interval_done();
  } else {
    enqueue_done_event();
  }
}

/**
 * Similar to priv_initialize(), but this is called when the interval is being
 * played backwards; it indicates that the interval should start at the
 * finishing state and undo any intervening intervals.
 */
void CMetaInterval::
priv_reverse_initialize(double t) {
  if (_processing_events) {
    enqueue_self_event(ET_reverse_initialize, t);
    return;
  }

  check_stopped(get_class_type(), "priv_reverse_initialize");
  // It may be tempting to flush the event_queue here, but don't do it.  Those
  // are events that must still be serviced from some previous interval
  // operation.  Throwing them away would be a mistake.

  recompute();
  _next_event_index = _events.size();
  _active.clear();

  int now = double_to_int_time(t);

  /*
  // One special case: if we step to t == 0.0, it really means to the very
  // beginning of the interval, *before* any events that occurred at time 0.
  // (Most of the time, stepping to a particular time means *after* any events
  // that occurred at that time.)
  if (t == 0.0) {
    now = -1;
  }
  */

  // Now look for events from the end down to the current time.
  _processing_events = true;
  ActiveEvents new_active;
  while (_next_event_index > 0 &&
         _events[_next_event_index - 1]->_time > now) {
    _next_event_index--;
    PlaybackEvent *event = _events[_next_event_index];

    // Do the indicated event.
    do_event_reverse(event, new_active, true);
  }
  finish_events_reverse(now, new_active);
  _processing_events = false;

  _curr_t = t;
  _state = S_started;
}

/**
 * This is called in lieu of priv_reverse_initialize() .. priv_step() ..
 * priv_reverse_finalize(), when everything is to happen within one frame.
 * The interval should initialize itself, then leave itself in the initial
 * state.
 */
void CMetaInterval::
priv_reverse_instant() {
  if (_processing_events) {
    enqueue_self_event(ET_reverse_instant);
    return;
  }

  check_stopped(get_class_type(), "priv_reverse_instant");
  recompute();
  _active.clear();

  // Apply all of the events.  This just means we invoke "instant" for any end
  // or instant event, ignoring the begin events.
  _processing_events = true;
  PlaybackEvents::reverse_iterator ei;
  for (ei = _events.rbegin(); ei != _events.rend(); ++ei) {
    PlaybackEvent *event = (*ei);
    if (event->_type != PET_begin) {
      enqueue_event(event->_n, ET_reverse_instant, true, 0);
    }
  }
  _processing_events = false;

  _next_event_index = 0;
  _curr_t = 0.0;
  _state = S_initial;
}

/**
 * Called generally following a priv_reverse_initialize(), this indicates the
 * interval should set itself to the initial state.
 */
void CMetaInterval::
priv_reverse_finalize() {
  if (_processing_events) {
    enqueue_self_event(ET_reverse_finalize);
    return;
  }

  if (_state == S_initial) {
    priv_initialize(0.0);
  }

  // Do all remaining events at the beginning.
  _processing_events = true;
  ActiveEvents new_active;

  while (_next_event_index > 0) {
    _next_event_index--;
    PlaybackEvent *event = _events[_next_event_index];

    do_event_reverse(event, new_active, true);
  }
  finish_events_reverse(0, new_active);
  _processing_events = false;

  _curr_t = 0.0;
  _state = S_initial;
}

/**
 * This is called while the interval is playing to indicate that it is about
 * to be interrupted; that is, priv_step() will not be called for a length of
 * time.  But the interval should remain in its current state in anticipation
 * of being eventually restarted when the calls to priv_step() eventually
 * resume.
 *
 * The purpose of this function is to allow self-running intervals like sound
 * intervals to stop the actual sound playback during the pause.
 */
void CMetaInterval::
priv_interrupt() {
  if (_processing_events) {
    enqueue_self_event(ET_interrupt);
    return;
  }

  _processing_events = true;
  ActiveEvents::iterator ai;
  for (ai = _active.begin(); ai != _active.end(); ++ai) {
    PlaybackEvent *event = (*ai);
    enqueue_event(event->_n, ET_interrupt, false);
  }
  _processing_events = false;

  if (_state == S_started) {
    _state = S_paused;
  }
}

/**
 * Acknowledges that the external interval on the top of the queue has been
 * extracted, and is about to be serviced by the scripting language.  This
 * prepares the interval so the next call to is_event_ready() will return
 * information about the next external interval on the queue, if any.
 */
void CMetaInterval::
pop_event() {
#ifndef NDEBUG
  nassertv(!_event_queue.empty());
  const EventQueueEntry &entry = _event_queue.front();
  const IntervalDef &def = _defs[entry._n];
  nassertv(def._type == DT_ext_index);
#endif
  _event_queue.pop_front();
}

/**
 *
 */
void CMetaInterval::
write(std::ostream &out, int indent_level) const {
  recompute();

  // How many digits of precision should we output for time?
  int num_decimals = (int)ceil(log10(_precision));
  int total_digits = num_decimals + 4;
  static const int max_digits = 32;  // totally arbitrary
  nassertv(total_digits <= max_digits);
  char format_str[16];
  sprintf(format_str, "%%%d.%df", total_digits, num_decimals);

  indent(out, indent_level) << get_name() << ":\n";

  int extra_indent_level = 1;
  Defs::const_iterator di;
  for (di = _defs.begin(); di != _defs.end(); ++di) {
    const IntervalDef &def = (*di);
    char time_str[max_digits + 1];
    sprintf(time_str, format_str, int_to_double_time(def._actual_begin_time));
    indent(out, indent_level) << time_str;

    write_event_desc(out, def, extra_indent_level);
  }
}

/**
 * Outputs a list of all events in the order in which they occur.
 */
void CMetaInterval::
timeline(std::ostream &out) const {
  recompute();

  // How many digits of precision should we output for time?
  int num_decimals = (int)ceil(log10(_precision));
  int total_digits = num_decimals + 4;
  static const int max_digits = 32;  // totally arbitrary
  nassertv(total_digits <= max_digits);
  char format_str[16];
  sprintf(format_str, "%%%d.%df", total_digits, num_decimals);

  int extra_indent_level = 0;
  PlaybackEvents::const_iterator ei;
  for (ei = _events.begin(); ei != _events.end(); ++ei) {
    const PlaybackEvent *event = (*ei);

    char time_str[max_digits + 1];
    sprintf(time_str, format_str, int_to_double_time(event->_time));
    out << time_str;

    switch (event->_type) {
    case PET_begin:
      out << " begin   ";
      break;
    case PET_end:
      out << " end     ";
      break;
    case PET_instant:
      out << " instant ";
      break;
    }

    int n = event->_n;
    nassertv(n >= 0 && n < (int)_defs.size());
    const IntervalDef &def = _defs[n];

    write_event_desc(out, def, extra_indent_level);
  }
}

/**
 * Recomputes all of the events (and the duration) according to the set of
 * interval defs.
 */
void CMetaInterval::
do_recompute() {
  _dirty = false;
  clear_events();

  int n = recompute_level(0, 0, _end_time);

  if (n != (int)_defs.size()) {
    interval_cat.warning()
      << "CMetaInterval pushes don't match pops.\n";
  }

  // We do a stable_sort() to guarantee ordering of events that have the same
  // start time.  These must be invoked in the order in which they appear.
  std::stable_sort(_events.begin(), _events.end(), IndirectLess<PlaybackEvent>());
  _duration = int_to_double_time(_end_time);
}

/**
 * Removes all entries from the _events list.
 */
void CMetaInterval::
clear_events() {
  PlaybackEvents::iterator ei;
  for (ei = _events.begin(); ei != _events.end(); ++ei) {
    PlaybackEvent *event = (*ei);
    delete event;
  }
  _events.clear();
  _active.clear();
}

/**
 * Process a single event in the interval, moving forwards in time.  If the
 * event represents a new begin, adds it to the new_active list; if it is an
 * end, finalizes it.
 *
 * If is_initial is true, it is as if we are in initialize or finalize:
 * instant events will be invoked only if they are marked open_ended.
 */
void CMetaInterval::
do_event_forward(CMetaInterval::PlaybackEvent *event,
                 CMetaInterval::ActiveEvents &new_active, bool is_initial) {
  switch (event->_type) {
  case PET_begin:
    nassertv(event->_begin_event == event);
    new_active.push_back(event);
    break;

  case PET_end:
    {
      // Erase the event from either the new active or the current active
      // lists.
      ActiveEvents::iterator ai;
      ai = find(new_active.begin(), new_active.end(), event->_begin_event);
      if (ai != new_active.end()) {
        new_active.erase(ai);
        // This interval was new this frame; we must invoke it as an instant
        // event.
        enqueue_event(event->_n, ET_instant, is_initial);

      } else {
        ai = find(_active.begin(), _active.end(), event->_begin_event);
        if (ai != _active.end()) {
          _active.erase(ai);
          enqueue_event(event->_n, ET_finalize, is_initial);

        } else {
          // Hmm, this event wasn't on either list.  Maybe there was a start
          // event on the list whose time was less than 0.
          interval_cat.error()
            << "Event " << event->_begin_event->_n << " not on active list.\n";
          nassertv(false);
        }
      }
    }
    break;

  case PET_instant:
    nassertv(event->_begin_event == event);
    enqueue_event(event->_n, ET_instant, is_initial);
    break;
  }
}

/**
 * After walking through the event list and adding a bunch of new events to
 * new_active, finished up by calling priv_step() on all of the events still
 * in _active and priv_initialize() on all the events in new_active, then
 * copying the events from new_active to active.
 */
void CMetaInterval::
finish_events_forward(int now, CMetaInterval::ActiveEvents &new_active) {
  // Do whatever's still active.
  ActiveEvents::iterator ai;
  for (ai = _active.begin(); ai != _active.end(); ++ai) {
    PlaybackEvent *event = (*ai);
    enqueue_event(event->_n, ET_step, false, now - event->_time);
  }

  // Initialize whatever new intervals we came across.
  for (ai = new_active.begin(); ai != new_active.end(); ++ai) {
    PlaybackEvent *event = (*ai);
    enqueue_event(event->_n, ET_initialize, false, now - event->_time);
    _active.push_back(event);
  }
}

/**
 * Process a single event in the interval, moving backwards in time.  This
 * undoes the indicated event.  If the event represents a new begin, adds it
 * to the new_active list; if it is an end, finalizes it.
 *
 * If is_initial is true, it is as if we are in reverse_initialize or
 * reverse_finalize: instant events will be invoked only if they are marked
 * open_ended.
 */
void CMetaInterval::
do_event_reverse(CMetaInterval::PlaybackEvent *event,
                 CMetaInterval::ActiveEvents &new_active, bool is_initial) {
  // Undo the indicated event.
  switch (event->_type) {
  case PET_begin:
    {
      nassertv(event->_begin_event == event);
      // Erase the event from either the new active or the current active
      // lists.
      ActiveEvents::iterator ai;
      ai = find(new_active.begin(), new_active.end(), event);
      if (ai != new_active.end()) {
        new_active.erase(ai);
        // This interval was new this frame; we invoke it as an instant event.
        enqueue_event(event->_n, ET_reverse_instant, is_initial);

      } else {
        ai = find(_active.begin(), _active.end(), event);
        if (ai != _active.end()) {
          _active.erase(ai);
          enqueue_event(event->_n, ET_reverse_finalize, is_initial);

        } else {
          // Hmm, this event wasn't on either list.  Maybe there was a stop
          // event on the list whose time was greater than the total, somehow.
          interval_cat.error()
            << "Event " << event->_n << " not on active list.\n";
          nassertv(false);
        }
      }
    }
    break;

  case PET_end:
    new_active.push_front(event->_begin_event);
    break;

  case PET_instant:
    nassertv(event->_begin_event == event);
    enqueue_event(event->_n, ET_reverse_instant, is_initial);
    break;
  }
}

/**
 * After walking through the event list and adding a bunch of new events to
 * new_active, finishes up by calling priv_step() on all of the events still
 * in _active and priv_reverse_initialize() on all the events in new_active,
 * then copying the events from new_active to active.
 */
void CMetaInterval::
finish_events_reverse(int now, CMetaInterval::ActiveEvents &new_active) {
  // Do whatever's still active.
  ActiveEvents::iterator ai;
  for (ai = _active.begin(); ai != _active.end(); ++ai) {
    PlaybackEvent *event = (*ai);
    enqueue_event(event->_n, ET_step, false, now - event->_time);
  }

  // Initialize whatever new intervals we came across.
  for (ai = new_active.begin(); ai != new_active.end(); ++ai) {
    PlaybackEvent *event = (*ai);
    enqueue_event(event->_n, ET_reverse_initialize, false, now - event->_time);
    _active.push_front(event);
  }
}

/**
 * Enqueues the indicated interval for invocation after we have finished
 * scanning for events that need processing this frame.
 *
 * is_initial is only relevant for event types ET_instant or
 * ET_reverse_instant, and indicates whether we are in the priv_initialize()
 * (or priv_reverse_initialize()) call, and should therefore only invoke open-
 * ended intervals.
 *
 * time is only relevant for ET_initialize, ET_reverse_initialize, and
 * ET_step.
 */
void CMetaInterval::
enqueue_event(int n, CInterval::EventType event_type, bool is_initial, int time) {
  nassertv(n >= 0 && n < (int)_defs.size());
  const IntervalDef &def = _defs[n];
  switch (def._type) {
  case DT_c_interval:
    if (is_initial &&
        (event_type == ET_instant || event_type == ET_reverse_instant) &&
        !def._c_interval->get_open_ended()) {
      // Ignore a non-open-ended interval that we skipped completely past on
      // priv_initialize().
      return;
    } else {
      if (_event_queue.empty()) {
        // if the event queue is empty, we can process this C++ interval
        // immediately.  We only need to defer it if there are external (e.g.
        // Python) intervals in the queue that need to be processed first.
        def._c_interval->priv_do_event(int_to_double_time(time), event_type);
        return;
      }
    }
    break;

  case DT_ext_index:
    if (is_initial &&
        (event_type == ET_instant || event_type == ET_reverse_instant) &&
        !def._ext_open_ended) {
      // Ignore a non-open-ended interval that we skipped completely past on
      // priv_initialize().
      return;
    }
    break;

  default:
    nassertv(false);
    return;
  }

  _event_queue.push_back(EventQueueEntry(n, event_type, time));
}

/**
 * Enqueues a reference to *this* interval.  This is called only when the
 * interval is recursively re-entered; the request will be serviced when the
 * current request is done processing.
 *
 * time is only relevant for ET_initialize, ET_reverse_initialize, and
 * ET_step.
 */
void CMetaInterval::
enqueue_self_event(CInterval::EventType event_type, double t) {
  interval_cat.info()
    << "Recursive reentry detected into " << *this << "\n";
  int time = double_to_int_time(t);
  _event_queue.push_back(EventQueueEntry(-1, event_type, time));
}

/**
 * Enqueues a special "event" that simply marks the end of processing of the
 * interval; the interval's done event should be thrown now, if it is defined.
 */
void CMetaInterval::
enqueue_done_event() {
  _event_queue.push_back(EventQueueEntry(-2, ET_finalize, 0));
}

/**
 * Invokes whatever C++ intervals might be at the head of the queue, and
 * prepares for passing an external interval to the scripting language.
 *
 * The return value is true if there remains at least one external event to be
 * serviced, false if all events are handled.
 */
bool CMetaInterval::
service_event_queue() {
  while (!_event_queue.empty()) {
    nassertr(!_processing_events, true);
    const EventQueueEntry &entry = _event_queue.front();
    if (entry._n == -1) {
      // Index -1 is a special code for *this* interval.
      priv_do_event(int_to_double_time(entry._time), entry._event_type);

    } else if (entry._n == -2) {
      // Index -2 is a special code to indicate the interval is now done, and
      // its done event should be thrown.
      interval_done();

    } else {
      nassertr(entry._n >= 0 && entry._n < (int)_defs.size(), false);
      const IntervalDef &def = _defs[entry._n];
      switch (def._type) {
      case DT_c_interval:
        // Handle the C++ event.
        def._c_interval->priv_do_event(int_to_double_time(entry._time), entry._event_type);
        break;

      case DT_ext_index:
        // Here's an external event; leave it there and return.
        return true;

      default:
        nassertr(false, false);
        return false;
      }
    }
    _event_queue.pop_front();
  }

  // No more events on the queue.
  nassertr(!_processing_events, false);
  return false;
}

/**
 * Recursively recomputes a complete level (delimited by push/pop
 * definitions).
 *
 * The value n on entry refers to the first entry after the push; the return
 * value will reference the matching pop, or an index greater than the last
 * element in the array if there was no matching pop.
 *
 * The level_begin value indicates the begin time of this level.  On return,
 * level_end is filled with the end time of this level.
 */
int CMetaInterval::
recompute_level(int n, int level_begin, int &level_end) {
  level_end = level_begin;
  int previous_begin = level_begin;
  int previous_end = level_begin;

  while (n < (int)_defs.size() && _defs[n]._type != DT_pop_level) {
    IntervalDef &def = _defs[n];
    int begin_time = previous_begin;
    int end_time = previous_end;
    switch (def._type) {
    case DT_c_interval:
      begin_time = get_begin_time(def, level_begin, previous_begin, previous_end);
      def._actual_begin_time = begin_time;
      end_time = begin_time + double_to_int_time(def._c_interval->get_duration());

      if (def._c_interval->is_exact_type(WaitInterval::get_class_type())) {
        // Don't bother enqueuing events for WaitIntervals; they're just there
        // to fill up time.

      } else {
        if (begin_time == end_time) {
          _events.push_back(new PlaybackEvent(begin_time, n, PET_instant));
        } else {
          PlaybackEvent *begin = new PlaybackEvent(begin_time, n, PET_begin);
          PlaybackEvent *end = new PlaybackEvent(end_time, n, PET_end);
          end->_begin_event = begin;
          _events.push_back(begin);
          _events.push_back(end);
        }
      }
      break;

    case DT_ext_index:
      begin_time = get_begin_time(def, level_begin, previous_begin, previous_end);
      def._actual_begin_time = begin_time;
      end_time = begin_time + double_to_int_time(def._ext_duration);
      if (begin_time == end_time) {
        _events.push_back(new PlaybackEvent(begin_time, n, PET_instant));
      } else {
        PlaybackEvent *begin = new PlaybackEvent(begin_time, n, PET_begin);
        PlaybackEvent *end = new PlaybackEvent(end_time, n, PET_end);
        end->_begin_event = begin;
        _events.push_back(begin);
        _events.push_back(end);
      }
      break;

    case DT_push_level:
      begin_time = get_begin_time(def, level_begin, previous_begin, previous_end);
      def._actual_begin_time = begin_time;
      n = recompute_level(n + 1, begin_time, end_time);
      break;

    case DT_pop_level:
      nassertr(false, _defs.size());
      break;
    }

    previous_begin = begin_time;
    previous_end = end_time;
    level_end = std::max(level_end, end_time);
    n++;
  }

  if (n < (int)_defs.size()) {
    IntervalDef &def = _defs[n];
    // If we have a pop record, check it for a phony duration.
    if (def._ext_duration >= 0.0) {
      level_end = level_begin + double_to_int_time(def._ext_duration);
    }

    // The final pop "begins" at the level end time, just for clarity on
    // output.
    def._actual_begin_time = level_end;
  }

  return n;
}

/**
 * Returns the integer begin time indicated by the given IntervalDef, given
 * the indicated level begin, previous begin, and previous end times.
 */
int CMetaInterval::
get_begin_time(const CMetaInterval::IntervalDef &def, int level_begin,
               int previous_begin, int previous_end) {
  switch (def._rel_to) {
  case RS_previous_end:
    return previous_end + double_to_int_time(def._rel_time);

  case RS_previous_begin:
    return previous_begin + double_to_int_time(def._rel_time);

  case RS_level_begin:
    return level_begin + double_to_int_time(def._rel_time);
  }

  nassertr(false, previous_end);
  return previous_end;
}

/**
 * Formats an event for output, for write() or timeline().
 */
void CMetaInterval::
write_event_desc(std::ostream &out, const CMetaInterval::IntervalDef &def,
                 int &extra_indent_level) const {
  switch (def._type) {
  case DT_c_interval:
    indent(out, extra_indent_level)
      << *def._c_interval;
    if (!def._c_interval->get_open_ended()) {
      out << " (!oe)";
    }
    out << "\n";
    break;

  case DT_ext_index:
    indent(out, extra_indent_level)
      << "*" << def._ext_name;
    if (def._ext_duration != 0.0) {
      out << " dur " << def._ext_duration;
    }
    if (!def._ext_open_ended) {
      out << " (!oe)";
    }
    out<< "\n";
    break;

  case DT_push_level:
    indent(out, extra_indent_level)
      << def._ext_name << " {\n";
    extra_indent_level += 2;
    break;

  case DT_pop_level:
    extra_indent_level -= 2;
    indent(out, extra_indent_level)
      << "}\n";
    break;
  }
}
