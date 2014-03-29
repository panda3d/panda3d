// Filename: physxJointDriveDesc.h
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

#ifndef PHYSXJOINTDRIVEDESC_H
#define PHYSXJOINTDRIVEDESC_H

#include "pandabase.h"

#include "physxEnums.h"
#include "physx_includes.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxJointDriveDesc
// Description : Used to describe drive properties for a
//               PhysxD6Joint.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxJointDriveDesc : public PhysxEnums {

PUBLISHED:
  INLINE PhysxJointDriveDesc();
  INLINE PhysxJointDriveDesc(float sping, float damping, float forceLimit);
  INLINE ~PhysxJointDriveDesc();

  void set_drive_type(PhysxD6JointDriveType type);
  void set_spring(float spring);
  void set_damping(float damping);
  void set_force_limit(float limit);

  PhysxD6JointDriveType get_drive_type() const;
  float get_spring() const;
  float get_damping() const;
  float get_force_limit() const;

public:
  NxJointDriveDesc _desc;
};

#include "physxJointDriveDesc.I"

#endif // PHYSXJOINTDRIVEDESC_H
