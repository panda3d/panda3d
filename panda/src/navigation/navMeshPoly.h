/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file navMeshPoly.h
 * @author Maxwell175
 * @date 2022-02-27
 */


#ifndef NAVMESHPOLY_H
#define NAVMESHPOLY_H

#include "pandaSystem.h"
#include "recastnavigation/DetourNavMesh.h"

class NavMesh;

typedef pvector<LPoint3> PointList;

/**
 * The NavMeshPath class describes a path that was found by NavMeshQuery.
 */
class EXPCL_NAVIGATION NavMeshPoly {
public:
  NavMeshPoly(const NavMeshPoly &copy);
  NavMeshPoly(PT(NavMesh) navMesh, dtPolyRef polyRef);

  INLINE dtPolyRef get_poly_ref() const;

PUBLISHED:
  INLINE PT(NavMesh) get_nav_mesh() const;

  LPoint3 get_center() const;
  MAKE_PROPERTY(center, get_center);

  BitMask16 get_flags() const;
  void set_flags(BitMask16 mask);
  MAKE_PROPERTY(flags, get_flags, set_flags);

  LColor get_debug_color() const;
  void set_debug_color(LColor color);
  MAKE_PROPERTY(debug_color, get_debug_color, set_debug_color);

  PointList get_verts() const;

  INLINE bool operator == (const NavMeshPoly &other) const;
  INLINE bool operator != (const NavMeshPoly &other) const;

private:
  PT(NavMesh) _navMesh;
  dtPolyRef _polyRef;

  LMatrix4 mat_from_y = LMatrix4::convert_mat(CS_yup_right, CS_default);
  LMatrix4 mat_to_y = LMatrix4::convert_mat(CS_default, CS_yup_right);
};

typedef pvector<NavMeshPoly> NavMeshPolys;

#include "navMeshPoly.I"

#endif // NAVMESHPOLY_H
