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


#include <process.h>
#include <Windows.h>
#include "pandabase.h"
#include "queuedConnectionManager.h"


class DirectD {
public:
  DirectD();
  ~DirectD();
  
  void set_host_name(const string& host_name);
  void set_port(int port);
  void spawn_background_server();
  
  void run_server();
  void run_client();
  void run_controller();
  
  void send_start_app(const string& cmd);
  void start_app(const string& cmd);
  
  void send_kill_app(const string& pid);
  void kill_app();

protected:
  QueuedConnectionManager _cm;
  string _host_name;
  int _port;
  intptr_t _app_pid;
  bool _controller;
  bool _verbose;
};


