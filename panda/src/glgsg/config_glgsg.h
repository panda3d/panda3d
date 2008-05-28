// Filename: config_glgsg.h
// Created by:  drose (06Oct99)
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

#ifndef CONFIG_GLGSG_H
#define CONFIG_GLGSG_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "dconfig.h"

ConfigureDecl(config_glgsg, EXPCL_PANDAGL, EXPTP_PANDAGL);
NotifyCategoryDecl(glgsg, EXPCL_PANDAGL, EXPTP_PANDAGL);

extern EXPCL_PANDAGL void init_libglgsg();

#endif
