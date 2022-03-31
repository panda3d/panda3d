/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatServer.h
 * @author drose
 * @date 2000-07-09
 */

#ifndef PSTATSERVER_H
#define PSTATSERVER_H

#include "pandatoolbase.h"
#include "pStatListener.h"
#include "connectionManager.h"
#include "vector_stdfloat.h"
#include "pmap.h"
#include "pdeque.h"

class PStatReader;

/**
 * The overall manager of the network connections.  This class gets the ball
 * rolling; to use this package, you need to derive from this and define
 * make_monitor() to allocate and return a PStatMonitor of the suitable type.
 *
 * Then create just one PStatServer object and call listen() with the port(s)
 * you would like to listen on.  It will automatically create PStatMonitors as
 * connections are established and mark the connections closed as they are
 * lost.
 */
class PStatServer : public ConnectionManager {
public:
  PStatServer();
  ~PStatServer();

  bool listen(int port = -1);

  void poll();
  void main_loop(bool *interrupt_flag = nullptr);

  virtual PStatMonitor *make_monitor()=0;
  void add_reader(Connection *connection, PStatReader *reader);
  void remove_reader(Connection *connection, PStatReader *reader);

  int get_udp_port();
  void release_udp_port(int port);

  int get_num_user_guide_bars() const;
  double get_user_guide_bar_height(int n) const;
  void move_user_guide_bar(int n, double height);
  int add_user_guide_bar(double height);
  void remove_user_guide_bar(int n);
  int find_user_guide_bar(double from_height, double to_height) const;

  virtual bool is_thread_safe();

protected:
  virtual void connection_reset(const PT(Connection) &connection,
                                bool okflag);

private:
  void user_guide_bars_changed();

  PStatListener *_listener;

  typedef pmap<PT(Connection), PStatReader *> Readers;
  Readers _readers;
  typedef pvector<PStatReader *> LostReaders;
  LostReaders _lost_readers;
  LostReaders _removed_readers;

  typedef pdeque<int> Ports;
  Ports _available_udp_ports;
  int _next_udp_port;

  typedef vector_stdfloat GuideBars;
  GuideBars _user_guide_bars;
};

#endif
