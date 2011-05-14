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
//  Description:
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
//  Description:
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
//  Description:
////////////////////////////////////////////////////////////////////
LVector3f BulletVehicle::
get_forward_vector() const {

  return btVector3_to_LVector3f(_vehicle->getForwardVector());
}

////////////////////////////////////////////////////////////////////
//     Function: BulletVehicle::get_chassis
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BulletRigidBodyNode *BulletVehicle::
get_chassis() {

  btRigidBody *bodyPtr = _vehicle->getRigidBody();
  return (bodyPtr) ? (BulletRigidBodyNode *)bodyPtr->getUserPointer() : NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletVehicle::get_current_speed_km_hour
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
float BulletVehicle::
get_current_speed_km_hour() const {

  return _vehicle->getCurrentSpeedKmHour();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletVehicle::reset_suspension
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletVehicle::
reset_suspension() {

  _vehicle->resetSuspension();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletVehicle::get_steering_value
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
float BulletVehicle::
get_steering_value(int idx) const {

  nassertr(idx < get_num_wheels(), 0.0f);
  return rad_2_deg(_vehicle->getSteeringValue(idx));
}

////////////////////////////////////////////////////////////////////
//     Function: BulletVehicle::set_steering_value
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletVehicle::
set_steering_value(float steering, int idx) {

  nassertv(idx < get_num_wheels());
  _vehicle->setSteeringValue(deg_2_rad(steering), idx);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletVehicle::apply_engine_force
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletVehicle::
apply_engine_force(float force, int idx) {

  nassertv(idx < get_num_wheels());
  _vehicle->applyEngineForce(force, idx);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletVehicle::set_brake
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletVehicle::
set_brake(float brake, int idx) {

  nassertv(idx < get_num_wheels());
  _vehicle->setBrake(brake, idx);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletVehicle::set_pitch_control
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletVehicle::
set_pitch_control(float pitch) {

  _vehicle->setPitchControl(pitch);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletVehicle::create_wheel
//       Access: Published
//  Description:
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
//  Description:
////////////////////////////////////////////////////////////////////
BulletWheel BulletVehicle::
get_wheel(int idx) const {

  nassertr(idx < get_num_wheels(), BulletWheel::empty());
  return BulletWheel(_vehicle->getWheelInfo(idx));
}

////////////////////////////////////////////////////////////////////
//     Function: BulletVehicle::post_step
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void BulletVehicle::
post_step() {

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

