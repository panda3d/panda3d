/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file navMeshQueryFilter.cxx
 * @author Maxwell175
 * @date 2022-02-27
 */

#include "navMeshQueryFilter.h"


/**
 * Copies an existing NavMeshPoly object.
 */
NavMeshQueryFilter::NavMeshQueryFilter(const NavMeshQueryFilter &copy) :
    _include_mask(copy.get_include_mask()), _exclude_mask(copy.get_exclude_mask()) { }

dtQueryFilter* NavMeshQueryFilter::get_filter() {
  auto filter = new dtQueryFilter();

  filter->setExcludeFlags(_exclude_mask.get_word());
  filter->setIncludeFlags(_include_mask.get_word());

  return filter;
}
