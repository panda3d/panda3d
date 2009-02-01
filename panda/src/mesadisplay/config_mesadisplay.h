// Filename: config_mesadisplay.h
// Created by:  drose (09Feb04)
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

#ifndef CONFIG_MESADISPLAY_H
#define CONFIG_MESADISPLAY_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "dconfig.h"

ConfigureDecl(config_mesadisplay, EXPCL_PANDAMESA, EXPTP_PANDAMESA);
NotifyCategoryDecl(mesadisplay, EXPCL_PANDAMESA, EXPTP_PANDAMESA);

extern EXPCL_PANDAMESA void init_libmesadisplay();
extern "C" EXPCL_PANDAMESA int get_pipe_type_mesadisplay();

#endif
