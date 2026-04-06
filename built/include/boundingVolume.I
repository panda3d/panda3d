/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file boundingVolume.I
 * @author drose
 * @date 1999-10-01
 */

/**
 *
 */
INLINE_MATHUTIL BoundingVolume::
BoundingVolume() {
  _flags = F_empty;
}

/**
 * Any kind of volume might be empty.  This is a degenerate volume that
 * contains no points; it's not the same as, for instance, a sphere with
 * radius zero, since that contains one point (the center).  It intersects
 * with no other volumes.
 */
INLINE_MATHUTIL bool BoundingVolume::
is_empty() const {
  return (_flags & F_empty) != 0;
}

/**
 * The other side of the empty coin is an infinite volume.  This is a
 * degenerate state of a normally finite volume that contains all points.
 * (Note that some kinds of infinite bounding volumes, like binary separating
 * planes, do not contain all points and thus correctly return is_infinite()
 * == false, even though they are technically infinite.  This is a special
 * case of the word 'infinite' meaning the volume covers all points in space.)
 *
 * It completely intersects with all other volumes except empty volumes.
 */
INLINE_MATHUTIL bool BoundingVolume::
is_infinite() const {
  return (_flags & F_infinite) != 0;
}

/**
 * Marks the volume as infinite, even if it is normally finite.  You can think
 * of this as an infinite extend_by() operation.
 */
INLINE_MATHUTIL void BoundingVolume::
set_infinite() {
  _flags = F_infinite;
}

/**
 * Increases the size of the volume to include the given volume.
 */
INLINE_MATHUTIL bool BoundingVolume::
extend_by(const BoundingVolume *vol) {
  if (vol->is_infinite()) {
    set_infinite();

  } else if (!vol->is_empty()) {
    // This is a double-dispatch.  We call this virtual function on the volume
    // we were given, which will in turn call the appropriate virtual function
    // in our own class to perform the operation.
    return vol->extend_other(this);
  }
  return true;
}


/**
 * Returns the appropriate set of IntersectionFlags to indicate the amount of
 * intersection with the indicated volume.
 */
INLINE_MATHUTIL int BoundingVolume::
contains(const BoundingVolume *vol) const {
  if (is_empty() || vol->is_empty()) {
    return IF_no_intersection;

  } else if (is_infinite()) {
    return IF_possible | IF_some | IF_all;

  } else if (vol->is_infinite()) {
    return IF_possible | IF_some;
  }

  // This is a double-dispatch.  We call this virtual function on the volume
  // we were given, which will in turn call the appropriate virtual function
  // in our own class to perform the operation.
  return vol->contains_other(this);
}

INLINE_MATHUTIL std::ostream &operator << (std::ostream &out, const BoundingVolume &bound) {
  bound.output(out);
  return out;
}
