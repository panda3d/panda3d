/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_nativenet.h
 * @author drose
 * @date 2007-03-01
 */

#ifndef CONFIG_NATIVENET_H
#define CONFIG_NATIVENET_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"

// Configure variables for nativenet package.

NotifyCategoryDecl(nativenet, EXPCL_PANDA_NATIVENET, EXPTP_PANDA_NATIVENET);

extern EXPCL_PANDA_NATIVENET void init_libnativenet();

#endif
