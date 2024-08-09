/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file navMeshQueryFilter.h
 * @author Maxwell175
 * @date 2022-02-29
 */


#ifndef NAVMESHQUERYFILTER_H
#define NAVMESHQUERYFILTER_H

#include "pandaSystem.h"
#include "recastnavigation/DetourNavMesh.h"

/**
 * The NavMeshPath class describes a path that was found by NavMeshQuery.
 */
class EXPCL_NAVIGATION NavMeshQueryFilter {
public:
  NavMeshQueryFilter() = default;
  NavMeshQueryFilter(const NavMeshQueryFilter &copy);

  dtQueryFilter* get_filter();

PUBLISHED:
  INLINE void set_include_mask(BitMask16 mask);
  INLINE BitMask16 get_include_mask() const;
  MAKE_PROPERTY(include_mask, get_include_mask, set_include_mask);

  INLINE void set_exclude_mask(BitMask16 mask);
  INLINE BitMask16 get_exclude_mask() const;
  MAKE_PROPERTY(exclude_mask, get_exclude_mask, set_exclude_mask);

  INLINE bool operator == (const NavMeshQueryFilter &other) const;
  INLINE bool operator != (const NavMeshQueryFilter &other) const;

private:
  BitMask16 _include_mask = BitMask16::all_on();
  BitMask16 _exclude_mask = BitMask16::all_off();

};

#include "navMeshQueryFilter.I"

#endif // NAVMESHQUERYFILTER_H
