// Filename: directd.h
// Created by:  skyler 2002.04.08
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





// This is a work in progress, if  you have any questions, ask skyler.






#include <process.h>
#include <Windows.h>
#include "pandabase.h"
#include "directsymbols.h"
#include "queuedConnectionManager.h"
#include "queuedConnectionReader.h"
#include "connectionWriter.h"
#include "queuedConnectionListener.h"

#ifdef CPPPARSER //[
// hack for interrogate
typedef int intptr_t;
#endif //]


/*
    DirectD is a client/server app for starting panda/direct.
    
    Usage:
    DirectD is both the client and the server, what it does depends on
    which command line argumenta are given at startup.
    
    Start a directd server on each of the machines you which to start
    panda on.
    
    Start a directd client on the controlling machine or import
    ShowBaseGlobal with the xxxxx flag in your Configrc.  The client 
    will connact each of the servers in the xxxxx list in your Configrc.
*/
class EXPCL_DIRECT DirectD {
PUBLISHED:
  DirectD();
  ~DirectD();
  
  // Description: Call connect_to from client for each server.
  void connect_to(const string& server_host, int port);
  
  // Description: 
  void disconnect_from(const string& server_host, int port);
  
  // Description: Call listen_to in the server.
  //              port is a rendezvous port.
  //              
  //              backlog refers to how many connections can queue up
  //              before you handle them.  Consider setting backlog to
  //              the count you send to wait_for_servers(); or higher.
  void listen_to(int port, int backlog=8);
  
  // Description: process command string.
  void send_command(const string& cmd);

  // Description: Call this function from the client when
  //              import ShowbaseGlobal is nearly finished.
  int client_is_ready(const string& client_host, int port);

  // Description: Call this function from the client after
  //              calling <count> client_is_ready() calls.
  //              
  //              Call listen_to(port) prior to calling
  //              wait_for_servers() (or better yet, prior
  //              to calling client_is_ready()).
  bool wait_for_servers(int count, int timeout_ms);

  // Description: Call this function from the server when
  //              import ShowbaseGlobal is nearly finished.
  int server_is_ready(const string& client_host, int port);

public:
  void set_host_name(const string& host_name);
  void set_port(int port);
  void spawn_background_server();
  
  void run_server();
  void run_client();
  
  void send_start_app(const string& cmd);
  void start_app(const string& cmd);
  
  void send_kill_app(const string& pid);
  void kill_app();
  
  void cli_command(const string& cmd);
  void handle_command(const string& cmd);
  
  void handle_datagram(NetDatagram& datagram);

protected:
  QueuedConnectionManager _cm;
  QueuedConnectionReader _reader;
  ConnectionWriter _writer;
  QueuedConnectionListener _listener;

  string _host_name;
  int _port;
  intptr_t _app_pid;
  typedef pset< PT(Connection) > ConnectionSet;
  ConnectionSet _connections;

  bool _verbose;
  bool _shutdown;
  
  void check_for_new_clients();
  void check_for_datagrams();
  void check_for_lost_connection();
  
  void read_command(string& cmd);
};

class EXPCL_DIRECT DirectDServer: public DirectD {
public:
  DirectDServer();
  ~DirectDServer();
};

class EXPCL_DIRECT DirectDClient: public DirectD {
public:
  DirectDClient();
  ~DirectDClient();

  void handle_command(const string& cmd);
};

