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

// By including checkPandaVersion.h, we guarantee that runtime
// attempts to load libframework.so/.dll will fail if they
// inadvertently link with the wrong version of libdtool.so/.dll.

#include "checkPandaVersion.h"

Configure(config_framework);
NotifyCategoryDef(framework, "");

ConfigVariableDouble aspect_ratio
("aspect-ratio", 0.0);
ConfigVariableBool show_frame_rate_meter
("show-frame-rate-meter", false);

ConfigVariableString record_session
("record-session", "");
ConfigVariableString playback_session
("playback-session", "");


ConfigureFn(config_framework) {
  WindowFramework::init_type();
}
