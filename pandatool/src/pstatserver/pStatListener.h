// Filename: pStatListener.h
// Created by:  drose (09Jul00)
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

#ifndef PSTATLISTENER_H
#define PSTATLISTENER_H

#include "pandatoolbase.h"

#include "connectionListener.h"
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
