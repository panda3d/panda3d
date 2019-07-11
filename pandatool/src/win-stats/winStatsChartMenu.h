/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file winStatsChartMenu.h
 * @author drose
 * @date 2004-01-08
 */

#ifndef WINSTATSCHARTMENU_H
#define WINSTATSCHARTMENU_H

#include "pandatoolbase.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>

class WinStatsMonitor;
class PStatView;
class PStatViewLevel;

/**
 * A pulldown menu of charts available for a particular thread.
 */
class WinStatsChartMenu {
public:
  WinStatsChartMenu(WinStatsMonitor *monitor, int thread_index);
  ~WinStatsChartMenu();

  HMENU get_menu_handle();
  void add_to_menu_bar(HMENU menu_bar, int before_menu_id);

  void check_update();
  void do_update();

private:
  void add_view(HMENU parent_menu, const PStatViewLevel *view_level,
                bool show_level);

  WinStatsMonitor *_monitor;
  int _thread_index;

  int _last_level_index;
  HMENU _menu;
};

#endif
