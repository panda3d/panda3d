// Filename: boundingHexahedron.I
// Created by:  drose (03Oct99)
// 
////////////////////////////////////////////////////////////////////

INLINE BoundingHexahedron::
BoundingHexahedron() {
}

INLINE int BoundingHexahedron::
get_num_points() const {
  return num_points;
}

INLINE LPoint3f BoundingHexahedron::
get_point(int n) const {
  nassertr(n >= 0 && n < num_points, LPoint3f(0.0, 0.0, 0.0));
  return _points[n];
}

INLINE int BoundingHexahedron::
get_num_planes() const {
  return num_planes;
}

INLINE Planef BoundingHexahedron::
get_plane(int n) const {
  nassertr(n >= 0 && n < num_planes, Planef());
  return _planes[n];
}
