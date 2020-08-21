/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_tform.cxx
 * @author drose
 * @date 2000-02-23
 */

#include "config_tform.h"

#include "driveInterface.h"
#include "buttonThrower.h"
#include "mouseSubregion.h"
#include "mouseWatcher.h"
#include "mouseWatcherBase.h"
#include "mouseWatcherGroup.h"
#include "mouseWatcherRegion.h"
#include "trackball.h"
#include "transform2sg.h"

#include "dconfig.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_PANDA_TFORM)
  #error Buildsystem error: BUILDING_PANDA_TFORM not defined
#endif

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

ConfigVariableDouble inactivity_timeout
("inactivity-timeout", 0.0);

ConfigVariableBool trackball_use_alt_keys
("trackball-use-alt-keys", true,
 PRC_DESC("Set this true to use the command and option/control keys in "
          "conjunction with the first mouse button to simulate the behavior of "
          "the second and third mouse buttons in trackball mode.  Particularly "
          "useful for Macs, or laptops with limited mouse buttons."));

ConfigureFn(config_tform) {
  DriveInterface::init_type();
  ButtonThrower::init_type();
  MouseInterfaceNode::init_type();
  MouseSubregion::init_type();
  MouseWatcher::init_type();
  MouseWatcherBase::init_type();
  MouseWatcherGroup::init_type();
  MouseWatcherRegion::init_type();
  Trackball::init_type();
  Transform2SG::init_type();
}
