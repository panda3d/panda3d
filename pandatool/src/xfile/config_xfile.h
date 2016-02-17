/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_xfile.h
 * @author drose
 * @date 2001-06-22
 */

#ifndef CONFIG_XFILE_H
#define CONFIG_XFILE_H

#include "pandatoolbase.h"

#include "notifyCategoryProxy.h"

NotifyCategoryDeclNoExport(xfile);

extern bool xfile_one_mesh;

extern void init_libxfile();

#endif
