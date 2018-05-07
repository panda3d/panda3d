/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_vision.h
 * @author rdb
 * @date 2009-11-07
 */

#ifndef CONFIG_VISION_H
#define CONFIG_VISION_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableBool.h"

NotifyCategoryDecl(vision, EXPCL_VISION, EXPTP_VISION);

extern ConfigVariableBool v4l_blocking;

extern EXPCL_VISION void init_libvision();

#endif
