/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file navPath.h
 * @author Ashwani / 
 * @date 2024
 */

#ifndef NAVPATH_H
#define NAVPATH_H

#include "pandabase.h"
#include "typedReferenceCount.h"
#include "luse.h"
#include "pvector.h"

/**
 * A simple container for a navigation path.
 * Holds a sequence of points (waypoints) from start to destination.
 */
class EXPCL_PANDA_NAVMESH NavPath : public TypedReferenceCount {
PUBLISHED:
  NavPath();
  ~NavPath();

  // Returns the number of waypoints in the path.
  INLINE int get_num_points() const;

  // Returns the point at the given index.
  INLINE LPoint3 get_point(int n) const;

  // Returns true if the path is valid (has points).
  INLINE bool is_valid() const;

  // Adds a point to the path (mostly for internal use).
  void add_point(const LPoint3 &point);

  void output(std::ostream &out) const;

private:
  pvector<LPoint3> _points;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "NavPath",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "navPath.I"

#endif // NAVPATH_H

