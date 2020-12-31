/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_navmeshgen.h
 * @author ashwini
 * @date 2020-060-21
 */

#ifndef CONFIG_NAVMESHGEN_H
#define CONFIG_NAVMESHGEN_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableDouble.h"
#include "configVariableString.h"
#include "configVariableInt.h"

NotifyCategoryDecl(navmeshgen, EXPCL_NAVMESHGEN, EXPTP_NAVMESHGEN);

extern EXPCL_NAVMESHGEN void init_libnavmeshgen();

#endif

