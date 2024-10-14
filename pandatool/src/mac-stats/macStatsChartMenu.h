/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file macStatsChartMenu.h
 * @author rdb
 * @date 2023-08-18
 */

#ifndef MACSTATSCHARTMENU_H
#define MACSTATSCHARTMENU_H

#include "pandatoolbase.h"
#include "macStatsMonitor.h"

#include <Cocoa/Cocoa.h>

class PStatView;
class PStatViewLevel;

/**
 * A pulldown menu of charts available for a particular thread.
 */
class MacStatsChartMenu {
public:
  MacStatsChartMenu(MacStatsMonitor *monitor, int thread_index);
  ~MacStatsChartMenu();

  int get_thread_index() const { return _thread_index; }

  void add_to_menu(NSMenu *menu, int position);
  void remove_from_menu(NSMenu *menu);

  void check_update();
  void do_update();

private:
  bool add_view(NSMenu *parent_menu, const PStatViewLevel *view_level,
                bool show_level, int insert_at);
  NSMenuItem *make_menu_item(NSMenu *parent_menu, int insert_at,
                             const char *label, SEL action,
                             int collector_index = -1);

  MacStatsMonitor *_monitor;
  int _thread_index;

  int _last_level_index;
  NSMenu *_menu;

  // Pair of menu item, submenu
  std::vector<std::pair<NSMenuItem *, NSMenu *> > _collector_items;
  int _time_items_end = 0;
  int _level_items_end = 0;
};

#endif
