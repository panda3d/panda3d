/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file navMeshQuery.cxx
 * @author ashwini
 * @date 2020-060-21
 */

#include "navMeshQuery.h"
#include "DetourNavMeshQuery.h"

bool NavMeshQuery::nearest_point(LPoint3 &p) {
  if (!_nav_query) {
    std::cout << "\nNavMeshQuery not created!\n";
    return false;
  }

  const float center[3] = { p[0], p[1], p[2] };
  float *nearest_p = new float[3];
  const float extents[3] = { 2 , 4 , 2 };

  dtQueryFilter filter;
  filter.setIncludeFlags(RC_WALKABLE_AREA);
  filter.setExcludeFlags(RC_NULL_AREA);
  dtPolyRef nearest_poly_ref_id = 0;

  dtStatus status = _nav_query->findNearestPoly(center, extents, &filter, &nearest_poly_ref_id, nearest_p);

  if (dtStatusFailed(status)) {
    std::cout << "\nCannot find nearest point on polymesh.\n";
    return false;
  }
  p = LPoint3(nearest_p[0], nearest_p[1], nearest_p[2]);

  return true;
}


