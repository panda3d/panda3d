// Filename: connectionListener.cxx
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

#include "connectionListener.h"
#include "connection.h"
#include "connectionManager.h"
#include "netAddress.h"
#include "pprerror.h"
#include "config_net.h"

////////////////////////////////////////////////////////////////////
//     Function: ConnectionListener::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
ConnectionListener::
ConnectionListener(ConnectionManager *manager, int num_threads) :
  ConnectionReader(manager, num_threads)
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
void ConnectionListener::
process_incoming_data(SocketInfo *sinfo) {
  PRNetAddr addr;

  PRFileDesc *socket =
    PR_Accept(sinfo->get_socket(), &addr, PR_INTERVAL_NO_TIMEOUT);

  if (socket == (PRFileDesc *)NULL) {
    pprerror("PR_Accept");

  } else {
    NetAddress net_addr(addr);
    net_cat.info()
      << "Received TCP connection from client " << net_addr.get_ip_string()
      << " on port " << sinfo->_connection->get_address().get_port()
      << "\n";

    PT(Connection) new_connection = new Connection(_manager, socket);
    if (_manager != (ConnectionManager *)NULL) {
      _manager->new_connection(new_connection);
    }
    connection_opened(sinfo->_connection, net_addr, new_connection);
  }

  finish_socket(sinfo);
}
