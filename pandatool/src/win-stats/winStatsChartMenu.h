// Filename: winStatsChartMenu.h
// Created by:  drose (08Jan04)
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

#ifndef WINSTATSCHARTMENU_H
#define WINSTATSCHARTMENU_H

#include "pandatoolbase.h"

#include <windows.h>

class WinStatsMonitor;
class PStatView;
class PStatViewLevel;

////////////////////////////////////////////////////////////////////
//       Class : WinStatsChartMenu
// Description : A pulldown menu of charts available for a particular
//               thread.
////////////////////////////////////////////////////////////////////
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

