// Filename: config_pfm.cxx
// Created by:  drose (23Dec10)
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

#include "config_pfm.h"

#include "dconfig.h"

Configure(config_pfm);
NotifyCategoryDef(pfm, "");

ConfigVariableDouble pfm_bba_dist
("pfm-bba-dist", "0.2 0.05",
 PRC_DESC("Specifies the point_dist and sample_radius, in UV space, for "
          "compute bba files with pfm_trans."));

ConfigureFn(config_pfm) {
  init_libpfm();
}

////////////////////////////////////////////////////////////////////
//     Function: init_libpfm
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libpfm() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;
}

