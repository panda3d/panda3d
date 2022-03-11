/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file navMeshQueryFilter.I
 * @author Maxwell175
 * @date 2022-02-29
 */


/**
 * Returns the include mask that will filter the walkable polygons.
 */
#include "navMeshQueryFilter.h"

INLINE BitMask16 NavMeshQueryFilter::
get_include_mask() const {
  return _include_mask;
}

/**
 * Sets the include mask that will filter the walkable polygons.
 */
INLINE void NavMeshQueryFilter::
set_include_mask(BitMask16 mask) {
  _include_mask = mask;
}

/**
 * Returns the exclusion mask that will filter the walkable polygons.
 */
INLINE BitMask16 NavMeshQueryFilter::
get_exclude_mask() const {
  return _exclude_mask;
}

/**
 * Sets the exclusion mask that will filter the walkable polygons.
 */
INLINE void NavMeshQueryFilter::
set_exclude_mask(BitMask16 mask) {
  _exclude_mask = mask;
}

INLINE bool NavMeshQueryFilter::
operator == (const NavMeshQueryFilter &other) const {
  return _include_mask == other.get_include_mask() && _exclude_mask == other.get_exclude_mask();
}

INLINE bool NavMeshQueryFilter::
operator != (const NavMeshQueryFilter &other) const {
  return _include_mask != other.get_include_mask() || _exclude_mask != other.get_exclude_mask();
}
