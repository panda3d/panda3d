// Filename: trackerData.h
// Created by:  jason (04Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef TRACKERDATA_H
#define TRACKERDATA_H

#include <pandabase.h>
#include <luse.h>

////////////////////////////////////////////////////////////////////
//       Class : TrackerData
// Description : Stores the kinds of data that a tracker might output.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA TrackerData {
public:
  INLINE TrackerData();
  INLINE TrackerData(const TrackerData &copy);
  void operator = (const TrackerData &copy);

  INLINE void clear();

  INLINE void set_time(double time);
  INLINE bool has_time() const;
  INLINE double get_time() const;

  INLINE void set_pos(const LPoint3f &pos);
  INLINE bool has_pos() const;
  INLINE const LPoint3f &get_pos() const;

  INLINE void set_orient(const LOrientationf &orient);
  INLINE bool has_orient() const;
  INLINE const LOrientationf &get_orient() const;

  INLINE void set_dt(double dt);
  INLINE bool has_dt() const;
  INLINE double get_dt() const;

private:
  enum Flags {
    F_has_time    = 0x0001,
    F_has_pos     = 0x0002,
    F_has_orient  = 0x0004,
    F_has_dt      = 0x0008,
  };

  int _flags;

  double _time;
  LPoint3f _pos;
  LOrientationf _orient;
  double _dt;
};

#include "trackerData.I"

#endif
