// Filename: config_particlesystem.h
// Created by:  charles (05Jul00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
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
