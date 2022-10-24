/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file smoothMover.I
 * @author drose
 * @date 2001-10-19
 */

/**
 * Specifies the position of the SmoothMover at a particular time in the past.
 * When mark_position() is called, this will be recorded (along with hpr and
 * timestamp) in a position report, which will then be used along with all
 * other position reports to determine the smooth position at any particular
 * instant.
 *
 * The return value is true if any parameter has changed since the last call
 * to set_pos(), or false if they are the same.
 */
INLINE bool SmoothMover::
set_pos(const LVecBase3 &pos) {
  return set_pos(pos[0], pos[1], pos[2]);
}

/**
 * Specifies the position of the SmoothMover at a particular time in the past.
 * When mark_position() is called, this will be recorded (along with hpr and
 * timestamp) in a position report, which will then be used along with all
 * other position reports to determine the smooth position at any particular
 * instant.
 *
 * The return value is true if any parameter has changed since the last call
 * to set_pos(), or false if they are the same.
 */
INLINE bool SmoothMover::
set_pos(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z) {
  bool x_changed = set_x(x);
  bool y_changed = set_y(y);
  bool z_changed = set_z(z);
  return x_changed || y_changed || z_changed;
}

/**
 * Sets the X position only.  See set_pos().
 */
INLINE bool SmoothMover::
set_x(PN_stdfloat x) {
  bool result = (x != _sample._pos[0]);
  /*
  if (deadrec_cat.is_debug()) {
    deadrec_cat.debug() << "set_x " << x << "\n";
  }
  */
  _sample._pos[0] = x;
  return result;
}

/**
 * Sets the Y position only.  See set_pos().
 */
INLINE bool SmoothMover::
set_y(PN_stdfloat y) {
  bool result = (y != _sample._pos[1]);
  /*
  if (deadrec_cat.is_debug()) {
    deadrec_cat.debug() << "set_y " << y << "\n";
  }
  */
  _sample._pos[1] = y;
  return result;
}

/**
 * Sets the Z position only.  See set_pos().
 */
INLINE bool SmoothMover::
set_z(PN_stdfloat z) {
  bool result = (z != _sample._pos[2]);
  /*
  if (deadrec_cat.is_debug()) {
    deadrec_cat.debug() << "set_z " << z << "\n";
  }
  */
  _sample._pos[2] = z;
  return result;
}

/**
 * Specifies the orientation of the SmoothMover at a particular time in the
 * past.  When mark_position() is called, this will be recorded (along with
 * hpr and timestamp) in a position report, which will then be used along with
 * all other position reports to determine the smooth position at any
 * particular instant.
 *
 * The return value is true if any parameter has changed since the last call
 * to set_hpr(), or false if they are the same.
 */
INLINE bool SmoothMover::
set_hpr(const LVecBase3 &hpr) {
  return set_hpr(hpr[0], hpr[1], hpr[2]);
}

/**
 * Specifies the orientation of the SmoothMover at a particular time in the
 * past.  When mark_position() is called, this will be recorded (along with
 * hpr and timestamp) in a position report, which will then be used along with
 * all other position reports to determine the smooth position at any
 * particular instant.
 *
 * The return value is true if any parameter has changed since the last call
 * to set_hpr(), or false if they are the same.
 */
INLINE bool SmoothMover::
set_hpr(PN_stdfloat h, PN_stdfloat p, PN_stdfloat r) {
  bool h_changed = set_h(h);
  bool p_changed = set_p(p);
  bool r_changed = set_r(r);
  return h_changed || p_changed || r_changed;
}

/**
 * Sets the heading only.  See set_hpr().
 */
INLINE bool SmoothMover::
set_h(PN_stdfloat h) {
  bool result = (h != _sample._hpr[0]);
  /*
  if (deadrec_cat.is_debug()) {
    deadrec_cat.debug() << "set_h " << h << "\n";
  }
  */
  _sample._hpr[0] = h;
  return result;
}

/**
 * Sets the pitch only.  See set_hpr().
 */
INLINE bool SmoothMover::
set_p(PN_stdfloat p) {
  bool result = (p != _sample._hpr[1]);
  /*
  if (deadrec_cat.is_debug()) {
    deadrec_cat.debug() << "set_p " << p << "\n";
  }
  */
  _sample._hpr[1] = p;
  return result;
}

/**
 * Sets the roll only.  See set_hpr().
 */
INLINE bool SmoothMover::
set_r(PN_stdfloat r) {
  bool result = (r != _sample._hpr[2]);
  /*
  if (deadrec_cat.is_debug()) {
    deadrec_cat.debug() << "set_r " << r << "\n";
  }
  */
  _sample._hpr[2] = r;
  return result;
}

/**
 * Specifies the position and orientation of the SmoothMover at a particular
 * time in the past.  When mark_position() is called, this will be recorded
 * (along with timestamp) in a position report, which will then be used along
 * with all other position reports to determine the smooth position at any
 * particular instant.
 *
 * The return value is true if any parameter has changed since the last call
 * to set_pos_hpr(), or false if they are the same.
 */
INLINE bool SmoothMover::
set_pos_hpr(const LVecBase3 &pos, const LVecBase3 &hpr) {
  bool pos_changed = set_pos(pos);
  bool hpr_changed = set_hpr(hpr);
  return pos_changed || hpr_changed;
}

/**
 * Specifies the position of the SmoothMover at a particular time in the past.
 * When mark_position() is called, this will be recorded (along with
 * timestamp) in a position report, which will then be used along with all
 * other position reports to determine the smooth position at any particular
 * instant.
 *
 * The return value is true if any parameter has changed since the last call
 * to set_pos_hpr(), or false if they are the same.
 */
INLINE bool SmoothMover::
set_pos_hpr(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z, PN_stdfloat h, PN_stdfloat p, PN_stdfloat r) {
  bool pos_changed = set_pos(x, y, z);
  bool hpr_changed = set_hpr(h, p, r);
  return pos_changed || hpr_changed;
}

/**
 * Returns the current position of the working sample point.  This position is
 * updated periodically by set_x(), set_y(), etc., and its current value is
 * copied to the sample point table when mark_position() is called.
 */
INLINE const LPoint3 &SmoothMover::
get_sample_pos() const {
  return _sample._pos;
}

/**
 * Returns the current orientation of the working sample point.  This
 * orientation is updated periodically by set_h(), set_p(), etc., and its
 * current value is copied to the sample point table when mark_position() is
 * called.
 */
INLINE const LVecBase3 &SmoothMover::
get_sample_hpr() const {
  return _sample._hpr;
}

/**
 * Lies and specifies that the current position report was received now.  This
 * is usually used for very old position reports for which we're not sure of
 * the actual receipt time.
 */
INLINE void SmoothMover::
set_phony_timestamp(double timestamp, bool period_adjust) {
  double now = ClockObject::get_global_clock()->get_frame_time();
  if (timestamp != 0.0)
    // we were given a specific timestamp to use
    now = timestamp;

  // adjust by _delay when creating the timestamp since other timestamps
  // received from network updates are adjusted by this
  if (period_adjust) {
    _sample._timestamp = now - _expected_broadcast_period;
  }
  else
    _sample._timestamp = now;

  _has_most_recent_timestamp = true;
  _most_recent_timestamp = _sample._timestamp;

}

/**
 * Specifies the time that the current position report applies.  This should
 * be called, along with set_pos() and set_hpr(), before a call to
 * mark_position().
 */
INLINE void SmoothMover::
set_timestamp(double timestamp) {
  /*
  if (deadrec_cat.is_debug()) {
    deadrec_cat.debug() << "set_timestamp " << timestamp << "\n";
  }
  */
  _sample._timestamp = timestamp;
  _has_most_recent_timestamp = true;
  _most_recent_timestamp = timestamp;
  record_timestamp_delay(timestamp);
}

/**
 * Returns true if we have most recently recorded timestamp
 */
INLINE bool SmoothMover::
has_most_recent_timestamp() const {
  return _has_most_recent_timestamp;
}

/**
 * Returns most recently recorded timestamp
 */
INLINE double SmoothMover::
get_most_recent_timestamp() const {
  return _most_recent_timestamp;
}

/**
 * Computes the smoothed position (and orientation) of the mover at the
 * indicated point in time, based on the previous position reports.  After
 * this call has been made, get_smooth_pos() etc.  may be called to retrieve
 * the smoothed position.
 *
 * With no parameter, the function uses ClockObject::get_frame_time() as the
 * default time.
 */
INLINE bool SmoothMover::
compute_smooth_position() {
  return compute_smooth_position(ClockObject::get_global_clock()->get_frame_time());
}

/**
 * Returns the smoothed position as computed by a previous call to
 * compute_smooth_position().
 */
INLINE const LPoint3 &SmoothMover::
get_smooth_pos() const {
  return _smooth_pos;
}

/**
 * Returns the smoothed position as computed by a previous call to
 * compute_smooth_position().
 */
INLINE const LVecBase3 &SmoothMover::
get_forward_axis() const {
  return _forward_axis;
}

/**
 * Returns the smoothed orientation as computed by a previous call to
 * compute_smooth_position().
 */
INLINE const LVecBase3 &SmoothMover::
get_smooth_hpr() const {
  return _smooth_hpr;
}

/**
 * Applies the smoothed position to the indicated NodePath.  This is
 * equivalent to calling node.set_pos(smooth_mover->get_smooth_pos()).  It
 * exists as an optimization only, to avoid the overhead of passing the return
 * value through Python.
 */
INLINE void SmoothMover::
apply_smooth_pos(NodePath &node) const {
  node.set_pos(get_smooth_pos());
}

/**
 * Applies the smoothed position and orientation to the indicated NodePath.
 * This is equivalent to calling
 * node.set_pos_hpr(smooth_mover->get_smooth_pos(),
 * smooth_mover->get_smooth_hpr()).  It exists as an optimization only, to
 * avoid the overhead of passing the return value through Python.
 */
INLINE void SmoothMover::
apply_smooth_pos_hpr(NodePath &pos_node, NodePath &hpr_node) const {
  pos_node.set_pos(get_smooth_pos());
  hpr_node.set_hpr(get_smooth_hpr());
}

/**
 * Applies the smoothed orientation to the indicated NodePath.  This is
 * equivalent to calling node.set_hpr(smooth_mover->get_smooth_hpr()).  It
 * exists as an optimization only, to avoid the overhead of passing the return
 * value through Python.
 */
INLINE void SmoothMover::
apply_smooth_hpr(NodePath &node) const {
  node.set_hpr(get_smooth_hpr());
}

/**
 * A further optimization to reduce Python calls.  This computes the smooth
 * position and applies it to the indicated node in one call.
 */
INLINE void SmoothMover::
compute_and_apply_smooth_pos(NodePath &node) {
  if (compute_smooth_position()) {
    apply_smooth_pos(node);
  }
}

/**
 * A further optimization to reduce Python calls.  This computes the smooth
 * position and applies it to the indicated node or nodes in one call.  The
 * pos_node and hpr_node might be the same NodePath.
 */
INLINE void SmoothMover::
compute_and_apply_smooth_pos_hpr(NodePath &pos_node, NodePath &hpr_node) {
  if (compute_smooth_position()) {
    apply_smooth_pos(pos_node);
    apply_smooth_hpr(hpr_node);
  }
}

/**
 * A further optimization to reduce Python calls.  This computes the smooth
 * position and applies it to the indicated node or nodes in one call.  The
 * pos_node and hpr_node might be the same NodePath.
 */
INLINE void SmoothMover::
compute_and_apply_smooth_hpr(NodePath &hpr_node) {
  if (compute_smooth_position()) {
    apply_smooth_hpr(hpr_node);
  }
}

/**
 * Returns the speed at which the avatar is moving, in feet per second, along
 * its own forward axis (after applying the avatar's hpr).  This will be a
 * positive number if the avatar is moving forward, and a negative number if
 * it is moving backward.
 */
INLINE PN_stdfloat SmoothMover::
get_smooth_forward_velocity() const {
  return _smooth_forward_velocity;
}

/**
 * Returns the speed at which the avatar is moving, in feet per second, along
 * its own lateral axis (after applying the avatar's hpr).  This will be a
 * positive number if the avatar is moving right, and a negative number if it
 * is moving left.
 */
INLINE PN_stdfloat SmoothMover::
get_smooth_lateral_velocity() const {
  return _smooth_lateral_velocity;
}

/**
 * Returns the speed at which the avatar is rotating in the horizontal plane
 * (i.e.  heading), in degrees per second.  This may be positive or negative,
 * according to the direction of rotation.
 */
INLINE PN_stdfloat SmoothMover::
get_smooth_rotational_velocity() const {
  return _smooth_rotational_velocity;
}

/**
 * Sets the smoothing mode of all SmoothMovers in the world.  If this is
 * SM_off, no smoothing or prediction will be performed, and get_smooth_pos()
 * will simply return the position last set by mark_position().
 */
INLINE void SmoothMover::
set_smooth_mode(SmoothMover::SmoothMode mode) {
  _smooth_mode = mode;
}

/**
 * Returns the smoothing mode of all SmoothMovers in the world.  See
 * set_smooth_mode().
 */
INLINE SmoothMover::SmoothMode SmoothMover::
get_smooth_mode() {
  return _smooth_mode;
}

/**
 * Sets the predictioning mode of all SmoothMovers in the world.  If this is
 * PM_off, no prediction will be performed, but smoothing might still be
 * performed.
 */
INLINE void SmoothMover::
set_prediction_mode(SmoothMover::PredictionMode mode) {
  _prediction_mode = mode;
}

/**
 * Returns the predictioning mode of all SmoothMovers in the world.  See
 * set_prediction_mode().
 */
INLINE SmoothMover::PredictionMode SmoothMover::
get_prediction_mode() {
  return _prediction_mode;
}

/**
 * Sets the amount of time, in seconds, to delay the computed position of a
 * SmoothMover.  This is particularly useful when the prediction mode is off,
 * because it can allow the apparent motion of an avatar to appear smooth
 * without relying on prediction, at the cost of introducing additional lag in
 * the avatar's apparent position.
 */
INLINE void SmoothMover::
set_delay(double delay) {
  _delay = delay;
}

/**
 * Returns the amount of time, in seconds, to delay the computed position of a
 * SmoothMover.  See set_delay().
 */
INLINE double SmoothMover::
get_delay() {
  return _delay;
}

/**
 * Sets the 'accept clock skew' flag.  When this flag is true, clock skew from
 * the other clients will be tolerated by delaying each smooth mover's
 * position an additional amount, on top of that specified by set_delay(),
 * based on the measured average latency for timestamp messages received by
 * the client.
 *
 * In this way, if the other client has significant clock skew with respect to
 * our clock, it will be evident as a large positive or negative average
 * latency for timestamps.  By subtracting out this average latency, we
 * compensate for poor clock sync.
 */
INLINE void SmoothMover::
set_accept_clock_skew(bool flag) {
  _accept_clock_skew = flag;
}

/**
 * Returns the current state of the 'accept clock skew' flag.  See
 * set_accept_clock_skew().
 */
INLINE bool SmoothMover::
get_accept_clock_skew() {
  return _accept_clock_skew;
}

/**
 * Sets the maximum amount of time a position is allowed to remain unchanged
 * before assuming it represents the avatar actually standing still.
 */
INLINE void SmoothMover::
set_max_position_age(double age) {
  _max_position_age = age;
}

/**
 * Returns the maximum amount of time a position is allowed to remain
 * unchanged before assuming it represents the avatar actually standing still.
 */
INLINE double SmoothMover::
get_max_position_age() {
  return _max_position_age;
}

/**
 * Sets the interval at which we expect the SmoothNodes to broadcast their
 * position, in elapsed seconds.  This controls the length of time we assume
 * the object has truly stopped, when we receive a long sequence of no
 * updates.
 */
INLINE void SmoothMover::
set_expected_broadcast_period(double period) {
  _expected_broadcast_period = period;
}

/**
 * Returns the interval at which we expect the SmoothNodes to broadcast their
 * position, in elapsed seconds.  See set_expected_broadcast_period().
 */
INLINE double SmoothMover::
get_expected_broadcast_period() {
  return _expected_broadcast_period;
}

/**
 * Sets the amount of time that should elapse after the last position report
 * before the velocity is reset to 0.  This is similar to max_position_age,
 * but it is only used to determine the resetting of the reported velocity.
 * It should always be greater than or equal to max_position_age.
 */
INLINE void SmoothMover::
set_reset_velocity_age(double age) {
  _reset_velocity_age = age;
}

/**
 * Returns the amount of time that should elapse after the last position
 * report before the velocity is reset to 0.  See set_reset_velocity_age().
 */
INLINE double SmoothMover::
get_reset_velocity_age() {
  return _reset_velocity_age;
}

/**
 * Sets the flag that indicates whether the avatar's direction is considered
 * in computing the velocity.  When this is true, velocity is automatically
 * decomposed into a forward and a lateral velocity (and both may be positive
 * or negative); when it is false, all velocity is always returned as forward
 * velocity (and it is always positive).
 */
INLINE void SmoothMover::
set_directional_velocity(bool flag) {
  _directional_velocity = flag;
}

/**
 * Returns the current state of the 'directional velocity' flag.  See
 * set_directional_velocity().
 */
INLINE bool SmoothMover::
get_directional_velocity() {
  return _directional_velocity;
}

/**
 * Sets the flag that indicates whether to assume that the node stopped moving
 * during periods when we don't get enough position updates.  If true, the
 * object will stand still momentarily.  If false, the object will
 * continuously lerp between the position updates that we did get.
 */
INLINE void SmoothMover::
set_default_to_standing_still(bool flag) {
  _default_to_standing_still = flag;
}

/**
 * Returns the current state of the 'default to standing still' flag.  See
 * set_default_to_standing_still().
 */
INLINE bool SmoothMover::
get_default_to_standing_still() {
  return _default_to_standing_still;
}

/**
 * Returns the average delay observed in the last n timestamps received from
 * this client, in seconds.  This number represents the combination of the
 * network lag from this client, as well as the client's clock skew relative
 * to our clock.  It could be negative if the client's clock is running faster
 * than our clock.
 */
INLINE double SmoothMover::
get_avg_timestamp_delay() const {
  nassertr(!_timestamp_delays.empty(), 0.0);
  return (double)_net_timestamp_delay / (double)_timestamp_delays.size() / 1000.0;
}
