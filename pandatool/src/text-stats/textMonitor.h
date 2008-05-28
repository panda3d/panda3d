// Filename: textMonitor.h
// Created by:  drose (12Jul00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef TEXTMONITOR_H
#define TEXTMONITOR_H

#include "pandatoolbase.h"
#include "pStatMonitor.h"

class TextStats;

////////////////////////////////////////////////////////////////////
//       Class : TextMonitor
// Description : A simple, scrolling-text stats monitor.  Guaranteed
//               to compile on every platform.
////////////////////////////////////////////////////////////////////
class TextMonitor : public PStatMonitor {
public:
  TextMonitor(TextStats *server);
  TextStats *get_server();
 
  virtual string get_monitor_name();

  virtual void got_hello();
  virtual void got_bad_version(int client_major, int client_minor,
                               int server_major, int server_minor);
  virtual void new_data(int thread_index, int frame_number);
  virtual void lost_connection();
  virtual bool is_thread_safe();

  void show_ms(const PStatViewLevel *level, int indent_level);
  void show_level(const PStatViewLevel *level, int indent_level);
};

#include "textMonitor.I"

#endif
