/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file connectionListener.h
 * @author drose
 * @date 2000-02-09
 */

#ifndef CONNECTIONLISTENER_H
#define CONNECTIONLISTENER_H

#include "pandabase.h"

#include "connectionReader.h"

class NetAddress;

/**
 * This is a special kind of ConnectionReader that waits for activity on a
 * rendezvous port and accepts a TCP connection (instead of attempting to read
 * a datagram from the rendezvous port).
 *
 * It is itself an abstract class, as it doesn't define what to do with the
 * established connection.  See QueuedConnectionListener.
 */
class EXPCL_PANDA_NET ConnectionListener : public ConnectionReader {
PUBLISHED:
  ConnectionListener(ConnectionManager *manager, int num_threads,
                     const std::string &thread_name = std::string());

protected:
  virtual void receive_datagram(const NetDatagram &datagram);
  virtual void connection_opened(const PT(Connection) &rendezvous,
                                 const NetAddress &address,
                                 const PT(Connection) &new_connection)=0;

  virtual bool process_incoming_data(SocketInfo *sinfo);

private:
};

#endif
