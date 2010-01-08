// Filename: physxRevoluteJointDesc.h
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

#ifndef PHYSXREVOLUTEJOINTDESC_H
#define PHYSXREVOLUTEJOINTDESC_H

#include "pandabase.h"

#include "physxJointDesc.h"
#include "physx_includes.h"

class PhysxSpringDesc;
class PhysxMotorDesc;
class PhysxJointLimitDesc;

////////////////////////////////////////////////////////////////////
//       Class : PhysxRevoluteJointDesc
// Description : Descriptor class for distance joint. See
//               PhysxRevoluteJoint.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxRevoluteJointDesc : public PhysxJointDesc {

PUBLISHED:
  INLINE PhysxRevoluteJointDesc();
  INLINE ~PhysxRevoluteJointDesc();

  INLINE void set_to_default();
  INLINE bool is_valid() const;

  void set_projection_distance(float distance);
  void set_projection_angle(float angle);
  void set_spring(const PhysxSpringDesc &spring);
  void set_flag(PhysxRevoluteJointFlag flag, bool value);
  void set_motor(const PhysxMotorDesc &motor);
  void set_limit_low(const PhysxJointLimitDesc &low);
  void set_limit_high(const PhysxJointLimitDesc &high);
  void set_projection_mode(PhysxProjectionMode mode);

  float get_projection_distance() const;
  float get_projection_angle() const;
  bool get_flag(PhysxRevoluteJointFlag flag) const;
  PhysxSpringDesc get_spring() const;
  PhysxMotorDesc get_motor() const;
  PhysxJointLimitDesc get_limit_low() const;
  PhysxJointLimitDesc get_limit_high() const;
  PhysxProjectionMode get_projection_mode() const;

public:
  NxJointDesc *ptr() const { return (NxJointDesc *)&_desc; };
  NxRevoluteJointDesc _desc;
};

#include "physxRevoluteJointDesc.I"

#endif // PHYSXREVOLUTEJOINTDESC_H
