// Filename: config_eggcharbase.cxx
// Created by:  drose (26Feb01)
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

#include "config_eggcharbase.h"
#include "eggBackPointer.h"
#include "eggJointNodePointer.h"
#include "eggJointPointer.h"
#include "eggMatrixTablePointer.h"
#include "eggVertexPointer.h"

#include <dconfig.h>

Configure(config_eggcharbase);
NotifyCategoryDef(eggcharbase, "");

ConfigureFn(config_eggcharbase) {
  init_libeggcharbase();
}

////////////////////////////////////////////////////////////////////
//     Function: init_libeggcharbase
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libeggcharbase() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  EggBackPointer::init_type();
  EggJointNodePointer::init_type();
  EggJointPointer::init_type();
  EggMatrixTablePointer::init_type();
  EggVertexPointer::init_type();
}
