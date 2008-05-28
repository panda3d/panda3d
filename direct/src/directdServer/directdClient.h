// Filename: directdClient.h
// Created by:  skyler 2002.04.08
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

#include "directd.h"

// Description: DirectDClient is a test app for DriectDServer.
class DirectDClient: public DirectD {
public:
  DirectDClient();
  ~DirectDClient();

  void run_client(const string& host, int port);

protected:
  void cli_command(const string& cmd);
};

