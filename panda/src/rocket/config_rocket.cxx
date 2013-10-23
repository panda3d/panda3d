// Filename: config_rocket.cxx
// Created by:  rdb (04Nov11)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "config_rocket.h"
#include "rocketFileInterface.h"
#include "rocketInputHandler.h"
#include "rocketRegion.h"
#include "rocketSystemInterface.h"

#include "pandaSystem.h"
#include "dconfig.h"
#include "default_font.h"

// This is defined by both Panda and Rocket.
#define Factory RocketFactory
#include <Rocket/Core.h>
#undef Factory

Configure(config_rocket);
NotifyCategoryDef(rocket, "");

ConfigureFn(config_rocket) {
  init_librocket();
}

////////////////////////////////////////////////////////////////////
//     Function: init_librocket
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_librocket() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  RocketInputHandler::init_type();
  RocketRegion::init_type();

  RocketFileInterface* fi = new RocketFileInterface;
  Rocket::Core::SetFileInterface(fi);

  RocketSystemInterface* si = new RocketSystemInterface;
  Rocket::Core::SetSystemInterface(si);

  Rocket::Core::Initialise();

#ifdef COMPILE_IN_DEFAULT_FONT
#ifdef HAVE_FREETYPE
  // Load Panda's default compiled-in freetype font (Perspective Sans).
  Rocket::Core::FontDatabase::LoadFontFace(default_font_data, default_font_size);
#endif
#endif
}
