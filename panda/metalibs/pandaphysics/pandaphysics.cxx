// Filename: pandaphysics.cxx
// Created by:  drose (16May00)
// 
////////////////////////////////////////////////////////////////////

#include "pandaphysics.h"

#ifndef LINK_IN_PHYSICS
#include <config_physics.h>
#include <config_particlesystem.h>
#endif

////////////////////////////////////////////////////////////////////
//     Function: init_libpandaphysics
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libpandaphysics() {
#ifndef LINK_IN_PHYSICS
  init_libphysics();
  init_libparticlesystem();
#endif
}
