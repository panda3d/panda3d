// Filename: config_mayaegg.cxx
// Created by:  drose (15Apr02)
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

#include "config_mayaegg.h"
#include "mayaEggGroupUserData.h"
#include "mayaNodeDesc.h"

#include "dconfig.h"

Configure(config_mayaegg);
NotifyCategoryDef(mayaegg, ":maya");

ConfigureFn(config_mayaegg) {
  init_libmayaegg();
}

// These control the default behavior of the mayaegg converter, but
// not necessarily the default behavior of the maya2egg command-line
// tool (which has its own defaults).

// Should we respect the Maya double-sided flag (true) or ignore it
// and assume everything is single-sided (false)?
const bool maya_default_double_sided = config_mayaegg.GetBool("maya-default-double-sided", false);

// Should we apply vertex color even when a texture is applied (true)
// or only when no texture is applied or the vertex-color egg flag is
// set (false)?
const bool maya_default_vertex_color = config_mayaegg.GetBool("maya-default-vertex-color", true);

////////////////////////////////////////////////////////////////////
//     Function: init_libmayaegg
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libmayaegg() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  MayaEggGroupUserData::init_type();
  MayaNodeDesc::init_type();
}

