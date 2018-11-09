/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_express.h
 * @author cary
 * @date 2000-01-04
 */

#ifndef __CONFIG_EXPRESS_H__
#define __CONFIG_EXPRESS_H__

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "dconfig.h"

#include "configVariableBool.h"
#include "configVariableInt.h"
#include "configVariableDouble.h"
#include "configVariableList.h"
#include "configVariableFilename.h"

// Include these so interrogate can find them.
#include "executionEnvironment.h"
#include "lineStream.h"

ConfigureDecl(config_express, EXPCL_PANDA_EXPRESS, EXPTP_PANDA_EXPRESS);
NotifyCategoryDecl(express, EXPCL_PANDA_EXPRESS, EXPTP_PANDA_EXPRESS);
NotifyCategoryDecl(clock, EXPCL_PANDA_EXPRESS, EXPTP_PANDA_EXPRESS);

// Actually, we can't determine this config variable the normal way, because
// we must be able to access it at static init time.  Instead of declaring it
// a global constant, we'll make it a member of MemoryUsage.

// extern EXPCL_PANDA_EXPRESS const bool track_memory_usage;

EXPCL_PANDA_EXPRESS bool get_use_high_res_clock();
EXPCL_PANDA_EXPRESS bool get_paranoid_clock();
EXPCL_PANDA_EXPRESS bool get_paranoid_inheritance();
EXPCL_PANDA_EXPRESS bool get_verify_dcast();

extern ConfigVariableInt patchfile_window_size;
extern ConfigVariableInt patchfile_increment_size;
extern ConfigVariableInt patchfile_buffer_size;
extern ConfigVariableInt patchfile_zone_size;

extern EXPCL_PANDA_EXPRESS ConfigVariableBool keep_temporary_files;
extern ConfigVariableBool multifile_always_binary;

extern EXPCL_PANDA_EXPRESS ConfigVariableBool collect_tcp;
extern EXPCL_PANDA_EXPRESS ConfigVariableDouble collect_tcp_interval;

extern EXPCL_PANDA_EXPRESS void init_libexpress();

#endif /* __CONFIG_UTIL_H__ */
