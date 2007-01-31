// Filename: config_motiontrail.cxx
// Created by:  drose (27Aug02)
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

#include "config_motiontrail.h"
#include "dconfig.h"

extern EXPCL_DIRECT void init_libmotiontrail();

Configure(config_motiontrail);
NotifyCategoryDef(motiontrail, "");

ConfigureFn(config_motiontrail) {
  init_libmotiontrail();
}

////////////////////////////////////////////////////////////////////
//     Function: init_libmotiontrail
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libmotiontrail() {
  static bool initialized = false;
  if (initialized == false) {
    CMotionTrail::init_type();
    initialized = true;
  }
}
