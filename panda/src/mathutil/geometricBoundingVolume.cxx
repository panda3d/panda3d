// Filename: geometricBoundingVolume.cxx
// Created by:  drose (07Oct99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "geometricBoundingVolume.h"

TypeHandle GeometricBoundingVolume::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: GeometricBoundingVolume::as_geometric_bounding_volume
//       Access: Public, Virtual
//  Description: Virtual downcast method.  Returns this object as a
//               pointer of the indicated type, if it is in fact that
//               type.  Returns NULL if it is not that type.
////////////////////////////////////////////////////////////////////
const GeometricBoundingVolume *GeometricBoundingVolume::
as_geometric_bounding_volume() const {
  return this;
}

////////////////////////////////////////////////////////////////////
//     Function: GeometricBoundingVolume::extend_by_point
//       Access: Protected, Virtual
//  Description: Extends the volume to include the indicated point.
//               Returns true if possible, false if not.
////////////////////////////////////////////////////////////////////
bool GeometricBoundingVolume::
extend_by_point(const LPoint3 &) {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: GeometricBoundingVolume::around_points
//       Access: Protected, Virtual
//  Description: Puts the volume around the indicated list of points,
//               identified by an STL-style begin/end list.
////////////////////////////////////////////////////////////////////
bool GeometricBoundingVolume::
around_points(const LPoint3 *, const LPoint3 *) {
  _flags = F_empty;
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: GeometricBoundingVolume::contains_point
//       Access: Protected, Virtual
//  Description: Tests whether the volume contains the indicated
//               point.
////////////////////////////////////////////////////////////////////
int GeometricBoundingVolume::
contains_point(const LPoint3 &) const {
  return IF_dont_understand;
}

////////////////////////////////////////////////////////////////////
//     Function: GeometricBoundingVolume::contains_lineseg
//       Access: Protected, Virtual
//  Description: Tests whether the volume contains the indicated line
//               segment.
////////////////////////////////////////////////////////////////////
int GeometricBoundingVolume::
contains_lineseg(const LPoint3 &, const LPoint3 &) const {
  return IF_dont_understand;
}
