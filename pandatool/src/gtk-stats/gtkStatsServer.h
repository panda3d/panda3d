// Filename: gtkStatsServer.h
// Created by:  drose (14Jul00)
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

#ifndef GTKSTATSSERVER_H
#define GTKSTATSSERVER_H

#include "pandatoolbase.h"

#include "pStatServer.h"


////////////////////////////////////////////////////////////////////
//       Class : GtkStatsServer
// Description :
////////////////////////////////////////////////////////////////////
class GtkStatsServer : public PStatServer {
public:
  virtual PStatMonitor *make_monitor();
};

#endif
