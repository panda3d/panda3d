// Filename: boundingSphere.I
// Created by:  drose (02Oct99)
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
//     Function: BoundingSphere::Constructor
//       Access: Published
//  Description: Constructs an empty sphere.
////////////////////////////////////////////////////////////////////
INLINE_MATHUTIL BoundingSphere::
BoundingSphere() {
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingSphere::Constructor
//       Access: Published
//  Description: Constructs a specific sphere.
////////////////////////////////////////////////////////////////////
INLINE_MATHUTIL BoundingSphere::
BoundingSphere(const LPoint3 &center, PN_stdfloat radius) :
  _center(center), _radius(radius)
{
  _flags = 0;
  nassertd(!_center.is_nan() && !cnan(_radius)) {
    _flags = F_empty;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingSphere::get_center
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE_MATHUTIL LPoint3 BoundingSphere::
get_center() const {
  nassertr(!is_empty(), LPoint3::zero());
  nassertr(!is_infinite(), LPoint3::zero());
  return _center;
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingSphere::get_radius
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE_MATHUTIL PN_stdfloat BoundingSphere::
get_radius() const {
  nassertr(!is_empty(), 0.0f);
  nassertr(!is_infinite(), 0.0f);
  return _radius;
}

