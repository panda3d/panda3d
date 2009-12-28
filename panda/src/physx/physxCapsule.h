// Filename: physxCapsule.h
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

#ifndef PHYSXCAPSULE_H
#define PHYSXCAPSULE_H

#include "pandabase.h"

#include "config_physx.h"
#include "physxSegment.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxCapsule
// Description : Represents a capsule.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxCapsule {

PUBLISHED:
  INLINE PhysxCapsule();
  INLINE PhysxCapsule(const PhysxSegment &segment, float radius);
  INLINE ~PhysxCapsule();

  void compute_direction(LPoint3f &dir) const;
  float compute_length() const;
  void compute_point(LPoint3f &p, float t) const;
  float compute_square_length() const;
  LPoint3f get_origin() const;
  void set_origin_direction(const LPoint3f &origin, const LVector3f &direction);

  float get_radius() const;
  LPoint3f get_p0() const;
  LPoint3f get_p1() const;

  void set_radius(float value);
  void set_p0(LPoint3f p);
  void set_p1(LPoint3f p);

public:
  NxCapsule _capsule;
};

#include "physxCapsule.I"

#endif // PHYSCAPSULE_H
