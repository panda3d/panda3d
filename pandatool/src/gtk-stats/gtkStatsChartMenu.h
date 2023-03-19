/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file gtkStatsChartMenu.h
 * @author drose
 * @date 2006-01-16
 */

#ifndef GTKSTATSCHARTMENU_H
#define GTKSTATSCHARTMENU_H

#include "pandatoolbase.h"
#include "gtkStatsMonitor.h"

#include <gtk/gtk.h>

class PStatView;
class PStatViewLevel;

/**
 * A pulldown menu of charts available for a particular thread.
 */
class GtkStatsChartMenu {
public:
  typedef GtkStatsMonitor::ChartType ChartType;

  GtkStatsChartMenu(GtkStatsMonitor *monitor, int thread_index);
  ~GtkStatsChartMenu();

  int get_thread_index() const { return _thread_index; }

  GtkWidget *get_menu_widget();
  void add_to_menu_bar(GtkWidget *menu_bar, int position);
  void remove_from_menu_bar(GtkWidget *menu_bar);

  void check_update();
  void do_update();

private:
  bool add_view(GtkWidget *parent_menu, const PStatViewLevel *view_level,
                bool show_level, int insert_at);
  GtkWidget *make_menu_item(const char *label, int collector_index,
                            ChartType chart_type, bool show_level = false);

  static void remove_menu_child(GtkWidget *widget, gpointer data);
  static void activate_close_all(GtkWidget *widget, gpointer data);
  static void activate_reopen_default(GtkWidget *widget, gpointer data);
  static void activate_save_default(GtkWidget *widget, gpointer data);

  GtkStatsMonitor *_monitor;
  int _thread_index;

  int _last_level_index;
  GtkWidget *_menu;
  GtkWidget *_menu_item = nullptr;

  // Pair of menu item, submenu
  std::vector<std::pair<GtkWidget *, GtkWidget *> > _collector_items;
  int _time_items_end = 0;
  int _level_items_end = 0;
};

#endif
