// Filename: config_iphonedisplay.h
// Created by:  drose (08Apr09)
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

#ifndef CONFIG_IPHONEDISPLAY_H
#define CONFIG_IPHONEDISPLAY_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableBool.h"
#include "configVariableInt.h"

NotifyCategoryDecl(iphonedisplay, EXPCL_MISC, EXPTP_MISC);

extern ConfigVariableBool iphone_autorotate_view;

extern EXPCL_MISC void init_libiphonedisplay();
extern "C" EXPCL_MISC int get_pipe_type_iphonedisplay();

#endif  // CONFIG_IPHONEDISPLAY_H
