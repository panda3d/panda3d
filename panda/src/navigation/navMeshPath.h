/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file navMeshPath.h
 * @author Maxwell175
 * @date 2022-02-20
 */


#ifndef NAVMESHPATH_H
#define NAVMESHPATH_H

#include "pandaSystem.h"
#include "lpoint3.h"

/**
 * The NavMeshPath class describes a path that was found by NavMeshQuery.
 */
class EXPCL_NAVIGATION NavMeshPath {
public:
  NavMeshPath() = default;
  NavMeshPath(const NavMeshPath &copy);
  explicit NavMeshPath(pvector<LPoint3> &points);

  INLINE pvector<LPoint3> get_points() const;

PUBLISHED:
  INLINE size_t get_num_points() const;
  INLINE LPoint3 get_point(int index) const;
  MAKE_SEQ_PROPERTY(points, get_num_points, get_point);

  PN_stdfloat get_length();
  MAKE_PROPERTY(length, get_length);

  INLINE bool operator == (const NavMeshPath &other) const;
  INLINE bool operator != (const NavMeshPath &other) const;

private:
  pvector<LPoint3> _points;

};

#include "navMeshPath.I"

#endif // NAVMESHPATH_H
