// Filename: smoothMover.cxx
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

#include "smoothMover.h"
#include "notify.h"
#include "compose_matrix.h"

SmoothMover::SmoothMode SmoothMover::_smooth_mode = SmoothMover::SM_off;
SmoothMover::PredictionMode SmoothMover::_prediction_mode = SmoothMover::PM_off;
double SmoothMover::_delay = 0.2;
bool SmoothMover::_accept_clock_skew = true;
double SmoothMover::_max_position_age = 0.25;
double SmoothMover::_reset_velocity_age = 0.3;

////////////////////////////////////////////////////////////////////
//     Function: SmoothMover::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
SmoothMover::
SmoothMover() {
  _scale.set(1.0, 1.0, 1.0);
  _sample._pos.set(0.0, 0.0, 0.0);
  _sample._hpr.set(0.0, 0.0, 0.0);
  _sample._timestamp = 0.0;

  _smooth_pos.set(0.0, 0.0, 0.0);
  _smooth_hpr.set(0.0, 0.0, 0.0);
  _smooth_mat = LMatrix4f::ident_mat();
  _smooth_timestamp = 0.0;
  _smooth_position_known = false;
  _smooth_position_changed = true;
  _computed_smooth_mat = true;

  _smooth_forward_velocity = 0.0;
  _smooth_rotational_velocity = 0.0;

  _last_point_before = -1;
  _last_point_after = -1;

  _net_timestamp_delay = 0;
  // Record one delay of 0 on the top of the delays array, just to
  // guarantee that the array is never completely empty.
  _timestamp_delays.push_back(0);

  _last_heard_from = 0.0;
}

////////////////////////////////////////////////////////////////////
//     Function: SmoothMover::Destructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
SmoothMover::
~SmoothMover() {
}

////////////////////////////////////////////////////////////////////
//     Function: SmoothMover::set_mat
//       Access: Published
//  Description: Specifies the scale, hpr, and pos for the SmoothMover
//               at some particular point, based on the matrix.
////////////////////////////////////////////////////////////////////
bool SmoothMover::
set_mat(const LMatrix4f &mat) {
  bool result = false;

  LVecBase3f scale, hpr, pos;
  if (decompose_matrix(mat, scale, hpr, pos)) {
    result = set_scale(scale) | set_hpr(hpr) | set_pos(pos);
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: SmoothMover::mark_position
//       Access: Published
//  Description: Stores the position, orientation, and timestamp (if
//               relevant) indicated by previous calls to set_pos(),
//               set_hpr(), and set_timestamp() in a new position
//               report.
//
//               When compute_smooth_position() is called, it uses
//               these stored position reports to base its computation
//               of the known position.
////////////////////////////////////////////////////////////////////
void SmoothMover::
mark_position() {
  if (_smooth_mode == SM_off) {
    // With smoothing disabled, mark_position() simply stores its
    // current position in the smooth_position members.

    // In this mode, we also ignore the supplied timestamp, and just
    // use the current frame time--there's no need to risk trusting
    // the timestamp from another client.
    double timestamp = ClockObject::get_global_clock()->get_frame_time();

    // We also need to compute the velocity here.
    if (_smooth_position_known) {
      LVector3f pos_delta = _sample._pos - _smooth_pos;
      LVecBase3f hpr_delta = _sample._hpr - _smooth_hpr;
      double age = timestamp - _smooth_timestamp;
      age = min(age, _max_position_age);

      set_smooth_pos(_sample._pos, _sample._hpr, timestamp);
      if (age != 0.0) {
        compute_velocity(pos_delta, hpr_delta, age);
      }

    } else {
      // No velocity is possible, just position and orientation.
      set_smooth_pos(_sample._pos, _sample._hpr, timestamp);
    }

  } else {
    // Otherwise, smoothing is in effect and we store a true position
    // report.
    
    if (_points.full()) {
      // If we have too many position reports, throw away the oldest
      // one.
      _points.pop_front();

      // That invalidates the index numbers.
      _last_point_before = -1;
      _last_point_after = -1;
    }
    
    _points.push_back(_sample);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SmoothMover::clear_positions
//       Access: Published
//  Description: Erases all the old position reports.  This should be
//               done, for instance, prior to teleporting the avatar
//               to a new position; otherwise, the smoother might try
//               to lerp the avatar there.  If reset_velocity is true,
//               the velocity is also reset to 0.
////////////////////////////////////////////////////////////////////
void SmoothMover::
clear_positions(bool reset_velocity) {
  while (!_points.empty()) {
    _points.pop_front();
  }
  _last_point_before = -1;
  _last_point_after = -1;
  _smooth_position_known = false;

  if (reset_velocity) {
    _smooth_forward_velocity = 0.0;
    _smooth_rotational_velocity = 0.0;
  }
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
//               The return value is true if the value has changed (or
//               might have changed) since the last call to
//               compute_smooth_position(), or false if it remains the
//               same.
////////////////////////////////////////////////////////////////////
bool SmoothMover::
compute_smooth_position(double timestamp) {
  if (_points.empty()) {
    // With no position reports available, this function does nothing,
    // except to make sure that our velocity gets reset to zero after
    // a period of time.

    if (_smooth_position_known) {
      double age = timestamp - _smooth_timestamp;
      if (age > _reset_velocity_age) {
        _smooth_forward_velocity = 0.0;
        _smooth_rotational_velocity = 0.0;
      }
    }
    bool result = _smooth_position_changed;
    _smooth_position_changed = false;
    return result;
  }
  if (_smooth_mode == SM_off) {
    // With smoothing disabled, this function also does nothing,
    // except to ensure that any old bogus position reports are
    // cleared.
    clear_positions(false);
    bool result = _smooth_position_changed;
    _smooth_position_changed = false;
    return result;
  }

  // First, back up in time by the specified delay factor.
  double orig_timestamp = timestamp;
  timestamp -= _delay;
  if (_accept_clock_skew) {
    timestamp -= get_avg_timestamp_delay();
  }

  // Now look for the two bracketing position reports.
  int point_way_before = -1;
  int point_before = -1;
  double timestamp_before = 0.0;
  int point_after = -1;
  double timestamp_after = 0.0;

  // This loop doesn't make any assumptions about the ordering of
  // timestamps in the queue.  We could probably make it faster by
  // assuming timestamps are ordered from oldest to newest (as they
  // are generally guaranteed to be, except for the minor detail of
  // clients occasionally resetting their clocks).
  int num_points = _points.size();
  for (int i = 0; i < num_points; i++) {
    const SamplePoint &point = _points[i];
    if (point._timestamp < timestamp) {
      // This point is before the desired time.  Find the newest of
      // these.
      if (point_before == -1 || point._timestamp > timestamp_before) {
        point_way_before = point_before;
        point_before = i;
        timestamp_before = point._timestamp;
      }
    }
    if (point._timestamp >= timestamp) {
      // This point is after the desired time.  Find the oldest of
      // these.
      if (point_after == -1 || point._timestamp < timestamp_after) {
        point_after = i;
        timestamp_after = point._timestamp;
      }
    }
  }

  if (point_before == -1) {
    nassertr(point_after != -1, false);
    // If we only have an after point, we have to start there.
    bool result = !(_last_point_before == point_before && 
                    _last_point_after == point_after);
    const SamplePoint &point = _points[point_after];
    set_smooth_pos(point._pos, point._hpr, timestamp);
    _smooth_forward_velocity = 0.0;
    _smooth_rotational_velocity = 0.0;
    _last_point_before = point_before;
    _last_point_after = point_after;
    return result;
  }

  bool result = true;

  if (point_after == -1 && _prediction_mode != PM_off) {
    // With prediction in effect, we're allowed to anticipate where
    // the avatar is going by a tiny bit, if we don't have current
    // enough data.  This works only if we have at least two points of
    // old data.
    if (point_way_before != -1) {
      // To implement simple prediction, we simply back up in time to
      // the previous two timestamps, and base our linear
      // interpolation off of those two, extending into the future.
      SamplePoint &point = _points[point_way_before];
      point_after = point_before;
      timestamp_after = timestamp_before;
      point_before = point_way_before;
      timestamp_before = point._timestamp;
      
      if (timestamp > timestamp_after + _max_position_age) {
        // Don't allow the prediction to get too far into the
        // future.
        timestamp = timestamp_after + _max_position_age;
      }
    }
  }

  if (point_after == -1) {
    // If we only have a before point even after we've checked for the
    // possibility of using prediction, then we have to stop there.
    if (point_way_before != -1) {
      // Use the previous two points, if we've got 'em, so we can
      // still reflect the avatar's velocity.
      linear_interpolate(point_way_before, point_before, timestamp_before);

    } else {
      // If we really only have one point, use it.
      const SamplePoint &point = _points[point_before];
      set_smooth_pos(point._pos, point._hpr, timestamp);
    }

    if (orig_timestamp - _last_heard_from > _reset_velocity_age) {
      // Furthermore, if we haven't heard from this client in a while,
      // reset the velocity.  This decision is based entirely on our
      // local timestamps, not on the other client's reported
      // timestamps.
      _smooth_forward_velocity = 0.0;
      _smooth_rotational_velocity = 0.0;
    }

    result = !(_last_point_before == point_before && 
               _last_point_after == point_after);
    _last_point_before = point_before;
    _last_point_after = point_after;

  } else {
    // If we have two points, we can linearly interpolate between them.
    linear_interpolate(point_before, point_after, timestamp);
  }

  // Assume we'll never get another compute_smooth_position() request
  // for an older time than this, and remove all the timestamps at the
  // head of the queue before point_way_before.
  double timestamp_way_before = _points[point_way_before]._timestamp;
  while (!_points.empty() && _points.front()._timestamp < timestamp_way_before) {
    _points.pop_front();

    // This invalidates the index numbers.
    _last_point_before = -1;
    _last_point_after = -1;
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: SmoothMover::get_latest_position
//       Access: Published
//  Description: Updates the smooth_pos (and smooth_hpr, etc.) members
//               to reflect the absolute latest position known for
//               this avatar.  This may result in a pop to the most
//               recent position.
//
//               Returns true if the latest position is known, false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool SmoothMover::
get_latest_position() {
  if (_points.empty()) {
    // Nothing to do if there are no points.
    return _smooth_position_known;
  }

  const SamplePoint &point = _points.back();
  set_smooth_pos(point._pos, point._hpr, point._timestamp);
  _smooth_forward_velocity = 0.0;
  _smooth_rotational_velocity = 0.0;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: SmoothMover::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void SmoothMover::
output(ostream &out) const {
  out << "SmoothMover, " << _points.size() << " sample points.";
}

////////////////////////////////////////////////////////////////////
//     Function: SmoothMover::write
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void SmoothMover::
write(ostream &out) const {
  out << "SmoothMover, " << _points.size() << " sample points:\n";
  int num_points = _points.size();
  for (int i = 0; i < num_points; i++) {
    const SamplePoint &point = _points[i];
    out << "  " << i << ". time = " << point._timestamp << " pos = " 
        << point._pos << " hpr = " << point._hpr << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SmoothMover::set_smooth_pos
//       Access: Private
//  Description: Sets the computed smooth position and orientation for
//               the indicated timestamp.
////////////////////////////////////////////////////////////////////
void SmoothMover::
set_smooth_pos(const LPoint3f &pos, const LVecBase3f &hpr,
               double timestamp) {
  _smooth_pos = pos;
  _smooth_hpr = hpr;
  _smooth_timestamp = timestamp;
  _smooth_position_known = true;
  _smooth_position_changed = true;
  _computed_smooth_mat = false;
}

////////////////////////////////////////////////////////////////////
//     Function: SmoothMover::compose_smooth_mat
//       Access: Private
//  Description: Computes the smooth_mat based on smooth_pos and
//               smooth_hpr.
////////////////////////////////////////////////////////////////////
void SmoothMover::
compose_smooth_mat() {
  compose_matrix(_smooth_mat, _scale, _smooth_hpr, _smooth_pos);
  _computed_smooth_mat = true;
}

////////////////////////////////////////////////////////////////////
//     Function: SmoothMover::linear_interpolate
//       Access: Private
//  Description: Interpolates the smooth position linearly between the
//               two bracketing position reports.
////////////////////////////////////////////////////////////////////
void SmoothMover::
linear_interpolate(int point_before, int point_after, double timestamp) {
  SamplePoint &point_b = _points[point_before];
  const SamplePoint &point_a = _points[point_after];

  double age = (point_a._timestamp - point_b._timestamp);

  if (point_before == _last_point_before && 
      point_after == _last_point_after) {
    // If these are the same two points we found last time (which is
    // likely), we can save a bit of work.
    double t = (timestamp - point_b._timestamp) / age;
    set_smooth_pos(point_b._pos + t * (point_a._pos - point_b._pos),
                   point_b._hpr + t * (point_a._hpr - point_b._hpr),
                   timestamp);

    // The velocity remains the same as last time.

  } else {
    _last_point_before = point_before;
    _last_point_after = point_after;
    
    if (age > _max_position_age) {
      // If the first point is too old, assume there were a lot of
      // implicit standing still messages that weren't sent.  Reset
      // the first point's timestamp to reflect this.
      point_b._timestamp = min(timestamp, point_a._timestamp - _max_position_age);
      age = (point_a._timestamp - point_b._timestamp);
    }
    
    // To interpolate the hpr's, we must first make sure that both
    // angles are on the same side of the discontinuity.
    for (int j = 0; j < 3; j++) {
      if ((point_b._hpr[j] - point_a._hpr[j]) > 180.0) {
        point_b._hpr[j] -= 360.0;
      } else if ((point_b._hpr[j] - point_a._hpr[j]) < -180.0) {
        point_b._hpr[j] += 360.0;
      }
    }
    
    double t = (timestamp - point_b._timestamp) / age;
    LVector3f pos_delta = point_a._pos - point_b._pos;
    LVecBase3f hpr_delta = point_a._hpr - point_b._hpr;

    set_smooth_pos(point_b._pos + t * pos_delta, 
                   point_b._hpr + t * hpr_delta, 
                   timestamp);
    compute_velocity(pos_delta, hpr_delta, age);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SmoothMover::compute_velocity
//       Access: Private
//  Description: Computes the forward and rotational velocities of the
//               moving object.
////////////////////////////////////////////////////////////////////
void SmoothMover::
compute_velocity(const LVector3f &pos_delta, const LVecBase3f &hpr_delta,
                 double age) {
  // Also compute the velocity.  To get just the forward component
  // of velocity, we need to project the velocity vector onto the y
  // axis, as rotated by the current hpr.
  LMatrix3f rot_mat;
  compose_matrix(rot_mat, LVecBase3f(1.0, 1.0, 1.0), _smooth_hpr);
  LVector3f y_axis = LVector3f(0.0, 1.0, 0.0) * rot_mat;
  float forward_distance = pos_delta.dot(y_axis);
  
  _smooth_forward_velocity = forward_distance / age;
  _smooth_rotational_velocity = hpr_delta[0] / age;
}

////////////////////////////////////////////////////////////////////
//     Function: SmoothMover::record_timestamp_delay
//       Access: Private
//  Description: Records the delay measured in receiving this
//               particular timestamp.  The average delay of the last
//               n timestamps will be used to smooth the motion
//               properly.
////////////////////////////////////////////////////////////////////
void SmoothMover::
record_timestamp_delay(double timestamp) {
  double now = ClockObject::get_global_clock()->get_frame_time();

  // Convert the delay to an integer number of milliseconds.  Integers
  // are better than doubles because they don't accumulate errors over
  // time.
  int delay = (int)((now - timestamp) * 1000.0);
  if (_timestamp_delays.full()) {
    _net_timestamp_delay -= _timestamp_delays.front();
    _timestamp_delays.pop_front();
  }
  _net_timestamp_delay += delay;
  _timestamp_delays.push_back(delay);

  _last_heard_from = now;
}
