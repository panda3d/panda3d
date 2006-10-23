// Filename: config_deadrec.cxx
// Created by:  drose (23Oct06)
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

#include "config_deadrec.h"

#include "dconfig.h"

Configure(config_deadrec);
NotifyCategoryDef(deadrec, "");

ConfigureFn(config_deadrec) {
  init_libdeadrec();
}


////////////////////////////////////////////////////////////////////
//     Function: init_libdeadrec
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libdeadrec() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;
}

