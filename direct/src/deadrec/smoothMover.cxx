// Filename: smoothMover.cxx
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

#include "smoothMover.h"
#include "notify.h"
#include "compose_matrix.h"

SmoothMover::SmoothMode SmoothMover::_smooth_mode = SmoothMover::SM_off;
SmoothMover::PredictionMode SmoothMover::_prediction_mode = SmoothMover::PM_off;
double SmoothMover::_delay = 0.0;

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
  _sample._flags = 0;

  _smooth_pos.set(0.0, 0.0, 0.0);
  _smooth_hpr.set(0.0, 0.0, 0.0);
  _smooth_mat = LMatrix4f::ident_mat();
  _computed_smooth_mat = true;
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
    _smooth_pos = _sample._pos;
    _smooth_hpr = _sample._hpr;
    _computed_smooth_mat = false;

  } else {
    // Otherwise, smoothing is in effect and we store a true position
    // report.
    
    if (_points.full()) {
      // If we have too many position reports, throw away the oldest
      // one.
      _points.pop_front();
    }
    
    _points.push_back(_sample);
    _sample._flags = 0;
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
////////////////////////////////////////////////////////////////////
void SmoothMover::
compute_smooth_position(double timestamp) {
  if (_smooth_mode == SM_off || _points.empty()) {
    // With smoothing disabled, or with no position reports available,
    // this function does nothing.
    return;
  }

  // First, back up in time by the specified delay factor.
  timestamp -= _delay;

  // Now look for the two bracketing position reports.
  int point_before = -1;
  double timestamp_before = 0.0;
  int point_after = -1;
  double timestamp_after = 0.0;

  int num_points = _points.size();
  for (int i = 0; i < num_points; i++) {
    const SamplePoint &point = _points[i];
    if (point._timestamp <= timestamp) {
      // This point is before the desired time.  Find the newest of
      // these.
      if (point_before == -1 || point._timestamp > timestamp_before) {
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
    nassertv(point_after != -1);
    // If we only have an after point, we have to start there.
    const SamplePoint &point = _points[point_after];
    _smooth_pos = point._pos;
    _smooth_hpr = point._hpr;
    _computed_smooth_mat = false;
    return;
  }

  if (point_after == -1) {
    // If we only have a before point, we have to stop there, unless
    // we have prediction in effect.
    const SamplePoint &point = _points[point_before];
    _smooth_pos = point._pos;
    _smooth_hpr = point._hpr;
    _computed_smooth_mat = false;

  } else {
    // If we have two points, we can linearly interpolate between them.
    const SamplePoint &point_b = _points[point_before];
    const SamplePoint &point_a = _points[point_after];

    double t = (timestamp - timestamp_before) / (timestamp_after - timestamp_before);
    _smooth_pos = point_b._pos + t * (point_a._pos - point_b._pos);
    _smooth_hpr = point_b._hpr + t * (point_a._hpr - point_b._hpr);
    _computed_smooth_mat = false;
  }

  // Assume we'll never get another compute_smooth_position() request
  // for an older time than this, and remove all the timestamps at the
  // head of the queue before point_before.
  while (!_points.empty() && _points.front()._timestamp < timestamp_before) {
    _points.pop_front();
  }
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
