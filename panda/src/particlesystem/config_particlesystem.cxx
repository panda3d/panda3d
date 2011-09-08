// Filename: config_particlesystem.cxx
// Created by:  charles (05Jul00)
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

#include "config_particlesystem.h"
#include "particleSystem.h"
#include "geomParticleRenderer.h"
#include "geomNode.h"

ConfigureDef(config_particlesystem);
NotifyCategoryDef(particlesystem, "");

ConfigureFn(config_particlesystem) {
  ColorInterpolationFunction::init_type();
  ColorInterpolationFunctionConstant::init_type();
  ColorInterpolationFunctionLinear::init_type();
  ColorInterpolationFunctionStepwave::init_type();
  ColorInterpolationFunctionSinusoid::init_type();
  ParticleSystem::init_type();
  GeomNode::GeomList::init_type();  // repeated here to ensure instantiated templates get initialized too.
  init_libparticlesystem();
}

////////////////////////////////////////////////////////////////////
//     Function: init_libparticlesystem
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libparticlesystem() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  ParticleSystem::init_type();
}

