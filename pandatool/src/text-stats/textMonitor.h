/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file textMonitor.h
 * @author drose
 * @date 2000-07-12
 */

#ifndef TEXTMONITOR_H
#define TEXTMONITOR_H

#include "pandatoolbase.h"
#include "pStatMonitor.h"

// [PECI]
#include <iostream>
#include <fstream>

class TextStats;

/**
 * A simple, scrolling-text stats monitor.  Guaranteed to compile on every
 * platform.
 */
class TextMonitor : public PStatMonitor {
public:
  TextMonitor(TextStats *server, std::ostream *outStream, bool show_raw_data);
  TextStats *get_server();

  virtual std::string get_monitor_name();

  virtual void got_hello();
  virtual void got_bad_version(int client_major, int client_minor,
                               int server_major, int server_minor);
  virtual void new_data(int thread_index, int frame_number);
  virtual void lost_connection();
  virtual bool is_thread_safe();

  void show_ms(const PStatViewLevel *level, int indent_level);
  void show_level(const PStatViewLevel *level, int indent_level);

private:
  std::ostream *_outStream; //[PECI]
  bool _show_raw_data;
};

#include "textMonitor.I"

#endif
