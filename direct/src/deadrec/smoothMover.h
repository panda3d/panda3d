// Filename: smoothMover.h
// Created by:  drose (19Oct01)
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

#ifndef SMOOTHMOVER_H
#define SMOOTHMOVER_H

#include "directbase.h"
#include "luse.h"
#include "clockObject.h"
#include "circBuffer.h"

static const int max_position_reports = 10;

////////////////////////////////////////////////////////////////////
//       Class : SmoothMover
// Description : This class handles smoothing of sampled motion points
//               over time, e.g. for smoothing the apparent movement
//               of remote avatars, whose positions are sent via
//               occasional telemetry updates.
//
//               It can operate in any of three modes: off, in which
//               it does not smooth any motion but provides the last
//               position it was told; smoothing only, in which it
//               smooths motion information but never tries to
//               anticipate where the avatar might be going; or full
//               prediction, in which it smooths motion as well as
//               tries to predict the avatar's position in lead of the
//               last position update.  The assumption is that all
//               SmoothMovers in the world will be operating in the
//               same mode together.
////////////////////////////////////////////////////////////////////
class EXPCL_DIRECT SmoothMover {
PUBLISHED:
  SmoothMover();
  ~SmoothMover();

  // This method is just used to specify a scale which is only used
  // when composing the matrix for return by get_smooth_mat().  It
  // might change from time to time, but it is not smoothed.
  INLINE bool set_scale(float sx, float sy, float sz);
  INLINE bool set_sx(float sx);
  INLINE bool set_sy(float sy);
  INLINE bool set_sz(float sz);

  // These methods are used to specify each position update.  Call the
  // appropriate set_* function(s), as needed, and then call
  // mark_position().  The return value of each function is true if
  // the parameter value has changed, or false if it remains the same
  // as last time.
  INLINE bool set_pos(float x, float y, float z);
  INLINE bool set_x(float x);
  INLINE bool set_y(float y);
  INLINE bool set_z(float z);

  INLINE bool set_hpr(float h, float p, float r);
  INLINE bool set_h(float h);
  INLINE bool set_p(float p);
  INLINE bool set_r(float r);

  INLINE void set_timestamp();
  INLINE void set_timestamp(double timestamp);

  void mark_position();
  void clear_positions(bool reset_velocity);

  INLINE void compute_smooth_position();
  void compute_smooth_position(double timestamp);

  INLINE const LPoint3f &get_smooth_pos() const;
  INLINE const LVecBase3f &get_smooth_hpr() const;
  INLINE const LMatrix4f &get_smooth_mat();

  INLINE float get_smooth_forward_velocity() const;
  INLINE float get_smooth_rotational_velocity() const;


  // These static methods control the global properties of all
  // SmoothMovers.
  enum SmoothMode {
    SM_off,
    SM_on,
  };
  enum PredictionMode {
    PM_off,
    PM_on,
  };

  INLINE static void set_smooth_mode(SmoothMode mode);
  INLINE static SmoothMode get_smooth_mode();

  INLINE static void set_prediction_mode(PredictionMode mode);
  INLINE static PredictionMode get_prediction_mode();

  INLINE static void set_delay(double delay); 
  INLINE static double get_delay(); 

  INLINE static void set_max_position_age(double age); 
  INLINE static double get_max_position_age(); 

  INLINE static void set_reset_velocity_age(double age); 
  INLINE static double get_reset_velocity_age(); 

  void output(ostream &out) const;
  void write(ostream &out) const;

private:
  void set_smooth_pos(const LPoint3f &pos, const LVecBase3f &hpr,
                      double timestamp);
  void compose_smooth_mat();
  void linear_interpolate(int point_before, int point_after, double timestamp);
  void compute_velocity(const LVector3f &pos_delta, 
                        const LVecBase3f &hpr_delta,
                        double age);

  enum Flags {
    F_got_timestamp  = 0x0001,
  };

  LVecBase3f _scale;

  class SamplePoint {
  public:
    LPoint3f _pos;
    LVecBase3f _hpr;
    double _timestamp;
    int _flags;
  };
  SamplePoint _sample;

  LPoint3f _smooth_pos;
  LVecBase3f _smooth_hpr;
  LMatrix4f _smooth_mat;
  double _smooth_timestamp;
  bool _smooth_position_known;
  bool _computed_smooth_mat;

  double _smooth_forward_velocity;
  double _smooth_rotational_velocity;

  typedef CircBuffer<SamplePoint, max_position_reports> Points;
  Points _points;
  int _last_point_before;
  int _last_point_after;

  static SmoothMode _smooth_mode;
  static PredictionMode _prediction_mode;
  static double _delay;
  static double _max_position_age;
  static double _reset_velocity_age;
};

#include "smoothMover.I"

#endif
