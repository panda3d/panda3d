// Filename: boundingSphere.I
// Created by:  drose (02Oct99)
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

INLINE_MATHUTIL BoundingSphere::
BoundingSphere() {
}

INLINE_MATHUTIL BoundingSphere::
BoundingSphere(const LPoint3f &center, float radius) :
  _center(center), _radius(radius)
{
  _flags = 0;
  nassertd(!_center.is_nan() && !cnan(_radius)) {
    _flags = F_empty;
  }
}

INLINE_MATHUTIL const LPoint3f &BoundingSphere::
get_center() const {
  nassertr(!is_empty(), _center);
  nassertr(!is_infinite(), _center);
  return _center;
}

INLINE_MATHUTIL float BoundingSphere::
get_radius() const {
  nassertr(!is_empty(), 0.0f);
  nassertr(!is_infinite(), 0.0f);
  return _radius;
}

