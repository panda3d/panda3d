/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file smoothMover.cxx
 * @author drose
 * @date 2001-10-19
 */

#include "smoothMover.h"
#include "pnotify.h"
#include "config_deadrec.h"

/**
 *
 */
SmoothMover::
SmoothMover() {
  _sample._pos.set(0.0, 0.0, 0.0);
  _sample._hpr.set(0.0, 0.0, 0.0);
  _sample._timestamp = 0.0;

  _smooth_pos.set(0.0, 0.0, 0.0);
  _smooth_hpr.set(0.0, 0.0, 0.0);
  _forward_axis.set(0.0, 1.0, 0.0);
  _smooth_timestamp = 0.0;
  _smooth_position_known = false;
  _smooth_position_changed = true;
  _computed_forward_axis = true;

  _smooth_forward_velocity = 0.0;
  _smooth_lateral_velocity = 0.0;
  _smooth_rotational_velocity = 0.0;

  _has_most_recent_timestamp = false;

  _last_point_before = -1;
  _last_point_after = -1;

  _net_timestamp_delay = 0;
  // Record one delay of 0 on the top of the delays array, just to guarantee
  // that the array is never completely empty.
  _timestamp_delays.push_back(0);

  _last_heard_from = 0.0;

  _smooth_mode = SM_off;
  _prediction_mode = PM_off;
  _delay = 0.2;
  _accept_clock_skew = accept_clock_skew;
  _directional_velocity = true;
  _default_to_standing_still = true;
  _max_position_age = 0.25;
  _expected_broadcast_period = 0.2;
  _reset_velocity_age = 0.3;
}

/**
 *
 */
SmoothMover::
~SmoothMover() {
}

/**
 * Stores the position, orientation, and timestamp (if relevant) indicated by
 * previous calls to set_pos(), set_hpr(), and set_timestamp() in a new
 * position report.
 *
 * When compute_smooth_position() is called, it uses these stored position
 * reports to base its computation of the known position.
 */
void SmoothMover::
mark_position() {
  /*
  if (deadrec_cat.is_debug()) {
    deadrec_cat.debug() << "mark_position\n";
  }
  */
  if (_smooth_mode == SM_off) {
    // With smoothing disabled, mark_position() simply stores its current
    // position in the smooth_position members.

    // In this mode, we also ignore the supplied timestamp, and just use the
    // current frame time--there's no need to risk trusting the timestamp from
    // another client.
    double timestamp = ClockObject::get_global_clock()->get_frame_time();

    // We also need to compute the velocity here.
    if (_smooth_position_known) {
      LVector3 pos_delta = _sample._pos - _smooth_pos;
      LVecBase3 hpr_delta = _sample._hpr - _smooth_hpr;
      double age = timestamp - _smooth_timestamp;
      age = std::min(age, _max_position_age);

      set_smooth_pos(_sample._pos, _sample._hpr, timestamp);
      if (age != 0.0) {
        compute_velocity(pos_delta, hpr_delta, age);
      }

    } else {
      // No velocity is possible, just position and orientation.
      set_smooth_pos(_sample._pos, _sample._hpr, timestamp);
    }

  } else {
    // Otherwise, smoothing is in effect and we store a true position report.

    if (!_points.empty() && _points.back()._timestamp > _sample._timestamp) {
      if (deadrec_cat.is_debug()) {
        deadrec_cat.debug()
          << "*** timestamp out of order " << _points.back()._timestamp << " "
          << _sample._timestamp << "\n";
      }

      // If we get a timestamp out of order, one of us must have just reset
      // our clock.  Flush the sequence and start again.
      _points.clear();

      // That invalidates the index numbers.
      _last_point_before = -1;
      _last_point_after = -1;

      _points.push_back(_sample);

    } else if (!_points.empty() && _points.back()._timestamp == _sample._timestamp) {
      if (deadrec_cat.is_debug()) {
        deadrec_cat.debug()
          << "*** same timestamp\n";
      }
      // If the new timestamp is the same as the last timestamp, the value
      // simply replaces the previous value.
      _points.back() = _sample;

    } else if ((int)_points.size() >= max_position_reports) {
      if (deadrec_cat.is_debug()) {
        deadrec_cat.debug()
          << "*** dropped oldest position report\n";
      }
      // If we have too many position reports, throw away the oldest one.
      _points.pop_front();

      --_last_point_before;
      --_last_point_after;

      _points.push_back(_sample);

    } else {
      // In the ordinary case, just add another sample.
      _points.push_back(_sample);
    }
  }
  // cout << "mark_position: " << _points.back()._pos << endl;
}

/**
 * Erases all the old position reports.  This should be done, for instance,
 * prior to teleporting the avatar to a new position; otherwise, the smoother
 * might try to lerp the avatar there.  If reset_velocity is true, the
 * velocity is also reset to 0.
 */
void SmoothMover::
clear_positions(bool reset_velocity) {
  if (deadrec_cat.is_debug()) {
    deadrec_cat.debug()
      << "clear_positions " << reset_velocity << "\n";
  }

  _points.clear();
  _last_point_before = -1;
  _last_point_after = -1;
  _smooth_position_known = false;
  _has_most_recent_timestamp = false;

  if (reset_velocity) {
    _smooth_forward_velocity = 0.0;
    _smooth_lateral_velocity = 0.0;
    _smooth_rotational_velocity = 0.0;
  }
}

/**
 * Computes the smoothed position (and orientation) of the mover at the
 * indicated point in time, based on the previous position reports.  After
 * this call has been made, get_smooth_pos() etc.  may be called to retrieve
 * the smoothed position.
 *
 * The return value is true if the value has changed (or might have changed)
 * since the last call to compute_smooth_position(), or false if it remains
 * the same.
 */
bool SmoothMover::
compute_smooth_position(double timestamp) {
  if (deadrec_cat.is_spam()) {
    deadrec_cat.spam()
      << _points.size() << " points\n";
  }

  if (_points.empty()) {
    // With no position reports available, this function does nothing, except
    // to make sure that our velocity gets reset to zero after a period of
    // time.

    if (_smooth_position_known) {
      double age = timestamp - _smooth_timestamp;
      if (age > _reset_velocity_age) {
        if (deadrec_cat.is_debug()) {
          deadrec_cat.debug()
            << "points empty; reset velocity, age = " << age << "\n";
        }
        _smooth_forward_velocity = 0.0;
        _smooth_lateral_velocity = 0.0;
        _smooth_rotational_velocity = 0.0;
      }
    }
    bool result = _smooth_position_changed;
    _smooth_position_changed = false;

    if (deadrec_cat.is_spam()) {
      deadrec_cat.spam()
        << "  no points: " << result << "\n";
    }
    return result;
  }
  if (_smooth_mode == SM_off) {
    // With smoothing disabled, this function also does nothing, except to
    // ensure that any old bogus position reports are cleared.
    clear_positions(false);
    bool result = _smooth_position_changed;
    _smooth_position_changed = false;

    if (deadrec_cat.is_spam()) {
      deadrec_cat.spam()
        << "  disabled: " << result << "\n";
    }
    return result;
  }

  // First, back up in time by the specified delay factor.
  double orig_timestamp = timestamp;
  timestamp -= _delay;
  if (_accept_clock_skew) {
    timestamp -= get_avg_timestamp_delay();
  }

  if (deadrec_cat.is_spam()) {
    deadrec_cat.spam()
      << "time = " << timestamp << ", " << _points.size()
      << " points, last = " << _last_point_before << ", "
      << _last_point_after << "\n";
    deadrec_cat.spam(false)
      << "  ";
    for (int pi = 0; pi < (int)_points.size(); pi++) {
      deadrec_cat.spam(false) << _points[pi]._timestamp << " ";
    }
    deadrec_cat.spam(false) << "\n";
  }

  // Now look for the two bracketing position reports.
  int point_way_before = -1;
  int point_before = -1;
  double timestamp_before = 0.0;
  int point_after = -1;
  double timestamp_after = 0.0;

  int num_points = _points.size();
  int i;

  // Find the newest of the points before the indicated time.  Assume that
  // this will be no older than _last_point_before.
  i = std::max(0, _last_point_before);
  while (i < num_points && _points[i]._timestamp < timestamp) {
    point_before = i;
    timestamp_before = _points[i]._timestamp;
    ++i;
  }
  point_way_before = std::max(point_before - 1, -1);

  // Now the next point is presumably the oldest point after the indicated
  // time.
  if (i < num_points) {
    point_after = i;
    timestamp_after = _points[i]._timestamp;
  }

  if (deadrec_cat.is_spam()) {
    deadrec_cat.spam()
      << "  found points (" << point_way_before << ") " << point_before
      << ", " << point_after << "\n";
  }

  if (point_before < 0) {
    nassertr(point_after >= 0, false);
    // If we only have an after point, we have to start there.
    bool result = !(_last_point_before == point_before &&
                    _last_point_after == point_after);
    const SamplePoint &point = _points[point_after];
    set_smooth_pos(point._pos, point._hpr, timestamp);
    _smooth_forward_velocity = 0.0;
    _smooth_lateral_velocity = 0.0;
    _smooth_rotational_velocity = 0.0;
    _last_point_before = point_before;
    _last_point_after = point_after;
    if (deadrec_cat.is_spam()) {
      deadrec_cat.spam()
        << "  only an after point: " << _last_point_before << ", "
        << _last_point_after << "\n";
    }
    return result;
  }

  bool result = true;

  if (point_after < 0 && _prediction_mode != PM_off) {
    // With prediction in effect, we're allowed to anticipate where the avatar
    // is going by a tiny bit, if we don't have current enough data.  This
    // works only if we have at least two points of old data.
    if (point_way_before >= 0) {
      // To implement simple prediction, we simply back up in time to the
      // previous two timestamps, and base our linear interpolation off of
      // those two, extending into the future.
      SamplePoint &point = _points[point_way_before];
      point_after = point_before;
      timestamp_after = timestamp_before;
      point_before = point_way_before;
      timestamp_before = point._timestamp;

      if (timestamp > timestamp_after + _max_position_age) {
        // Don't allow the prediction to get too far into the future.
        timestamp = timestamp_after + _max_position_age;
      }
    }
  }

  if (point_after < 0) {
    // If we only have a before point even after we've checked for the
    // possibility of using prediction, then we have to stop there.
    if (point_way_before >= 0) {
      // Use the previous two points, if we've got 'em, so we can still
      // reflect the avatar's velocity.
      if (deadrec_cat.is_spam()) {
        deadrec_cat.spam()
          << "  previous two\n";
      }
      linear_interpolate(point_way_before, point_before, timestamp_before);

    } else {
      if (deadrec_cat.is_spam()) {
        deadrec_cat.spam()
          << "  one point\n";
      }
      // If we really only have one point, use it.
      const SamplePoint &point = _points[point_before];
      set_smooth_pos(point._pos, point._hpr, timestamp);
    }

    double age = timestamp - timestamp_before;
    if (age > _reset_velocity_age) {
      if (deadrec_cat.is_spam()) {
        deadrec_cat.spam()
          << "  reset_velocity, age = " << age << "\n";
      }
      _smooth_forward_velocity = 0.0;
      _smooth_lateral_velocity = 0.0;
      _smooth_rotational_velocity = 0.0;
    }

    result = !(_last_point_before == point_before &&
               _last_point_after == point_after);
  } else {
    // If we have two points, we can linearly interpolate between them.
    if (deadrec_cat.is_spam()) {
      deadrec_cat.spam()
        << "  normal interpolate\n";
    }
    SamplePoint &point_b = _points[point_before];
    const SamplePoint &point_a = _points[point_after];

    if (point_b._pos == point_a._pos && point_b._hpr == point_a._hpr) {
      // The points are equivalent, so just return that.
      if (deadrec_cat.is_spam()) {
        deadrec_cat.spam()
          << "Points are equivalent\n";
      }
      set_smooth_pos(point_b._pos, point_b._hpr, timestamp);

      // This implies that velocity is 0.
      _smooth_forward_velocity = 0.0;
      _smooth_lateral_velocity = 0.0;
      _smooth_rotational_velocity = 0.0;

    } else {
      // The points are different, so we have to do some work.
      double age = (point_a._timestamp - point_b._timestamp);

      if (_default_to_standing_still && (age > _max_position_age)) {
        // If the first point is too old, assume there were a lot of implicit
        // standing still messages that weren't sent.  Insert a new sample
        // point to reflect this.
        if (deadrec_cat.is_spam()) {
          deadrec_cat.spam()
            << "  first point too old: age = " << age << "\n";
        }
        SamplePoint new_point = point_b;
        new_point._timestamp = point_a._timestamp - _expected_broadcast_period;
        if (deadrec_cat.is_spam()) {
          deadrec_cat.spam()
            << "  constructed new timestamp at " << new_point._timestamp
            << "\n";
        }
        if (new_point._timestamp > point_b._timestamp) {
          _points.insert(_points.begin() + point_after, new_point);

          // Now we've monkeyed with the sequence.  Start over.
          if (deadrec_cat.is_spam()) {
            deadrec_cat.spam()
              << "  recursing after time adjustment.\n";
          }
          return compute_smooth_position(orig_timestamp);
        }
      }

      linear_interpolate(point_before, point_after, timestamp);
    }
  }

  if (deadrec_cat.is_spam()) {
    deadrec_cat.spam()
      << "  changing " << _last_point_before << ", " << _last_point_after
      << " to " << point_before << ", " << point_after << "\n";
  }
  _last_point_before = point_before;
  _last_point_after = point_after;


  // Assume we'll never get another compute_smooth_position() request for an
  // older time than this, and remove all the timestamps at the head of the
  // queue up to but not including point_way_before.
  while (point_way_before > 0) {
    nassertr(!_points.empty(), result);
    _points.pop_front();

    --point_way_before;
    --_last_point_before;
    --_last_point_after;
    if (deadrec_cat.is_spam()) {
      deadrec_cat.spam()
        << "  popping old point: " << _last_point_before << ", "
        << _last_point_after << "\n";
    }
  }

  // If we are not using prediction mode, we can also remove point_way_before.
  if (_prediction_mode == PM_off) {
    if (point_way_before == 0) {
      nassertr(!_points.empty(), result);
      _points.pop_front();

      --point_way_before;
      --_last_point_before;
      --_last_point_after;
      if (deadrec_cat.is_spam()) {
        deadrec_cat.spam()
          << "  popping way_before point: " << _last_point_before << ", "
          << _last_point_after << "\n";
      }
    }

    // And if there's only one point left, remove even that one after a while.
    /* jbutler: commented this out, seems to cause the smoothing pop that occurs
                when this object is stopped for a while then starts moving again
    if (_points.size() == 1) {
      double age = timestamp - _points.back()._timestamp;
      if (deadrec_cat.is_spam()) {
        deadrec_cat.spam()
          << "considering clearing all points, age = " << age << "\n";
      }
      if (age > _reset_velocity_age) {
        if (deadrec_cat.is_spam()) {
          deadrec_cat.spam()
            << "clearing all points.\n";
        }
        _points.clear();
      }
    }
    */
  }

  if (deadrec_cat.is_spam()) {
    deadrec_cat.spam()
      << "  result = " << result << "\n";
  }

  return result;
}

/**
 * Updates the smooth_pos (and smooth_hpr, etc.) members to reflect the
 * absolute latest position known for this avatar.  This may result in a pop
 * to the most recent position.
 *
 * Returns true if the latest position is known, false otherwise.
 */
bool SmoothMover::
get_latest_position() {
  if (deadrec_cat.is_debug()) {
    deadrec_cat.debug() << "get_latest_position\n";
  }
  if (_points.empty()) {
    // Nothing to do if there are no points.
    return _smooth_position_known;
  }

  const SamplePoint &point = _points.back();
  set_smooth_pos(point._pos, point._hpr, point._timestamp);
  _smooth_forward_velocity = 0.0;
  _smooth_lateral_velocity = 0.0;
  _smooth_rotational_velocity = 0.0;
  return true;
}

/**
 *
 */
void SmoothMover::
output(std::ostream &out) const {
  out << "SmoothMover, " << _points.size() << " sample points.";
}

/**
 *
 */
void SmoothMover::
write(std::ostream &out) const {
  out << "SmoothMover, " << _points.size() << " sample points:\n";
  int num_points = _points.size();
  for (int i = 0; i < num_points; i++) {
    const SamplePoint &point = _points[i];
    out << "  " << i << ". time = " << point._timestamp << " pos = "
        << point._pos << " hpr = " << point._hpr << "\n";
  }
}

/**
 * Sets the computed smooth position and orientation for the indicated
 * timestamp.
 */
void SmoothMover::
set_smooth_pos(const LPoint3 &pos, const LVecBase3 &hpr,
               double timestamp) {
  if (deadrec_cat.is_spam()) {
    deadrec_cat.spam()
      << "set_smooth_pos(" << pos << ", " << hpr << ", "
      << timestamp << ")\n";
  }

  if (_smooth_pos != pos) {
    _smooth_pos = pos;
    _smooth_position_changed = true;
  }
  if (_smooth_hpr != hpr) {
    _smooth_hpr = hpr;
    _smooth_position_changed = true;
    _computed_forward_axis = false;
  }

  _smooth_timestamp = timestamp;
  _smooth_position_known = true;
}

/**
 * Interpolates the smooth position linearly between the two bracketing
 * position reports.
 */
void SmoothMover::
linear_interpolate(int point_before, int point_after, double timestamp) {
  SamplePoint &point_b = _points[point_before];
  const SamplePoint &point_a = _points[point_after];

  double age = (point_a._timestamp - point_b._timestamp);

  /*
  Points::const_iterator pi;
  cout << "linear_interpolate: ";
  for (pi = _points.begin(); pi != _points.end(); ++pi) {
    cout << "(" << (*pi)._pos << "), ";
  }
  cout << endl;
  */

  if (point_before == _last_point_before &&
      point_after == _last_point_after) {
    if (deadrec_cat.is_spam()) {
      deadrec_cat.spam()
        << "  same two points\n";
    }

    // If these are the same two points we found last time (which is likely),
    // we can save a bit of work.
    double t = (timestamp - point_b._timestamp) / age;

    if (deadrec_cat.is_spam()) {
      deadrec_cat.spam()
        << "   interp " << t << ": " << point_b._pos << " to " << point_a._pos
        << "\n";
    }
    set_smooth_pos(point_b._pos + t * (point_a._pos - point_b._pos),
                   point_b._hpr + t * (point_a._hpr - point_b._hpr),
                   timestamp);

    // The velocity remains the same as last time.

  } else {
    // To interpolate the hpr's, we must first make sure that both angles are
    // on the same side of the discontinuity.
    for (int j = 0; j < 3; j++) {
      if ((point_b._hpr[j] - point_a._hpr[j]) > 180.0) {
        point_b._hpr[j] -= 360.0;
      } else if ((point_b._hpr[j] - point_a._hpr[j]) < -180.0) {
        point_b._hpr[j] += 360.0;
      }
    }

    double t = (timestamp - point_b._timestamp) / age;
    LVector3 pos_delta = point_a._pos - point_b._pos;
    LVecBase3 hpr_delta = point_a._hpr - point_b._hpr;

    if (deadrec_cat.is_spam()) {
      deadrec_cat.spam()
        << "   interp " << t << ": " << point_b._pos << " to " << point_a._pos
        << "\n";
    }
    set_smooth_pos(point_b._pos + t * pos_delta,
                   point_b._hpr + t * hpr_delta,
                   timestamp);
    compute_velocity(pos_delta, hpr_delta, age);
  }
}

/**
 * Computes the forward and rotational velocities of the moving object.
 */
void SmoothMover::
compute_velocity(const LVector3 &pos_delta, const LVecBase3 &hpr_delta,
                 double age) {
  _smooth_rotational_velocity = hpr_delta[0] / age;

  if (_directional_velocity) {
    // To get just the forward component of velocity, we need to project the
    // velocity vector onto the y axis, as rotated by the current hpr.
    if (!_computed_forward_axis) {
      LMatrix3 rot_mat;
      compose_matrix(rot_mat, LVecBase3(1.0, 1.0, 1.0), _smooth_hpr);
      _forward_axis = LVector3(0.0, 1.0, 0.0) * rot_mat;

      if (deadrec_cat.is_spam()) {
        deadrec_cat.spam()
          << "  compute forward_axis = " << _forward_axis << "\n";
      }
    }

    LVector3 lateral_axis = _forward_axis.cross(LVector3(0.0,0.0,1.0));

    PN_stdfloat forward_distance = pos_delta.dot(_forward_axis);
    PN_stdfloat lateral_distance = pos_delta.dot(lateral_axis);

    _smooth_forward_velocity = forward_distance / age;
    _smooth_lateral_velocity = lateral_distance / age;

  } else {
    _smooth_forward_velocity = pos_delta.length();
    _smooth_lateral_velocity = 0.0f;
  }

  if (deadrec_cat.is_spam()) {
    deadrec_cat.spam()
      << "  compute_velocity = " << _smooth_forward_velocity << "\n";
  }
}

/**
 * Records the delay measured in receiving this particular timestamp.  The
 * average delay of the last n timestamps will be used to smooth the motion
 * properly.
 */
void SmoothMover::
record_timestamp_delay(double timestamp) {
  double now = ClockObject::get_global_clock()->get_frame_time();

  // Convert the delay to an integer number of milliseconds.  Integers are
  // better than doubles because they don't accumulate errors over time.
  int delay = (int)((now - timestamp) * 1000.0);
  if (_timestamp_delays.full()) {
    _net_timestamp_delay -= _timestamp_delays.front();
    _timestamp_delays.pop_front();
  }
  _net_timestamp_delay += delay;
  _timestamp_delays.push_back(delay);

  _last_heard_from = now;
}

/**
 * Node is being wrtReparented, update recorded sample positions to reflect
 * new parent
 */
void SmoothMover::
handle_wrt_reparent(NodePath &old_parent, NodePath &new_parent) {
  Points::iterator pi;
  NodePath np = old_parent.attach_new_node("smoothMoverWrtReparent");

  // cout << "handle_wrt_reparent: ";
  for (pi = _points.begin(); pi != _points.end(); pi++) {
    np.set_pos_hpr((*pi)._pos, (*pi)._hpr);
    (*pi)._pos = np.get_pos(new_parent);
    (*pi)._hpr = np.get_hpr(new_parent);
    // cout << "(" << (*pi)._pos << "), ";
  }
  // cout << endl;

  np.set_pos_hpr(_sample._pos, _sample._hpr);
  _sample._pos = np.get_pos(new_parent);
  _sample._hpr = np.get_hpr(new_parent);

  np.set_pos_hpr(_smooth_pos, _smooth_hpr);
  _smooth_pos = np.get_pos(new_parent);
  _smooth_hpr = np.get_hpr(new_parent);

  _computed_forward_axis = false;

  np.detach_node();
}
