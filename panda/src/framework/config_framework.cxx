// Filename: config_framework.cxx
// Created by:  drose (06Sep00)
// 
////////////////////////////////////////////////////////////////////

#include "config_framework.h"

#include <dconfig.h>

Configure(config_framework);
NotifyCategoryDef(framework, "");

ConfigureFn(config_framework) {
}

// This is the height above the ground your eye should maintain while
// driving using the "D" interface.
const double drive_height = config_framework.GetDouble("drive-height", 6.0);
const CollideMask drive_mask = config_framework.GetInt("drive-mask", ~0);
