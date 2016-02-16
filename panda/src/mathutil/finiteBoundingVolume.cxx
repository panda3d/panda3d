/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file finiteBoundingVolume.cxx
 * @author drose
 * @date 1999-10-02
 */

#include "finiteBoundingVolume.h"
#include "boundingBox.h"
#include "config_mathutil.h"

TypeHandle FiniteBoundingVolume::_type_handle;

/**

 */
PN_stdfloat FiniteBoundingVolume::
get_volume() const {
  nassertr(!is_infinite(), 0.0f);
  if (is_empty()) {
    return 0.0f;
  }

  mathutil_cat.warning()
    << get_type() << "::get_volume() called\n";

  // We don't know how to compute the volume of this shape correctly;
  // just calculate the volume of its containing box.
  BoundingBox box(get_min(), get_max());
  box.local_object();
  return box.get_volume();
}

/**
 * Virtual downcast method.  Returns this object as a pointer of the indicated
 * type, if it is in fact that type.  Returns NULL if it is not that type.
 */
const FiniteBoundingVolume *FiniteBoundingVolume::
as_finite_bounding_volume() const {
  return this;
}

/**
 * Double-dispatch support: called by around_other() when the type of the first
 * element in the list is known to be a nonempty line.
 */
bool FiniteBoundingVolume::
around_lines(const BoundingVolume **, const BoundingVolume **) {
  _flags = F_infinite;

  // Since it's a FiniteBoundingVolume, we can't do any better than
  // making it infinite.  So we return true.
  return true;
}

/**
 * Double-dispatch support: called by around_other() when the type of the first
 * element in the list is known to be a nonempty plane.
 */
bool FiniteBoundingVolume::
around_planes(const BoundingVolume **, const BoundingVolume **) {
  _flags = F_infinite;

  // Since it's a FiniteBoundingVolume, we can't do any better than
  // making it infinite.  So we return true.
  return true;
}
