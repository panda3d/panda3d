// Filename: winStatsServer.cxx
// Created by:  drose (02Dec03)
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

#include "winStatsServer.h"
#include "winStatsMonitor.h"

////////////////////////////////////////////////////////////////////
//     Function: WinStatsServer::make_monitor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PStatMonitor *WinStatsServer::
make_monitor() {
  return new WinStatsMonitor(this);
}
