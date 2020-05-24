/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_particlesystem.h
 * @author charles
 * @date 2000-07-05
 */

#ifndef CONFIG_PARTICLESYSTEM_H
#define CONFIG_PARTICLESYSTEM_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "dconfig.h"

ConfigureDecl(config_particlesystem, EXPCL_PANDA_PARTICLESYSTEM, EXPTP_PANDA_PARTICLESYSTEM);
NotifyCategoryDecl(particlesystem, EXPCL_PANDA_PARTICLESYSTEM, EXPTP_PANDA_PARTICLESYSTEM);

extern EXPCL_PANDA_PARTICLESYSTEM void init_libparticlesystem();

#endif // CONFIG_PARTICLESYSTEM_H
