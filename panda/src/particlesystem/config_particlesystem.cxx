// Filename: config_particlesystem.cxx
// Created by:  charles (05Jul00)
//
////////////////////////////////////////////////////////////////////

#include "config_particlesystem.h"
#include "particleSystem.h"
#include "geomParticleRenderer.h"

ConfigureDef(config_particlesystem);
NotifyCategoryDef(particlesystem, "");

ConfigureFn(config_particlesystem) {
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

}

