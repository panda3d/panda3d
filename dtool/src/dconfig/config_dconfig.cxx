/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_dconfig.cxx
 * @author drose
 * @date 2000-05-15
 */

#include "config_dconfig.h"

#if !defined(CPPPARSER) && !defined(BUILDING_DTOOL_DCONFIG)
  #error Buildsystem error: BUILDING_DTOOL_DCONFIG not defined
#endif

NotifyCategoryDef(dconfig, "");
NotifyCategoryDef(microconfig, "dconfig");
