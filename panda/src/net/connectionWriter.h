// Filename: connectionWriter.h
// Created by:  drose (08Feb00)
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

#ifndef CONNECTIONWRITER_H
#define CONNECTIONWRITER_H

#include "pandabase.h"

#include "datagramQueue.h"
#include "connection.h"

#include "pointerTo.h"

#include <prthread.h>
#include "pvector.h"

class ConnectionManager;
class NetAddress;

////////////////////////////////////////////////////////////////////
//       Class : ConnectionWriter
// Description : This class handles threaded delivery of datagrams to
//               various TCP or UDP sockets.
//
//               A ConnectionWriter may define an arbitrary number of
//               threads (at least one) to write its datagrams to
//               sockets.  The number of threads is specified at
//               construction time and cannot be changed.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ConnectionWriter {
PUBLISHED:
  ConnectionWriter(ConnectionManager *manager, int num_threads);
  ~ConnectionWriter();

  bool send(const Datagram &datagram,
            const PT(Connection) &connection);

  bool send(const Datagram &datagram,
            const PT(Connection) &connection,
            const NetAddress &address);

  bool is_valid_for_udp(const Datagram &datagram) const;

  ConnectionManager *get_manager() const;
  bool is_immediate() const;
  int get_num_threads() const;

  void set_raw_mode(bool mode);
  bool get_raw_mode() const;

protected:
  void clear_manager();

private:
  static void thread_start(void *data);
  void thread_run();
  bool send_datagram(const NetDatagram &datagram);

protected:
  ConnectionManager *_manager;

private:
  bool _raw_mode;
  DatagramQueue _queue;

  typedef pvector<PRThread *> Threads;
  Threads _threads;
  bool _immediate;

friend class ConnectionManager;
};

#endif


