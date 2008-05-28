// Filename: gtkStatsServer.cxx
// Created by:  drose (16Jan06)
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

#include "gtkStatsServer.h"
#include "gtkStatsMonitor.h"

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsServer::make_monitor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PStatMonitor *GtkStatsServer::
make_monitor() {
  return new GtkStatsMonitor(this);
}
