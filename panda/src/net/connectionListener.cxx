// Filename: connectionListener.cxx
// Created by:  drose (09Feb00)
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

#include "dcast.h"
#include "connectionListener.h"
#include "connection.h"
#include "connectionManager.h"
#include "netAddress.h"
#include "config_net.h"
#include "socket_tcp_listen.h"

static string 
listener_thread_name(const string &thread_name) {
  if (!thread_name.empty()) {
    return thread_name;
  }
  return "ListenerThread";
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionListener::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
ConnectionListener::
ConnectionListener(ConnectionManager *manager, int num_threads,
                   const string &thread_name) :
  ConnectionReader(manager, num_threads, listener_thread_name(thread_name))
{
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionListener::receive_datagram
//       Access: Protected, Virtual
//  Description: This function must be declared because it is pure
//               virtual in the base class, but it isn't used in this
//               class and doesn't do anything.
////////////////////////////////////////////////////////////////////
void ConnectionListener::
receive_datagram(const NetDatagram &) {
  net_cat.error()
    << "ConnectionListener::receive_datagram called.\n";
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionListener::process_incoming_data
//       Access: Protected, Virtual
//  Description: This is the function that is called when activity is
//               detected on a rendezvous port.  In this case, it
//               performs the accept().
////////////////////////////////////////////////////////////////////
bool ConnectionListener::
process_incoming_data(SocketInfo *sinfo) {
  Socket_TCP_Listen *socket;
  DCAST_INTO_R(socket, sinfo->get_socket(), false);

  Socket_Address addr;
  Socket_TCP *session = new Socket_TCP;

  bool got_connection = socket->GetIncomingConnection(*session, addr);
#if defined(HAVE_THREADS) && defined(SIMPLE_THREADS)
  while (!got_connection && socket->GetLastError() == LOCAL_BLOCKING_ERROR) {
    Thread::force_yield();
    got_connection = socket->GetIncomingConnection(*session, addr);
  }
#endif  // SIMPLE_THREADS

  if (!got_connection) {
    net_cat.error()
      << "Error when accepting new connection.\n";
    delete session;
    finish_socket(sinfo);
    return false;
  }

  NetAddress net_addr(addr);
  net_cat.info()
    << "Received TCP connection from client " << net_addr.get_ip_string()
    << " on port " << sinfo->_connection->get_address().get_port()
    << "\n";
  
  PT(Connection) new_connection = new Connection(_manager, session);
  if (_manager != (ConnectionManager *)NULL) {
    _manager->new_connection(new_connection);
  }
  connection_opened(sinfo->_connection, net_addr, new_connection);

  finish_socket(sinfo);
  return true;
}
