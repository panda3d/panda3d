// Filename: config_tform.h
// Created by:  drose (23Feb00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef CONFIG_TFORM_H
#define CONFIG_TFORM_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"

NotifyCategoryDecl(tform, EXPCL_PANDA, EXPTP_PANDA);

// Configure variables for tform package.
extern const double EXPCL_PANDA drive_forward_speed;
extern const double EXPCL_PANDA drive_reverse_speed;
extern const double EXPCL_PANDA drive_rotate_speed;
extern const double EXPCL_PANDA drive_vertical_dead_zone;
extern const double EXPCL_PANDA drive_vertical_center;
extern const double EXPCL_PANDA drive_horizontal_dead_zone;
extern const double EXPCL_PANDA drive_horizontal_center;
extern const double EXPCL_PANDA drive_vertical_ramp_up_time;
extern const double EXPCL_PANDA drive_vertical_ramp_down_time;
extern const double EXPCL_PANDA drive_horizontal_ramp_up_time;
extern const double EXPCL_PANDA drive_horizontal_ramp_down_time;

#endif
