// Filename: gtkStatsChartMenu.h
// Created by:  drose (16Jan06)
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

#ifndef GTKSTATSCHARTMENU_H
#define GTKSTATSCHARTMENU_H

#include "pandatoolbase.h"

#include <gtk/gtk.h>

class GtkStatsMonitor;
class PStatView;
class PStatViewLevel;

////////////////////////////////////////////////////////////////////
//       Class : GtkStatsChartMenu
// Description : A pulldown menu of charts available for a particular
//               thread.
////////////////////////////////////////////////////////////////////
class GtkStatsChartMenu {
public:
  GtkStatsChartMenu(GtkStatsMonitor *monitor, int thread_index);
  ~GtkStatsChartMenu();

  GtkWidget *get_menu_widget();
  void add_to_menu_bar(GtkWidget *menu_bar, int position);

  void check_update();
  void do_update();

private:
  void add_view(GtkWidget *parent_menu, const PStatViewLevel *view_level,
                bool show_level);

  static void handle_menu(gpointer data);
  static void remove_menu_child(GtkWidget *widget, gpointer data);

  GtkStatsMonitor *_monitor;
  int _thread_index;

  int _last_level_index;
  GtkWidget *_menu;
};

#endif

