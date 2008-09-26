// Filename: textStats.h
// Created by:  drose (12Jul00)
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

#ifndef TEXTSTATS_H
#define TEXTSTATS_H

#include "pandatoolbase.h"

#include "programBase.h"
#include "pStatServer.h"

#include <iostream>
#include <fstream>

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

private:  
  int _port;
  bool _show_raw_data;
  
  //[PECI]
  bool _got_outputFileName;
  string _outputFileName;
  ostream *_outFile;
};

#endif

