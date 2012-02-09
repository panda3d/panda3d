// Filename: intersectionBoundingVolume.I
// Created by:  drose (08Feb12)
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



////////////////////////////////////////////////////////////////////
//     Function: IntersectionBoundingVolume::Constructor
//       Access: Published
//  Description: Constructs an empty intersection.
////////////////////////////////////////////////////////////////////
INLINE_MATHUTIL IntersectionBoundingVolume::
IntersectionBoundingVolume() {
  _flags = F_infinite;
}

////////////////////////////////////////////////////////////////////
//     Function: IntersectionBoundingVolume::get_num_components
//       Access: Published
//  Description: Returns the number of components in the intersection.
////////////////////////////////////////////////////////////////////
INLINE_MATHUTIL int IntersectionBoundingVolume::
get_num_components() const {
  return (int)_components.size();
}

////////////////////////////////////////////////////////////////////
//     Function: IntersectionBoundingVolume::get_component
//       Access: Published
//  Description: Returns the nth component in the intersection.
////////////////////////////////////////////////////////////////////
INLINE_MATHUTIL const GeometricBoundingVolume *IntersectionBoundingVolume::
get_component(int n) const {
  nassertr(n >= 0 && n < (int)_components.size(), NULL);
  return _components[n];
}
