// Filename: config_eggcharbase.cxx
// Created by:  drose (26Feb01)
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

#include "config_eggcharbase.h"
#include "eggBackPointer.h"
#include "eggComponentData.h"
#include "eggJointData.h"
#include "eggJointNodePointer.h"
#include "eggJointPointer.h"
#include "eggMatrixTablePointer.h"
#include "eggScalarTablePointer.h"
#include "eggSliderData.h"
#include "eggSliderPointer.h"
#include "eggVertexPointer.h"

#include "dconfig.h"


Configure(config_eggcharbase);

// NotifyCategoryDef(eggcharbase, "");

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
  EggComponentData::init_type();
  EggJointData::init_type();
  EggJointNodePointer::init_type();
  EggJointPointer::init_type();
  EggMatrixTablePointer::init_type();
  EggScalarTablePointer::init_type();
  EggSliderData::init_type();
  EggSliderPointer::init_type();
  EggVertexPointer::init_type();
}
