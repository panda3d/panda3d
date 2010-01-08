// Filename: physxPulleyJointDesc.h
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

#ifndef PHYSXPULLEYJOINTDESC_H
#define PHYSXPULLEYJOINTDESC_H

#include "pandabase.h"

#include "physxJointDesc.h"
#include "physx_includes.h"

class PhysxMotorDesc;

////////////////////////////////////////////////////////////////////
//       Class : PhysxPulleyJointDesc
// Description : Descriptor class for distance joint. See
//               PhysxPulleyJoint.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxPulleyJointDesc : public PhysxJointDesc {

PUBLISHED:
  INLINE PhysxPulleyJointDesc();
  INLINE ~PhysxPulleyJointDesc();

  INLINE void set_to_default();
  INLINE bool is_valid() const;

  void set_distance(float distance);
  void set_stiffness(float stiffness);
  void set_ratio(float ration);
  void set_pulley(unsigned int idx, const LPoint3f pos);
  void set_motor(const PhysxMotorDesc &motor);
  void set_flag(PhysxPulleyJointFlag flag, bool value);

  float get_distance() const;
  float get_stiffness() const;
  float get_ratio() const;
  bool get_flag(PhysxPulleyJointFlag flag) const;
  LPoint3f get_pulley(unsigned int idx) const;
  PhysxMotorDesc get_motor() const;

public:
  NxJointDesc *ptr() const { return (NxJointDesc *)&_desc; };
  NxPulleyJointDesc _desc;
};

#include "physxPulleyJointDesc.I"

#endif // PHYSXPULLEYJOINTDESC_H
