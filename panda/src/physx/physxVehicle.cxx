// Filename: physxVehicle.cxx
// Created by:  enn0x (23Mar10)
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

#include "physxVehicle.h"
#include "physxActor.h"
#include "physxWheel.h"
#include "physxScene.h"

TypeHandle PhysxVehicle::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PhysxVehicle::create
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxVehicle::
create(PhysxScene *scene, PhysxVehicleDesc &desc) {

  nassertv(_error_type == ET_empty);

  _scene = scene;

  // TODO !!!

  _error_type = ET_ok;
  _scene->_vehicles.add(this);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxVehicle::release
//       Access: Published
//  Description: Destroys this vehicle.
////////////////////////////////////////////////////////////////////
void PhysxVehicle::
release() {

  nassertv(_error_type == ET_ok);

  _error_type = ET_released;
  _scene->_vehicles.remove(this);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxVehicle::update_vehicle
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxVehicle::
update_vehicle(float dt) {

  nassertv(_error_type == ET_ok);

  // TODO !!!
}

/*
////////////////////////////////////////////////////////////////////
//     Function: PhysxVehicle::get_actor
//       Access: Published
//  Description: Returns the actor for this vehicle.
////////////////////////////////////////////////////////////////////
PhysxActor *PhysxVehicle::
get_actor() const {

  nassertr(_error_type == ET_ok, NULL);
  return _actor;
}
*/

/*
////////////////////////////////////////////////////////////////////
//     Function: PhysxVehicle::get_num_wheels
//       Access: Published
//  Description: Returns the number of wheels on this vehicle.
////////////////////////////////////////////////////////////////////
unsigned int PhysxVehicle::
get_num_wheels() const {

  nassertr(_error_type == ET_ok, 0);
  return _wheels.size();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxVehicle::get_wheel
//       Access: Published
//  Description: Returns the n-th wheel of this vehicle.
////////////////////////////////////////////////////////////////////
PhysxWheel *PhysxVehicle::
get_wheel(unsigned int idx) const {

  nassertr(_error_type == ET_ok, NULL);
  return _wheels[idx];
}
*/

