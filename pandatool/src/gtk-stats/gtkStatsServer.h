/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file gtkStatsServer.h
 * @author drose
 * @date 2006-01-16
 */

#ifndef GTKSTATSSERVER_H
#define GTKSTATSSERVER_H

#include "pandatoolbase.h"
#include "pStatServer.h"
#include "gtkStatsMonitor.h"

/**
 * The class that owns the main loop, waiting for client connections.
 */
class GtkStatsServer : public PStatServer {
public:
  GtkStatsServer();

  virtual PStatMonitor *make_monitor(const NetAddress &address);
  virtual void lost_connection(PStatMonitor *monitor);

  bool new_session();
  bool open_session();
  bool open_last_session();
  bool save_session();
  bool export_session();
  bool close_session();

  GtkWidget *get_window() const;
  GtkWidget *get_menu_bar() const;
  GtkWidget *get_status_bar() const;

  int get_time_units() const;
  void set_time_units(int unit_mask);

private:
  void create_window();
  void setup_session_menu();
  void setup_options_menu();

  static gboolean status_bar_button_event(GtkWidget *widget,
                                          GdkEventButton *event,
                                          gpointer data);

private:
  PT(GtkStatsMonitor) _monitor;

  Filename _last_session;
  Filename _save_filename;

  GtkWidget *_window;
  GtkWidget *_menu_bar;
  GtkWidget *_session_menu;
  GtkWidget *_options_menu;
  GtkWidget *_status_bar;
  GtkWidget *_status_bar_label = nullptr;
  GtkWidget *_new_session_menu_item;
  GtkWidget *_open_last_session_menu_item;
  GtkWidget *_save_session_menu_item;
  GtkWidget *_close_session_menu_item;
  GtkWidget *_export_session_menu_item;
  int _time_units;
};

#endif
