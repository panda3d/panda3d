// Filename: connectionListener.h
// Created by:  drose (09Feb00)
//
////////////////////////////////////////////////////////////////////

#ifndef CONNECTIONLISTENER_H
#define CONNECTIONLISTENER_H

#include <pandabase.h>

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
