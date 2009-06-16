// Filename: config_gles2gsg.h
// Created by:  pro-rsoft (14Jun09)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef CONFIG_GLES2GSG_H
#define CONFIG_GLES2GSG_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "dconfig.h"

ConfigureDecl(config_gles2gsg, EXPCL_PANDAGLES2, EXPTP_PANDAGLES2);
NotifyCategoryDecl(gles2gsg, EXPCL_PANDAGLES2, EXPTP_PANDAGLES2);

extern EXPCL_PANDAGLES2 void init_libgles2gsg();

#endif
