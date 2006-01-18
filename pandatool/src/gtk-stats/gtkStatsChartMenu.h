// Filename: gtkStatsChartMenu.h
// Created by:  drose (16Jan06)
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
  void add_to_menu_bar(GtkWidget *menu_bar, int before_menu_id);

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

