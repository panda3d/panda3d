/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_recorder.h
 * @author drose
 * @date 2004-01-28
 */

#ifndef CONFIG_RECORDER_H
#define CONFIG_RECORDER_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "dconfig.h"

ConfigureDecl(config_recorder, EXPCL_PANDA_RECORDER, EXPTP_PANDA_RECORDER);
NotifyCategoryDecl(recorder, EXPCL_PANDA_RECORDER, EXPTP_PANDA_RECORDER);

#endif // CONFIG_RECORDER_H
