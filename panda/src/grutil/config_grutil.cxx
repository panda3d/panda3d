// Filename: config_grutil.cxx
// Created by:  drose (24May00)
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

#include "config_grutil.h"
#include "frameRateMeter.h"

#include "dconfig.h"

Configure(config_grutil);
NotifyCategoryDef(grutil, "");

ConfigureFn(config_grutil) {
  init_libgrutil();
}

const double frame_rate_meter_update_interval = config_grutil.GetDouble("frame-rate-meter-update-interval", 1.0);
const string frame_rate_meter_text_pattern = config_grutil.GetString("frame-rate-meter-text-pattern", "%0.1f fps");
const int frame_rate_meter_layer_sort = config_grutil.GetInt("frame-rate-meter-layer-sort", 1000);
const float frame_rate_meter_scale = config_grutil.GetFloat("frame-rate-meter-scale", 0.05f);
const float frame_rate_meter_side_margins = config_grutil.GetFloat("frame-rate-meter-side-margins", 0.5f);


////////////////////////////////////////////////////////////////////
//     Function: init_libgrutil
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libgrutil() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  FrameRateMeter::init_type();
}

