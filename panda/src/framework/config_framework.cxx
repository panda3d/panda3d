// Filename: config_framework.cxx
// Created by:  drose (06Sep00)
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

#include "config_framework.h"

#include "dconfig.h"
#include "windowFramework.h"

Configure(config_framework);
NotifyCategoryDef(framework, "");

ConfigVariableDouble aspect_ratio
("aspect-ratio", 0.0);
ConfigVariableBool show_frame_rate_meter
("show-frame-rate-meter", false);

// The default window background color.
ConfigVariableDouble win_background_r
("win-background-r", 0.41);
ConfigVariableDouble win_background_g
("win-background-g", 0.41);
ConfigVariableDouble win_background_b
("win-background-b", 0.41);

ConfigVariableString record_session
("record-session", "");
ConfigVariableString playback_session
("playback-session", "");


ConfigureFn(config_framework) {
  WindowFramework::init_type();
}
