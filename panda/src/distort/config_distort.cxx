// Filename: config_distort.cxx
// Created by:  drose (11Dec01)
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

#include "config_distort.h"
#include "cylindricalLens.h"
#include "fisheyeLens.h"
#include "pSphereLens.h"
#include "projectionScreen.h"

#include "dconfig.h"

Configure(config_distort);
NotifyCategoryDef(distort, "");

ConfigureFn(config_distort) {
  init_libdistort();
}

// Set this true to compensate for a graphics driver bug in which the
// driver inverts the image when it copies framebuffer-to-texture.
// (This bug is arguably a problem with the OpenGL spec, which seems
// to be unclear about the proper ordering of pixels in this
// operation.)
const bool project_invert_uvs = config_distort.GetBool("project-invert-uvs", false);

////////////////////////////////////////////////////////////////////
//     Function: init_libdistort
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libdistort() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  CylindricalLens::init_type();
  FisheyeLens::init_type();
  PSphereLens::init_type();
  ProjectionScreen::init_type();
}
