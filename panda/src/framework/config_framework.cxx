// Filename: config_framework.cxx
// Created by:  drose (06Sep00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
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
