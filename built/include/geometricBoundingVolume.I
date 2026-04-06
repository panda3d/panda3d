/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geometricBoundingVolume.I
 * @author drose
 * @date 1999-10-07
 */

/**
 *
 */
INLINE_MATHUTIL GeometricBoundingVolume::
GeometricBoundingVolume() {
#ifdef DO_MEMORY_USAGE
  MemoryUsage::update_type(this, this);
#endif
}

/**
 * Increases the size of the volume to include the given volume.
 */
INLINE_MATHUTIL bool GeometricBoundingVolume::
extend_by(const GeometricBoundingVolume *vol) {
  return BoundingVolume::extend_by(vol);
}

/**
 * Increases the size of the volume to include the given point.
 */
INLINE_MATHUTIL bool GeometricBoundingVolume::
extend_by(const LPoint3 &point) {
  return extend_by_point(point);
}

/**
 * Resets the volume to enclose only the volumes indicated.
 */
INLINE_MATHUTIL bool GeometricBoundingVolume::
around(const GeometricBoundingVolume **first,
       const GeometricBoundingVolume **last) {
  return BoundingVolume::around((const BoundingVolume **)first,
                                (const BoundingVolume **)last);
}

/**
 * Resets the volume to enclose only the points indicated.
 */
INLINE_MATHUTIL bool GeometricBoundingVolume::
around(const LPoint3 *first, const LPoint3 *last) {
  _flags = F_empty;
  if (first != last) {
    return around_points(first, last);
  }
  return true;
}


/**
 * Returns the appropriate set of IntersectionFlags to indicate the amount of
 * intersection with the indicated volume.
 */
INLINE_MATHUTIL int GeometricBoundingVolume::
contains(const GeometricBoundingVolume *vol) const {
  return BoundingVolume::contains(vol);
}

/**
 * Returns the appropriate set of IntersectionFlags to indicate the amount of
 * intersection with the indicated point.
 */
INLINE_MATHUTIL int GeometricBoundingVolume::
contains(const LPoint3 &point) const {
  if (is_empty()) {
    return IF_no_intersection;
  }

  return contains_point(point);
}

/**
 * Returns the appropriate set of IntersectionFlags to indicate the amount of
 * intersection with the indicated line segment.
 */
INLINE_MATHUTIL int GeometricBoundingVolume::
contains(const LPoint3 &a, const LPoint3 &b) const {
  if (is_empty()) {
    return IF_no_intersection;
  }

  return contains_lineseg(a, b);
}
