// Filename: config_tform.cxx
// Created by:  drose (23Feb00)
// 
////////////////////////////////////////////////////////////////////

#include "config_tform.h"
#include "driveInterface.h"
#include "buttonThrower.h"
#include "mouseWatcher.h"
#include "mouseWatcherRegion.h"
#include "planarSlider.h"
#include "trackball.h"
#include "transform2sg.h"
#include "trackerTransform.h"

#include <dconfig.h>

Configure(config_tform);
NotifyCategoryDef(tform, "");

const double drive_forward_speed = config_tform.GetDouble("drive-forward-speed", 20.0);
const double drive_reverse_speed = config_tform.GetDouble("drive-reverse-speed", 10.0);
const double drive_rotate_speed = config_tform.GetDouble("drive-rotate-speed", 80.0);
const double drive_vertical_dead_zone = config_tform.GetDouble("drive-vertical-dead-zone", 0.1);
const double drive_horizontal_dead_zone = config_tform.GetDouble("drive-horizontal-dead-zone", 0.1);
const double drive_vertical_ramp_up_time = config_tform.GetDouble("drive-vertical-ramp-up-time", 0.0);
const double drive_vertical_ramp_down_time = config_tform.GetDouble("drive-vertical-ramp-down-time", 0.0);
const double drive_horizontal_ramp_up_time = config_tform.GetDouble("drive-horizontal-ramp-up-time", 0.0);
const double drive_horizontal_ramp_down_time = config_tform.GetDouble("drive-horizontal-ramp-down-time", 0.0);

ConfigureFn(config_tform) {
  DriveInterface::init_type();
  ButtonThrower::init_type();
  MouseWatcher::init_type();
  MouseWatcherRegion::init_type();
  PlanarSlider::init_type();
  Trackball::init_type();
  Transform2SG::init_type();
  TrackerTransform::init_type();
}
