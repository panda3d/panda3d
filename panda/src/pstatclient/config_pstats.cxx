// Filename: config_pstats.cxx
// Created by:  drose (09Jul00)
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

#include "config_pstats.h"

#include "dconfig.h"

ConfigureDef(config_pstats);
NotifyCategoryDef(pstats, "");

ConfigureFn(config_pstats) {
  init_libpstatclient();
}

ConfigVariableString pstats_name
("pstats-name", "Panda Stats");

ConfigVariableDouble pstats_max_rate
("pstats-max-rate", 30.0);

ConfigVariableBool pstats_threaded_write
("pstats-threaded-write", false);

ConfigVariableDouble pstats_tcp_ratio
("pstats-tcp-ratio", 0.01,
 PRC_DESC("This specifies the ratio of frame update messages that are eligible "
          "for UDP that are sent via TCP instead.  It does not count messages "
          "that are too large for UDP and must be sent via TCP anyway.  1.0 "
          "means all messages are sent TCP; 0.0 means all are sent UDP."));

ConfigVariableString pstats_host
("pstats-host", "localhost");

// The default port for PStats used to be 5180, but that's used by AIM.
ConfigVariableInt pstats_port
("pstats-port", 5185);

ConfigVariableDouble pstats_target_frame_rate
("pstats-target-frame-rate", 30.0);

// The rest are different in that they directly control the server,
// not the client.
ConfigVariableBool pstats_scroll_mode
("pstats-scroll-mode", true);
ConfigVariableDouble pstats_history
("pstats-history", 60.0);
ConfigVariableDouble pstats_average_time
("pstats-average-time", 3.0);

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

