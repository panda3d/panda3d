// Filename: connection.h
// Created by:  jns (07Feb00)
//
////////////////////////////////////////////////////////////////////

#ifndef CONNECTION_H
#define CONNECTION_H

#include <pandabase.h>

#include <referenceCount.h>

#include "netAddress.h"

#include <prio.h>
#include <prlock.h>

class ConnectionManager;
class NetDatagram;

////////////////////////////////////////////////////////////////////
// 	 Class : Connection
// Description : Represents a single TCP or UDP socket for input or
//               output.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA Connection : public ReferenceCount {
public:
  Connection(ConnectionManager *manager, PRFileDesc *socket);
  ~Connection();

  NetAddress get_address() const;
  ConnectionManager *get_manager() const;

  PRFileDesc *get_socket() const;

private:
  bool send_datagram(const NetDatagram &datagram);

  ConnectionManager *_manager;
  PRFileDesc *_socket;
  PRLock *_write_mutex;

friend class ConnectionWriter;
};

#endif
