/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file trackerData.I
 * @author jason
 * @date 2000-08-04
 */

/**
 *
 */
INLINE TrackerData::
TrackerData() :
  _flags(0)
{
}

/**
 *
 */
INLINE TrackerData::
TrackerData(const TrackerData &copy) {
  (*this) = copy;
}

/**
 * Removes all data from the structure.
 */
INLINE void TrackerData::
clear() {
  _flags = 0;
}

/**
 * Indicates the time at which the position information (pos and orient) are
 * effective.  This is a time elapsed in seconds since some undefined epoch;
 * it may or may not correspond to the clock time indicated in the global
 * ClockObject.
 */
INLINE void TrackerData::
set_time(double time) {
  _time = time;
  _flags |= F_has_time;
}

/**
 * Returns true if the position information time is available.  See
 * set_time().
 */
INLINE bool TrackerData::
has_time() const {
  return (_flags & F_has_time) != 0;
}

/**
 * Returns the time at which the position information (pos and orient) are
 * effective.  It is an error to call this if has_time() does not return true.
 * See set_time().
 */
INLINE double TrackerData::
get_time() const {
  nassertr(has_time(), 0.0);
  return _time;
}

/**
 * Indicates the current position of the tracker sensor in space.  The
 * coordinate system of this position is defined by the tracker.
 */
INLINE void TrackerData::
set_pos(const LPoint3 &pos) {
  _pos = pos;
  _flags |= F_has_pos;
}

/**
 * Returns true if the current position is available.  See set_pos().
 */
INLINE bool TrackerData::
has_pos() const {
  return (_flags & F_has_pos) != 0;
}

/**
 * Returns the current position of the tracker.  It is legal to call this if
 * has_pos() returns false; in this case, the position will always be (0, 0,
 * 0).
 */
INLINE const LPoint3 &TrackerData::
get_pos() const {
  if (has_pos()) {
    return _pos;
  } else {
    static LPoint3 zero(0.0, 0.0, 0.0);
    return zero;
  }
}

/**
 * Indicates the current orientation of the tracker sensor in space.  The
 * coordinate system of this orientation is defined by the tracker, but should
 * be the same coordinate system as that reflected by set_pos().
 */
INLINE void TrackerData::
set_orient(const LOrientation &orient) {
  _orient = orient;
  _flags |= F_has_orient;
}

/**
 * Returns true if the current orientation is available.  See set_orient().
 */
INLINE bool TrackerData::
has_orient() const {
  return (_flags & F_has_orient) != 0;
}

/**
 * Returns the current orientation of the tracker.  It is legal to call this
 * if has_orient() returns false; in this case, the result is always the
 * identity orientation.
 */
INLINE const LOrientation &TrackerData::
get_orient() const {
  if (has_orient()) {
    return _orient;
  } else {
    static LOrientation ident = LOrientation::ident_quat();
    return ident;
  }
}

/**
 * Indicates the amount of elapsed time over which which the information (pos
 * and orient) were computed.  This only makes sense if the information
 * represents velocity or acceleration, rather than position.  This is an
 * elapsed time in seconds.
 */
INLINE void TrackerData::
set_dt(double dt) {
  _dt = dt;
  _flags |= F_has_dt;
}

/**
 * Returns true if the computed elapsed time is available.  See set_dt().
 */
INLINE bool TrackerData::
has_dt() const {
  return (_flags & F_has_dt) != 0;
}

/**
 * Returns the amount of elapsed time over which the information (pos and
 * orient) were computed.  It is an error to call this if has_dt() does not
 * return true.  See set_dt().
 */
INLINE double TrackerData::
get_dt() const {
  nassertr(has_dt(), 0.0);
  return _dt;
}
