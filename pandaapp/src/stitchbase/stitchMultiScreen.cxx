// Filename: stitchMultiScreen.cxx
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

#include "stitchMultiScreen.h"

////////////////////////////////////////////////////////////////////
//     Function: StitchMultiScreen::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
StitchMultiScreen::
StitchMultiScreen() {
}

////////////////////////////////////////////////////////////////////
//     Function: StitchMultiScreen::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
StitchMultiScreen::
~StitchMultiScreen() {
}

////////////////////////////////////////////////////////////////////
//     Function: StitchMultiScreen::add_screen
//       Access: Public
//  Description: Adds a new screen to the set of screens in the set.
////////////////////////////////////////////////////////////////////
void StitchMultiScreen::
add_screen(StitchScreen *screen) {
  _screens.insert(screen);
}

////////////////////////////////////////////////////////////////////
//     Function: StitchMultiScreen::clear_screens
//       Access: Public
//  Description: Empties the set of screens from the set.
////////////////////////////////////////////////////////////////////
void StitchMultiScreen::
clear_screens() {
  _screens.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: StitchMultiScreen::is_empty
//       Access: Public
//  Description: Returns true if there are no screens in the set, or
//               false if there is at least one.
////////////////////////////////////////////////////////////////////
bool StitchMultiScreen::
is_empty() const {
  return _screens.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: StitchMultiScreen::intersect
//       Access: Public, Virtual
//  Description: Determines the intersection point of a ray starting
//               at the indicated origin, and continuing infinitely in
//               the indicated direction, with the screen.
//
//               If there is no intersection, leaves result undefined
//               and returns false.  If there is an intersection, sets
//               result to the intersection point and returns true.
////////////////////////////////////////////////////////////////////
bool StitchMultiScreen::
intersect(LPoint3d &result, 
          const LPoint3d &origin,
          const LVector3d &direction) const {
  if (_screens.empty()) {
    // If we have no screens, the direction is the same as the result.
    // This acts as if we have an infinitely large screen at infinity.
    result = direction;
    return true;
  }

  return StitchScreen::intersect(result, origin, direction);
}

////////////////////////////////////////////////////////////////////
//     Function: StitchMultiScreen::compute_intersect
//       Access: Protected, Virtual
//  Description: Computes the intersection point for the ray beginning
//               at the indicated origin point and continuing in the
//               indicated direction, to infinity.  The return value
//               is a number >= 0.0 that indicates the parametric
//               intersection point along the ray with the screen
//               geometry, or a number < 0.0 if there is no
//               intersection with the screen.
////////////////////////////////////////////////////////////////////
double StitchMultiScreen::
compute_intersect(const LPoint3d &origin, const LVector3d &direction) const {
  LPoint3d p = origin * _inv_transform;
  LVector3d d = direction * _inv_transform;
  double best = -1.0;

  // Walk through all of our screens and find the closest valid
  // intersection point.
  Screens::const_iterator si;
  for (si = _screens.begin(); si != _screens.end(); ++si) {
    double t = (*si)->compute_intersect(p, d);
    if (t >= 0.0) {
      if (best >= 0.0) {
        best = min(best, t);
      } else {
        best = t;
      }
    }
  }

  return best;
}
