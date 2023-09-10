/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file macStatsChartMenu.mm
 * @author rdb
 * @date 2023-08-18
 */

#include "macStatsChartMenu.h"
#include "macStatsChartMenuDelegate.h"
#include "macStatsMonitor.h"

/**
 *
 */
MacStatsChartMenu::
MacStatsChartMenu(MacStatsMonitor *monitor, int thread_index) :
  _monitor(monitor),
  _thread_index(thread_index)
{
  _menu = [[NSMenu alloc] init];
  _menu.delegate = [[MacStatsChartMenuDelegate alloc] initWithMonitor:monitor threadIndex:thread_index];

  if (thread_index == 0) {
    _menu.title = @"Graphs";

    // Timeline goes first.
    make_menu_item(_menu, -1, "Timeline", @selector(handleOpenTimeline:));

    // Then the piano roll (even though it's not very useful nowadays)
    make_menu_item(_menu, -1, "Piano Roll", @selector(handleOpenPianoRoll:));
  }
  else {
    make_menu_item(_menu, -1, "Open Strip Chart", @selector(handleOpenStripChart:), 0);
    make_menu_item(_menu, -1, "Open Flame Graph", @selector(handleOpenFlameGraph:));
  }

  [_menu addItem:[NSMenuItem separatorItem]];
  _time_items_end = 3;

  // Put a separator between time items and level items.
  [_menu addItem:[NSMenuItem separatorItem]];
  _level_items_end = _time_items_end + 1;

  // For the main thread menu, also some options relating to all graph windows.
  if (thread_index == 0) {
    [_menu addItem:[NSMenuItem separatorItem]];
    make_menu_item(_menu, -1, "Close All Graphs", @selector(handleCloseAllGraphs:));
    make_menu_item(_menu, -1, "Reopen Default Graphs", @selector(handleReopenDefaultGraphs:));
    make_menu_item(_menu, -1, "Save Current Layout as Default", @selector(handleSaveDefaultGraphs:));
  }

  do_update();
}

/**
 *
 */
MacStatsChartMenu::
~MacStatsChartMenu() {
  MacStatsChartMenuDelegate *delegate = (MacStatsChartMenuDelegate *)_menu.delegate;
  [_menu release];
  [delegate release];
}

/**
 * Adds the menu to the end of the indicated menu bar.
 */
void MacStatsChartMenu::
add_to_menu(NSMenu *menu, int position) {
  NSMenuItem *item = [[NSMenuItem alloc] init];
  [menu insertItem:item atIndex:position];
  [menu setSubmenu:_menu forItem:item];
  [item release];
}

/**
 * Removes the menu from the menu bar.
 */
void MacStatsChartMenu::
remove_from_menu(NSMenu *menu) {
  int index = [menu indexOfItemWithSubmenu:_menu];
  if (index >= 0) {
    [menu removeItemAtIndex:index];
  }
}

/**
 * Checks to see if the menu needs to be updated (e.g.  because of new data
 * from the client), and updates it if necessary.
 */
void MacStatsChartMenu::
check_update() {
  PStatView &view = _monitor->get_view(_thread_index);
  if (view.get_level_index() != _last_level_index) {
    do_update();
  }
}

/**
 * Unconditionally updates the menu with the latest data from the client.
 */
void MacStatsChartMenu::
do_update() {
  PStatView &view = _monitor->get_view(_thread_index);
  _last_level_index = view.get_level_index();

  const PStatClientData *client_data = _monitor->get_client_data();
  if (_thread_index != 0) {
    std::string thread_name = client_data->get_thread_name(_thread_index);
    _menu.title = [NSString stringWithUTF8String:thread_name.c_str()];
  }

  if (client_data->get_num_collectors() > _collector_items.size()) {
    _collector_items.resize(client_data->get_num_collectors(), std::make_pair(nullptr, nullptr));
  }

  // The menu item(s) for the thread's frame time goes second.
  const PStatViewLevel *view_level = view.get_top_level();
  if (_thread_index == 0) {
    if (add_view(_menu, view_level, false, _time_items_end)) {
      ++_time_items_end;
      ++_level_items_end;
    }
  } else {
    for (int c = 0; c < view_level->get_num_children(); ++c) {
      if (add_view(_menu, view_level->get_child(c), false, _time_items_end)) {
        ++_time_items_end;
        ++_level_items_end;
      }
    }
  }

  // And then the menu item(s) for each of the level values.
  int num_toplevel_collectors = client_data->get_num_toplevel_collectors();
  for (int tc = 0; tc < num_toplevel_collectors; tc++) {
    int collector = client_data->get_toplevel_collector(tc);
    if (client_data->has_collector(collector) &&
        client_data->get_collector_has_level(collector, _thread_index)) {

      PStatView &level_view = _monitor->get_level_view(collector, _thread_index);
      add_view(_menu, level_view.get_top_level(), true, _level_items_end);
    }
  }
}

/**
 * Adds a new entry or entries to the menu for the indicated view and its
 * children.  Returns true if an item was added, false if not.
 */
bool MacStatsChartMenu::
add_view(NSMenu *parent_menu, const PStatViewLevel *view_level,
         bool show_level, int insert_at) {
  int collector = view_level->get_collector();

  NSMenuItem *&menu_item = _collector_items[collector].first;
  NSMenu *&menu = _collector_items[collector].second;

  const PStatClientData *client_data = _monitor->get_client_data();

  int num_children = view_level->get_num_children();
  if (menu == nullptr && num_children == 0) {
    // For a collector without children, no point in making a submenu.  We just
    // have the item open a strip chart directly (no point in creating a flame
    // graph if there are no children).
    if (menu_item != nullptr) {
      // Already exists.
      return false;
    }

    std::string collector_name = client_data->get_collector_name(collector);
    if (show_level) {
      menu_item = make_menu_item(parent_menu, insert_at,
        collector_name.c_str(), @selector(handleOpenStripChartLevel:), collector);
    } else {
      menu_item = make_menu_item(parent_menu, insert_at,
        collector_name.c_str(), @selector(handleOpenStripChart:), collector);
    }
    return true;
  }
  else if (menu_item != nullptr && menu == nullptr) {
    // Unhook the signal handler, we are creating a submenu.
    menu_item.action = nil;
  }

  // Create a submenu.
  bool added_item = false;
  if (menu_item == nullptr) {
    std::string collector_name = client_data->get_collector_name(collector);
    menu_item = make_menu_item(parent_menu, insert_at, collector_name.c_str(), nil);
    added_item = true;
  }

  if (menu == nullptr) {
    menu = [[NSMenu alloc] init];
    [parent_menu setSubmenu:menu forItem:menu_item];

    if (show_level) {
      make_menu_item(menu, -1, "Open Strip Chart",
                     @selector(handleOpenStripChartLevel:), collector);
    } else {
      make_menu_item(menu, -1, "Open Strip Chart",
                     @selector(handleOpenStripChart:), collector);

      if (collector == 0) {
        collector = -1;
      }

      make_menu_item(menu, -1, "Open Flame Graph",
                     @selector(handleOpenFlameGraph:), collector);
    }

    [menu addItem:[NSMenuItem separatorItem]];
    [menu release];
  }

  for (int c = 0; c < num_children; ++c) {
    add_view(menu, view_level->get_child(c), show_level, 2 + !show_level);
  }

  return added_item;
}

/**
 *
 */
NSMenuItem *MacStatsChartMenu::
make_menu_item(NSMenu *parent_menu, int insert_at, const char *label, SEL action, int collector_index) {
  NSMenuItem *menu_item = [[NSMenuItem alloc] init];
  menu_item.title = [NSString stringWithUTF8String:label];
  menu_item.target = _menu.delegate;
  menu_item.action = action;
  menu_item.tag = collector_index;
  if (insert_at >= 0) {
    [parent_menu insertItem:menu_item atIndex:insert_at];
  } else {
    [parent_menu addItem:menu_item];
  }
  [menu_item release];
  return menu_item;
}
