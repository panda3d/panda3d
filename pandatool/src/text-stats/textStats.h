// Filename: textStats.h
// Created by:  drose (12Jul00)
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

#ifndef TEXTSTATS_H
#define TEXTSTATS_H

#include "pandatoolbase.h"

#include "programBase.h"
#include "pStatServer.h"

////////////////////////////////////////////////////////////////////
//       Class : TextStats
// Description : A simple, scrolling-text stats server.  Guaranteed to
//               compile on every platform.
////////////////////////////////////////////////////////////////////
class TextStats : public ProgramBase, public PStatServer {
public:
  TextStats();

  virtual PStatMonitor *make_monitor();

  void run();

  int _port;
};

#endif

