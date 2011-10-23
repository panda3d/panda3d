// Filename: bulletVehicle.cxx
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

#include "bulletVehicle.h"
#include "bulletWorld.h"
#include "bulletRigidBodyNode.h"
#include "bulletWheel.h"

TypeHandle BulletVehicle::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: BulletVehicle::Constructor
//       Access: Published
//  Description: Creates a new BulletVehicle instance in the given
//               world and with a chassis node.
////////////////////////////////////////////////////////////////////
BulletVehicle::
BulletVehicle(BulletWorld *world, BulletRigidBodyNode *chassis) {

  btRigidBody *body = btRigidBody::upcast(chassis->get_object());

  _raycaster = new btDefaultVehicleRaycaster(world->get_world());
  _vehicle = new btRaycastVehicle(_tuning._, body, _raycaster);

  set_coordinate_system(get_default_up_axis());
}

////////////////////////////////////////////////////////////////////
//     Function: BulletVehicle::set_coordinate_system
//       Access: Published
//  Description: Specifies which axis is "up". Nessecary for the
//               vehicle's suspension to work properly!
////////////////////////////////////////////////////////////////////
void BulletVehicle::
set_coordinate_system(BulletUpAxis up) {

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
    bullet_cat.error() << "invalid up axis:" << up << endl;
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BulletVehicle::get_forward_vector
//       Access: Published
//  Description: Returns the forward vector representing the car's
//               actual direction of movement. The forward vetcor
//               is given in global coordinates.
////////////////////////////////////////////////////////////////////
LVector3 BulletVehicle::
get_forward_vector() const {

  return btVector3_to_LVector3(_vehicle->getForwardVector());
}

////////////////////////////////////////////////////////////////////
//     Function: BulletVehicle::get_chassis
//       Access: Published
//  Description: Returns the chassis of this vehicle. The chassis
//               is a rigid body node.
////////////////////////////////////////////////////////////////////
BulletRigidBodyNode *BulletVehicle::
get_chassis() {

  btRigidBody *bodyPtr = _vehicle->getRigidBody();
  return (bodyPtr) ? (BulletRigidBodyNode *)bodyPtr->getUserPointer() : NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletVehicle::get_current_speed_km_hour
//       Access: Published
//  Description: Returns the current speed in kilometers per hour.
//               Convert to miles using: km/h * 0.62 = mph
////////////////////////////////////////////////////////////////////
PN_stdfloat BulletVehicle::
get_current_speed_km_hour() const {

  return (PN_stdfloat)_vehicle->getCurrentSpeedKmHour();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletVehicle::reset_suspension
//       Access: Published
//  Description: Resets the vehicle's suspension.
////////////////////////////////////////////////////////////////////
void BulletVehicle::
reset_suspension() {

  _vehicle->resetSuspension();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletVehicle::get_steering_value
//       Access: Published
//  Description: Returns the steering angle of the wheel with index
//               idx in degrees.
////////////////////////////////////////////////////////////////////
PN_stdfloat BulletVehicle::
get_steering_value(int idx) const {

  nassertr(idx < get_num_wheels(), 0.0f);
  return rad_2_deg(_vehicle->getSteeringValue(idx));
}

////////////////////////////////////////////////////////////////////
//     Function: BulletVehicle::set_steering_value
//       Access: Published
//  Description: Sets the steering value (in degrees) of the wheel
//               with index idx.
////////////////////////////////////////////////////////////////////
void BulletVehicle::
set_steering_value(PN_stdfloat steering, int idx) {

  nassertv(idx < get_num_wheels());
  _vehicle->setSteeringValue(deg_2_rad(steering), idx);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletVehicle::apply_engine_force
//       Access: Published
//  Description: Applies force at the wheel with index idx for
//               acceleration.
////////////////////////////////////////////////////////////////////
void BulletVehicle::
apply_engine_force(PN_stdfloat force, int idx) {

  nassertv(idx < get_num_wheels());
  _vehicle->applyEngineForce(force, idx);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletVehicle::set_brake
//       Access: Published
//  Description: Applies braking force to the wheel with index idx.
////////////////////////////////////////////////////////////////////
void BulletVehicle::
set_brake(PN_stdfloat brake, int idx) {

  nassertv(idx < get_num_wheels());
  _vehicle->setBrake(brake, idx);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletVehicle::set_pitch_control
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletVehicle::
set_pitch_control(PN_stdfloat pitch) {

  _vehicle->setPitchControl(pitch);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletVehicle::create_wheel
//       Access: Published
//  Description: Factory method for creating wheels for this
//               vehicle instance.
////////////////////////////////////////////////////////////////////
BulletWheel BulletVehicle::
create_wheel() {

  btVector3 pos(0.0, 0.0, 0.0);
  btVector3 direction = get_axis(_vehicle->getUpAxis());
  btVector3 axle = get_axis(_vehicle->getRightAxis());

  btScalar suspension(0.4);
  btScalar radius(0.3);

  btWheelInfo &info = _vehicle->addWheel(pos, direction, axle, suspension, radius, _tuning._, false);

  info.m_clientInfo = NULL;

  return BulletWheel(info);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletVehicle::get_axis
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: BulletVehicle::get_wheel
//       Access: Published
//  Description: Returns the BulletWheel with index idx. Causes an
//               AssertionError if idx is equal or larger than the
//               number of wheels.
////////////////////////////////////////////////////////////////////
BulletWheel BulletVehicle::
get_wheel(int idx) const {

  nassertr(idx < get_num_wheels(), BulletWheel::empty());
  return BulletWheel(_vehicle->getWheelInfo(idx));
}

////////////////////////////////////////////////////////////////////
//     Function: BulletVehicle::sync_b2p
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void BulletVehicle::
sync_b2p() {

  for (int i=0; i < get_num_wheels(); i++) {
    btWheelInfo info = _vehicle->getWheelInfo(i);

    PandaNode *node = (PandaNode *)info.m_clientInfo;
    if (node) {

      CPT(TransformState) ts = btTrans_to_TransformState(info.m_worldTransform);

      // <A> Transform relative to wheel node's parent
      //node->set_transform(ts);

      // <B> Transform absolute
      NodePath np = NodePath::any_path(node);
      np.set_transform(np.get_top(), ts);
    }
  }
}

