// Filename: textMonitor.h
// Created by:  drose (12Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef TEXTMONITOR_H
#define TEXTMONITOR_H

#include <pandatoolbase.h>

#include <pStatMonitor.h>

////////////////////////////////////////////////////////////////////
//       Class : TextMonitor
// Description : A simple, scrolling-text stats monitor.  Guaranteed
//               to compile on every platform.
////////////////////////////////////////////////////////////////////
class TextMonitor : public PStatMonitor {
public:
  TextMonitor();

  virtual string get_monitor_name();

  virtual void got_hello();
  virtual void got_bad_version(int client_major, int client_minor,
                               int server_major, int server_minor);
  virtual void new_data(int thread_index, int frame_number);
  virtual void lost_connection();
  virtual bool is_thread_safe();

  void show_level(const PStatViewLevel *level, int indent_level);
};

#endif
