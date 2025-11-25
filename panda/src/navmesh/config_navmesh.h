/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_navmesh.h
 * @author Ashwani / 
 * @date 2024
 */

#ifndef CONFIG_NAVMESH_H
#define CONFIG_NAVMESH_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "dconfig.h"

ConfigureDecl(config_navmesh, EXPCL_PANDA_NAVMESH, EXPTP_PANDA_NAVMESH);
NotifyCategoryDecl(navmesh, EXPCL_PANDA_NAVMESH, EXPTP_PANDA_NAVMESH);

extern EXPCL_PANDA_NAVMESH void init_libnavmesh();

#endif

