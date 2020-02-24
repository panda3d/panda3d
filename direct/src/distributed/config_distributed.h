/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_distributed.h
 * @author drose
 * @date 2004-05-19
 */

#ifndef CONFIG_DISTRIBUTED_H
#define CONFIG_DISTRIBUTED_H

#include "directbase.h"
#include "notifyCategoryProxy.h"
#include "dconfig.h"
#include "configVariableInt.h"
#include "configVariableDouble.h"
#include "configVariableBool.h"

NotifyCategoryDecl(distributed, EXPCL_DIRECT_DISTRIBUTED, EXPTP_DIRECT_DISTRIBUTED);

extern EXPCL_DIRECT_DISTRIBUTED ConfigVariableInt game_server_timeout_ms;
extern EXPCL_DIRECT_DISTRIBUTED ConfigVariableDouble min_lag;
extern EXPCL_DIRECT_DISTRIBUTED ConfigVariableDouble max_lag;
extern EXPCL_DIRECT_DISTRIBUTED ConfigVariableBool handle_datagrams_internally;

extern EXPCL_DIRECT_DISTRIBUTED void init_libdistributed();

#endif
