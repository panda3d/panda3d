// Filename: connectionManager.h
// Created by:  jns (07Feb00)
//
////////////////////////////////////////////////////////////////////

#ifndef CONNECTIONMANAGER_H
#define CONNECTIONMANAGER_H

#include <pandabase.h>

#include "netDatagram.h"
#include "connection.h"

#include <pointerTo.h>

#include <prlock.h>
#include <set>

class NetAddress;
class ConnectionReader;
class ConnectionWriter;

////////////////////////////////////////////////////////////////////
// 	 Class : ConnectionManager
// Description : The primary interface to the low-level networking
//               layer in this package.  A ConnectionManager is used
//               to establish and destroy TCP and UDP connections.
//               Communication on these connections, once established,
//               is handled via ConnectionReader, ConnectionWriter,
//               and ConnectionListener.
//
//               This is actually an abstract class, since it does not
//               define what to do when a connection is externally
//               reset (i.e. closed on the other end, or dropped
//               because of network errors).  See
//               QueuedConnectionManager.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ConnectionManager {
public:
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
  virtual void connection_reset(const PT(Connection) &connection)=0;
 
  void add_reader(ConnectionReader *reader);
  void remove_reader(ConnectionReader *reader);
  void add_writer(ConnectionWriter *writer);
  void remove_writer(ConnectionWriter *writer);

  typedef set<PT(Connection)> Connections;
  typedef set<ConnectionReader *> Readers;
  typedef set<ConnectionWriter *> Writers;
  Connections _connections;
  Readers _readers;
  Writers _writers;
  PRLock *_set_mutex;

private:
  friend class ConnectionReader;
  friend class ConnectionWriter;
  friend class ConnectionListener;
  friend class Connection;
};

#endif
