// Filename: pStatServer.h
// Created by:  drose (09Jul00)
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

#ifndef PSTATSERVER_H
#define PSTATSERVER_H

#include "pandatoolbase.h"

#include "pStatListener.h"

#include "connectionManager.h"

#include "pmap.h"
#include "pdeque.h"

class PStatReader;

////////////////////////////////////////////////////////////////////
//       Class : PStatServer
// Description : The overall manager of the network connections.  This
//               class gets the ball rolling; to use this package, you
//               need to derive from this and define make_monitor() to
//               allocate and return a PStatMonitor of the suitable
//               type.
//
//               Then create just one PStatServer object and call
//               listen() with the port(s) you would like to listen
//               on.  It will automatically create PStatMonitors as
//               connections are established and mark the connections
//               closed as they are lost.
////////////////////////////////////////////////////////////////////
class PStatServer : public ConnectionManager {
public:
  PStatServer();
  ~PStatServer();

  bool listen(int port = -1);

  void poll();
  void main_loop(bool *interrupt_flag = NULL);

  virtual PStatMonitor *make_monitor()=0;
  void add_reader(Connection *connection, PStatReader *reader);
  void remove_reader(Connection *connection, PStatReader *reader);

  int get_udp_port();
  void release_udp_port(int port);

  virtual bool is_thread_safe();

protected:
  virtual void connection_reset(const PT(Connection) &connection, 
                                PRErrorCode errcode);

private:
  PStatListener *_listener;

  typedef pmap<PT(Connection), PStatReader *> Readers;
  Readers _readers;
  typedef pvector<PStatReader *> LostReaders;
  LostReaders _lost_readers;
  LostReaders _removed_readers;

  typedef pdeque<int> Ports;
  Ports _available_udp_ports;
  int _next_udp_port;
};

#endif
