// Filename: pStatListener.h
// Created by:  drose (09Jul00)
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

#ifndef PSTATLISTENER_H
#define PSTATLISTENER_H

#include "pandatoolbase.h"

#include <connectionListener.h>
#include "referenceCount.h"

class PStatServer;
class PStatMonitor;

////////////////////////////////////////////////////////////////////
//       Class : PStatListener
// Description : This is the TCP rendezvous socket listener.  We need
//               one of these to listen for new connections on the
//               socket(s) added to the PStatServer.
////////////////////////////////////////////////////////////////////
class PStatListener : public ConnectionListener {
public:
  PStatListener(PStatServer *manager);

protected:
  virtual void connection_opened(const PT(Connection) &rendezvous,
                                 const NetAddress &address,
                                 const PT(Connection) &new_connection);

private:
  PStatServer *_manager;
};

#endif
