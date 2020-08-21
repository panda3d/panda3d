/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_cocoadisplay.h
 * @author rdb
 * @date 2012-05-17
 */

#ifndef CONFIG_COCOADISPLAY_H
#define CONFIG_COCOADISPLAY_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableBool.h"

NotifyCategoryDecl(cocoadisplay, EXPCL_PANDA_COCOADISPLAY, EXPTP_PANDA_COCOADISPLAY);

extern EXPCL_PANDA_COCOADISPLAY void init_libcocoadisplay();

#endif
