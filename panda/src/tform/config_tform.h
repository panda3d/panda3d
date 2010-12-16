// Filename: config_tform.h
// Created by:  drose (23Feb00)
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

#ifndef CONFIG_TFORM_H
#define CONFIG_TFORM_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableDouble.h"
#include "configVariableBool.h"

NotifyCategoryDecl(tform, EXPCL_PANDA_TFORM, EXPTP_PANDA_TFORM);

// Configure variables for tform package.
extern EXPCL_PANDA_TFORM ConfigVariableDouble drive_forward_speed;
extern EXPCL_PANDA_TFORM ConfigVariableDouble drive_reverse_speed;
extern EXPCL_PANDA_TFORM ConfigVariableDouble drive_rotate_speed;
extern EXPCL_PANDA_TFORM ConfigVariableDouble drive_vertical_dead_zone;
extern EXPCL_PANDA_TFORM ConfigVariableDouble drive_vertical_center;
extern EXPCL_PANDA_TFORM ConfigVariableDouble drive_horizontal_dead_zone;
extern EXPCL_PANDA_TFORM ConfigVariableDouble drive_horizontal_center;
extern EXPCL_PANDA_TFORM ConfigVariableDouble drive_vertical_ramp_up_time;
extern EXPCL_PANDA_TFORM ConfigVariableDouble drive_vertical_ramp_down_time;
extern EXPCL_PANDA_TFORM ConfigVariableDouble drive_horizontal_ramp_up_time;
extern EXPCL_PANDA_TFORM ConfigVariableDouble drive_horizontal_ramp_down_time;

extern EXPCL_PANDA_TFORM ConfigVariableDouble inactivity_timeout;

extern EXPCL_PANDA_TFORM ConfigVariableBool trackball_use_alt_keys;

#endif
