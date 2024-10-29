/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file gtkStatsChartMenu.cxx
 * @author drose
 * @date 2006-01-16
 */

#include "gtkStatsChartMenu.h"
#include "gtkStatsMonitor.h"

/**
 *
 */
GtkStatsChartMenu::
GtkStatsChartMenu(GtkStatsMonitor *monitor, int thread_index) :
  _monitor(monitor),
  _thread_index(thread_index)
{
  _menu = gtk_menu_new();

  if (thread_index == 0) {
    // Timeline goes first.
    gtk_menu_shell_append(GTK_MENU_SHELL(_menu),
      make_menu_item("Timeline", -1, GtkStatsMonitor::CT_timeline, false));

    // Then the piano roll (even though it's not very useful nowadays)
    gtk_menu_shell_append(GTK_MENU_SHELL(_menu),
      make_menu_item("Piano Roll", -1, GtkStatsMonitor::CT_piano_roll, false));
  }
  else {
    gtk_menu_shell_append(GTK_MENU_SHELL(_menu),
      make_menu_item("Open Strip Chart", 0, GtkStatsMonitor::CT_strip_chart, false));
    gtk_menu_shell_append(GTK_MENU_SHELL(_menu),
      make_menu_item("Open Flame Graph", -1, GtkStatsMonitor::CT_flame_graph, false));
  }

  {
    GtkWidget *sep = gtk_separator_menu_item_new();
    gtk_widget_show(sep);
    gtk_menu_shell_append(GTK_MENU_SHELL(_menu), sep);
  }
  _time_items_end = 3;

  // Put a separator between time items and level items.
  {
    GtkWidget *sep = gtk_separator_menu_item_new();
    gtk_widget_show(sep);
    gtk_menu_shell_append(GTK_MENU_SHELL(_menu), sep);
    _level_items_end = _time_items_end + 1;
  }

  // For the main thread menu, also some options relating to all graph windows.
  if (thread_index == 0) {
    GtkWidget *sep = gtk_separator_menu_item_new();
    gtk_widget_show(sep);
    gtk_menu_shell_append(GTK_MENU_SHELL(_menu), sep);

    {
      GtkWidget *menu_item = gtk_menu_item_new_with_label("Close All Graphs");
      gtk_widget_show(menu_item);
      gtk_menu_shell_append(GTK_MENU_SHELL(_menu), menu_item);

      g_signal_connect(G_OBJECT(menu_item), "activate",
                       G_CALLBACK(activate_close_all),
                       (void *)_monitor);
    }

    {
      GtkWidget *menu_item = gtk_menu_item_new_with_label("Reopen Default Graphs");
      gtk_widget_show(menu_item);
      gtk_menu_shell_append(GTK_MENU_SHELL(_menu), menu_item);

      g_signal_connect(G_OBJECT(menu_item), "activate",
                       G_CALLBACK(activate_reopen_default),
                       (void *)_monitor);
    }

    {
      GtkWidget *menu_item = gtk_menu_item_new_with_label("Save Current Layout as Default");
      gtk_widget_show(menu_item);
      gtk_menu_shell_append(GTK_MENU_SHELL(_menu), menu_item);

      g_signal_connect(G_OBJECT(menu_item), "activate",
                       G_CALLBACK(activate_save_default),
                       (void *)_monitor);
    }
  }

  do_update();
  gtk_widget_show(_menu);
}

/**
 *
 */
GtkStatsChartMenu::
~GtkStatsChartMenu() {
}

/**
 * Returns the gtk widget for this particular menu.
 */
GtkWidget *GtkStatsChartMenu::
get_menu_widget() {
  return _menu;
}

/**
 * Adds the menu to the end of the indicated menu bar.
 */
void GtkStatsChartMenu::
add_to_menu_bar(GtkWidget *menu_bar, int position) {
  const PStatClientData *client_data = _monitor->get_client_data();
  std::string thread_name;
  if (_thread_index == 0) {
    // A special case for the main thread.
    thread_name = "Graphs";
  } else {
    thread_name = client_data->get_thread_name(_thread_index);
  }

  _menu_item = gtk_menu_item_new_with_label(thread_name.c_str());
  gtk_widget_show(_menu_item);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(_menu_item), _menu);

  gtk_menu_shell_insert(GTK_MENU_SHELL(menu_bar), _menu_item, position);
}

/**
 * Removes the menu from the menu bar.
 */
void GtkStatsChartMenu::
remove_from_menu_bar(GtkWidget *menu_bar) {
  gtk_container_remove(GTK_CONTAINER(menu_bar), _menu_item);
}

/**
 * Checks to see if the menu needs to be updated (e.g.  because of new data
 * from the client), and updates it if necessary.
 */
void GtkStatsChartMenu::
check_update() {
  PStatView &view = _monitor->get_view(_thread_index);
  if (view.get_level_index() != _last_level_index) {
    do_update();
  }
}

/**
 * Unconditionally updates the menu with the latest data from the client.
 */
void GtkStatsChartMenu::
do_update() {
  PStatView &view = _monitor->get_view(_thread_index);
  _last_level_index = view.get_level_index();

  const PStatClientData *client_data = _monitor->get_client_data();
  if ((size_t)client_data->get_num_collectors() > _collector_items.size()) {
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
bool GtkStatsChartMenu::
add_view(GtkWidget *parent_menu, const PStatViewLevel *view_level,
         bool show_level, int insert_at) {
  int collector = view_level->get_collector();

  GtkWidget *&menu_item = _collector_items[collector].first;
  GtkWidget *&menu = _collector_items[collector].second;

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
    menu_item = make_menu_item(
      collector_name.c_str(), collector, GtkStatsMonitor::CT_strip_chart, show_level);
    gtk_menu_shell_insert(GTK_MENU_SHELL(parent_menu), menu_item, insert_at);
    return true;
  }
  else if (menu_item != nullptr && menu == nullptr) {
    // Unhook the signal handler, we are creating a submenu.
    GtkStatsMonitor::MenuDef smd(GtkStatsMonitor::CT_strip_chart, _thread_index, collector, -1, show_level);
    const GtkStatsMonitor::MenuDef *menu_def = _monitor->add_menu(smd);

    g_signal_handlers_disconnect_by_data(G_OBJECT(menu_item), (void *)menu_def);
  }

  // Create a submenu.
  bool added_item = false;
  if (menu_item == nullptr) {
    std::string collector_name = client_data->get_collector_name(collector);
    menu_item = gtk_menu_item_new_with_label(collector_name.c_str());
    gtk_widget_show(menu_item);
    gtk_menu_shell_insert(GTK_MENU_SHELL(parent_menu), menu_item, insert_at);
    added_item = true;
  }

  if (menu == nullptr) {
    menu = gtk_menu_new();
    gtk_widget_show(menu);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);

    gtk_menu_shell_append(GTK_MENU_SHELL(menu),
      make_menu_item("Open Strip Chart", collector, GtkStatsMonitor::CT_strip_chart, show_level));

    if (!show_level) {
      if (collector == 0) {
        collector = -1;
      }

      gtk_menu_shell_append(GTK_MENU_SHELL(menu),
        make_menu_item("Open Flame Graph", collector, GtkStatsMonitor::CT_flame_graph));
    }

    GtkWidget *sep = gtk_separator_menu_item_new();
    gtk_widget_show(sep);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), sep);
  }

  for (int c = 0; c < num_children; ++c) {
    add_view(menu, view_level->get_child(c), show_level, 3 + !show_level);
  }

  return added_item;
}

/**
 *
 */
GtkWidget *GtkStatsChartMenu::
make_menu_item(const char *label, int collector_index, ChartType chart_type,
               bool show_level) {
  GtkStatsMonitor::MenuDef smd(chart_type, _thread_index, collector_index, -1, show_level);
  const GtkStatsMonitor::MenuDef *menu_def = _monitor->add_menu(smd);

  GtkWidget *menu_item = gtk_menu_item_new_with_label(label);
  gtk_widget_show(menu_item);

  g_signal_connect(G_OBJECT(menu_item), "activate",
                   G_CALLBACK(GtkStatsMonitor::menu_activate),
                   (void *)menu_def);

  return menu_item;
}

/**
 * Removes a previous menu child from the menu.
 */
void GtkStatsChartMenu::
remove_menu_child(GtkWidget *widget, gpointer data) {
  GtkWidget *menu = (GtkWidget *)data;
  gtk_container_remove(GTK_CONTAINER(menu), widget);
}

/**
 * Callback for Close All Graphs.
 */
void GtkStatsChartMenu::
activate_close_all(GtkWidget *widget, gpointer data) {
  GtkStatsMonitor *monitor = (GtkStatsMonitor *)data;
  monitor->remove_all_graphs();
}

/**
 * Callback for Reopen Default Graphs.
 */
void GtkStatsChartMenu::
activate_reopen_default(GtkWidget *widget, gpointer data) {
  GtkStatsMonitor *monitor = (GtkStatsMonitor *)data;
  monitor->remove_all_graphs();
  monitor->open_default_graphs();
}

/**
 * Callback for Save Current Layout as Default.
 */
void GtkStatsChartMenu::
activate_save_default(GtkWidget *widget, gpointer data) {
  GtkStatsMonitor *monitor = (GtkStatsMonitor *)data;
  monitor->save_default_graphs();
}
