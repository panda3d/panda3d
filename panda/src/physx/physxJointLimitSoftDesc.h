// Filename: physxJointLimitSoftDesc.h
// Created by:  enn0x (01Oct09)
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

#ifndef PHYSXJOINTLIMITSOFTDESC_H
#define PHYSXJOINTLIMITSOFTDESC_H

#include "pandabase.h"

#include "physx_includes.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxJointLimitSoftDesc
// Description : Describes a joint limit.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxJointLimitSoftDesc {

PUBLISHED:
  INLINE PhysxJointLimitSoftDesc();
  INLINE PhysxJointLimitSoftDesc(float value, float restitution, float spring, float damping);
  INLINE ~PhysxJointLimitSoftDesc();

  void set_value(float value);
  void set_restitution(float restitution);
  void set_spring(float spring);
  void set_damping(float damping);

  float get_value() const;
  float get_restitution() const;
  float get_spring() const;
  float get_damping() const;

public:
  NxJointLimitSoftDesc _desc;
};

#include "physxJointLimitSoftDesc.I"

#endif // PHYSXJOINTLIMITSOFTDESC_H
