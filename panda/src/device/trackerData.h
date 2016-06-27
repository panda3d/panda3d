/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file trackerData.h
 * @author jason
 * @date 2000-08-04
 */

#ifndef TRACKERDATA_H
#define TRACKERDATA_H

#include "pandabase.h"
#include "luse.h"

/**
 * Stores the kinds of data that a tracker might output.
 */
class EXPCL_PANDA_DEVICE TrackerData {
public:
  INLINE TrackerData();
  INLINE TrackerData(const TrackerData &copy);
  void operator = (const TrackerData &copy);

  INLINE void clear();

  INLINE void set_time(double time);
  INLINE bool has_time() const;
  INLINE double get_time() const;

  INLINE void set_pos(const LPoint3 &pos);
  INLINE bool has_pos() const;
  INLINE const LPoint3 &get_pos() const;

  INLINE void set_orient(const LOrientation &orient);
  INLINE bool has_orient() const;
  INLINE const LOrientation &get_orient() const;

  INLINE void set_dt(double dt);
  INLINE bool has_dt() const;
  INLINE double get_dt() const;

PUBLISHED:
  MAKE_PROPERTY(time, get_time, set_time);
  MAKE_PROPERTY(pos, get_pos, set_pos);
  MAKE_PROPERTY(orient, get_orient, set_orient);
  MAKE_PROPERTY(dt, get_dt, set_dt);

private:
  enum Flags {
    F_has_time    = 0x0001,
    F_has_pos     = 0x0002,
    F_has_orient  = 0x0004,
    F_has_dt      = 0x0008,
  };

  int _flags;

  double _time;
  LPoint3 _pos;
  LOrientation _orient;
  double _dt;
};

#include "trackerData.I"

#endif
