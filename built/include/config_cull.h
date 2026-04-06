/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_cull.h
 * @author drose
 * @date 2006-03-23
 */

#ifndef CONFIG_CULL_H
#define CONFIG_CULL_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "dconfig.h"
#include "configVariableInt.h"
#include "configVariableDouble.h"
#include "configVariableBool.h"

class DSearchPath;

ConfigureDecl(config_cull, EXPCL_PANDA_CULL, EXPTP_PANDA_CULL);
NotifyCategoryDecl(cull, EXPCL_PANDA_CULL, EXPTP_PANDA_CULL);

extern EXPCL_PANDA_CULL void init_libcull();

#endif
