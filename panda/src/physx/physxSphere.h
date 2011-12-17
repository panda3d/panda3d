// Filename: physxSphere.h
// Created by:  enn0x (31Oct09)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef PHYSXSPHERE_H
#define PHYSXSPHERE_H

#include "pandabase.h"
#include "luse.h"

#include "config_physx.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxSphere
// Description : Represents a sphere defined by its center point
//               and radius.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxSphere {

PUBLISHED:
  INLINE PhysxSphere();
  INLINE PhysxSphere(const PhysxSphere &sphere);
  INLINE ~PhysxSphere();

  bool contains(const LPoint3f &p) const;
  bool contains(const PhysxSphere &sphere) const;
  bool contains(const LPoint3f &min, const LPoint3f &max) const;
  bool intersect(const PhysxSphere &sphere) const;
  bool is_valid() const;

  LPoint3f get_center() const;
  float get_radius() const;

  void set_center(LPoint3f value);
  void set_radius(float value);

public:
  NxSphere _sphere;
};

#include "physxSphere.I"

#endif // PHYSSPHERE_H
