// Filename: winStatsServer.h
// Created by:  drose (02Dec03)
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

#ifndef WINSTATSSERVER_H
#define WINSTATSSERVER_H

#include "pandatoolbase.h"
#include "pStatServer.h"

////////////////////////////////////////////////////////////////////
//       Class : WinStatsServer
// Description : The class that owns the main loop, waiting for client
//               connections.
////////////////////////////////////////////////////////////////////
class WinStatsServer : public PStatServer {
public:
  virtual PStatMonitor *make_monitor();
};

#endif

