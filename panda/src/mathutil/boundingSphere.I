// Filename: boundingSphere.I
// Created by:  drose (02Oct99)
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

