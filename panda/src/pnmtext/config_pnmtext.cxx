// Filename: config_pnmtext.cxx
// Created by:  drose (08Sep03)
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

#include "config_pnmtext.h"

#include "dconfig.h"

Configure(config_pnmtext);
NotifyCategoryDef(pnmtext, "");

ConfigureFn(config_pnmtext) {
  init_libpnmtext();
}

const float text_point_size = config_pnmtext.GetFloat("text-point-size", 10.0f);
const float text_pixels_per_unit = config_pnmtext.GetFloat("text-pixels-per-unit", 30.0f);
const float text_scale_factor = config_pnmtext.GetFloat("text-scale-factor", 2.0f);

////////////////////////////////////////////////////////////////////
//     Function: init_libpnmtext
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libpnmtext() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

}
