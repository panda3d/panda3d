// Filename: config_x11display.h
// Created by:  rdb (07Jul09)
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

#ifndef CONFIG_X11DISPLAY_H
#define CONFIG_X11DISPLAY_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableString.h"
#include "configVariableBool.h"
#include "configVariableInt.h"

NotifyCategoryDecl(x11display, EXPCL_PANDAX11, EXPTP_PANDAX11);

extern EXPCL_PANDAX11 void init_libx11display();

extern ConfigVariableString display_cfg;
extern ConfigVariableBool x_error_abort;

extern ConfigVariableInt x_wheel_up_button;
extern ConfigVariableInt x_wheel_down_button;
extern ConfigVariableInt x_wheel_left_button;
extern ConfigVariableInt x_wheel_right_button;

extern ConfigVariableString x_wm_class_name;
extern ConfigVariableString x_wm_class;

#endif
