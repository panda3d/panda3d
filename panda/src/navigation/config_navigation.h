/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_navigation.h
 * @author ashwini
 * @date 2020-060-21
 */

#ifndef CONFIG_NAVIGATION_H
#define CONFIG_NAVIGATION_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableDouble.h"
#include "configVariableString.h"
#include "configVariableInt.h"

NotifyCategoryDecl(navigation, EXPCL_NAVIGATION, EXPTP_NAVIGATION);

extern EXPCL_NAVIGATION void init_libnavigation();

#endif
