// Filename: config_distributed.cxx
// Created by:  drose (19May04)
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

#include "config_distributed.h"
#include "dconfig.h"

Configure(config_distributed);
NotifyCategoryDef(distributed, "");

ConfigureFn(config_distributed) {
  init_libdistributed();
}

ConfigVariableInt game_server_timeout_ms
("game-server-timeout-ms", 20000,
 PRC_DESC("This represents the amount of time to block waiting for the TCP "
          "connection to the game server.  It is only used when the connection "
          "method is NSPR."));

ConfigVariableDouble min_lag
("min-lag", 0.0,
 PRC_DESC("This represents the time in seconds by which to artificially lag "
          "inbound messages.  It is only used when the connection method is "
          "NSPR."));

ConfigVariableDouble max_lag
("max-lag", 0.0,
 PRC_DESC("This represents the time in seconds by which to artificially lag "
          "inbound messages.  It is only used when the connection method is "
          "NSPR."));

////////////////////////////////////////////////////////////////////
//     Function: init_libdistributed
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libdistributed() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

}

