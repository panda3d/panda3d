/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cMetaInterval.I
 * @author drose
 * @date 2002-08-27
 */

/**
 * Indicates the precision with which time measurements are compared.  For
 * numerical accuracy, all floating-point time values are converted to integer
 * values internally by scaling by the precision factor.  The larger the
 * number given here, the smaller the delta of time that can be
 * differentiated; the limit is the maximum integer that can be represented in
 * the system.
 */
INLINE void CMetaInterval::
set_precision(double precision) {
  _precision = precision;
  mark_dirty();
}

/**
 * Returns the precision with which time measurements are compared.  See
 * set_precision().
 */
INLINE double CMetaInterval::
get_precision() const {
  return _precision;
}

/**
 * Returns the number of interval and push/pop definitions that have been
 * added to the meta interval.
 */
INLINE int CMetaInterval::
get_num_defs() const {
  return (int)_defs.size();
}

/**
 * Returns the type of the nth interval definition that has been added.
 */
INLINE CMetaInterval::DefType CMetaInterval::
get_def_type(int n) const {
  nassertr(n >= 0 && n < (int)_defs.size(), DT_c_interval);
  return _defs[n]._type;
}

/**
 * Return the CInterval pointer associated with the nth interval definition.
 * It is only valid to call this if get_def_type(n) returns DT_c_interval.
 */
INLINE CInterval *CMetaInterval::
get_c_interval(int n) const {
  nassertr(n >= 0 && n < (int)_defs.size(), nullptr);
  nassertr(_defs[n]._type == DT_c_interval, nullptr);
  return _defs[n]._c_interval;
}

/**
 * Return the external interval index number associated with the nth interval
 * definition.  It is only valid to call this if get_def_type(n) returns
 * DT_ext_index.
 */
INLINE int CMetaInterval::
get_ext_index(int n) const {
  nassertr(n >= 0 && n < (int)_defs.size(), -1);
  nassertr(_defs[n]._type == DT_ext_index, -1);
  return _defs[n]._ext_index;
}

/**
 * Returns true if a recent call to priv_initialize(), priv_step(), or
 * priv_finalize() has left some external intervals ready to play.  If this
 * returns true, call get_event_index(), get_event_t(), and pop_event() to
 * retrieve the relevant information.
 */
INLINE bool CMetaInterval::
is_event_ready() {
  return service_event_queue();
}

/**
 * If a previous call to is_event_ready() returned true, this returns the
 * index number (added via add_event_index()) of the external interval that
 * needs to be played.
 */
INLINE int CMetaInterval::
get_event_index() const {
  nassertr(!_event_queue.empty(), -1);
  const EventQueueEntry &entry = _event_queue.front();
  const IntervalDef &def = _defs[entry._n];
  nassertr(def._type == DT_ext_index, -1);
  return def._ext_index;
}

/**
 * If a previous call to is_event_ready() returned true, this returns the t
 * value that should be fed to the given interval.
 */
INLINE double CMetaInterval::
get_event_t() const {
  nassertr(!_event_queue.empty(), 0.0f);
  return int_to_double_time(_event_queue.front()._time);
}

/**
 * If a previous call to is_event_ready() returned true, this returns the type
 * of the event (initialize, step, finalize, etc.) for the given interval.
 */
INLINE CInterval::EventType CMetaInterval::
get_event_type() const {
  nassertr(!_event_queue.empty(), ET_step);
  return _event_queue.front()._event_type;
}

/**
 * Converts from an external double time value or offset in seconds to an
 * internal integer value or offset.
 */
INLINE int CMetaInterval::
double_to_int_time(double t) const {
  // Use floor() just in case there are negative values involved.
  return (int)floor(t * _precision + 0.5);
}

/**
 * Converts from an internal integer time value or offset to an external
 * double time value or offset in seconds.
 */
INLINE double CMetaInterval::
int_to_double_time(int time) const {
  return (double)time / _precision;
}

/**
 *
 */
INLINE CMetaInterval::PlaybackEvent::
PlaybackEvent(int time, int n,
              CMetaInterval::PlaybackEventType type) :
  _time(time),
  _n(n),
  _type(type)
{
  _begin_event = this;
}

/**
 *
 */
INLINE bool CMetaInterval::PlaybackEvent::
operator < (const CMetaInterval::PlaybackEvent &other) const {
  return _time < other._time;
}

/**
 *
 */
INLINE CMetaInterval::EventQueueEntry::
EventQueueEntry(int n, CInterval::EventType event_type, int time) :
  _n(n),
  _event_type(event_type),
  _time(time)
{
}
