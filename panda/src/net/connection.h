// Filename: connection.h
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
//       Class : Connection
// Description : Represents a single TCP or UDP socket for input or
//               output.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA Connection : public ReferenceCount {
PUBLISHED:
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
