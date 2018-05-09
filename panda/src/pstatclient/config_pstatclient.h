/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_pstatclient.h
 * @author drose
 * @date 2000-07-09
 */

#ifndef CONFIG_PSTATS_H
#define CONFIG_PSTATS_H

#include "pandabase.h"

#include "notifyCategoryProxy.h"
#include "dconfig.h"
#include "configVariableString.h"
#include "configVariableInt.h"
#include "configVariableDouble.h"
#include "configVariableBool.h"

// Configure variables for pstats package.

ConfigureDecl(config_pstatclient, EXPCL_PANDA_PSTATCLIENT, EXPTP_PANDA_PSTATCLIENT);
NotifyCategoryDecl(pstats, EXPCL_PANDA_PSTATCLIENT, EXPTP_PANDA_PSTATCLIENT);

extern EXPCL_PANDA_PSTATCLIENT ConfigVariableString pstats_name;
extern EXPCL_PANDA_PSTATCLIENT ConfigVariableDouble pstats_max_rate;
extern EXPCL_PANDA_PSTATCLIENT ConfigVariableBool pstats_threaded_write;
extern EXPCL_PANDA_PSTATCLIENT ConfigVariableInt pstats_max_queue_size;
extern EXPCL_PANDA_PSTATCLIENT ConfigVariableDouble pstats_tcp_ratio;

extern EXPCL_PANDA_PSTATCLIENT ConfigVariableString pstats_host;
extern EXPCL_PANDA_PSTATCLIENT ConfigVariableInt pstats_port;
extern EXPCL_PANDA_PSTATCLIENT ConfigVariableDouble pstats_target_frame_rate;
extern EXPCL_PANDA_PSTATCLIENT ConfigVariableBool pstats_gpu_timing;

extern EXPCL_PANDA_PSTATCLIENT ConfigVariableBool pstats_scroll_mode;
extern EXPCL_PANDA_PSTATCLIENT ConfigVariableDouble pstats_history;
extern EXPCL_PANDA_PSTATCLIENT ConfigVariableDouble pstats_average_time;

extern EXPCL_PANDA_PSTATCLIENT ConfigVariableBool pstats_mem_other;

extern EXPCL_PANDA_PSTATCLIENT void init_libpstatclient();

#endif
