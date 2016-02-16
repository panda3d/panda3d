/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletVehicle.I
 * @author enn0x
 * @date 2010-02-16
 */

/**

 */
INLINE BulletVehicle::
~BulletVehicle() {

  delete _vehicle;
}

/**

 */
btRaycastVehicle *BulletVehicle::
get_vehicle() const {

  return _vehicle;
}

/**
 * Returns a reference to the BulletVehicleTuning object of this vehicle which
 * offers various vehicle-global tuning options.  Make sure to configure this
 * before adding wheels!
 */
BulletVehicleTuning &BulletVehicle::
get_tuning() {

  return _tuning;
}

/**
 * Returns the number of wheels this vehicle has.
 */
INLINE int BulletVehicle::
get_num_wheels() const {

  return _vehicle->getNumWheels();
}

/**

 */
void BulletVehicleTuning::
set_suspension_stiffness(PN_stdfloat value) {

  _.m_suspensionStiffness = (btScalar)value;
}

/**

 */
void BulletVehicleTuning::
set_suspension_compression(PN_stdfloat value) {

  _.m_suspensionCompression = (btScalar)value;
}

/**

 */
void BulletVehicleTuning::
set_suspension_damping(PN_stdfloat value) {

  _.m_suspensionDamping = (btScalar)value;
}

/**

 */
void BulletVehicleTuning::
set_max_suspension_travel_cm(PN_stdfloat value) {

  _.m_maxSuspensionTravelCm = (btScalar)value;
}

/**

 */
void BulletVehicleTuning::
set_friction_slip(PN_stdfloat value) {

  _.m_frictionSlip = (btScalar)value;
}

/**

 */
void BulletVehicleTuning::
set_max_suspension_force(PN_stdfloat value) {

  _.m_maxSuspensionForce = (btScalar)value;
}

/**

 */
PN_stdfloat BulletVehicleTuning::
get_suspension_stiffness() const {

  return (PN_stdfloat)_.m_suspensionStiffness;
}

/**

 */
PN_stdfloat BulletVehicleTuning::
get_suspension_compression() const {

  return (PN_stdfloat)_.m_suspensionCompression;
}

/**

 */
PN_stdfloat BulletVehicleTuning::
get_suspension_damping() const {

  return (PN_stdfloat)_.m_suspensionDamping;
}

/**

 */
PN_stdfloat BulletVehicleTuning::
get_max_suspension_travel_cm() const {

  return (PN_stdfloat)_.m_maxSuspensionTravelCm;
}

/**

 */
PN_stdfloat BulletVehicleTuning::
get_friction_slip() const {

  return (PN_stdfloat)_.m_frictionSlip;
}

/**

 */
PN_stdfloat BulletVehicleTuning::
get_max_suspension_force() const {

  return (PN_stdfloat)_.m_maxSuspensionForce;
}
