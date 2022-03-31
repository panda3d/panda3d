/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletVehicle.cxx
 * @author enn0x
 * @date 2010-02-16
 */

#include "bulletVehicle.h"

#include "config_bullet.h"

#include "bulletWorld.h"
#include "bulletRigidBodyNode.h"
#include "bulletWheel.h"

TypeHandle BulletVehicle::_type_handle;

/**
 * Creates a new BulletVehicle instance in the given world and with a chassis
 * node.
 */
BulletVehicle::
BulletVehicle(BulletWorld *world, BulletRigidBodyNode *chassis) {

  btRigidBody *body = btRigidBody::upcast(chassis->get_object());

  _raycaster = new btDefaultVehicleRaycaster(world->get_world());
  _vehicle = new btRaycastVehicle(_tuning._, body, _raycaster);

  set_coordinate_system(get_default_up_axis());
}

/**
 * Specifies which axis is "up". Nessecary for the vehicle's suspension to
 * work properly!
 */
void BulletVehicle::
set_coordinate_system(BulletUpAxis up) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  switch (up) {
  case X_up:
    _vehicle->setCoordinateSystem(1, 0, 2);
    break;
  case Y_up:
    _vehicle->setCoordinateSystem(0, 1, 2);
    break;
  case Z_up:
    _vehicle->setCoordinateSystem(0, 2, 1);
    break;
  default:
    bullet_cat.error() << "invalid up axis:" << up << std::endl;
    break;
  }
}

/**
 * Returns the forward vector representing the car's actual direction of
 * movement.  The forward vetcor is given in global coordinates.
 */
LVector3 BulletVehicle::
get_forward_vector() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return btVector3_to_LVector3(_vehicle->getForwardVector());
}

/**
 * Returns the chassis of this vehicle.  The chassis is a rigid body node.
 * Assumes the lock(bullet global lock) is held by the caller
 */
BulletRigidBodyNode *BulletVehicle::
do_get_chassis() {

  btRigidBody *bodyPtr = _vehicle->getRigidBody();
  return (bodyPtr) ? (BulletRigidBodyNode *)bodyPtr->getUserPointer() : nullptr;
}

/**
 * Returns the chassis of this vehicle.  The chassis is a rigid body node.
 */
BulletRigidBodyNode *BulletVehicle::
get_chassis() {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return do_get_chassis();
}

/**
 * Returns the current speed in kilometers per hour.  Convert to miles using:
 * km/h * 0.62 = mph
 */
PN_stdfloat BulletVehicle::
get_current_speed_km_hour() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_vehicle->getCurrentSpeedKmHour();
}

/**
 * Resets the vehicle's suspension.
 */
void BulletVehicle::
reset_suspension() {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _vehicle->resetSuspension();
}

/**
 * Returns the steering angle of the wheel with index idx in degrees.
 */
PN_stdfloat BulletVehicle::
get_steering_value(int idx) const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertr(idx < _vehicle->getNumWheels(), 0.0f);
  return rad_2_deg(_vehicle->getSteeringValue(idx));
}

/**
 * Sets the steering value (in degrees) of the wheel with index idx.
 */
void BulletVehicle::
set_steering_value(PN_stdfloat steering, int idx) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertv(idx < _vehicle->getNumWheels());
  _vehicle->setSteeringValue(deg_2_rad(steering), idx);
}

/**
 * Applies force at the wheel with index idx for acceleration.
 */
void BulletVehicle::
apply_engine_force(PN_stdfloat force, int idx) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertv(idx < _vehicle->getNumWheels());
  _vehicle->applyEngineForce(force, idx);
}

/**
 * Applies braking force to the wheel with index idx.
 */
void BulletVehicle::
set_brake(PN_stdfloat brake, int idx) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertv(idx < _vehicle->getNumWheels());
  _vehicle->setBrake(brake, idx);
}

/**
 *
 */
void BulletVehicle::
set_pitch_control(PN_stdfloat pitch) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _vehicle->setPitchControl(pitch);
}

/**
 * Factory method for creating wheels for this vehicle instance.
 */
BulletWheel BulletVehicle::
create_wheel(PN_stdfloat suspension_rest_length) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  btVector3 pos(0.0, 0.0, 0.0);
  btVector3 direction = get_axis(_vehicle->getUpAxis());
  btVector3 axle = get_axis(_vehicle->getRightAxis());

  btScalar suspension(suspension_rest_length);
  btScalar radius(0.3);

  btWheelInfo &info = _vehicle->addWheel(pos, direction, axle, suspension, radius, _tuning._, false);

  info.m_clientInfo = nullptr;

  return BulletWheel(info);
}

/**
 *
 */
btVector3 BulletVehicle::
get_axis(int idx) {

  switch (idx) {
  case 0:
    return btVector3(1.0, 0.0, 0.0);
  case 1:
    return btVector3(0.0, 1.0, 0.0);
  case 2:
    return btVector3(0.0, 0.0, 1.0);
  default:
    return btVector3(0.0, 0.0, 0.0);
  }
}

/**
 * Returns the number of wheels this vehicle has.
 */
int BulletVehicle::
get_num_wheels() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return _vehicle->getNumWheels();
}

/**
 * Returns the BulletWheel with index idx.  Causes an AssertionError if idx is
 * equal or larger than the number of wheels.
 */
BulletWheel BulletVehicle::
get_wheel(int idx) const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertr(idx < _vehicle->getNumWheels(), BulletWheel::empty());
  return BulletWheel(_vehicle->getWheelInfo(idx));
}

/**
 * Assumes the lock(bullet global lock) is held by the caller
 */
void BulletVehicle::
do_sync_b2p() {

  for (int i=0; i < _vehicle->getNumWheels(); i++) {
    btWheelInfo &info = _vehicle->getWheelInfo(i);

    // synchronize the wheels with the (interpolated) chassis worldtransform.
    // It resets the m_isInContact flag, so restore that afterwards.
    bool in_contact = info.m_raycastInfo.m_isInContact;
    _vehicle->updateWheelTransform(i, true);
    info.m_raycastInfo.m_isInContact = in_contact;

    PandaNode *node = (PandaNode *)info.m_clientInfo;
    if (node) {

      CPT(TransformState) ts = btTrans_to_TransformState(info.m_worldTransform);

      // <A> Transform relative to wheel node's parent
      // node->set_transform(ts);

      // <B> Transform absolute
      NodePath np = NodePath::any_path(node);
      np.set_transform(np.get_top(), ts);
    }
  }
}

/**
 *
 */
void BulletVehicleTuning::
set_suspension_stiffness(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _.m_suspensionStiffness = (btScalar)value;
}

/**
 *
 */
void BulletVehicleTuning::
set_suspension_compression(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _.m_suspensionCompression = (btScalar)value;
}

/**
 *
 */
void BulletVehicleTuning::
set_suspension_damping(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _.m_suspensionDamping = (btScalar)value;
}

/**
 *
 */
void BulletVehicleTuning::
set_max_suspension_travel_cm(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _.m_maxSuspensionTravelCm = (btScalar)value;
}

/**
 *
 */
void BulletVehicleTuning::
set_friction_slip(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _.m_frictionSlip = (btScalar)value;
}

/**
 *
 */
void BulletVehicleTuning::
set_max_suspension_force(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _.m_maxSuspensionForce = (btScalar)value;
}

/**
 *
 */
PN_stdfloat BulletVehicleTuning::
get_suspension_stiffness() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_.m_suspensionStiffness;
}

/**
 *
 */
PN_stdfloat BulletVehicleTuning::
get_suspension_compression() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_.m_suspensionCompression;
}

/**
 *
 */
PN_stdfloat BulletVehicleTuning::
get_suspension_damping() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_.m_suspensionDamping;
}

/**
 *
 */
PN_stdfloat BulletVehicleTuning::
get_max_suspension_travel_cm() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_.m_maxSuspensionTravelCm;
}

/**
 *
 */
PN_stdfloat BulletVehicleTuning::
get_friction_slip() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_.m_frictionSlip;
}

/**
 *
 */
PN_stdfloat BulletVehicleTuning::
get_max_suspension_force() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_.m_maxSuspensionForce;
}

