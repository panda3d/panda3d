// Filename: physxPlane.h
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

#ifndef PHYSXPLANE_H
#define PHYSXPLANE_H

#include "pandabase.h"
#include "luse.h"

#include "config_physx.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxPlane
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxPlane {

PUBLISHED:
  INLINE PhysxPlane();
  INLINE PhysxPlane(const PhysxPlane &plane);
  INLINE ~PhysxPlane();

  float distance(const LPoint3f &p) const;
  bool belongs(const LPoint3f &p) const;
  void normalize();
  LPoint3f point_in_plane() const;
  LPoint3f project(const LPoint3f &p) const;
  void transform(const LMatrix4f &transform, PhysxPlane &transformed) const;
  void inverse_transform(const LMatrix4f &transform, PhysxPlane &transformed) const;

  PhysxPlane set(const LPoint3f &p0, const LPoint3f &p1, const LPoint3f &p2);
  PhysxPlane zero();

  float get_d() const;
  LVector3f get_normal() const;

  void set_d(float d);
  void set_normal(LVector3f normal);

public:
  NxPlane _plane;
};

#include "physxPlane.I"

#endif // PHYSPLANE_H
