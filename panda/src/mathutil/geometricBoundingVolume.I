// Filename: geometricBoundingVolume.I
// Created by:  drose (07Oct99)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: GeometricBoundingVolume::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE GeometricBoundingVolume::
GeometricBoundingVolume() {
}

////////////////////////////////////////////////////////////////////
//     Function: GeometricBoundingVolume::extend_by
//       Access: Public
//  Description: Increases the size of the volume to include the given
//               volume.
////////////////////////////////////////////////////////////////////
INLINE bool GeometricBoundingVolume::
extend_by(const GeometricBoundingVolume *vol) {
  return BoundingVolume::extend_by(vol);
}

////////////////////////////////////////////////////////////////////
//     Function: GeometricBoundingVolume::extend_by
//       Access: Public
//  Description: Increases the size of the volume to include the given
//               point.
////////////////////////////////////////////////////////////////////
INLINE bool GeometricBoundingVolume::
extend_by(const LPoint3f &point) {
  return extend_by_point(point);
}

////////////////////////////////////////////////////////////////////
//     Function: GeometricBoundingVolume::around
//       Access: Public
//  Description: Resets the volume to enclose only the volumes
//               indicated.
////////////////////////////////////////////////////////////////////
INLINE bool GeometricBoundingVolume::
around(const GeometricBoundingVolume **first, 
       const GeometricBoundingVolume **last) {
  return BoundingVolume::around((const BoundingVolume **)first, 
				(const BoundingVolume **)last);
}

////////////////////////////////////////////////////////////////////
//     Function: GeometricBoundingVolume::around
//       Access: Public
//  Description: Resets the volume to enclose only the points
//               indicated.
////////////////////////////////////////////////////////////////////
INLINE bool GeometricBoundingVolume::
around(const LPoint3f *first, const LPoint3f *last) {
  _flags = F_empty;
  if (first != last) {
    return around_points(first, last);
  }
  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: GeometricBoundingVolume::contains
//       Access: Public
//  Description: Returns the appropriate set of IntersectionFlags to
//               indicate the amount of intersection with the
//               indicated volume.
////////////////////////////////////////////////////////////////////
INLINE int GeometricBoundingVolume::
contains(const GeometricBoundingVolume *vol) const {
  return BoundingVolume::contains(vol);
}

////////////////////////////////////////////////////////////////////
//     Function: GeometricBoundingVolume::contains
//       Access: Public
//  Description: Returns the appropriate set of IntersectionFlags to
//               indicate the amount of intersection with the
//               indicated point.
////////////////////////////////////////////////////////////////////
INLINE int GeometricBoundingVolume::
contains(const LPoint3f &point) const {
  if (is_empty()) {
    return IF_no_intersection;
  }

  return contains_point(point);
}

////////////////////////////////////////////////////////////////////
//     Function: GeometricBoundingVolume::contains
//       Access: Public
//  Description: Returns the appropriate set of IntersectionFlags to
//               indicate the amount of intersection with the
//               indicated line segment.
////////////////////////////////////////////////////////////////////
INLINE int GeometricBoundingVolume::
contains(const LPoint3f &a, const LPoint3f &b) const {
  if (is_empty()) {
    return IF_no_intersection;
  }

  return contains_lineseg(a, b);
}
