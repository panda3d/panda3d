/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_event.h
 * @author drose
 * @date 1999-12-14
 */

#ifndef CONFIG_EVENT_H
#define CONFIG_EVENT_H

#include "pandabase.h"

#include "notifyCategoryProxy.h"

NotifyCategoryDecl(event, EXPCL_PANDA_EVENT, EXPTP_PANDA_EVENT);
NotifyCategoryDecl(task, EXPCL_PANDA_EVENT, EXPTP_PANDA_EVENT);

#endif
