// Filename: config_tform.cxx
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

#include "config_tform.h"

#include "driveInterface.h"
#include "buttonThrower.h"
#include "mouseWatcher.h"
#include "mouseWatcherGroup.h"
#include "mouseWatcherRegion.h"
#include "trackball.h"
#include "transform2sg.h"

#include "dconfig.h"

Configure(config_tform);
NotifyCategoryDef(tform, "");

ConfigVariableDouble drive_forward_speed
("drive-forward-speed", 20.0);
ConfigVariableDouble drive_reverse_speed
("drive-reverse-speed", 10.0);
ConfigVariableDouble drive_rotate_speed
("drive-rotate-speed", 80.0);
ConfigVariableDouble drive_vertical_dead_zone
("drive-vertical-dead-zone", 0.1);
ConfigVariableDouble drive_vertical_center
("drive-vertical-center", 0.0);
ConfigVariableDouble drive_horizontal_dead_zone
("drive-horizontal-dead-zone", 0.1);
ConfigVariableDouble drive_horizontal_center
("drive-horizontal-center", 0.0);
ConfigVariableDouble drive_vertical_ramp_up_time
("drive-vertical-ramp-up-time", 0.0);
ConfigVariableDouble drive_vertical_ramp_down_time
("drive-vertical-ramp-down-time", 0.0);
ConfigVariableDouble drive_horizontal_ramp_up_time
("drive-horizontal-ramp-up-time", 0.0);
ConfigVariableDouble drive_horizontal_ramp_down_time
("drive-horizontal-ramp-down-time", 0.0);

ConfigureFn(config_tform) {
  DriveInterface::init_type();
  ButtonThrower::init_type();
  MouseInterfaceNode::init_type();
  MouseWatcher::init_type();
  MouseWatcherGroup::init_type();
  MouseWatcherRegion::init_type();
  Trackball::init_type();
  Transform2SG::init_type();
}
