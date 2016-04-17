/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_rocket.h
 * @author rdb
 * @date 2011-11-04
 */

#ifndef CONFIG_ROCKET_H
#define CONFIG_ROCKET_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"

NotifyCategoryDecl(rocket, EXPCL_ROCKET, EXPTP_ROCKET);

extern EXPCL_ROCKET void init_librocket();

#endif
