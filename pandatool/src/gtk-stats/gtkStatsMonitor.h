// Filename: gtkStatsMonitor.h
// Created by:  drose (14Jul00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef GTKSTATSMONITOR_H
#define GTKSTATSMONITOR_H

#include "pandatoolbase.h"

#include "pStatMonitor.h"
#include "pointerTo.h"

#include "pset.h"

class GtkStatsServer;
class GtkStatsWindow;
class Gdk_Color;

////////////////////////////////////////////////////////////////////
//       Class : GtkStatsMonitor
// Description :
////////////////////////////////////////////////////////////////////
class GtkStatsMonitor : public PStatMonitor {
public:
  GtkStatsMonitor(GtkStatsServer *server);
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

  typedef pset<GtkStatsWindow *> Windows;
  Windows _windows;

  bool _destructing;
  bool _new_collector;
};

#endif
