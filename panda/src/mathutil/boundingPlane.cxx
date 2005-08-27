// Filename: boundingPlane.cxx
// Created by:  drose (19Aug05)
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

#include "boundingPlane.h"
#include "boundingSphere.h"
#include "config_mathutil.h"

TypeHandle BoundingPlane::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: BoundingPlane::make_copy
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
BoundingVolume *BoundingPlane::
make_copy() const {
  return new BoundingPlane(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingPlane::get_approx_center
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
LPoint3f BoundingPlane::
get_approx_center() const {
  nassertr(!is_empty(), LPoint3f(0.0f, 0.0f, 0.0f));
  nassertr(!is_infinite(), LPoint3f(0.0f, 0.0f, 0.0f));
  return _plane.get_point();
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingPlane::xform
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void BoundingPlane::
xform(const LMatrix4f &mat) {
  nassertv(!mat.is_nan());

  if (!is_empty() && !is_infinite()) {
    _plane.xform(mat);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingPlane::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void BoundingPlane::
output(ostream &out) const {
  if (is_empty()) {
    out << "bplane, empty";
  } else if (is_infinite()) {
    out << "bplane, infinite";
  } else {
    out << "bplane: " << _plane;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingPlane::extend_other
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
bool BoundingPlane::
extend_other(BoundingVolume *other) const {
  return other->extend_by_plane(this);
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingPlane::around_other
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
bool BoundingPlane::
around_other(BoundingVolume *other,
             const BoundingVolume **first,
             const BoundingVolume **last) const {
  return other->around_planes(first, last);
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingPlane::contains_other
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
int BoundingPlane::
contains_other(const BoundingVolume *other) const {
  return other->contains_plane(this);
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingPlane::extend_by_plane
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
bool BoundingPlane::
extend_by_plane(const BoundingPlane *plane) {
  nassertr(!plane->is_empty() && !plane->is_infinite(), false);
  nassertr(!is_infinite(), false);

  if (is_empty()) {
    _plane = plane->get_plane();
    _flags = 0;
  } else {
    _flags = F_infinite;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingPlane::contains_sphere
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
int BoundingPlane::
contains_sphere(const BoundingSphere *sphere) const {
  nassertr(!is_empty() && !is_infinite(), 0);
  nassertr(!sphere->is_empty() && !sphere->is_infinite(), 0);

  float r = sphere->get_radius();
  float d = _plane.dist_to_plane(sphere->get_center());

  if (d <= -r) {
    // The sphere is completely behind the plane.
    return IF_all | IF_possible | IF_some;

  } else if (d <= r) {
    // The sphere is intersecting with the plane itself.
    return IF_possible | IF_some;

  } else {
    // The sphere is completely in front of the plane.
    return IF_no_intersection;
  }
}
