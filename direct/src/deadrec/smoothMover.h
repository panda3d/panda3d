/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file smoothMover.h
 * @author drose
 * @date 2001-10-19
 */

#ifndef SMOOTHMOVER_H
#define SMOOTHMOVER_H

#include "directbase.h"
#include "luse.h"
#include "clockObject.h"
#include "circBuffer.h"
#include "nodePath.h"
#include "pdeque.h"

static const int max_position_reports = 10;
static const int max_timestamp_delays = 10;


/**
 * This class handles smoothing of sampled motion points over time, e.g.  for
 * smoothing the apparent movement of remote avatars, whose positions are sent
 * via occasional telemetry updates.
 *
 * It can operate in any of three modes: off, in which it does not smooth any
 * motion but provides the last position it was told; smoothing only, in which
 * it smooths motion information but never tries to anticipate where the
 * avatar might be going; or full prediction, in which it smooths motion as
 * well as tries to predict the avatar's position in lead of the last position
 * update.  The assumption is that all SmoothMovers in the world will be
 * operating in the same mode together.
 */
class EXPCL_DIRECT_DEADREC SmoothMover {
PUBLISHED:
  SmoothMover();
  ~SmoothMover();

  // These methods are used to specify each position update.  Call the
  // appropriate set_* function(s), as needed, and then call mark_position().
  // The return value of each function is true if the parameter value has
  // changed, or false if it remains the same as last time.
  INLINE bool set_pos(const LVecBase3 &pos);
  INLINE bool set_pos(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z);
  INLINE bool set_x(PN_stdfloat x);
  INLINE bool set_y(PN_stdfloat y);
  INLINE bool set_z(PN_stdfloat z);

  INLINE bool set_hpr(const LVecBase3 &hpr);
  INLINE bool set_hpr(PN_stdfloat h, PN_stdfloat p, PN_stdfloat r);
  INLINE bool set_h(PN_stdfloat h);
  INLINE bool set_p(PN_stdfloat p);
  INLINE bool set_r(PN_stdfloat r);

  INLINE bool set_pos_hpr(const LVecBase3 &pos, const LVecBase3 &hpr);
  INLINE bool set_pos_hpr(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z, PN_stdfloat h, PN_stdfloat p, PN_stdfloat r);

  INLINE const LPoint3 &get_sample_pos() const;
  INLINE const LVecBase3 &get_sample_hpr() const;

  INLINE void set_phony_timestamp(double timestamp = 0.0, bool period_adjust = false);

  INLINE void set_timestamp(double timestamp);

  INLINE bool has_most_recent_timestamp() const;
  INLINE double get_most_recent_timestamp() const;

  void mark_position();
  void clear_positions(bool reset_velocity);

  INLINE bool compute_smooth_position();
  bool compute_smooth_position(double timestamp);
  bool get_latest_position();

  INLINE const LPoint3 &get_smooth_pos() const;
  INLINE const LVecBase3 &get_smooth_hpr() const;

  INLINE void apply_smooth_pos(NodePath &node) const;
  INLINE void apply_smooth_pos_hpr(NodePath &pos_node, NodePath &hpr_node) const;
  INLINE void apply_smooth_hpr(NodePath &node) const;

  INLINE void compute_and_apply_smooth_pos(NodePath &node);
  INLINE void compute_and_apply_smooth_pos_hpr(NodePath &pos_node, NodePath &hpr_node);
  INLINE void compute_and_apply_smooth_hpr(NodePath &hpr_node);

  INLINE PN_stdfloat get_smooth_forward_velocity() const;
  INLINE PN_stdfloat get_smooth_lateral_velocity() const;
  INLINE PN_stdfloat get_smooth_rotational_velocity() const;
  INLINE const LVecBase3 &get_forward_axis() const;

  void handle_wrt_reparent(NodePath &old_parent, NodePath &new_parent);

  enum SmoothMode {
    SM_off,
    SM_on,
    // We might conceivably add more kinds of smooth modes later, for
    // instance, SM_spline.
  };
  enum PredictionMode {
    PM_off,
    PM_on,
    // Similarly for other kinds of prediction modes.  I don't know why,
    // though; linear interpolation seems to work pretty darn well.
  };

  INLINE void set_smooth_mode(SmoothMode mode);
  INLINE SmoothMode get_smooth_mode();

  INLINE void set_prediction_mode(PredictionMode mode);
  INLINE PredictionMode get_prediction_mode();

  INLINE void set_delay(double delay);
  INLINE double get_delay();

  INLINE void set_accept_clock_skew(bool flag);
  INLINE bool get_accept_clock_skew();

  INLINE void set_max_position_age(double age);
  INLINE double get_max_position_age();

  INLINE void set_expected_broadcast_period(double period);
  INLINE double get_expected_broadcast_period();

  INLINE void set_reset_velocity_age(double age);
  INLINE double get_reset_velocity_age();

  INLINE void set_directional_velocity(bool flag);
  INLINE bool get_directional_velocity();

  INLINE void set_default_to_standing_still(bool flag);
  INLINE bool get_default_to_standing_still();

  void output(std::ostream &out) const;
  void write(std::ostream &out) const;

private:
  void set_smooth_pos(const LPoint3 &pos, const LVecBase3 &hpr,
                      double timestamp);
  void linear_interpolate(int point_before, int point_after, double timestamp);
  void compute_velocity(const LVector3 &pos_delta,
                        const LVecBase3 &hpr_delta,
                        double age);

  void record_timestamp_delay(double timestamp);
  INLINE double get_avg_timestamp_delay() const;

public:
  // This internal class is declared public to work around compiler issues.
  class SamplePoint {
  public:
    LPoint3 _pos;
    LVecBase3 _hpr;
    double _timestamp;
  };

private:
  SamplePoint _sample;

  LPoint3 _smooth_pos;
  LVecBase3 _smooth_hpr;
  LVector3 _forward_axis;
  double _smooth_timestamp;
  bool _smooth_position_known;
  bool _smooth_position_changed;
  bool _computed_forward_axis;

  double _smooth_forward_velocity;
  double _smooth_lateral_velocity;
  double _smooth_rotational_velocity;

  bool _has_most_recent_timestamp;
  double _most_recent_timestamp;

  // typedef CircBuffer<SamplePoint, max_position_reports> Points;
  typedef pdeque<SamplePoint> Points;
  Points _points;
  int _last_point_before;
  int _last_point_after;

  // This array is used to record the average delay in receiving timestamps
  // from a particular client, in milliseconds.  This value will measure both
  // the latency and clock skew from that client, allowing us to present
  // smooth motion in spite of extreme latency or poor clock synchronization.
  typedef CircBuffer<int, max_timestamp_delays> TimestampDelays;
  TimestampDelays _timestamp_delays;
  int _net_timestamp_delay;
  double _last_heard_from;

  SmoothMode _smooth_mode;
  PredictionMode _prediction_mode;
  double _delay;
  bool _accept_clock_skew;
  double _max_position_age;
  double _expected_broadcast_period;
  double _reset_velocity_age;
  bool _directional_velocity;
  bool _default_to_standing_still;
};

#include "smoothMover.I"

#endif
