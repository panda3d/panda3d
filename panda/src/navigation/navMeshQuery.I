/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file navMeshQuery.I
 * @author Maxwell175
 * @date 2022-02-21
 */


/**
 * Returns the exclusion mask that will filter the walkable polygons.
 */
INLINE NavMeshQueryFilter NavMeshQuery::
get_filter() const {
  return _filter;
}

/**
 * Sets the exclusion mask that will filter the walkable polygons.
 */
INLINE void NavMeshQuery::
set_filter(NavMeshQueryFilter &filter) {
  _filter = filter;
}
