// Filename: boundingHexahedron.I
// Created by:  drose (03Oct99)
// 
////////////////////////////////////////////////////////////////////

INLINE_MATHUTIL BoundingHexahedron::
BoundingHexahedron() {
}

INLINE_MATHUTIL int BoundingHexahedron::
get_num_points() const {
  return num_points;
}

INLINE_MATHUTIL LPoint3f BoundingHexahedron::
get_point(int n) const {
  nassertr(n >= 0 && n < num_points, LPoint3f(0.0f, 0.0f, 0.0f));
  return _points[n];
}

INLINE_MATHUTIL int BoundingHexahedron::
get_num_planes() const {
  return num_planes;
}

INLINE_MATHUTIL Planef BoundingHexahedron::
get_plane(int n) const {
  nassertr(n >= 0 && n < num_planes, Planef());
  return _planes[n];
}
