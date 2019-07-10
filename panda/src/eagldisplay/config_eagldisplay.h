/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_eagldisplay.h
 * @author D. Lawrence
 * @date 2019-01-03
 */

#ifndef CONFIG_EAGLDISPLAY_H
#define CONFIG_EAGLDISPLAY_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableBool.h"
#include "configVariableInt.h"

NotifyCategoryDecl(eagldisplay, EXPORT_CLASS, EXPORT_TEMPL);

extern void init_libeagldisplay();

#endif  // CONFIG_IPHONE_H
