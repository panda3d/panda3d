// Filename: physxBox.h
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

#ifndef PHYSXBOX_H
#define PHYSXBOX_H

#include "pandabase.h"
#include "luse.h"

#include "config_physx.h"
#include "physx_includes.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxBox
// Description : Represents an oriented bounding box, as a center
//               point, extents(radii) and a rotation. i.e. the
//               center of the box is at the center point, the box
//               is rotated around this point with the rotation and
//               it is 2*extents in width, height and depth.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxBox {

PUBLISHED:
  INLINE PhysxBox();
  INLINE ~PhysxBox();
  PhysxBox(const LPoint3f &center, const LVector3f &extents, const LMatrix3f &rot);

  bool is_valid() const;
  void rotate(const LMatrix4f &m, PhysxBox &obb) const;
  void set_empty();

  LPoint3f get_center() const;
  LVector3f get_extents() const;
  LMatrix3f get_rot() const;

  void set_center(LPoint3f center);
  void set_extents(LVector3f extents);
  void set_rot(LMatrix3f rot);

public:
  NxBox _box;
};

#include "physxBox.I"

#endif // PHYSBOX_H
