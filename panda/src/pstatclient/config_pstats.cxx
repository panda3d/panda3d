// Filename: config_pstats.cxx
// Created by:  drose (09Jul00)
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

#include "config_pstats.h"

#include <dconfig.h>

ConfigureDef(config_pstats);
NotifyCategoryDef(pstats, "");

ConfigureFn(config_pstats) {
  init_libpstatclient();
}

string get_pstats_name() {
  return config_pstats.GetString("pstats-name", "Panda Stats");
}

float get_pstats_max_rate() {
  return config_pstats.GetFloat("pstats-max-rate", 30.0);
}

const string pstats_host = config_pstats.GetString("pstats-host", "localhost");
const int pstats_port = config_pstats.GetInt("pstats-port", 5180);
const float pstats_target_frame_rate = config_pstats.GetFloat("pstats-target-frame-rate", 30.0);

// The rest are different in that they directly control the server,
// not the client.
const bool pstats_scroll_mode = config_pstats.GetBool("pstats-scroll-mode", true);
const float pstats_history = config_pstats.GetFloat("pstats-history", 60.0);
const bool pstats_threaded_write = config_pstats.GetBool("pstats-threaded-write", false);

////////////////////////////////////////////////////////////////////
//     Function: init_libpstatclient
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libpstatclient() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;
}

