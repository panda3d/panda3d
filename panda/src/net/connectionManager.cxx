// Filename: connectionManager.cxx
// Created by:  jns (07Feb00)
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

#include "connectionManager.h"
#include "connection.h"
#include "connectionReader.h"
#include "connectionWriter.h"
#include "netAddress.h"
#include "pprerror.h"
#include "config_net.h"

#include <prerror.h>

#ifdef WIN32_VC
#include <winsock.h>  // For gethostname()
#endif

////////////////////////////////////////////////////////////////////
//     Function: ConnectionManager::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
ConnectionManager::
ConnectionManager() {
  _set_mutex = PR_NewLock();
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionManager::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
ConnectionManager::
~ConnectionManager() {
  // Notify all of our associated readers and writers that we're gone.
  Readers::iterator ri;
  for (ri = _readers.begin(); ri != _readers.end(); ++ri) {
    (*ri)->clear_manager();
  }
  Writers::iterator wi;
  for (wi = _writers.begin(); wi != _writers.end(); ++wi) {
    (*wi)->clear_manager();
  }

  PR_DestroyLock(_set_mutex);
}


////////////////////////////////////////////////////////////////////
//     Function: ConnectionManager::open_UDP_connection
//       Access: Public
//  Description: Opens a socket for sending and/or receiving UDP
//               packets.  If the port number is negative, it will not
//               be bound to a socket; this is generally a pointless
//               thing to do.  If the port number is zero, a random
//               socket will be chosen.  Otherwise, the specified
//               port number is used.  Normally, you don't care what
//               port a UDP connection is opened on, so you should use
//               the default value of zero.
//
//               Use a ConnectionReader and ConnectionWriter to handle
//               the actual communication.
////////////////////////////////////////////////////////////////////
PT(Connection) ConnectionManager::
open_UDP_connection(int port) {
  NetAddress address;
  address.set_any(port);

  PRFileDesc *socket = PR_NewUDPSocket();
  if (socket == (PRFileDesc *)NULL) {
    pprerror("PR_NewUDPSocket");
    return PT(Connection)();
  }

  if (port >= 0) {
    PRStatus result = PR_Bind(socket, address.get_addr());
    if (result != PR_SUCCESS) {
      pprerror("PR_Bind");
      PR_Close(socket);
      return PT(Connection)();
    }

    net_cat.info()
      << "Creating UDP connection for port " << port << "\n";
  } else {
    net_cat.info()
      << "Creating UDP connection\n";
  }

  PT(Connection) connection = new Connection(this, socket);
  new_connection(connection);
  return connection;
}


////////////////////////////////////////////////////////////////////
//     Function: ConnectionManager::open_TCP_server_rendezvous
//       Access: Public
//  Description: Creates a socket to be used as a rendezvous socket
//               for a server to listen for TCP connections.  The
//               socket returned by this call should only be added to
//               a ConnectionListener (not to a generic
//               ConnectionReader).
//
//               backlog is the maximum length of the queue of pending
//               connections.
////////////////////////////////////////////////////////////////////
PT(Connection) ConnectionManager::
open_TCP_server_rendezvous(int port, int backlog) {
  NetAddress address;
  address.set_any(port);

  PRFileDesc *socket = PR_NewTCPSocket();
  if (socket == (PRFileDesc *)NULL) {
    pprerror("PR_NewTCPSocket");
    return PT(Connection)();
  }

  PRStatus result = PR_Bind(socket, address.get_addr());
  if (result != PR_SUCCESS) {
    pprerror("PR_Bind");
    net_cat.info()
      << "Unable to bind to port " << port << " for TCP.\n";
    PR_Close(socket);
    return PT(Connection)();
  }

  result = PR_Listen(socket, backlog);
  if (result != PR_SUCCESS) {
    pprerror("PR_Listen");
    net_cat.info()
      << "Unable to listen to port " << port << " for TCP.\n";
    PR_Close(socket);
    return PT(Connection)();
  }

  net_cat.info()
    << "Listening for TCP connections on port " << port << "\n";

  PT(Connection) connection = new Connection(this, socket);
  new_connection(connection);
  return connection;
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionManager::open_TCP_client_connection
//       Access: Public
//  Description: Attempts to establish a TCP client connection to a
//               server at the indicated address.  If the connection
//               is not established within timeout_ms milliseconds, a
//               null connection is returned.
////////////////////////////////////////////////////////////////////
PT(Connection) ConnectionManager::
open_TCP_client_connection(const NetAddress &address, int timeout_ms) {
  PRFileDesc *socket = PR_NewTCPSocket();
  if (socket == (PRFileDesc *)NULL) {
    pprerror("PR_NewTCPSocket");
    return PT(Connection)();
  }

  PRStatus result = PR_Connect(socket, address.get_addr(),
                               PR_MillisecondsToInterval(timeout_ms));
  if (result != PR_SUCCESS) {
    if (PR_GetError() != PR_CONNECT_RESET_ERROR) {
      pprerror("PR_Connect");
    }
    net_cat.info()
      << "Unable to open TCP connection to server "
      << address.get_ip_string() << " on port " << address.get_port() << "\n";
    PR_Close(socket);
    return PT(Connection)();
  }

  net_cat.info()
    << "Opened TCP connection to server " << address.get_ip_string() << " "
    << " on port " << address.get_port() << "\n";

  PT(Connection) connection = new Connection(this, socket);
  new_connection(connection);
  return connection;
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionManager::open_TCP_client_connection
//       Access: Public
//  Description: This is a shorthand version of the function to
//               directly establish communcations to a named host and
//               port.
////////////////////////////////////////////////////////////////////
PT(Connection) ConnectionManager::
open_TCP_client_connection(const string &hostname, int port,
                           int timeout_ms) {
  NetAddress address;
  if (!address.set_host(hostname, port)) {
    return PT(Connection)();
  }

  return open_TCP_client_connection(address, timeout_ms);
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionManager::close_connection
//       Access: Public
//  Description: Terminates a UDP or TCP socket previously opened.
//               This also removes it from any associated
//               ConnectionReader or ConnectionListeners.
//
//               The socket itself may not be immediately closed--it
//               will not be closed until all outstanding pointers to
//               it are cleared, including any pointers remaining in
//               NetDatagrams recently received from the socket.
//
//               The return value is true if the connection was marked
//               to be closed, or false if close_connection() had
//               already been called (or the connection did not belong
//               to this ConnectionManager).  In neither case can you
//               infer anything about whether the connection has
//               *actually* been closed yet based on the return value.
////////////////////////////////////////////////////////////////////
bool ConnectionManager::
close_connection(const PT(Connection) &connection) {
  PR_Lock(_set_mutex);
  Connections::iterator ci = _connections.find(connection);
  if (ci == _connections.end()) {
    // Already closed, or not part of this ConnectionManager.
    PR_Unlock(_set_mutex);
    return false;
  }
  _connections.erase(ci);

  Readers::iterator ri;
  for (ri = _readers.begin(); ri != _readers.end(); ++ri) {
    (*ri)->remove_connection(connection);
  }
  PR_Unlock(_set_mutex);

  PRFileDesc *socket = connection->get_socket();

  if (PR_GetDescType(socket) == PR_DESC_SOCKET_TCP) {
    // We can't *actually* close the connection right now, because
    // there might be outstanding pointers to it.  But we can at least
    // shut it down.  It will be eventually closed when all the
    // pointers let go.

    net_cat.info()
      << "Shutting down connection " << (void *)connection
      << " locally.\n";

    PRStatus result = PR_Shutdown(socket, PR_SHUTDOWN_BOTH);
    if (result != PR_SUCCESS) {
      PRErrorCode errcode = PR_GetError();
      if (errcode != PR_NOT_CONNECTED_ERROR) {
        pprerror("PR_Shutdown");
      }
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionManager::get_host_name
//       Access: Public, Static
//  Description: Returns the name of this particular machine on the
//               network, if available, or the empty string if the
//               hostname cannot be determined.
////////////////////////////////////////////////////////////////////
string ConnectionManager::
get_host_name() {
  char temp_buff[1024];
  if (gethostname(temp_buff, 1024) == 0) {
    return string(temp_buff);
  }

  return string();
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionManager::new_connection
//       Access: Protected
//  Description: This internal function is called whenever a new
//               connection is established.  It allows the
//               ConnectionManager to save all of the pointers to open
//               connections so they can't be inadvertently deleted
//               until close_connection() is called.
////////////////////////////////////////////////////////////////////
void ConnectionManager::
new_connection(const PT(Connection) &connection) {
  PR_Lock(_set_mutex);
  _connections.insert(connection);
  PR_Unlock(_set_mutex);
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionManager::connection_reset
//       Access: Protected, Virtual
//  Description: An internal function called by the ConnectionReader,
//               ConnectionWriter, or ConnectionListener when a
//               connection has been externally reset.  This adds the
//               connection to the queue of those which have recently
//               been reset.
////////////////////////////////////////////////////////////////////
void ConnectionManager::
connection_reset(const PT(Connection) &connection, PRErrorCode errcode) {
  if (net_cat.is_info()) {
    if (errcode == 0) {
      net_cat.info()
        << "Connection " << (void *)connection
        << " was closed normally by the other end.\n";

    } else {
      net_cat.info()
        << "Lost connection " << (void *)connection
        << " unexpectedly: ";

      switch (errcode) {
      case PR_CONNECT_RESET_ERROR:
        net_cat.info(false)
          << "connection reset\n";
        break;
        
#ifdef PR_SOCKET_SHUTDOWN_ERROR
      case PR_SOCKET_SHUTDOWN_ERROR:
        net_cat.info(false)
          << "socket shutdown\n";
        break;
        
      case PR_CONNECT_ABORTED_ERROR:
        net_cat.info(false)
          << "connection aborted\n";
        break;
#endif

      default:
        net_cat.info(false)
          << "NSPR error code " << errcode << "\n";
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionManager::add_reader
//       Access: Protected
//  Description: This internal function is called by ConnectionReader
//               when it is constructed.
////////////////////////////////////////////////////////////////////
void ConnectionManager::
add_reader(ConnectionReader *reader) {
  PR_Lock(_set_mutex);
  _readers.insert(reader);
  PR_Unlock(_set_mutex);
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionManager::remove_reader
//       Access: Protected
//  Description: This internal function is called by ConnectionReader
//               when it is destructed.
////////////////////////////////////////////////////////////////////
void ConnectionManager::
remove_reader(ConnectionReader *reader) {
  PR_Lock(_set_mutex);
  _readers.erase(reader);
  PR_Unlock(_set_mutex);
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionManager::add_writer
//       Access: Protected
//  Description: This internal function is called by ConnectionWriter
//               when it is constructed.
////////////////////////////////////////////////////////////////////
void ConnectionManager::
add_writer(ConnectionWriter *writer) {
  PR_Lock(_set_mutex);
  _writers.insert(writer);
  PR_Unlock(_set_mutex);
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionManager::remove_writer
//       Access: Protected
//  Description: This internal function is called by ConnectionWriter
//               when it is destructed.
////////////////////////////////////////////////////////////////////
void ConnectionManager::
remove_writer(ConnectionWriter *writer) {
  PR_Lock(_set_mutex);
  _writers.erase(writer);
  PR_Unlock(_set_mutex);
}
