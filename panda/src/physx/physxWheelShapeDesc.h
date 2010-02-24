// Filename: physxWheelShapeDesc.h
// Created by:  enn0x (09Nov09)
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

#ifndef PHYSXWHEELSHAPEDESC_H
#define PHYSXWHEELSHAPEDESC_H

#include "pandabase.h"

#include "physxShapeDesc.h"
#include "physx_includes.h"

class PhysxSpringDesc;

////////////////////////////////////////////////////////////////////
//       Class : PhysxWheelShapeDesc
// Description : Descriptor class for PhysxWheelShape.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxWheelShapeDesc : public PhysxShapeDesc {

PUBLISHED:
  INLINE PhysxWheelShapeDesc();
  INLINE ~PhysxWheelShapeDesc();

  void set_to_default();
  INLINE bool is_valid() const;

  void set_radius(float radius);
  void set_suspension_travel(float suspensionTravel);
  void set_inverse_wheel_mass(float inverseWheelMass);
  void set_motor_torque(float motorTorque);
  void set_brake_torque(float brakeTorque);
  void set_steer_angle(float steerAngle);
  void set_wheel_flag(PhysxWheelShapeFlag flag, bool value);
  void set_suspension(const PhysxSpringDesc &spring);

  float get_radius() const;
  float get_suspension_travel() const;
  float get_inverse_wheel_mass() const;
  float get_motor_torque() const;
  float get_brake_torque() const;
  float get_steer_angle() const;
  bool get_wheel_flag(PhysxWheelShapeFlag flag) const;
  PhysxSpringDesc get_suspension() const;

public:
  NxShapeDesc *ptr() const { return (NxShapeDesc *)&_desc; };
  NxWheelShapeDesc _desc;
};

#include "physxWheelShapeDesc.I"

#endif // PHYSXWHEELSHAPEDESC_H
