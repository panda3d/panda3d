/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_device.h
 * @author drose
 * @date 2000-05-04
 */

#ifndef CONFIG_DEVICE_H
#define CONFIG_DEVICE_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableBool.h"
#include "configVariableInt.h"

NotifyCategoryDecl(device, EXPCL_PANDA_DEVICE, EXPTP_PANDA_DEVICE);

extern ConfigVariableBool asynchronous_clients;

extern EXPCL_PANDA_DEVICE void init_libdevice();

#endif
