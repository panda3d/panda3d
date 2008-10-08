// Filename: connectionManager.h
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

#ifndef CONNECTIONMANAGER_H
#define CONNECTIONMANAGER_H

#include "pandabase.h"

#include "netDatagram.h"
#include "connection.h"
#include "pointerTo.h"
#include "pset.h"
#include "lightMutex.h"

class NetAddress;
class ConnectionReader;
class ConnectionWriter;

////////////////////////////////////////////////////////////////////
//       Class : ConnectionManager
// Description : The primary interface to the low-level networking
//               layer in this package.  A ConnectionManager is used
//               to establish and destroy TCP and UDP connections.
//               Communication on these connections, once established,
//               is handled via ConnectionReader, ConnectionWriter,
//               and ConnectionListener.
//
//               You may use this class directly if you don't care
//               about tracking which connections have been
//               unexpectedly closed; otherwise, you should use
//               QueuedConnectionManager to get reports about these
//               events (or derive your own class to handle these
//               events properly).
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_NET ConnectionManager {
PUBLISHED:
  ConnectionManager();
  virtual ~ConnectionManager();

  PT(Connection) open_UDP_connection(int port = 0);

  PT(Connection) open_TCP_server_rendezvous(int port, int backlog);
  PT(Connection) open_TCP_client_connection(const NetAddress &address,
                                            int timeout_ms);
  PT(Connection) open_TCP_client_connection(const string &hostname, int port,
                                            int timeout_ms);

  bool close_connection(const PT(Connection) &connection);

  static string get_host_name();

protected:
  void new_connection(const PT(Connection) &connection);
  virtual void connection_reset(const PT(Connection) &connection, 
                                bool okflag);

  void add_reader(ConnectionReader *reader);
  void remove_reader(ConnectionReader *reader);
  void add_writer(ConnectionWriter *writer);
  void remove_writer(ConnectionWriter *writer);

  typedef phash_set< PT(Connection) > Connections;
  typedef phash_set<ConnectionReader *, pointer_hash> Readers;
  typedef phash_set<ConnectionWriter *, pointer_hash> Writers;
  Connections _connections;
  Readers _readers;
  Writers _writers;
  LightMutex _set_mutex;

private:
  friend class ConnectionReader;
  friend class ConnectionWriter;
  friend class ConnectionListener;
  friend class Connection;
};

#endif
