/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_char.h
 * @author drose
 * @date 2000-02-28
 */

#ifndef CONFIG_CHAR_H
#define CONFIG_CHAR_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableBool.h"

// CPPParser can't handle token-pasting to a keyword.
#ifndef CPPPARSER
NotifyCategoryDecl(char, EXPCL_PANDA_CHAR, EXPTP_PANDA_CHAR);
#endif

// Configure variables for char package.
extern EXPCL_PANDA_CHAR ConfigVariableBool even_animation;

extern EXPCL_PANDA_CHAR void init_libchar();

#endif
