// Filename: pStatServer.h
// Created by:  drose (09Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PSTATSERVER_H
#define PSTATSERVER_H

#include <pandatoolbase.h>

#include "pStatListener.h"

#include <connectionManager.h>

#include <map>
#include <deque>

class PStatReader;

////////////////////////////////////////////////////////////////////
// 	 Class : PStatServer
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

private:
  virtual void connection_reset(const PT(Connection) &connection);

  PStatListener *_listener;

  typedef map<PT(Connection), PStatReader *> Readers;
  Readers _readers;
  typedef vector<PStatReader *> LostReaders;
  LostReaders _lost_readers;

  typedef deque<int> Ports;
  Ports _available_udp_ports;
  int _next_udp_port;
};

#endif
