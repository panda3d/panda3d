// Filename: gtkStatsMonitor.h
// Created by:  drose (14Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef GTKSTATSMONITOR_H
#define GTKSTATSMONITOR_H

#include <pandatoolbase.h>

#include <pStatMonitor.h>
#include <pointerTo.h>

#include <set>


class GtkStatsWindow;
class Gdk_Color;

////////////////////////////////////////////////////////////////////
// 	 Class : GtkStatsMonitor
// Description : 
////////////////////////////////////////////////////////////////////
class GtkStatsMonitor : public PStatMonitor {
public:
  GtkStatsMonitor();
  ~GtkStatsMonitor();

  PT(PStatMonitor) close_all_windows();

  virtual string get_monitor_name();

  virtual void initialized();
  virtual void got_hello();
  virtual void got_bad_version(int client_major, int client_minor,
                               int server_major, int server_minor);
  virtual void new_collector(int collector_index);
  virtual void new_data(int thread_index, int frame_number);
  virtual void lost_connection();
  virtual void idle();
  virtual bool has_idle();
  virtual bool is_thread_safe();

public:
  void add_window(GtkStatsWindow *window);
  void remove_window(GtkStatsWindow *window);

  typedef set<GtkStatsWindow *> Windows;
  Windows _windows;

  bool _destructing;
  bool _new_collector;
};

#endif
