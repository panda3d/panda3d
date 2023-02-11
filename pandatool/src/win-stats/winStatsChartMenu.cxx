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
#include "winStatsMenuId.h"
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
  DestroyMenu(_menu);
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

  mii.fMask = MIIM_STRING | MIIM_FTYPE | MIIM_SUBMENU | MIIM_ID;
  mii.fType = MFT_STRING;
  mii.wID = 1000 | _thread_index;
  mii.hSubMenu = _menu;
  mii.dwTypeData = (char *)thread_name.c_str();
  InsertMenuItem(menu_bar, before_menu_id, FALSE, &mii);
}

/**
 *
 */
void WinStatsChartMenu::
remove_from_menu_bar(HMENU menu_bar) {
  RemoveMenu(menu_bar, 1000 | _thread_index, MF_BYCOMMAND);
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
  MENUITEMINFO mii;
  memset(&mii, 0, sizeof(mii));
  mii.cbSize = sizeof(mii);

  if (_thread_index == 0) {
    // Timeline goes first.
    {
      WinStatsMonitor::MenuDef menu_def(_thread_index, -1, WinStatsMonitor::CT_timeline, false);
      int menu_id = _monitor->get_menu_id(menu_def);

      mii.fMask = MIIM_STRING | MIIM_FTYPE | MIIM_ID;
      mii.fType = MFT_STRING;
      mii.wID = menu_id;
      mii.dwTypeData = "Timeline";
      InsertMenuItem(_menu, GetMenuItemCount(_menu), TRUE, &mii);
    }

    // And the piano roll (even though it's not very useful nowadays)
    {
      WinStatsMonitor::MenuDef menu_def(_thread_index, -1, WinStatsMonitor::CT_piano_roll, false);
      int menu_id = _monitor->get_menu_id(menu_def);

      mii.fMask = MIIM_STRING | MIIM_FTYPE | MIIM_ID;
      mii.fType = MFT_STRING;
      mii.wID = menu_id;
      mii.dwTypeData = "Piano Roll";
      InsertMenuItem(_menu, GetMenuItemCount(_menu), TRUE, &mii);
    }

    mii.fMask = MIIM_FTYPE;
    mii.fType = MFT_SEPARATOR;
    InsertMenuItem(_menu, GetMenuItemCount(_menu), TRUE, &mii);
  }

  // The menu item(s) for the thread's frame time goes second.
  add_view(_menu, view.get_top_level(), false);

  bool needs_separator = true;

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

  // For the main thread menu, also some options relating to all graph windows.
  if (_thread_index == 0) {
    mii.fMask = MIIM_FTYPE;
    mii.fType = MFT_SEPARATOR;
    InsertMenuItem(_menu, GetMenuItemCount(_menu), TRUE, &mii);

    mii.fMask = MIIM_STRING | MIIM_FTYPE | MIIM_ID;
    mii.fType = MFT_STRING;
    mii.wID = MI_graphs_close_all;
    mii.dwTypeData = "Close All Graphs";
    InsertMenuItem(_menu, GetMenuItemCount(_menu), TRUE, &mii);

    mii.fMask = MIIM_STRING | MIIM_FTYPE | MIIM_ID;
    mii.fType = MFT_STRING;
    mii.wID = MI_graphs_reopen_default;
    mii.dwTypeData = "Reopen Default Graphs";
    InsertMenuItem(_menu, GetMenuItemCount(_menu), TRUE, &mii);

    mii.fMask = MIIM_STRING | MIIM_FTYPE | MIIM_ID;
    mii.fType = MFT_STRING;
    mii.wID = MI_graphs_save_default;
    mii.dwTypeData = "Save Current Layout as Default";
    InsertMenuItem(_menu, GetMenuItemCount(_menu), TRUE, &mii);
  }
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

  MENUITEMINFO mii;
  memset(&mii, 0, sizeof(mii));
  mii.cbSize = sizeof(mii);

  int num_children = view_level->get_num_children();
  if (show_level && num_children == 0) {
    // For a level collector without children, no point in making a submenu.
    WinStatsMonitor::MenuDef menu_def(_thread_index, collector, WinStatsMonitor::CT_strip_chart, show_level);
    int menu_id = _monitor->get_menu_id(menu_def);

    mii.fMask = MIIM_STRING | MIIM_FTYPE | MIIM_ID;
    mii.fType = MFT_STRING;
    mii.wID = menu_id;
    mii.dwTypeData = (char *)collector_name.c_str();
    InsertMenuItem(parent_menu, GetMenuItemCount(parent_menu), TRUE, &mii);
    return;
  }

  HMENU menu;
  if (!show_level && collector == 0 && num_children == 0) {
    // Root collector without children, just add the options directly to the
    // parent menu.
    menu = parent_menu;
  }
  else {
    // Create a submenu.
    menu = CreatePopupMenu();

    mii.fMask = MIIM_STRING | MIIM_FTYPE | MIIM_SUBMENU;
    mii.fType = MFT_STRING;
    mii.hSubMenu = menu;
    mii.dwTypeData = (char *)collector_name.c_str();
    InsertMenuItem(parent_menu, GetMenuItemCount(parent_menu), TRUE, &mii);
  }

  {
    WinStatsMonitor::MenuDef menu_def(_thread_index, collector, WinStatsMonitor::CT_strip_chart, show_level);
    int menu_id = _monitor->get_menu_id(menu_def);

    mii.fMask = MIIM_STRING | MIIM_FTYPE | MIIM_ID;
    mii.fType = MFT_STRING;
    mii.wID = menu_id;
    mii.dwTypeData = "Open Strip Chart";
    InsertMenuItem(menu, GetMenuItemCount(menu), TRUE, &mii);
  }

  if (!show_level) {
    if (collector == 0 && num_children == 0) {
      collector = -1;
    }

    WinStatsMonitor::MenuDef menu_def(_thread_index, collector, WinStatsMonitor::CT_flame_graph);
    int menu_id = _monitor->get_menu_id(menu_def);

    mii.fMask = MIIM_STRING | MIIM_FTYPE | MIIM_ID;
    mii.fType = MFT_STRING;
    mii.wID = menu_id;
    mii.dwTypeData = "Open Flame Graph";
    InsertMenuItem(menu, GetMenuItemCount(menu), TRUE, &mii);
  }

  if (num_children > 0) {
    mii.fMask = MIIM_FTYPE;
    mii.fType = MFT_SEPARATOR;
    InsertMenuItem(menu, GetMenuItemCount(menu), TRUE, &mii);

    // Reverse the order since the menus are listed from the top down; we want
    // to be visually consistent with the graphs, which list these labels from
    // the bottom up.
    for (int c = num_children - 1; c >= 0; c--) {
      add_view(menu, view_level->get_child(c), show_level);
    }
  }
}
