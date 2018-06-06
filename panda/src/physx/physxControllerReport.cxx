/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxControllerReport.cxx
 * @author enn0x
 * @date 2009-09-24
 */

#include "physxControllerReport.h"

PStatCollector PhysxControllerReport::_pcollector("App:PhysX:Controller Reporting");

/**
 *
 */
void PhysxControllerReport::
enable() {

  _enabled = true;

  _shape_hit_cbobj = nullptr;
  _controller_hit_cbobj = nullptr;
}

/**
 *
 */
void PhysxControllerReport::
disable() {

  _enabled = false;
}

/**
 *
 */
bool PhysxControllerReport::
is_enabled() const {

  return _enabled;
}

/**
 *
 */
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

/**
 *
 */
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
      // For now other controllers are unpushable.  --TODO-- return
      // NX_ACTION_PUSH; is not implemented!
    }
  }

  _pcollector.stop();

  return NX_ACTION_NONE;
}
