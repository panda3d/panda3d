/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_gsgbase.cxx
 * @author drose
 * @date 1999-10-06
 */

#include "config_gsgbase.h"
#include "graphicsOutputBase.h"
#include "graphicsStateGuardianBase.h"

#include "dconfig.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_PANDA_GSGBASE)
  #error Buildsystem error: BUILDING_PANDA_GSGBASE not defined
#endif

Configure(config_gsgbase);

ConfigureFn(config_gsgbase) {
  GraphicsOutputBase::init_type();
  GraphicsStateGuardianBase::init_type();
}
