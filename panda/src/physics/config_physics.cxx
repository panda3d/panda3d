// Filename: config_physics.cxx
// Created by:  charles (17Jul00)
// 
////////////////////////////////////////////////////////////////////

#include "config_physics.h"
#include "physicsObject.h"
#include "physicalNode.h"
#include "linearIntegrator.h"
#include "angularIntegrator.h"
#include "forceNode.h"
#include "forces.h"
#include <dconfig.h>

ConfigureDef(config_physics);
NotifyCategoryDef(physics, "");

ConfigureFn(config_physics) {
  init_libphysics();
}

const float LinearIntegrator::_max_linear_dt = 
  config_physics.GetFloat("default_max_linear_dt", 1.0f / 30.0f);

const float AngularIntegrator::_max_angular_dt =
  config_physics.GetFloat("default_max_angular_dt", 1.0f / 30.0f);

int LinearNoiseForce::_random_seed = 
  config_physics.GetInt("default_noise_force_seed", 665);

const float PhysicsObject::_default_terminal_velocity = 
  config_physics.GetFloat("default_terminal_velocity", 400.0f);

////////////////////////////////////////////////////////////////////
//     Function: init_libphysics
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libphysics() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  PhysicsObject::init_type();
  Physical::init_type();
  PhysicalNode::init_type();
  ForceNode::init_type();
  BaseForce::init_type();
  LinearForce::init_type();
  LinearVectorForce::init_type();
  LinearRandomForce::init_type();
  LinearJitterForce::init_type();
  LinearNoiseForce::init_type();
  LinearDistanceForce::init_type();
  LinearSinkForce::init_type();
  LinearSourceForce::init_type();
  LinearFrictionForce::init_type();
  LinearUserDefinedForce::init_type();
  LinearCylinderVortexForce::init_type();
}
