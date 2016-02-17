/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_assimp.h
 * @author rdb
 * @date 2011-03-29
 */

#ifndef CONFIG_ASSIMP_H
#define CONFIG_ASSIMP_H

#include "pandatoolbase.h"

#include "dconfig.h"

ConfigureDecl(config_assimp, EXPCL_ASSIMP, EXPTP_ASSIMP);
NotifyCategoryDecl(assimp, EXPCL_ASSIMP, EXPTP_ASSIMP);

extern EXPCL_ASSIMP void init_libassimp();

#endif
