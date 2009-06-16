// Filename: config_glesgsg.h
// Created by:  pro-rsoft (21May09)
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

#ifndef CONFIG_GLESGSG_H
#define CONFIG_GLESGSG_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "dconfig.h"

ConfigureDecl(config_glesgsg, EXPCL_PANDAGLES, EXPTP_PANDAGLES);
NotifyCategoryDecl(glesgsg, EXPCL_PANDAGLES, EXPTP_PANDAGLES);

extern EXPCL_PANDAGLES void init_libglesgsg();

#endif
