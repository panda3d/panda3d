// Filename: connectionListener.h
// Created by:  drose (09Feb00)
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

#ifndef CONNECTIONLISTENER_H
#define CONNECTIONLISTENER_H

#include "pandabase.h"

#include "connectionReader.h"

class NetAddress;

////////////////////////////////////////////////////////////////////
//       Class : ConnectionListener
// Description : This is a special kind of ConnectionReader that waits
//               for activity on a rendezvous port and accepts a TCP
//               connection (instead of attempting to read a datagram
//               from the rendezvous port).
//
//               It is itself an abstract class, as it doesn't define
//               what to do with the established connection.  See
//               QueuedConnectionListener.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ConnectionListener : public ConnectionReader {
PUBLISHED:
  ConnectionListener(ConnectionManager *manager, int num_threads);

protected:
  virtual void receive_datagram(const NetDatagram &datagram);
  virtual void connection_opened(const PT(Connection) &rendezvous,
                                 const NetAddress &address,
                                 const PT(Connection) &new_connection)=0;

  virtual void process_incoming_data(SocketInfo *sinfo);

private:
};

#endif
