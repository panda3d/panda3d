// Filename: boundingLine.I
// Created by:  drose (04Jul00)
// 
////////////////////////////////////////////////////////////////////

INLINE BoundingLine::
BoundingLine() {
}

INLINE BoundingLine::
BoundingLine(const LPoint3f &a, const LPoint3f &b) :
  _origin(a), _vector(b - a) 
{
  _vector.normalize();
  _flags = 0;
  nassertd(!_origin.is_nan() && !_vector.is_nan()) {
    _flags = F_empty;
  }
}

INLINE const LPoint3f &BoundingLine::
get_point_a() const {
  nassertr(!is_empty(), _origin);
  nassertr(!is_infinite(), _origin);
  return _origin;
}

INLINE LPoint3f BoundingLine::
get_point_b() const {
  nassertr(!is_empty(), _origin);
  nassertr(!is_infinite(), _origin);
  return _origin + _vector;
}
