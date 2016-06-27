/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxWheelShapeDesc.cxx
 * @author enn0x
 * @date 2009-11-09
 */

#include "physxWheelShapeDesc.h"

/**
 * (re)sets the structure to the default.
 */
void PhysxWheelShapeDesc::
set_to_default() {

  _desc.setToDefault();
  set_name("");

  _desc.localPose = PhysxManager::mat4_to_nxMat34(LMatrix4f::y_to_z_up_mat());
}

/**
 * Radius of shape.  Must be positive.
 */
void PhysxWheelShapeDesc::
set_radius(float radius) {

  _desc.radius = radius;
}

/**
 * Radius of shape.
 */
float PhysxWheelShapeDesc::
get_radius() const {

  return _desc.radius;
}

/**
 * Set the maximum extension distance of suspension along shape's -Y axis.
 */
void PhysxWheelShapeDesc::
set_suspension_travel(float suspensionTravel) {

  _desc.suspensionTravel = suspensionTravel;
}

/**
 *
 */
float PhysxWheelShapeDesc::
get_suspension_travel() const {

  return _desc.suspensionTravel;
}

/**
 * Set the inverse mass of the wheel.
 */
void PhysxWheelShapeDesc::
set_inverse_wheel_mass(float inverseWheelMass) {

  _desc.inverseWheelMass = inverseWheelMass;
}

/**
 *
 */
float PhysxWheelShapeDesc::
get_inverse_wheel_mass() const {

  return _desc.inverseWheelMass;
}

/**
 * Set the sum engine torque on the wheel axle.
 */
void PhysxWheelShapeDesc::
set_motor_torque(float motorTorque) {

  _desc.motorTorque = motorTorque;
}

/**
 *
 */
float PhysxWheelShapeDesc::
get_motor_torque() const {

  return _desc.motorTorque;
}

/**
 * Set the amount of torque applied for braking.
 */
void PhysxWheelShapeDesc::
set_brake_torque(float brakeTorque) {

  _desc.brakeTorque = brakeTorque;
}

/**
 *
 */
float PhysxWheelShapeDesc::
get_brake_torque() const {

  return _desc.brakeTorque;
}

/**
 * Set the steering angle, around shape Y axis.  The steering angle is
 * measured in degrees.
 */
void PhysxWheelShapeDesc::
set_steer_angle(float steerAngle) {

  _desc.steerAngle = NxMath::degToRad(steerAngle);
}

/**
 *
 */
float PhysxWheelShapeDesc::
get_steer_angle() const {

  return _desc.steerAngle;
}

/**
 * Turn the specified wheel shape flag on or off.
 */
void PhysxWheelShapeDesc::
set_wheel_flag(PhysxWheelShapeFlag flag, bool value) {

  if (value == true) {
    _desc.wheelFlags |= flag;
  }
  else {
    _desc.wheelFlags &= ~(flag);
  }
}

/**
 * Returns the specified wheel shape flag.
 */
bool PhysxWheelShapeDesc::
get_wheel_flag(const PhysxWheelShapeFlag flag) const {

  return (_desc.wheelFlags & flag) ? true : false;
}

/**
 * Set the data intended for car wheel suspension effects.
 */
void PhysxWheelShapeDesc::
set_suspension(const PhysxSpringDesc &spring) {

  _desc.suspension = spring._desc;
}

/**
 *
 */
PhysxSpringDesc PhysxWheelShapeDesc::
get_suspension() const {

  PhysxSpringDesc value;
  value._desc = _desc.suspension;
  return value;
}
