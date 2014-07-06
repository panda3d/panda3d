// Filename: physxJointLimitDesc.h
// Created by:  enn0x (28Sep09)
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

#ifndef PHYSXJOINTLIMITDESC_H
#define PHYSXJOINTLIMITDESC_H

#include "pandabase.h"

#include "physx_includes.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxJointLimitDesc
// Description : Describes a joint limit.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxJointLimitDesc {

PUBLISHED:
  INLINE PhysxJointLimitDesc();
  INLINE PhysxJointLimitDesc(float value, float restitution, float hardness);
  INLINE ~PhysxJointLimitDesc();

  void set_value(float value);
  void set_restitution(float restitution);
  void set_hardness(float hardness);

  float get_value() const;
  float get_restitution() const;
  float get_hardness() const;

public:
  NxJointLimitDesc _desc;
};

#include "physxJointLimitDesc.I"

#endif // PHYSXJOINTLIMITDESC_H
