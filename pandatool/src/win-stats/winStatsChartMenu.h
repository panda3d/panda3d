// Filename: winStatsChartMenu.h
// Created by:  drose (08Jan04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
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
  void add_view(HMENU parent_menu, const PStatViewLevel *view_level);

  WinStatsMonitor *_monitor;
  int _thread_index;

  int _last_level_index;
  HMENU _menu;
};

#endif

