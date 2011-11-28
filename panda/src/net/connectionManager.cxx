// Filename: connectionManager.cxx
// Created by:  jns (07Feb00)
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

#include "connectionManager.h"
#include "connection.h"
#include "connectionReader.h"
#include "connectionWriter.h"
#include "netAddress.h"
#include "config_net.h"
#include "lightMutexHolder.h"
#include "trueClock.h"

#if defined(WIN32_VC) || defined(WIN64_VC)
#include <winsock2.h>  // For gethostname()
#endif

////////////////////////////////////////////////////////////////////
//     Function: ConnectionManager::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
ConnectionManager::
ConnectionManager() : _set_mutex("ConnectionManager::_set_mutex") 
{
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
}


////////////////////////////////////////////////////////////////////
//     Function: ConnectionManager::open_UDP_connection
//       Access: Public
//  Description: Opens a socket for sending and/or receiving UDP
//               packets.  If the port number is greater than zero,
//               the UDP connection will be opened for listening on
//               the indicated port; otherwise, it will be useful only
//               for sending.
//
//               Use a ConnectionReader and ConnectionWriter to handle
//               the actual communication.
////////////////////////////////////////////////////////////////////
PT(Connection) ConnectionManager::
open_UDP_connection(int port) {
  Socket_UDP *socket = new Socket_UDP;

  if (port > 0) {
    NetAddress address;
    address.set_any(port);
    
    if (!socket->OpenForInput(address.get_addr())) {
      net_cat.error()
        << "Unable to bind to port " << port << " for UDP.\n";
      delete socket;
      return PT(Connection)();
    }

    net_cat.info()
      << "Creating UDP connection for port " << port << "\n";

  } else {
    if (!socket->InitNoAddress()) {
      net_cat.error()
        << "Unable to initialize outgoing UDP.\n";
      delete socket;
      return PT(Connection)();
    }

    net_cat.info()
      << "Creating outgoing UDP connection\n";
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
//               This variant of this method accepts a single port,
//               and will listen to that port on all available
//               interfaces.
//
//               backlog is the maximum length of the queue of pending
//               connections.
////////////////////////////////////////////////////////////////////
PT(Connection) ConnectionManager::
open_TCP_server_rendezvous(int port, int backlog) {
  NetAddress address;
  address.set_any(port);
  return open_TCP_server_rendezvous(address, backlog);
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
//               This variant of this method accepts a "hostname",
//               which is usually just an IP address in dotted
//               notation, and a port number.  It will listen on the
//               interface indicated by the IP address.  If the IP
//               address is empty string, it will listen on all
//               interfaces.
//
//               backlog is the maximum length of the queue of pending
//               connections.
////////////////////////////////////////////////////////////////////
PT(Connection) ConnectionManager::
open_TCP_server_rendezvous(const string &hostname, int port, int backlog) {
  NetAddress address;
  if (hostname.empty()) {
    address.set_any(port);
  } else {
    address.set_host(hostname, port);
  }
  return open_TCP_server_rendezvous(address, backlog);
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
//               This variant of this method accepts a NetAddress,
//               which allows you to specify a specific interface to
//               listen to.
//
//               backlog is the maximum length of the queue of pending
//               connections.
////////////////////////////////////////////////////////////////////
PT(Connection) ConnectionManager::
open_TCP_server_rendezvous(const NetAddress &address, int backlog) {
  ostringstream strm;
  if (address.get_ip() == 0) {
    strm << "port " << address.get_port();  
  } else {
    strm << address.get_ip_string() << ":" << address.get_port();
  }

  Socket_TCP_Listen *socket = new Socket_TCP_Listen;
  bool okflag = socket->OpenForListen(address.get_addr(), backlog);
  if (!okflag) {
    net_cat.info()
      << "Unable to listen to " << strm.str() << " for TCP.\n";
    delete socket;
    return PT(Connection)();
  }

  net_cat.info()
    << "Listening for TCP connections on " << strm.str() << "\n";

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
  Socket_TCP *socket = new Socket_TCP;

  // We always open the connection with non-blocking mode first, so we
  // can implement the timeout.
  bool okflag = socket->ActiveOpenNonBlocking(address.get_addr());
  if (okflag && socket->GetLastError() == LOCAL_CONNECT_BLOCKING) {
    // Now wait for the socket to connect.
    TrueClock *clock = TrueClock::get_global_ptr();
    double start = clock->get_short_time();
    Thread::force_yield();
    Socket_fdset fset;
    fset.setForSocket(*socket);
    int ready = fset.WaitForWrite(true, 0);
    while (ready == 0) {
      double elapsed = clock->get_short_time() - start;
      if (elapsed * 1000.0 > timeout_ms) {
        // Timeout.
        okflag = false;
        break;
      }
      Thread::force_yield();
      fset.setForSocket(*socket);
      ready = fset.WaitForWrite(true, 0);
    }
  }

  if (okflag) {
    // So, the connect() operation finished, but did it succeed or fail?
    if (socket->GetPeerName().GetIPAddressRaw() == 0) {
      // No peer means it failed.
      okflag = false;
    }
  }

  if (!okflag) {
    net_cat.error()
      << "Unable to open TCP connection to server "
      << address.get_ip_string() << " on port " << address.get_port() << "\n";
    delete socket;
    return PT(Connection)();
  }

#if !defined(HAVE_THREADS) || !defined(SIMPLE_THREADS)
  // Now we have opened the socket in nonblocking mode.  Unless we're
  // using SIMPLE_THREADS, though, we really want the socket in
  // blocking mode (since that's what we support here).  Change it.
  socket->SetBlocking();

#endif  // SIMPLE_THREADS

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
//               directly establish communications to a named host and
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
  if (connection != (Connection *)NULL) {
    connection->flush();
  }

  {
    LightMutexHolder holder(_set_mutex);
    Connections::iterator ci = _connections.find(connection);
    if (ci == _connections.end()) {
      // Already closed, or not part of this ConnectionManager.
      return false;
    }
    _connections.erase(ci);
    
    Readers::iterator ri;
    for (ri = _readers.begin(); ri != _readers.end(); ++ri) {
      (*ri)->remove_connection(connection);
    }
  }

  Socket_IP *socket = connection->get_socket();

  // We can't *actually* close the connection right now, because
  // there might be outstanding pointers to it.  But we can at least
  // shut it down.  It will be eventually closed when all the
  // pointers let go.
  
  net_cat.info()
    << "Shutting down connection " << (void *)connection
    << " locally.\n";
  socket->Close();

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
  LightMutexHolder holder(_set_mutex);
  _connections.insert(connection);
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionManager::flush_read_connection
//       Access: Protected, Virtual
//  Description: An internal function called by ConnectionWriter only
//               when a write failure has occurred.  This method
//               ensures that all of the read data has been flushed
//               from the pipe before the connection is fully removed.
////////////////////////////////////////////////////////////////////
void ConnectionManager::
flush_read_connection(Connection *connection) {
  Readers readers;
  {
    LightMutexHolder holder(_set_mutex);
    Connections::iterator ci = _connections.find(connection);
    if (ci == _connections.end()) {
      // Already closed, or not part of this ConnectionManager.
      return;
    }
    _connections.erase(ci);

    // Get a copy first, so we can release the lock before traversing.
    readers = _readers;
  }
  Readers::iterator ri;
  for (ri = readers.begin(); ri != readers.end(); ++ri) {
    (*ri)->flush_read_connection(connection);
  }

  Socket_IP *socket = connection->get_socket();
  socket->Close();
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
connection_reset(const PT(Connection) &connection, bool okflag) {
  if (net_cat.is_info()) {
    if (okflag) {
      net_cat.info()
        << "Connection " << (void *)connection
        << " was closed normally by the other end";

    } else {
      net_cat.info()
        << "Lost connection " << (void *)connection
        << " unexpectedly\n";
    }
  }

  // Turns out we do need to explicitly mark the connection as closed
  // immediately, rather than waiting for the user to do it, since
  // otherwise we'll keep trying to listen for noise on the socket and
  // we'll always hear a "yes" answer.
  close_connection(connection);
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionManager::add_reader
//       Access: Protected
//  Description: This internal function is called by ConnectionReader
//               when it is constructed.
////////////////////////////////////////////////////////////////////
void ConnectionManager::
add_reader(ConnectionReader *reader) {
  LightMutexHolder holder(_set_mutex);
  _readers.insert(reader);
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionManager::remove_reader
//       Access: Protected
//  Description: This internal function is called by ConnectionReader
//               when it is destructed.
////////////////////////////////////////////////////////////////////
void ConnectionManager::
remove_reader(ConnectionReader *reader) {
  LightMutexHolder holder(_set_mutex);
  _readers.erase(reader);
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionManager::add_writer
//       Access: Protected
//  Description: This internal function is called by ConnectionWriter
//               when it is constructed.
////////////////////////////////////////////////////////////////////
void ConnectionManager::
add_writer(ConnectionWriter *writer) {
  LightMutexHolder holder(_set_mutex);
  _writers.insert(writer);
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionManager::remove_writer
//       Access: Protected
//  Description: This internal function is called by ConnectionWriter
//               when it is destructed.
////////////////////////////////////////////////////////////////////
void ConnectionManager::
remove_writer(ConnectionWriter *writer) {
  LightMutexHolder holder(_set_mutex);
  _writers.erase(writer);
}
