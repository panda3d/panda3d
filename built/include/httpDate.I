/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file httpDate.I
 * @author drose
 * @date 2003-01-28
 */

/**
 *
 */
INLINE HTTPDate::
HTTPDate() : _time(-1) {
}

/**
 *
 */
INLINE HTTPDate::
HTTPDate(time_t time) : _time(time) {
}

/**
 *
 */
INLINE HTTPDate::
HTTPDate(const HTTPDate &copy) : _time(copy._time) {
}

/**
 *
 */
INLINE void HTTPDate::
operator = (const HTTPDate &copy) {
  _time = copy._time;
}

/**
 * Returns an HTTPDate that represents the current time and date.
 */
INLINE HTTPDate HTTPDate::
now() {
  return HTTPDate(time(nullptr));
}

/**
 * Returns true if the date is meaningful, or false if it is -1 (which
 * generally indicates the source string could not be parsed.)
 */
INLINE bool HTTPDate::
is_valid() const {
  return (_time != (time_t)(-1));
}

/**
 * Returns the date as a C time_t value.
 */
INLINE time_t HTTPDate::
get_time() const {
  return _time;
}

/**
 *
 */
INLINE bool HTTPDate::
operator == (const HTTPDate &other) const {
  return _time == other._time;
}

/**
 *
 */
INLINE bool HTTPDate::
operator != (const HTTPDate &other) const {
  return !operator == (other);
}

/**
 *
 */
INLINE bool HTTPDate::
operator < (const HTTPDate &other) const {
  return _time < other._time;
}

/**
 *
 */
INLINE bool HTTPDate::
operator > (const HTTPDate &other) const {
  return _time > other._time;
}

/**
 * Returns a number less than zero if this HTTPDate sorts before the other
 * one, greater than zero if it sorts after, or zero if they are equivalent.
 */
INLINE int HTTPDate::
compare_to(const HTTPDate &other) const {
  return (int)(_time - other._time);
}

/**
 *
 */
INLINE void HTTPDate::
operator += (int seconds) {
  _time += seconds;
}

/**
 *
 */
INLINE void HTTPDate::
operator -= (int seconds) {
  _time -= seconds;
}

/**
 *
 */
INLINE HTTPDate HTTPDate::
operator + (int seconds) const {
  return HTTPDate(_time + seconds);
}

/**
 *
 */
INLINE HTTPDate HTTPDate::
operator - (int seconds) const {
  return HTTPDate(_time - seconds);
}

/**
 *
 */
INLINE int HTTPDate::
operator - (const HTTPDate &other) const {
  return (int)(_time - other._time);
}


INLINE std::istream &
operator >> (std::istream &in, HTTPDate &date) {
  if (!date.input(in)) {
    in.clear(std::ios::failbit | in.rdstate());
  }
  return in;
}

INLINE std::ostream &
operator << (std::ostream &out, const HTTPDate &date) {
  date.output(out);
  return out;
}
