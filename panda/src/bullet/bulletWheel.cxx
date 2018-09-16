/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletWheel.cxx
 * @author enn0x
 * @date 2010-02-17
 */

#include "bulletWheel.h"

#include "bulletWorld.h"

/**
 *
 */
BulletWheel::
BulletWheel(btWheelInfo &info) : _info(info) {

}

/**
 *
 */
BulletWheelRaycastInfo::
BulletWheelRaycastInfo(btWheelInfo::RaycastInfo &info) : _info(info) {

}

/**
 *
 */
BulletWheelRaycastInfo BulletWheel::
get_raycast_info() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return BulletWheelRaycastInfo(_info.m_raycastInfo);
}

/**
 * Returns the length of the suspension when the vehicle is standing still.
 */
PN_stdfloat BulletWheel::
get_suspension_rest_length() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_info.getSuspensionRestLength();
}

/**
 * Sets how stiff the suspension shall be.
 */
void BulletWheel::
set_suspension_stiffness(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _info.m_suspensionStiffness = (btScalar)value;
}

/**
 * Returns the stiffness of the suspension.
 */
PN_stdfloat BulletWheel::
get_suspension_stiffness() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_info.m_suspensionStiffness;
}

/**
 * Sets the maximum distance the suspension can travel out of the resting
 * position in centimeters.
 */
void BulletWheel::
set_max_suspension_travel_cm(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _info.m_maxSuspensionTravelCm = (btScalar)value;
}

/**
 *
 */
PN_stdfloat BulletWheel::
get_max_suspension_travel_cm() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_info.m_maxSuspensionTravelCm;
}

/**
 * Sets the slipperyness of the tyre.
 */
void BulletWheel::
set_friction_slip(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _info.m_frictionSlip = (btScalar)value;
}

/**
 * Returns how slippery the tyres are.
 */
PN_stdfloat BulletWheel::
get_friction_slip() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_info.m_frictionSlip;
}

/**
 * Sets the maximum suspension force the wheel can handle.
 */
void BulletWheel::
set_max_suspension_force(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _info.m_maxSuspensionForce = (btScalar)value;
}

/**
 * Returns the maximum force (weight) the suspension can handle.
 */
PN_stdfloat BulletWheel::
get_max_suspension_force() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_info.m_maxSuspensionForce;
}

/**
 * Sets the damping forces applied when the suspension gets compressed.
 */
void BulletWheel::
set_wheels_damping_compression(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _info.m_wheelsDampingCompression = (btScalar)value;
}

/**
 * Returns the  damping applied to the compressing suspension.
 */
PN_stdfloat BulletWheel::
get_wheels_damping_compression() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_info.m_wheelsDampingCompression;
}

/**
 * Sets the damping forces applied when the suspension relaxes.
 */
void BulletWheel::
set_wheels_damping_relaxation(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _info.m_wheelsDampingRelaxation = (btScalar)value;
}

/**
 * Returns the damping applied to the relaxing suspension.
 */
PN_stdfloat BulletWheel::
get_wheels_damping_relaxation() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_info.m_wheelsDampingRelaxation;
}

/**
 * Defines a scaling factor for roll forces that affect the chassis.  0.0
 * means no roll - the chassis won't ever flip over - while 1.0 means original
 * physical behaviour.  Basically, this allows moving the center of mass up
 * and down.
 */
void BulletWheel::
set_roll_influence(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _info.m_rollInfluence = (btScalar)value;
}

/**
 * Returns the factor by which roll forces are scaled.  See
 * set_roll_influence.
 */
PN_stdfloat BulletWheel::
get_roll_influence() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_info.m_rollInfluence;
}

/**
 * Sets the wheel radius.
 */
void BulletWheel::
set_wheel_radius(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _info.m_wheelsRadius = (btScalar)value;
}

/**
 * Returns the wheel radius.
 */
PN_stdfloat BulletWheel::
get_wheel_radius() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_info.m_wheelsRadius;
}

/**
 * Sets the steering angle.
 */
void BulletWheel::
set_steering(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _info.m_steering = (btScalar)value;
}

/**
 * Returns the steering angle in degrees.
 */
PN_stdfloat BulletWheel::
get_steering() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_info.m_steering;
}

/**
 *
 */
void BulletWheel::
set_rotation(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _info.m_rotation = (btScalar)value;
}

/**
 *
 */
PN_stdfloat BulletWheel::
get_rotation() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_info.m_rotation;
}

/**
 *
 */
void BulletWheel::
set_delta_rotation(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _info.m_deltaRotation = (btScalar)value;
}

/**
 *
 */
PN_stdfloat BulletWheel::
get_delta_rotation() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_info.m_deltaRotation;
}

/**
 * Defines how much force should be used to rotate the wheel.
 */
void BulletWheel::
set_engine_force(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _info.m_engineForce = (btScalar)value;
}

/**
 * Returns the amount of accelleration force currently applied.
 */
PN_stdfloat BulletWheel::
get_engine_force() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_info.m_engineForce;
}

/**
 *
 */
void BulletWheel::
set_brake(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _info.m_brake = (btScalar)value;
}

/**
 * Returns the amount of braking force currently applied.
 */
PN_stdfloat BulletWheel::
get_brake() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_info.m_brake;
}

/**
 *
 */
void BulletWheel::
set_skid_info(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _info.m_skidInfo = (btScalar)value;
}

/**
 *
 */
PN_stdfloat BulletWheel::
get_skid_info() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_info.m_skidInfo;
}

/**
 *
 */
void BulletWheel::
set_wheels_suspension_force(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _info.m_wheelsSuspensionForce = (btScalar)value;
}

/**
 *
 */
PN_stdfloat BulletWheel::
get_wheels_suspension_force() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_info.m_wheelsSuspensionForce;
}

/**
 *
 */
void BulletWheel::
set_suspension_relative_velocity(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _info.m_suspensionRelativeVelocity = (btScalar)value;
}

/**
 *
 */
PN_stdfloat BulletWheel::
get_suspension_relative_velocity() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_info.m_suspensionRelativeVelocity;
}

/**
 *
 */
void BulletWheel::
set_clipped_inv_connection_point_cs(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _info.m_clippedInvContactDotSuspension = (btScalar)value;
}

/**
 *
 */
PN_stdfloat BulletWheel::
get_clipped_inv_connection_point_cs() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_info.m_clippedInvContactDotSuspension;
}

/**
 * Sets the point where the wheel is connected to the chassis.
 */
void BulletWheel::
set_chassis_connection_point_cs(const LPoint3 &pos) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertv(!pos.is_nan());
  _info.m_chassisConnectionPointCS = LVecBase3_to_btVector3(pos);
}

/**
 * Returns the point where the wheel is connected to the chassis.
 */
LPoint3 BulletWheel::
get_chassis_connection_point_cs() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return btVector3_to_LPoint3(_info.m_chassisConnectionPointCS);
}

/**
 * Sets the wheel's forward vector.  (Most likely orthogonal to the axle
 * vector.)
 */
void BulletWheel::
set_wheel_direction_cs(const LVector3 &dir) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertv(!dir.is_nan());
  _info.m_wheelDirectionCS = LVecBase3_to_btVector3(dir);
}

/**
 * Returns the wheel's forward vector relative to the chassis.
 */
LVector3 BulletWheel::
get_wheel_direction_cs() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return btVector3_to_LVector3(_info.m_wheelDirectionCS);
}

/**
 * Determines the wheel axle normal vector.
 */
void BulletWheel::
set_wheel_axle_cs(const LVector3 &axle) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertv(!axle.is_nan());
  _info.m_wheelAxleCS = LVecBase3_to_btVector3(axle);
}

/**
 * Returns the normal vector of the wheel axle.
 */
LVector3 BulletWheel::
get_wheel_axle_cs() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return btVector3_to_LVector3(_info.m_wheelAxleCS);
}

/**
 *
 */
void BulletWheel::
set_world_transform(const LMatrix4 &mat) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertv(!mat.is_nan());
  _info.m_worldTransform = LMatrix4_to_btTrans(mat);
}

/**
 *
 */
LMatrix4 BulletWheel::
get_world_transform() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return btTrans_to_LMatrix4(_info.m_worldTransform);
}

/**
 * Sets if the wheel is steerable.
 */
void BulletWheel::
set_front_wheel(bool value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _info.m_bIsFrontWheel = value;
}

/**
 * Determines if a wheel is steerable.
 */
bool BulletWheel::
is_front_wheel() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return _info.m_bIsFrontWheel;
}

/**
 * Sets the PandaNode which representates the visual appearance of this wheel.
 */
void BulletWheel::
set_node(PandaNode *node) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _info.m_clientInfo = (void *)node;
}

/**
 * Returns the PandaNode which representates the visual appearance of this
 * wheel, if such a representation has been set previously.
 */
PandaNode *BulletWheel::
get_node() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (_info.m_clientInfo == nullptr) ? nullptr : (PandaNode *)_info.m_clientInfo;
}

/**
 *
 */
bool BulletWheelRaycastInfo::
is_in_contact() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return _info.m_isInContact;
}

/**
 *
 */
PN_stdfloat BulletWheelRaycastInfo::
get_suspension_length() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return _info.m_suspensionLength;
}

/**
 *
 */
LPoint3 BulletWheelRaycastInfo::
get_contact_point_ws() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return btVector3_to_LPoint3(_info.m_contactPointWS);
}

/**
 *
 */
LPoint3 BulletWheelRaycastInfo::
get_hard_point_ws() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return btVector3_to_LPoint3(_info.m_hardPointWS);
}

/**
 *
 */
LVector3 BulletWheelRaycastInfo::
get_contact_normal_ws() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return btVector3_to_LVector3(_info.m_contactNormalWS);
}

/**
 *
 */
LVector3 BulletWheelRaycastInfo::
get_wheel_direction_ws() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return btVector3_to_LVector3(_info.m_wheelDirectionWS);
}

/**
 *
 */
LVector3 BulletWheelRaycastInfo::
get_wheel_axle_ws() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return btVector3_to_LVector3(_info.m_wheelAxleWS);
}

/**
 *
 */
PandaNode *BulletWheelRaycastInfo::
get_ground_object() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return _info.m_groundObject ? (PandaNode *)_info.m_groundObject : nullptr;
}
