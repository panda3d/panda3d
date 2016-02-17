/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geometricBoundingVolume.cxx
 * @author drose
 * @date 1999-10-07
 */

#include "geometricBoundingVolume.h"

TypeHandle GeometricBoundingVolume::_type_handle;


/**
 * Virtual downcast method.  Returns this object as a pointer of the indicated
 * type, if it is in fact that type.  Returns NULL if it is not that type.
 */
GeometricBoundingVolume *GeometricBoundingVolume::
as_geometric_bounding_volume() {
  return this;
}

/**
 * Virtual downcast method.  Returns this object as a pointer of the indicated
 * type, if it is in fact that type.  Returns NULL if it is not that type.
 */
const GeometricBoundingVolume *GeometricBoundingVolume::
as_geometric_bounding_volume() const {
  return this;
}

/**
 * Extends the volume to include the indicated point.  Returns true if possible,
 * false if not.
 */
bool GeometricBoundingVolume::
extend_by_point(const LPoint3 &) {
  return false;
}

/**
 * Puts the volume around the indicated list of points, identified by an STL-
 * style begin/end list.
 */
bool GeometricBoundingVolume::
around_points(const LPoint3 *, const LPoint3 *) {
  _flags = F_empty;
  return false;
}

/**
 * Tests whether the volume contains the indicated point.
 */
int GeometricBoundingVolume::
contains_point(const LPoint3 &) const {
  return IF_dont_understand;
}

/**
 * Tests whether the volume contains the indicated line segment.
 */
int GeometricBoundingVolume::
contains_lineseg(const LPoint3 &, const LPoint3 &) const {
  return IF_dont_understand;
}
