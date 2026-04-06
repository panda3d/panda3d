/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_ptloader.h
 * @author drose
 * @date 2001-04-26
 */

#ifndef CONFIG_PTLOADER_H
#define CONFIG_PTLOADER_H

#include "pandatoolbase.h"
#include "notifyCategoryProxy.h"
#include "dconfig.h"
#include "distanceUnit.h"
#include "configVariableEnum.h"
#include "configVariableBool.h"

ConfigureDecl(config_ptloader, EXPCL_PTLOADER, EXPTP_PTLOADER);
NotifyCategoryDecl(ptloader, EXPCL_PTLOADER, EXPTP_PTLOADER);

extern ConfigVariableEnum<DistanceUnit> ptloader_units;
extern ConfigVariableBool ptloader_load_node;

extern EXPCL_PTLOADER void init_libptloader();

#endif
