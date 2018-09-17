/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxWheelShape.cxx
 * @author enn0x
 * @date 2009-11-09
 */

#include "physxWheelShape.h"
#include "physxWheelShapeDesc.h"
#include "physxSpringDesc.h"

TypeHandle PhysxWheelShape::_type_handle;

/**
 *
 */
void PhysxWheelShape::
link(NxShape *shapePtr) {

  _ptr = shapePtr->isWheel();
  _ptr->userData = this;
  _error_type = ET_ok;

  set_name(shapePtr->getName());

  PhysxActor *actor = (PhysxActor *)_ptr->getActor().userData;
  actor->_shapes.add(this);
}

/**
 *
 */
void PhysxWheelShape::
unlink() {

  _ptr->userData = nullptr;
  _error_type = ET_released;

  PhysxActor *actor = (PhysxActor *)_ptr->getActor().userData;
  actor->_shapes.remove(this);
}

/**
 * Saves the state of the shape object to a descriptor.
 */
void PhysxWheelShape::
save_to_desc(PhysxWheelShapeDesc &shapeDesc) const {

  nassertv(_error_type == ET_ok);
  _ptr->saveToDesc(shapeDesc._desc);
}

/**
 * Sets the sphere radius.
 */
void PhysxWheelShape::
set_radius(float radius) {

  nassertv(_error_type == ET_ok);
  _ptr->setRadius(radius);
}

/**
 * Returns the radius of the sphere.
 */
float PhysxWheelShape::
get_radius() const {

  nassertr(_error_type == ET_ok, 0.0f);
  return _ptr->getRadius();
}

/**
 * Set the maximum extension distance of suspension along shape's -Y axis.
 * The minimum extension is always 0.
 */
void PhysxWheelShape::
set_suspension_travel(float travel) {

  nassertv(_error_type == ET_ok);
  _ptr->setSuspensionTravel(travel);
}

/**
 * Returns the suspension travel
 */
float PhysxWheelShape::
get_suspension_travel() const {

  nassertr(_error_type == ET_ok, 0.0f);
  return _ptr->getSuspensionTravel();
}

/**
 * Set the inverse mass of the wheel.  Determines the wheel velocity that
 * wheel torques can achieve.
 */
void PhysxWheelShape::
set_inverse_wheel_mass(float invMass) {

  nassertv(_error_type == ET_ok);
  _ptr->setInverseWheelMass(invMass);
}

/**
 * Returns the inverse mass of the wheel.  Determines the wheel velocity that
 * wheel torques can achieve.
 */
float PhysxWheelShape::
get_inverse_wheel_mass() const {

  nassertr(_error_type == ET_ok, 0.0f);
  return _ptr->getInverseWheelMass();
}

/**
 * Set the sum engine torque on the wheel axle.  Positive or negative
 * depending on direction
 */
void PhysxWheelShape::
set_motor_torque(float torque) {

  nassertv(_error_type == ET_ok);
  _ptr->setMotorTorque(torque);
}

/**
 * Retrieves the sum engine torque on the wheel axle.  Positive or negative
 * depending on direction
 */
float PhysxWheelShape::
get_motor_torque() const {

  nassertr(_error_type == ET_ok, 0.0f);
  return _ptr->getMotorTorque();
}

/**
 * Must be nonnegative.  Very large values should lock wheel but should be
 * stable.
 */
void PhysxWheelShape::
set_brake_torque(float torque) {

  nassertv(_error_type == ET_ok);
  _ptr->setBrakeTorque(torque);
}

/**
 * Must be nonnegative.  Very large values should lock wheel but should be
 * stable.
 */
float PhysxWheelShape::
get_brake_torque() const {

  nassertr(_error_type == ET_ok, 0.0f);
  return _ptr->getBrakeTorque();
}

/**
 * Set the steering angle, around shape Y axis.  The steering angle is
 * measured in degrees.
 */
void PhysxWheelShape::
set_steer_angle(float angle) {

  nassertv(_error_type == ET_ok);
  _ptr->setSteerAngle(NxMath::degToRad(-1.0f * angle));
}

/**
 * Retrieves the steering angle, around shape Y axis.  The steering angle is
 * measured in degrees.
 */
float PhysxWheelShape::
get_steer_angle() const {

  nassertr(_error_type == ET_ok, 0.0f);
  return -1.0f * NxMath::radToDeg(_ptr->getSteerAngle());
}

/**
 * Set the steering angle, around shape Y axis.  The steering angle is
 * measured in radians.
 */
void PhysxWheelShape::
set_steer_angle_rad(float angle) {

  nassertv(_error_type == ET_ok);
  _ptr->setSteerAngle(-1.0f * angle);
}

/**
 * Retrieves the steering angle, around shape Y axis.  The steering angle is
 * measured in radians.
 */
float PhysxWheelShape::
get_steer_angle_rad() const {

  nassertr(_error_type == ET_ok, 0.0f);
  return -1.0f * _ptr->getSteerAngle();
}

/**
 * Set the current axle rotation speed.  Note: WSF_axle_speed_override flag
 * must be raised for this to have effect!
 */
void PhysxWheelShape::
set_axle_speed(float speed) {

  nassertv(_error_type == ET_ok);
  _ptr->setAxleSpeed(speed);
}

/**
 * Retrieves the current axle rotation speed.
 */
float PhysxWheelShape::
get_axle_speed() const {

  nassertr(_error_type == ET_ok, 0.0f);
  return _ptr->getAxleSpeed();
}

/**
 * Turns the specified wheel shape flag on or off.
 */
void PhysxWheelShape::
set_wheel_flag(PhysxWheelShapeFlag flag, bool value) {

  nassertv(_error_type == ET_ok);
  NxU32 flags = _ptr->getWheelFlags();

  if (value == true) {
    flags |= flag;
  } else {
    flags &= ~(flag);
  }

  _ptr->setWheelFlags(flags);
}

/**
 * Returns the value of the specified wheel shape flag.
 */
bool PhysxWheelShape::
get_wheel_flag(PhysxWheelShapeFlag flag) const {

  nassertr(_error_type == ET_ok, false);
  return (_ptr->getWheelFlags() & flag) ? true : false;
}

/**
 * Set the data intended for car wheel suspension effects.
 */
void PhysxWheelShape::
set_suspension(const PhysxSpringDesc &spring) {

  nassertv(_error_type == ET_ok);
  return _ptr->setSuspension(spring._desc);
}
