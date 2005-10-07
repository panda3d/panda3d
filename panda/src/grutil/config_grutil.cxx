// Filename: config_grutil.cxx
// Created by:  drose (24May00)
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

#include "config_grutil.h"
#include "frameRateMeter.h"
#include "openCVTexture.h"
#include "pandaSystem.h"
#include "texturePool.h"

#include "dconfig.h"

Configure(config_grutil);
NotifyCategoryDef(grutil, "");

ConfigureFn(config_grutil) {
  init_libgrutil();
}

ConfigVariableDouble frame_rate_meter_update_interval
("frame-rate-meter-update-interval", 1.5);

ConfigVariableString frame_rate_meter_text_pattern
("frame-rate-meter-text-pattern", "%0.1f fps");

ConfigVariableInt frame_rate_meter_layer_sort
("frame-rate-meter-layer-sort", 1000);

ConfigVariableDouble frame_rate_meter_scale
("frame-rate-meter-scale", 0.05);

ConfigVariableDouble frame_rate_meter_side_margins
("frame-rate-meter-side-margins", 0.5);


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

#ifdef HAVE_OPENCV
  OpenCVTexture::init_type();
  OpenCVTexture::register_with_read_factory();

  PandaSystem *ps = PandaSystem::get_global_ptr();
  ps->add_system("OpenCV");
  TexturePool *ts = TexturePool::get_global_ptr();
  ts->register_texture_type(OpenCVTexture::make_texture, "avi mpg");
#endif // HAVE_OPENCV
}

