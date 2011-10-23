// Filename: bulletVehicle.I
// Created by:  enn0x (16Feb10)
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


////////////////////////////////////////////////////////////////////
//     Function: BulletVehicle::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BulletVehicle::
~BulletVehicle() {

  delete _vehicle;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletVehicle::get_vehicle
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
btRaycastVehicle *BulletVehicle::
get_vehicle() const {

  return _vehicle;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletVehicle::get_tuning
//       Access: Published
//  Description: Returns a reference to the BulletVehicleTuning
//               object of this vehicle which offers various
//               vehicle-global tuning options. Make sure to
//               configure this before adding wheels!
////////////////////////////////////////////////////////////////////
BulletVehicleTuning &BulletVehicle::
get_tuning() {

  return _tuning;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletVehicle::get_num_wheels
//       Access: Published
//  Description: Returns the number of wheels this vehicle has.
////////////////////////////////////////////////////////////////////
INLINE int BulletVehicle::
get_num_wheels() const {

  return _vehicle->getNumWheels();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletVehicleTuning::set_suspension_stiffness
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletVehicleTuning::
set_suspension_stiffness(PN_stdfloat value) {

  _.m_suspensionStiffness = (btScalar)value;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletVehicleTuning::set_suspension_compression
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletVehicleTuning::
set_suspension_compression(PN_stdfloat value) {

  _.m_suspensionCompression = (btScalar)value;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletVehicleTuning::set_suspension_damping
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletVehicleTuning::
set_suspension_damping(PN_stdfloat value) {

  _.m_suspensionDamping = (btScalar)value;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletVehicleTuning::set_max_suspension_travel_cm
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletVehicleTuning::
set_max_suspension_travel_cm(PN_stdfloat value) {

  _.m_maxSuspensionTravelCm = (btScalar)value;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletVehicleTuning::set_friction_slip
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletVehicleTuning::
set_friction_slip(PN_stdfloat value) {

  _.m_frictionSlip = (btScalar)value;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletVehicleTuning::set_max_suspension_force
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletVehicleTuning::
set_max_suspension_force(PN_stdfloat value) {

  _.m_maxSuspensionForce = (btScalar)value;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletVehicleTuning::get_suspension_stiffness
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PN_stdfloat BulletVehicleTuning::
get_suspension_stiffness() const {

  return (PN_stdfloat)_.m_suspensionStiffness;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletVehicleTuning::get_suspension_compression
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PN_stdfloat BulletVehicleTuning::
get_suspension_compression() const {

  return (PN_stdfloat)_.m_suspensionCompression;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletVehicleTuning::get_suspension_damping
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PN_stdfloat BulletVehicleTuning::
get_suspension_damping() const {

  return (PN_stdfloat)_.m_suspensionDamping;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletVehicleTuning::get_max_suspension_travel_cm
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PN_stdfloat BulletVehicleTuning::
get_max_suspension_travel_cm() const {

  return (PN_stdfloat)_.m_maxSuspensionTravelCm;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletVehicleTuning::get_friction_slip
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PN_stdfloat BulletVehicleTuning::
get_friction_slip() const {

  return (PN_stdfloat)_.m_frictionSlip;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletVehicleTuning::get_max_suspension_force
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PN_stdfloat BulletVehicleTuning::
get_max_suspension_force() const {

  return (PN_stdfloat)_.m_maxSuspensionForce;
}

