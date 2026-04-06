/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_glgsg.h
 * @author drose
 * @date 1999-10-06
 */

#ifndef CONFIG_GLGSG_H
#define CONFIG_GLGSG_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "dconfig.h"

ConfigureDecl(config_glgsg, EXPCL_PANDA_GLGSG, EXPTP_PANDA_GLGSG);
NotifyCategoryDecl(glgsg, EXPCL_PANDA_GLGSG, EXPTP_PANDA_GLGSG);

extern EXPCL_PANDA_GLGSG void init_libglgsg();

#endif
