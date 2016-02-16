/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_awesomium.h
 * @author rurbino
 * @date 2009-10-12
 */

#ifndef CONFIG_AWESOMIUM_H
#define CONFIG_AWESOMIUM_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"

#include "dconfig.h"

NotifyCategoryDecl(awesomium, EXPCL_PANDAAWESOMIUM, EXPTP_PANDAAWESOMIUM);

extern EXPCL_PANDAAWESOMIUM void init_libawesomium();

#endif /* CONFIG_AWESOMIUM_H */
