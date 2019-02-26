/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_shaderpipeline.h
 * @author Mitchell Stokes
 * @date 2019-02-16
 */

#ifndef CONFIG_SHADERPIPELINE_H
#define CONFIG_SHADERPIPELINE_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "dconfig.h"

ConfigureDecl(config_shaderpipeline, EXPCL_PANDA_SHADERPIPELINE, EXPTP_PANDA_SHADERPIPELINE);
NotifyCategoryDecl(shaderpipeline, EXPCL_PANDA_SHADERPIPELINE, EXPTP_PANDA_SHADERPIPELINE);

extern EXPCL_PANDA_PGRAPH void init_libshaderpipeline();

#endif
