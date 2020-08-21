/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file winStatsChartMenu.cxx
 * @author drose
 * @date 2004-01-08
 */

#include "winStatsChartMenu.h"
#include "winStatsMonitor.h"

/**
 *
 */
WinStatsChartMenu::
WinStatsChartMenu(WinStatsMonitor *monitor, int thread_index) :
  _monitor(monitor),
  _thread_index(thread_index)
{
  _menu = CreatePopupMenu();
  do_update();
}

/**
 *
 */
WinStatsChartMenu::
~WinStatsChartMenu() {
}

/**
 * Returns the Windows menu handle for this particular menu.
 */
HMENU WinStatsChartMenu::
get_menu_handle() {
  return _menu;
}

/**
 * Adds the menu to the end of the indicated menu bar.
 */
void WinStatsChartMenu::
add_to_menu_bar(HMENU menu_bar, int before_menu_id) {
  const PStatClientData *client_data = _monitor->get_client_data();
  std::string thread_name;
  if (_thread_index == 0) {
    // A special case for the main thread.
    thread_name = "Graphs";
  } else {
    thread_name = client_data->get_thread_name(_thread_index);
  }

  MENUITEMINFO mii;
  memset(&mii, 0, sizeof(mii));
  mii.cbSize = sizeof(mii);

  mii.fMask = MIIM_STRING | MIIM_FTYPE | MIIM_SUBMENU;
  mii.fType = MFT_STRING;
  mii.hSubMenu = _menu;
  mii.dwTypeData = (char *)thread_name.c_str();
  InsertMenuItem(menu_bar, before_menu_id, FALSE, &mii);
}

/**
 * Checks to see if the menu needs to be updated (e.g.  because of new data
 * from the client), and updates it if necessary.
 */
void WinStatsChartMenu::
check_update() {
  PStatView &view = _monitor->get_view(_thread_index);
  if (view.get_level_index() != _last_level_index) {
    do_update();
  }
}

/**
 * Unconditionally updates the menu with the latest data from the client.
 */
void WinStatsChartMenu::
do_update() {
  PStatView &view = _monitor->get_view(_thread_index);
  _last_level_index = view.get_level_index();

  // First, remove all of the old entries from the menu.
  int num_items = GetMenuItemCount(_menu);
  for (int i = num_items - 1; i >= 0; i--) {
    DeleteMenu(_menu, i, MF_BYPOSITION);
  }

  // Now rebuild the menu with the new set of entries.

  // The menu item(s) for the thread's frame time goes first.
  add_view(_menu, view.get_top_level(), false);

  bool needs_separator = true;
  MENUITEMINFO mii;
  memset(&mii, 0, sizeof(mii));
  mii.cbSize = sizeof(mii);

  // And then the menu item(s) for each of the level values.
  const PStatClientData *client_data = _monitor->get_client_data();
  int num_toplevel_collectors = client_data->get_num_toplevel_collectors();
  for (int tc = 0; tc < num_toplevel_collectors; tc++) {
    int collector = client_data->get_toplevel_collector(tc);
    if (client_data->has_collector(collector) &&
        client_data->get_collector_has_level(collector, _thread_index)) {

      // We put a separator between the above frame collector and the first
      // level collector.
      if (needs_separator) {
        mii.fMask = MIIM_FTYPE;
        mii.fType = MFT_SEPARATOR;
        InsertMenuItem(_menu, GetMenuItemCount(_menu), TRUE, &mii);

        needs_separator = false;
      }

      PStatView &level_view = _monitor->get_level_view(collector, _thread_index);
      add_view(_menu, level_view.get_top_level(), true);
    }
  }

  // Also a menu item for a piano roll (following a separator).
  mii.fMask = MIIM_FTYPE;
  mii.fType = MFT_SEPARATOR;
  InsertMenuItem(_menu, GetMenuItemCount(_menu), TRUE, &mii);

  WinStatsMonitor::MenuDef menu_def(_thread_index, -1, false);
  int menu_id = _monitor->get_menu_id(menu_def);

  mii.fMask = MIIM_STRING | MIIM_FTYPE | MIIM_ID;
  mii.fType = MFT_STRING;
  mii.wID = menu_id;
  mii.dwTypeData = "Piano Roll";
  InsertMenuItem(_menu, GetMenuItemCount(_menu), TRUE, &mii);
}

/**
 * Adds a new entry or entries to the menu for the indicated view and its
 * children.
 */
void WinStatsChartMenu::
add_view(HMENU parent_menu, const PStatViewLevel *view_level, bool show_level) {
  int collector = view_level->get_collector();

  const PStatClientData *client_data = _monitor->get_client_data();
  std::string collector_name = client_data->get_collector_name(collector);

  WinStatsMonitor::MenuDef menu_def(_thread_index, collector, show_level);
  int menu_id = _monitor->get_menu_id(menu_def);

  MENUITEMINFO mii;
  memset(&mii, 0, sizeof(mii));
  mii.cbSize = sizeof(mii);

  mii.fMask = MIIM_STRING | MIIM_FTYPE | MIIM_ID;
  mii.fType = MFT_STRING;
  mii.wID = menu_id;
  mii.dwTypeData = (char *)collector_name.c_str();
  InsertMenuItem(parent_menu, GetMenuItemCount(parent_menu), TRUE, &mii);

  int num_children = view_level->get_num_children();
  if (num_children > 1) {
    // If the collector has more than one child, add a menu entry to go
    // directly to each of its children.
    HMENU submenu = CreatePopupMenu();
    std::string submenu_name = collector_name + " components";

    mii.fMask = MIIM_STRING | MIIM_FTYPE | MIIM_SUBMENU;
    mii.fType = MFT_STRING;
    mii.hSubMenu = submenu;
    mii.dwTypeData = (char *)submenu_name.c_str();
    InsertMenuItem(parent_menu, GetMenuItemCount(parent_menu), TRUE, &mii);

    // Reverse the order since the menus are listed from the top down; we want
    // to be visually consistent with the graphs, which list these labels from
    // the bottom up.
    for (int c = num_children - 1; c >= 0; c--) {
      add_view(submenu, view_level->get_child(c), show_level);
    }
  }
}
