// Filename: smoothMover.I
// Created by:  drose (19Oct01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: SmoothMover::set_scale
//       Access: Published
//  Description: Specifies the current scale that should be applied to
//               the transform.  This is not smoothed along with pos
//               and hpr, but rather takes effect immediately; it is
//               only here at all so we can return a complete matrix
//               in get_smooth_mat().
////////////////////////////////////////////////////////////////////
INLINE bool SmoothMover::
set_scale(const LVecBase3f &scale) {
  return set_sx(scale[0]) | set_sy(scale[1]) | set_sz(scale[2]);
}

////////////////////////////////////////////////////////////////////
//     Function: SmoothMover::set_scale
//       Access: Published
//  Description: Specifies the current scale that should be applied to
//               the transform.  This is not smoothed along with pos
//               and hpr, but rather takes effect immediately; it is
//               only here at all so we can return a complete matrix
//               in get_smooth_mat().
////////////////////////////////////////////////////////////////////
INLINE bool SmoothMover::
set_scale(float sx, float sy, float sz) {
  return set_sx(sx) | set_sy(sy) | set_sz(sz);
}

////////////////////////////////////////////////////////////////////
//     Function: SmoothMover::set_sx
//       Access: Published
//  Description: Sets the X-axis scale only.  See set_scale().
////////////////////////////////////////////////////////////////////
INLINE bool SmoothMover::
set_sx(float sx) {
  bool result = (sx != _scale[0]);
  _scale[0] = sx;
  _computed_smooth_mat = _computed_smooth_mat && !result;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: SmoothMover::set_sy
//       Access: Published
//  Description: Sets the Y-axis scale only.  See set_scale().
////////////////////////////////////////////////////////////////////
INLINE bool SmoothMover::
set_sy(float sy) {
  bool result = (sy != _scale[1]);
  _scale[1] = sy;
  _computed_smooth_mat = _computed_smooth_mat && !result;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: SmoothMover::set_sz
//       Access: Published
//  Description: Sets the Z-axis scale only.  See set_scale().
////////////////////////////////////////////////////////////////////
INLINE bool SmoothMover::
set_sz(float sz) {
  bool result = (sz != _scale[2]);
  _scale[2] = sz;
  _computed_smooth_mat = _computed_smooth_mat && !result;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: SmoothMover::set_pos
//       Access: Published
//  Description: Specifies the position of the SmoothMover at a
//               particular time in the past.  When mark_position() is
//               called, this will be recorded (along with hpr and
//               timestamp) in a position report, which will then be
//               used along with all other position reports to
//               determine the smooth position at any particular
//               instant.
//
//               The return value is true if any parameter has changed
//               since the last call to set_pos(), or false if they
//               are the same.
////////////////////////////////////////////////////////////////////
INLINE bool SmoothMover::
set_pos(const LVecBase3f &pos) {
  return set_x(pos[0]) | set_y(pos[1]) | set_z(pos[2]);
}

////////////////////////////////////////////////////////////////////
//     Function: SmoothMover::set_pos
//       Access: Published
//  Description: Specifies the position of the SmoothMover at a
//               particular time in the past.  When mark_position() is
//               called, this will be recorded (along with hpr and
//               timestamp) in a position report, which will then be
//               used along with all other position reports to
//               determine the smooth position at any particular
//               instant.
//
//               The return value is true if any parameter has changed
//               since the last call to set_pos(), or false if they
//               are the same.
////////////////////////////////////////////////////////////////////
INLINE bool SmoothMover::
set_pos(float x, float y, float z) {
  return set_x(x) | set_y(y) | set_z(z);
}

////////////////////////////////////////////////////////////////////
//     Function: SmoothMover::set_x
//       Access: Published
//  Description: Sets the X position only.  See set_pos().
////////////////////////////////////////////////////////////////////
INLINE bool SmoothMover::
set_x(float x) {
  bool result = (x != _sample._pos[0]);
  _sample._pos[0] = x;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: SmoothMover::set_y
//       Access: Published
//  Description: Sets the Y position only.  See set_pos().
////////////////////////////////////////////////////////////////////
INLINE bool SmoothMover::
set_y(float y) {
  bool result = (y != _sample._pos[1]);
  _sample._pos[1] = y;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: SmoothMover::set_z
//       Access: Published
//  Description: Sets the Z position only.  See set_pos().
////////////////////////////////////////////////////////////////////
INLINE bool SmoothMover::
set_z(float z) {
  bool result = (z != _sample._pos[2]);
  _sample._pos[2] = z;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: SmoothMover::set_hpr
//       Access: Published
//  Description: Specifies the orientation of the SmoothMover at a
//               particular time in the past.  When mark_position() is
//               called, this will be recorded (along with hpr and
//               timestamp) in a position report, which will then be
//               used along with all other position reports to
//               determine the smooth position at any particular
//               instant.
//
//               The return value is true if any parameter has changed
//               since the last call to set_hpr(), or false if they
//               are the same.
////////////////////////////////////////////////////////////////////
INLINE bool SmoothMover::
set_hpr(const LVecBase3f &hpr) {
  return set_h(hpr[0]) | set_p(hpr[1]) | set_r(hpr[2]);
}

////////////////////////////////////////////////////////////////////
//     Function: SmoothMover::set_hpr
//       Access: Published
//  Description: Specifies the orientation of the SmoothMover at a
//               particular time in the past.  When mark_position() is
//               called, this will be recorded (along with hpr and
//               timestamp) in a position report, which will then be
//               used along with all other position reports to
//               determine the smooth position at any particular
//               instant.
//
//               The return value is true if any parameter has changed
//               since the last call to set_hpr(), or false if they
//               are the same.
////////////////////////////////////////////////////////////////////
INLINE bool SmoothMover::
set_hpr(float h, float p, float r) {
  return set_h(h) | set_p(p) | set_r(r);
}

////////////////////////////////////////////////////////////////////
//     Function: SmoothMover::set_h
//       Access: Published
//  Description: Sets the heading only.  See set_hpr().
////////////////////////////////////////////////////////////////////
INLINE bool SmoothMover::
set_h(float h) {
  bool result = (h != _sample._hpr[0]);
  _sample._hpr[0] = h;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: SmoothMover::set_p
//       Access: Published
//  Description: Sets the pitch only.  See set_hpr().
////////////////////////////////////////////////////////////////////
INLINE bool SmoothMover::
set_p(float p) {
  bool result = (p != _sample._hpr[1]);
  _sample._hpr[1] = p;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: SmoothMover::set_r
//       Access: Published
//  Description: Sets the roll only.  See set_hpr().
////////////////////////////////////////////////////////////////////
INLINE bool SmoothMover::
set_r(float r) {
  bool result = (r != _sample._hpr[2]);
  _sample._hpr[2] = r;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: SmoothMover::set_phony_timestamp
//       Access: Published
//  Description: Lies and specifies that the current position report
//               was received now.  This is usually used for very old
//               position reports for which we're not sure of the
//               actual receipt time.
////////////////////////////////////////////////////////////////////
INLINE void SmoothMover::
set_phony_timestamp() {
  double now = ClockObject::get_global_clock()->get_frame_time();
  _sample._timestamp = now;
}

////////////////////////////////////////////////////////////////////
//     Function: SmoothMover::set_timestamp
//       Access: Published
//  Description: Specifies the time that the current position report
//               applies.  This should be called, along with set_pos()
//               and set_hpr(), before a call to mark_position().
////////////////////////////////////////////////////////////////////
INLINE void SmoothMover::
set_timestamp(double timestamp) {
  _sample._timestamp = timestamp;
  record_timestamp_delay(timestamp);
}

////////////////////////////////////////////////////////////////////
//     Function: SmoothMover::compute_smooth_position
//       Access: Published
//  Description: Computes the smoothed position (and orientation) of
//               the mover at the indicated point in time, based on
//               the previous position reports.  After this call has
//               been made, get_smooth_pos() etc. may be called to
//               retrieve the smoothed position.
//
//               With no parameter, the function uses
//               ClockObject::get_frame_time() as the default time.
////////////////////////////////////////////////////////////////////
INLINE bool SmoothMover::
compute_smooth_position() {
  return compute_smooth_position(ClockObject::get_global_clock()->get_frame_time());
}

////////////////////////////////////////////////////////////////////
//     Function: SmoothMover::get_smooth_pos
//       Access: Published
//  Description: Returns the smoothed position as computed by a
//               previous call to compute_smooth_position().
////////////////////////////////////////////////////////////////////
INLINE const LPoint3f &SmoothMover::
get_smooth_pos() const {
  return _smooth_pos;
}

////////////////////////////////////////////////////////////////////
//     Function: SmoothMover::get_smooth_hpr
//       Access: Published
//  Description: Returns the smoothed orientation as computed by a
//               previous call to compute_smooth_position().
////////////////////////////////////////////////////////////////////
INLINE const LVecBase3f &SmoothMover::
get_smooth_hpr() const {
  return _smooth_hpr;
}

////////////////////////////////////////////////////////////////////
//     Function: SmoothMover::get_smooth_mat
//       Access: Published
//  Description: Returns the complete smoothed transformation matrix
//               as computed by a previous call to
//               compute_smooth_position().
////////////////////////////////////////////////////////////////////
INLINE const LMatrix4f &SmoothMover::
get_smooth_mat() {
  if (!_computed_smooth_mat) {
    compose_smooth_mat();
  }
  return _smooth_mat;
}

////////////////////////////////////////////////////////////////////
//     Function: SmoothMover::get_smooth_forward_velocity
//       Access: Published
//  Description: Returns the speed at which the avatar is moving, in
//               feet per second, along its own forward axis (after
//               applying the avatar's hpr).  This will be a positive
//               number if the avatar is moving forward, and a
//               negative number if it is moving backward.
////////////////////////////////////////////////////////////////////
INLINE float SmoothMover::
get_smooth_forward_velocity() const {
  return _smooth_forward_velocity;
}

////////////////////////////////////////////////////////////////////
//     Function: SmoothMover::get_smooth_rotational_velocity
//       Access: Published
//  Description: Returns the speed at which the avatar is rotating in
//               the horizontal plane (i.e. heading), in degrees per
//               second.  This may be positive or negative, according
//               to the direction of rotation.
////////////////////////////////////////////////////////////////////
INLINE float SmoothMover::
get_smooth_rotational_velocity() const {
  return _smooth_rotational_velocity;
}

////////////////////////////////////////////////////////////////////
//     Function: SmoothMover::set_smooth_mode
//       Access: Published, Static
//  Description: Sets the smoothing mode of all SmoothMovers in the
//               world.  If this is SM_off, no smoothing or prediction
//               will be performed, and get_smooth_pos() will simply
//               return the position last set by mark_position().
////////////////////////////////////////////////////////////////////
INLINE void SmoothMover::
set_smooth_mode(SmoothMover::SmoothMode mode) {
  _smooth_mode = mode;
}

////////////////////////////////////////////////////////////////////
//     Function: SmoothMover::get_smooth_mode
//       Access: Published, Static
//  Description: Returns the smoothing mode of all SmoothMovers in the
//               world.  See set_smooth_mode().
////////////////////////////////////////////////////////////////////
INLINE SmoothMover::SmoothMode SmoothMover::
get_smooth_mode() {
  return _smooth_mode;
}

////////////////////////////////////////////////////////////////////
//     Function: SmoothMover::set_prediction_mode
//       Access: Published, Static
//  Description: Sets the predictioning mode of all SmoothMovers in the
//               world.  If this is PM_off, no prediction will be
//               performed, but smoothing might still be performed.
////////////////////////////////////////////////////////////////////
INLINE void SmoothMover::
set_prediction_mode(SmoothMover::PredictionMode mode) {
  _prediction_mode = mode;
}

////////////////////////////////////////////////////////////////////
//     Function: SmoothMover::get_prediction_mode
//       Access: Published, Static
//  Description: Returns the predictioning mode of all SmoothMovers in the
//               world.  See set_prediction_mode().
////////////////////////////////////////////////////////////////////
INLINE SmoothMover::PredictionMode SmoothMover::
get_prediction_mode() {
  return _prediction_mode;
}

////////////////////////////////////////////////////////////////////
//     Function: SmoothMover::set_delay
//       Access: Published, Static
//  Description: Sets the amount of time, in seconds, to delay the
//               computed position of a SmoothMover.  This is
//               particularly useful when the prediction mode is off,
//               because it can allow the apparent motion of an avatar
//               to appear smooth without relying on prediction, at
//               the cost of introducing additional lag in the
//               avatar's apparent position.
////////////////////////////////////////////////////////////////////
INLINE void SmoothMover::
set_delay(double delay) {
  _delay = delay;
}

////////////////////////////////////////////////////////////////////
//     Function: SmoothMover::get_delay
//       Access: Published, Static
//  Description: Returns the amount of time, in seconds, to delay the
//               computed position of a SmoothMover.  See set_delay().
////////////////////////////////////////////////////////////////////
INLINE double SmoothMover::
get_delay() {
  return _delay;
}

////////////////////////////////////////////////////////////////////
//     Function: SmoothMover::set_accept_clock_skew
//       Access: Published, Static
//  Description: Sets the 'accept clock skew' flag.  When this flag is
//               true, clock skew from the other clients will be
//               tolerated by delaying each smooth mover's position an
//               additional amount, on top of that specified by
//               set_delay(), based on the measured average latency
//               for timestamp messages received by the client.
//
//               In this way, if the other client has significant
//               clock skew with respect to our clock, it will be
//               evident as a large positive or negative average
//               latency for timestamps.  By subtracting out this
//               average latency, we compensate for poor clock sync.
////////////////////////////////////////////////////////////////////
INLINE void SmoothMover::
set_accept_clock_skew(bool flag) {
  _accept_clock_skew = flag;
}

////////////////////////////////////////////////////////////////////
//     Function: SmoothMover::get_accept_clock_skew
//       Access: Published, Static
//  Description: Returns the current state of the 'accept clock skew'
//               flag.  See set_accept_clock_skew().
////////////////////////////////////////////////////////////////////
INLINE bool SmoothMover::
get_accept_clock_skew() {
  return _accept_clock_skew;
}

////////////////////////////////////////////////////////////////////
//     Function: SmoothMover::set_max_position_age
//       Access: Published, Static
//  Description: Sets the maximum amount of time a position is allowed
//               to remain unchanged before assuming it represents the
//               avatar actually standing still.
////////////////////////////////////////////////////////////////////
INLINE void SmoothMover::
set_max_position_age(double age) {
  _max_position_age = age;
}

////////////////////////////////////////////////////////////////////
//     Function: SmoothMover::get_max_position_age
//       Access: Published, Static
//  Description: Returns the maximum amount of time a position is
//               allowed to remain unchanged before assuming it
//               represents the avatar actually standing still.
////////////////////////////////////////////////////////////////////
INLINE double SmoothMover::
get_max_position_age() {
  return _max_position_age;
}

////////////////////////////////////////////////////////////////////
//     Function: SmoothMover::set_reset_velocity_age
//       Access: Published, Static
//  Description: Sets the amount of time that should elapse after the
//               last position report before the velocity is reset to
//               0.  This is similar to max_position_age, but it is
//               only used to determine the resetting of the reported
//               velocity.  It should always be greater than or equal
//               to max_position_age.
////////////////////////////////////////////////////////////////////
INLINE void SmoothMover::
set_reset_velocity_age(double age) {
  _reset_velocity_age = age;
}

////////////////////////////////////////////////////////////////////
//     Function: SmoothMover::get_reset_velocity_age
//       Access: Published, Static
//  Description: Returns the amount of time that should elapse after
//               the last position report before the velocity is reset
//               to 0.  See set_reset_velocity_age().
////////////////////////////////////////////////////////////////////
INLINE double SmoothMover::
get_reset_velocity_age() {
  return _reset_velocity_age;
}

////////////////////////////////////////////////////////////////////
//     Function: SmoothMover::get_avg_timestamp_delay
//       Access: Private
//  Description: Returns the average delay observed in the last n
//               timestamps received from this client, in seconds.
//               This number represents the combination of the network
//               lag from this client, as well as the client's clock
//               skew relative to our clock.  It could be negative if
//               the client's clock is running faster than our clock.
////////////////////////////////////////////////////////////////////
INLINE double SmoothMover::
get_avg_timestamp_delay() const {
  nassertr(!_timestamp_delays.empty(), 0.0);
  return (double)_net_timestamp_delay / (double)_timestamp_delays.size() / 1000.0;
}
