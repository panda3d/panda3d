// Filename: config_particlesystem.h
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

#ifndef CONFIG_PARTICLESYSTEM_H
#define CONFIG_PARTICLESYSTEM_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "dconfig.h"

ConfigureDecl(config_particlesystem, EXPCL_PANDAPHYSICS, EXPTP_PANDAPHYSICS);
NotifyCategoryDecl(particlesystem, EXPCL_PANDAPHYSICS, EXPTP_PANDAPHYSICS);

extern EXPCL_PANDAPHYSICS void init_libparticlesystem();

#ifndef NDEBUG //[
  // Non-release build:
  #define PARTICLE_SYSTEM_DEBUG
#else //][
  // Release build:
  #undef PARTICLE_SYSTEM_DEBUG
#endif //]

#endif // CONFIG_PARTICLESYSTEM_H
