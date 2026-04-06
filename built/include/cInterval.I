/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cInterval.I
 * @author drose
 * @date 2002-08-27
 */

/**
 * Returns the interval's name.
 */
INLINE const std::string &CInterval::
get_name() const {
  return _name;
}

/**
 * Returns the duration of the interval in seconds.
 */
INLINE double CInterval::
get_duration() const {
  recompute();
  return _duration;
}

/**
 * Returns the state of the "open_ended" flag.  This is primarily intended for
 * instantaneous intervals like FunctionIntervals; it indicates true if the
 * interval has some lasting effect that should be applied even if the
 * interval doesn't get started until after its finish time, or false if the
 * interval is a transitive thing that doesn't need to be called late.
 */
INLINE bool CInterval::
get_open_ended() const {
  return _open_ended;
}

/**
 * Indicates the state the interval believes it is in: whether it has been
 * started, is currently in the middle, or has been finalized.
 */
INLINE CInterval::State CInterval::
get_state() const {
  return _state;
}

/**
 * Returns true if the interval is in either its initial or final states (but
 * not in a running or paused state).
 */
INLINE bool CInterval::
is_stopped() const {
  return (_state == S_initial || _state == S_final);
}

/**
 * Sets the event that is generated whenever the interval reaches its final
 * state, whether it is explicitly finished or whether it gets there on its
 * own.
 */
INLINE void CInterval::
set_done_event(const std::string &event) {
  _done_event = event;
}

/**
 * Returns the event that is generated whenever the interval reaches its final
 * state, whether it is explicitly finished or whether it gets there on its
 * own.
 */
INLINE const std::string &CInterval::
get_done_event() const {
  return _done_event;
}

/**
 * Returns the current time of the interval: the last value of t passed to
 * priv_initialize(), priv_step(), or priv_finalize().
 */
INLINE double CInterval::
get_t() const {
  return _curr_t;
}

/**
 * Changes the state of the 'auto_pause' flag.  If this is true, the interval
 * may be arbitrarily interrupted when the system needs to reset due to some
 * external event by calling CIntervalManager::interrupt().  If this is false
 * (the default), the interval must always be explicitly finished or paused.
 */
INLINE void CInterval::
set_auto_pause(bool auto_pause) {
  _auto_pause = auto_pause;
}

/**
 * Returns the state of the 'auto_pause' flag.  See set_auto_pause().
 */
INLINE bool CInterval::
get_auto_pause() const {
  return _auto_pause;
}

/**
 * Changes the state of the 'auto_finish' flag.  If this is true, the interval
 * may be arbitrarily finished when the system needs to reset due to some
 * external event by calling CIntervalManager::interrupt().  If this is false
 * (the default), the interval must always be explicitly finished or paused.
 */
INLINE void CInterval::
set_auto_finish(bool auto_finish) {
  _auto_finish = auto_finish;
}

/**
 * Returns the state of the 'auto_finish' flag.  See set_auto_finish().
 */
INLINE bool CInterval::
get_auto_finish() const {
  return _auto_finish;
}

/**
 * Changes the state of the 'wants_t_callback' flag.  If this is true, the
 * interval will be returned by CIntervalManager::get_event() each time the
 * interval's time value has been changed, regardless of whether it has any
 * external events.
 */
INLINE void CInterval::
set_wants_t_callback(bool wants_t_callback) {
  _wants_t_callback = wants_t_callback;
  _last_t_callback = -1.0;
}

/**
 * Returns the state of the 'wants_t_callback' flag.  See
 * set_wants_t_callback().
 */
INLINE bool CInterval::
get_wants_t_callback() const {
  return _wants_t_callback;
}

/**
 * Indicates the CIntervalManager object which will be responsible for playing
 * this interval.  This defaults to the global CIntervalManager; you should
 * need to change this only if you have special requirements for playing this
 * interval.
 */
INLINE void CInterval::
set_manager(CIntervalManager *manager) {
  _manager = manager;
}

/**
 * Returns the CIntervalManager object which will be responsible for playing
 * this interval.  Note that this can only return a C++ object; if the
 * particular CIntervalManager object has been extended in the scripting
 * language, this will return the encapsulated C++ object, not the full
 * extended object.
 */
INLINE CIntervalManager *CInterval::
get_manager() const {
  return _manager;
}

/**
 * Returns true if the wants_t_callback() flag is true and the interval's t
 * value has changed since the last call to check_t_callback(), false
 * otherwise.
 */
INLINE bool CInterval::
check_t_callback() {
  if (get_wants_t_callback() && get_t() != _last_t_callback) {
    _last_t_callback = get_t();
    return true;
  }
  return false;
}

/**
 * Calls do_recompute() if the dirty flag has been set.
 */
INLINE void CInterval::
recompute() const {
  if (_dirty) {
    ((CInterval *)this)->do_recompute();
  }
}

/**
 * Issues a warning if our internal state is not in one of the stopped states.
 */
INLINE void CInterval::
check_stopped(TypeHandle type, const char *method_name) const {
  if (_state == S_started) {
    interval_cat.warning()
      << type.get_name() << "::" << method_name << "() called for "
      << get_name() << " in state " << _state << ".\n";
    nassertv(!verify_intervals);
  }
}

/**
 * Issues a warning if our internal state is not in one of the started states.
 */
INLINE void CInterval::
check_started(TypeHandle type, const char *method_name) const {
  if (_state != S_started && _state != S_paused) {
    interval_cat.warning()
      << type.get_name() << "::" << method_name << "() called for "
      << get_name() << " in state " << _state << ".\n";
    nassertv(!verify_intervals);
  }
}

INLINE std::ostream &
operator << (std::ostream &out, const CInterval &ival) {
  ival.output(out);
  return out;
}
