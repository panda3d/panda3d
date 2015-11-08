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
BoundingSphere() : _center(0) {
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

////////////////////////////////////////////////////////////////////
//     Function: BoundingSphere::set_center
//       Access: Published
//  Description: Sets the center point of the sphere.
////////////////////////////////////////////////////////////////////
INLINE_MATHUTIL void BoundingSphere::
set_center(const LPoint3 &center) {
  nassertv(!center.is_nan());
  _center = center;
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingSphere::set_radius
//       Access: Published
//  Description: Sets the radius of the sphere.
////////////////////////////////////////////////////////////////////
INLINE_MATHUTIL void BoundingSphere::
set_radius(PN_stdfloat radius) {
  nassertv(!cnan(radius));
  _radius = radius;
  _flags = 0;
}
