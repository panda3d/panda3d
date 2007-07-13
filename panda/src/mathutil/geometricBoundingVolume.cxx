// Filename: geometricBoundingVolume.cxx
// Created by:  drose (07Oct99)
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
extend_by_point(const LPoint3f &) {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: GeometricBoundingVolume::around_points
//       Access: Protected, Virtual
//  Description: Puts the volume around the indicated list of points,
//               identified by an STL-style begin/end list.
////////////////////////////////////////////////////////////////////
bool GeometricBoundingVolume::
around_points(const LPoint3f *, const LPoint3f *) {
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
contains_point(const LPoint3f &) const {
  return IF_dont_understand;
}

////////////////////////////////////////////////////////////////////
//     Function: GeometricBoundingVolume::contains_lineseg
//       Access: Protected, Virtual
//  Description: Tests whether the volume contains the indicated line
//               segment.
////////////////////////////////////////////////////////////////////
int GeometricBoundingVolume::
contains_lineseg(const LPoint3f &, const LPoint3f &) const {
  return IF_dont_understand;
}
