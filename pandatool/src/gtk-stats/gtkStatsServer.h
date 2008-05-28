// Filename: gtkStatsServer.h
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

#ifndef GTKSTATSSERVER_H
#define GTKSTATSSERVER_H

#include "pandatoolbase.h"
#include "pStatServer.h"

////////////////////////////////////////////////////////////////////
//       Class : GtkStatsServer
// Description : The class that owns the main loop, waiting for client
//               connections.
////////////////////////////////////////////////////////////////////
class GtkStatsServer : public PStatServer {
public:
  virtual PStatMonitor *make_monitor();
};

#endif

