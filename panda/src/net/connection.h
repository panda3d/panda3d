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

  // Socket options.
  void set_nonblock(bool flag);
  void set_linger(bool flag, double time);
  void set_reuse_addr(bool flag);
  void set_keep_alive(bool flag);
  void set_recv_buffer_size(int size);
  void set_send_buffer_size(int size);
  void set_ip_time_to_live(int ttl);
  void set_ip_type_of_service(int tos);
  void set_no_delay(bool flag);
  void set_max_segment(int size);

private:
  bool send_datagram(const NetDatagram &datagram);
  bool send_raw_datagram(const NetDatagram &datagram);

  ConnectionManager *_manager;
  PRFileDesc *_socket;
  PRLock *_write_mutex;

friend class ConnectionWriter;
};

#endif
