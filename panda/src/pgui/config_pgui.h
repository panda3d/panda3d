/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_pgui.h
 * @author drose
 * @date 2001-07-02
 */

#ifndef CONFIG_PGUI_H
#define CONFIG_PGUI_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableDouble.h"

NotifyCategoryDecl(pgui, EXPCL_PANDA_PGUI, EXPTP_PANDA_PGUI);

// Configure variables for pgui package.
extern ConfigVariableDouble scroll_initial_delay;
extern ConfigVariableDouble scroll_continued_delay;

extern EXPCL_PANDA_PGUI void init_libpgui();

#endif
