// Filename: stitchCylindricalScreen.cxx
// Created by:  drose (16Jul01)
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

#include "stitchCylindricalScreen.h"
#include "deg_2_rad.h"


////////////////////////////////////////////////////////////////////
//     Function: StitchCylindricalScreen::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
StitchCylindricalScreen::
StitchCylindricalScreen() {
  // The full-circle range is -pi to pi; we round up here to ensure we
  // get the full circle.
  _start_angle = -4.0;
  _end_angle = 4.0;

  _bottom = -1.0;
  _top = 1.0;
  _radius = 1.0;
  _flags = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: StitchCylindricalScreen::set_angle
//       Access: Public
//  Description: Sets the angular range of the screen.  The range is
//               in degrees, from -180 to 180, where 0 degrees is
//               directly ahead, -90 is to the left, and +90 is to the
//               right.
//
//               If this is unset, the default is a full-circle
//               cylinder.
////////////////////////////////////////////////////////////////////
void StitchCylindricalScreen::
set_angle(double start, double end) {
  // We store the angles in radians.
  _start_angle = deg_2_rad(start);
  _end_angle = deg_2_rad(end);
  _flags |= F_angle;
}

////////////////////////////////////////////////////////////////////
//     Function: StitchCylindricalScreen::set_height
//       Access: Public
//  Description: Sets the bottom and top limits of the screen.  These
//               are the heights above (or below) the XY plane for the
//               bottom edge and top edge of the screen.
//
//               If this is unset, the default is an infinitely tall
//               cylinder.
////////////////////////////////////////////////////////////////////
void StitchCylindricalScreen::
set_height(double bottom, double top) {
  _bottom = bottom;
  _top = top;
  _flags |= F_height;
}

////////////////////////////////////////////////////////////////////
//     Function: StitchCylindricalScreen::set_radius
//       Access: Public
//  Description: Sets the radius of the screen.  The default is 1.0.
////////////////////////////////////////////////////////////////////
void StitchCylindricalScreen::
set_radius(double radius) {
  _radius = radius;
}

////////////////////////////////////////////////////////////////////
//     Function: StitchCylindricalScreen::compute_intersect
//       Access: Protected, Virtual
//  Description: Computes the intersection point for the ray beginning
//               at the indicated origin point and continuing in the
//               indicated direction, to infinity.  The return value
//               is a number >= 0.0 that indicates the parametric
//               intersection point along the ray with the screen
//               geometry, or a number < 0.0 if there is no
//               intersection with the screen.
////////////////////////////////////////////////////////////////////
double StitchCylindricalScreen::
compute_intersect(const LPoint3d &origin, const LVector3d &direction) const {
  LPoint3d p = origin * _inv_transform;
  LVector3d d = direction * _inv_transform;

  // The cylinder is always centered on the origin with its axis
  // parallel to the Z axis.  Project our ray into the XY plane and
  // determine the point of intersection with the circular
  // cross-section.

  double Px = p[0];
  double Py = p[1];
  double Dx = d[0];
  double Dy = d[1];

  // Now use the quadratic equation to solve for the intersection of
  // the line passing thing p2 and (p2+d2), and the circle centered at
  // the origin with the radius _radius.

  double a = Dx * Dx + Dy * Dy;
  double b = 2 * (Px * Dx + Py * Dy);
  double c = Px * Px + Py * Py - _radius * _radius;

  // If the radical is negative, there is no intersection with the
  // circle.
  double radical = b * b - 4.0 * a * c;
  if (radical < 0.0) {
    return -1.0;
  }

  // There are two possible intersection points.  We have to consider
  // both, and if both are valid, we return the closer.
  double sqrt_radical = sqrt(radical);
  double t1 = (-b + sqrt_radical) / (2.0 * a);
  double t2 = (-b - sqrt_radical) / (2.0 * a);

  validate_point(t1, p, d);
  validate_point(t2, p, d);

  if (t1 >= 0.0) {
    if (t2 >= 0.0) {
      return min(t1, t2);
    } else {
      return t1;
    }
  } else {
    return t2;
  }
}

// Check that the given intersection point defines a point in front of
// the ray and on the surface of the cylindrical screen.  If it is off
// the screen, resets t to -1; otherwise, leaves it alone.
void StitchCylindricalScreen::
validate_point(double &t,
               const LPoint3d &origin, 
               const LVector3d &direction) const {
  // Only test points that are in front of the ray anyway.
  if (t >= 0.0) {
    // Now get the 3-d point of intersection.
    LPoint3d p = origin + direction * t;

    // If this is above the top of the cylinder, or below the bottom
    // of it, there's no intersection.
    if ((_flags & F_height) != 0) {
      if (p[2] < _bottom || p[2] > _top) {
        t = -1.0;
        return;
      }
    }

    if ((_flags & F_angle) != 0) {
      // Project the point into the XY plane and get the angle from
      // the Y axis, in radians.
      double angle = atan2(p[0], p[1]);

      // If the angle is outside of the range of the screen, reject it
      // also.
      if (angle < _start_angle || angle > _end_angle) {
        t = -1.0;
        return;
      }
    }
  }
}
