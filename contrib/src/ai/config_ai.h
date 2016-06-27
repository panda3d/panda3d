/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_ai.h
 * @author Pandai
 * @date 2009-09-13
 */

#ifndef CONFIG_AI_H
#define CONFIG_AI_H

#include "contribbase.h"
#include "notifyCategoryProxy.h"

NotifyCategoryDecl(ai, EXPCL_PANDAAI, EXPTP_PANDAAI);

extern EXPCL_PANDAAI void init_libai();

#endif
