// Filename: directdServer.h
// Created by:  skyler 2002.04.08
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

#include "queuedConnectionReader.h"
#include "directd.h"

// Description: Start a directdServer on each of the machines you
//              which to start panda on.
//              
//              Start a directdClient on the controlling machine
//              or import ShowBaseGlobal with the xxxxx flag in
//              your Configrc.  The client will connact each of
//              the servers in the xxxxx list in your Configrc.
class DirectDServer: public DirectD {
public:
  DirectDServer();
  ~DirectDServer();
  
  void run_server(int port);

protected:
  void read_command(string& cmd);
  virtual void handle_command(const string& cmd);
};

