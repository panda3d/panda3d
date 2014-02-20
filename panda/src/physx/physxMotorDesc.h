// Filename: physxMotorDesc.h
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

#ifndef PHYSXMOTORDESC_H
#define PHYSXMOTORDESC_H

#include "pandabase.h"

#include "physx_includes.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxMotorDesc
// Description : Describes a joint motor. Some joints can be
//               motorized, this allows them to apply a force to
//               cause attached actors to move. Joints which can be
//               motorized:
//               - PhysxPulleyJoint
//               - PhysxRevoluteJoint
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxMotorDesc {

PUBLISHED:
  INLINE PhysxMotorDesc();
  INLINE PhysxMotorDesc(float velTarget, float maxForce=0, bool freeSpin=0);
  INLINE ~PhysxMotorDesc();

  void set_vel_target(float velTarget);
  void set_max_force(float maxForce);
  void set_free_spin(bool freeSpin);

  float get_vel_target() const;
  float get_max_force() const;
  bool get_free_spin() const;

public:
  NxMotorDesc _desc;
};

#include "physxMotorDesc.I"

#endif // PHYSXMOTORDESC_H
