// Filename: boundingSphere.I
// Created by:  drose (02Oct99)
// 
////////////////////////////////////////////////////////////////////

INLINE BoundingSphere::
BoundingSphere() {
}

INLINE BoundingSphere::
BoundingSphere(const LPoint3f &center, float radius) :
  _center(center), _radius(radius) 
{
  _flags = 0;
  nassertd(!_center.is_nan() && !cnan(_radius)) {
    _flags = F_empty;
  }
}

INLINE const LPoint3f &BoundingSphere::
get_center() const {
  nassertr(!is_empty(), _center);
  nassertr(!is_infinite(), _center);
  return _center;
}

INLINE float BoundingSphere::
get_radius() const {
  nassertr(!is_empty(), 0.0);
  nassertr(!is_infinite(), 0.0);
  return _radius;
}

