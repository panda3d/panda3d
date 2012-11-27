// Filename: physxControllerReport.cxx
// Created by:  enn0x (24Sep09)
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

#include "physxControllerReport.h"

PStatCollector PhysxControllerReport::_pcollector("App:PhysX:Controller Reporting");

////////////////////////////////////////////////////////////////////
//     Function: PhysxControllerReport::enable
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void PhysxControllerReport::
enable() {

  _enabled = true;

  _shape_hit_cbobj = NULL;
  _controller_hit_cbobj = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxControllerReport::disable
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void PhysxControllerReport::
disable() {

  _enabled = false;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxControllerReport::is_enabled
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool PhysxControllerReport::
is_enabled() const {

  return _enabled;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxControllerReport::onShapeHit
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
NxControllerAction PhysxControllerReport::
onShapeHit( const NxControllerShapeHit& hit ) {

  if (!_enabled) {
    return NX_ACTION_NONE;
  }

  _pcollector.start();

  if (_shape_hit_cbobj) {
    // Callback
    PhysxControllerShapeHit cbdata(hit);
    _shape_hit_cbobj->do_callback(&cbdata);
  } 
  else {
    // Default implementation
    if (1 && hit.shape) {
      NxActor& actor = hit.shape->getActor();
      if (actor.isDynamic() && !actor.readBodyFlag(NX_BF_KINEMATIC)) {
        if (hit.dir.z == 0.0f) {
          NxF32 controllerMass = hit.controller->getActor()->getMass();
          NxF32 coeff = actor.getMass() * hit.length * controllerMass;
          actor.addForceAtLocalPos(hit.dir*coeff, NxVec3(0,0,0), NX_IMPULSE);
        }
      }
    }
  }

  _pcollector.stop();

  return NX_ACTION_NONE;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxControllerReport::onControllerHit
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
NxControllerAction PhysxControllerReport::
onControllerHit(const NxControllersHit& hit) {

  if (!_enabled) {
    return NX_ACTION_NONE;
  }

  _pcollector.start();

  if (_controller_hit_cbobj) {
    // Callback
    PhysxControllersHit cbdata(hit);
    _controller_hit_cbobj->do_callback(&cbdata);
  } 
  else {
    // Default implementation
    if (1 && hit.other) {
      // For now other controllers are unpushable. --TODO--
      //return NX_ACTION_PUSH; is not implemented!
    }
  }

  _pcollector.stop();

  return NX_ACTION_NONE;
}

