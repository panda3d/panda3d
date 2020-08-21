/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file gtkStatsMonitor.cxx
 * @author drose
 * @date 2006-01-16
 */

#include "gtkStatsMonitor.h"
#include "gtkStats.h"
#include "gtkStatsServer.h"
#include "gtkStatsStripChart.h"
#include "gtkStatsChartMenu.h"
#include "gtkStatsPianoRoll.h"
#include "gtkStatsMenuId.h"
#include "pStatGraph.h"
#include "pStatCollectorDef.h"
#include "indent.h"

typedef void vc();

GtkItemFactoryEntry GtkStatsMonitor::menu_entries[] = {
  { (gchar *)"/Options", nullptr, nullptr, 0, (gchar *)"<Branch>" },
  { (gchar *)"/Options/Units", nullptr, nullptr, 0, (gchar *)"<Branch>" },
  { (gchar *)"/Options/Units/ms", nullptr, (vc *)&handle_menu_command, MI_time_ms, (gchar *)"<RadioItem>" },
  { (gchar *)"/Options/Units/Hz", nullptr, (vc *)&handle_menu_command, MI_time_hz, (gchar *)"/Options/Units/ms" },
  { (gchar *)"/Speed", nullptr, nullptr, 0, (gchar *)"<Branch>" },
  { (gchar *)"/Speed/1", nullptr, (vc *)&handle_menu_command, MI_speed_1, (gchar *)"<RadioItem>" },
  { (gchar *)"/Speed/2", nullptr, (vc *)&handle_menu_command, MI_speed_2, (gchar *)"/Speed/1" },
  { (gchar *)"/Speed/3", nullptr, (vc *)&handle_menu_command, MI_speed_3, (gchar *)"/Speed/1" },
  { (gchar *)"/Speed/6", nullptr, (vc *)&handle_menu_command, MI_speed_6, (gchar *)"/Speed/1" },
  { (gchar *)"/Speed/12", nullptr, (vc *)&handle_menu_command, MI_speed_12, (gchar *)"/Speed/1" },
  { (gchar *)"/Speed/sep", nullptr, nullptr, 0, (gchar *)"<Separator>" },
  { (gchar *)"/Speed/pause", nullptr, (vc *)&handle_menu_command, MI_pause, (gchar *)"<CheckItem>" },
};

int GtkStatsMonitor::num_menu_entries = sizeof(menu_entries) / sizeof(GtkItemFactoryEntry);

/**
 *
 */
GtkStatsMonitor::
GtkStatsMonitor(GtkStatsServer *server) : PStatMonitor(server) {
  _window = nullptr;
  _item_factory = nullptr;

  // These will be filled in later when the menu is created.
  _time_units = 0;
  _scroll_speed = 0.0;
  _pause = false;
}

/**
 *
 */
GtkStatsMonitor::
~GtkStatsMonitor() {
  shutdown();
}

/**
 * Should be redefined to return a descriptive name for the type of
 * PStatsMonitor this is.
 */
std::string GtkStatsMonitor::
get_monitor_name() {
  return "GtkStats";
}

/**
 * Called after the monitor has been fully set up.  At this time, it will have
 * a valid _client_data pointer, and things like is_alive() and close() will
 * be meaningful.  However, we may not yet know who we're connected to
 * (is_client_known() may return false), and we may not know anything about
 * the threads or collectors we're about to get data on.
 */
void GtkStatsMonitor::
initialized() {
}

/**
 * Called when the "hello" message has been received from the client.  At this
 * time, the client's hostname and program name will be known.
 */
void GtkStatsMonitor::
got_hello() {
  create_window();
  open_strip_chart(0, 0, false);
}

/**
 * Like got_hello(), this is called when the "hello" message has been received
 * from the client.  At this time, the client's hostname and program name will
 * be known.  However, the client appears to be an incompatible version and
 * the connection will be terminated; the monitor should issue a message to
 * that effect.
 */
void GtkStatsMonitor::
got_bad_version(int client_major, int client_minor,
                int server_major, int server_minor) {
  std::ostringstream str;
  str << "Unable to honor connection attempt from "
      << get_client_progname() << " on " << get_client_hostname()
      << ": unsupported PStats version "
      << client_major << "." << client_minor;

  if (server_minor == 0) {
    str << " (server understands version " << server_major
        << "." << server_minor << " only).";
  } else {
    str << " (server understands versions " << server_major
        << ".0 through " << server_major << "." << server_minor << ").";
  }

  std::string message = str.str();
  GtkWidget *dialog =
    gtk_message_dialog_new(GTK_WINDOW(main_window),
                           GTK_DIALOG_DESTROY_WITH_PARENT,
                           GTK_MESSAGE_ERROR,
                           GTK_BUTTONS_CLOSE,
                           "%s", message.c_str());
  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);
}

/**
 * Called whenever a new Collector definition is received from the client.
 * Generally, the client will send all of its collectors over shortly after
 * connecting, but there's no guarantee that they will all be received before
 * the first frames are received.  The monitor should be prepared to accept
 * new Collector definitions midstream.
 */
void GtkStatsMonitor::
new_collector(int collector_index) {
  Graphs::iterator gi;
  for (gi = _graphs.begin(); gi != _graphs.end(); ++gi) {
    GtkStatsGraph *graph = (*gi);
    graph->new_collector(collector_index);
  }

  // We might need to update our menus.
  ChartMenus::iterator mi;
  for (mi = _chart_menus.begin(); mi != _chart_menus.end(); ++mi) {
    (*mi)->do_update();
  }
}

/**
 * Called whenever a new Thread definition is received from the client.
 * Generally, the client will send all of its threads over shortly after
 * connecting, but there's no guarantee that they will all be received before
 * the first frames are received.  The monitor should be prepared to accept
 * new Thread definitions midstream.
 */
void GtkStatsMonitor::
new_thread(int thread_index) {
  GtkStatsChartMenu *chart_menu = new GtkStatsChartMenu(this, thread_index);
  GtkWidget *menu_bar = gtk_item_factory_get_widget(_item_factory, "<PStats>");
  chart_menu->add_to_menu_bar(menu_bar, _next_chart_index);
  ++_next_chart_index;
  _chart_menus.push_back(chart_menu);
}

/**
 * Called as each frame's data is made available.  There is no guarantee the
 * frames will arrive in order, or that all of them will arrive at all.  The
 * monitor should be prepared to accept frames received out-of-order or
 * missing.
 */
void GtkStatsMonitor::
new_data(int thread_index, int frame_number) {
  Graphs::iterator gi;
  for (gi = _graphs.begin(); gi != _graphs.end(); ++gi) {
    GtkStatsGraph *graph = (*gi);
    graph->new_data(thread_index, frame_number);
  }
}


/**
 * Called whenever the connection to the client has been lost.  This is a
 * permanent state change.  The monitor should update its display to represent
 * this, and may choose to close down automatically.
 */
void GtkStatsMonitor::
lost_connection() {
  nout << "Lost connection to " << get_client_hostname() << "\n";

  shutdown();
}

/**
 * If has_idle() returns true, this will be called periodically to allow the
 * monitor to update its display or whatever it needs to do.
 */
void GtkStatsMonitor::
idle() {
  // Check if any of our chart menus need updating.
  ChartMenus::iterator mi;
  for (mi = _chart_menus.begin(); mi != _chart_menus.end(); ++mi) {
    (*mi)->check_update();
  }

  // Update the frame rate label from the main thread (thread 0).
  const PStatThreadData *thread_data = get_client_data()->get_thread_data(0);
  double frame_rate = thread_data->get_frame_rate();
  if (frame_rate != 0.0f) {
    char buffer[128];
    sprintf(buffer, "%0.1f ms / %0.1f Hz", 1000.0f / frame_rate, frame_rate);

    gtk_label_set_text(GTK_LABEL(_frame_rate_label), buffer);
  }
}

/**
 * Should be redefined to return true if you want to redefine idle() and
 * expect it to be called.
 */
bool GtkStatsMonitor::
has_idle() {
  return true;
}

/**
 * Called when the user guide bars have been changed.
 */
void GtkStatsMonitor::
user_guide_bars_changed() {
  Graphs::iterator gi;
  for (gi = _graphs.begin(); gi != _graphs.end(); ++gi) {
    GtkStatsGraph *graph = (*gi);
    graph->user_guide_bars_changed();
  }
}

/**
 * Returns the window handle to the monitor's window.
 */
GtkWidget *GtkStatsMonitor::
get_window() const {
  return _window;
}

/**
 * Opens a new strip chart showing the indicated data.
 */
void GtkStatsMonitor::
open_strip_chart(int thread_index, int collector_index, bool show_level) {
  GtkStatsStripChart *graph =
    new GtkStatsStripChart(this, thread_index, collector_index, show_level);
  add_graph(graph);

  graph->set_time_units(_time_units);
  graph->set_scroll_speed(_scroll_speed);
  graph->set_pause(_pause);
}

/**
 * Opens a new piano roll showing the indicated data.
 */
void GtkStatsMonitor::
open_piano_roll(int thread_index) {
  GtkStatsPianoRoll *graph = new GtkStatsPianoRoll(this, thread_index);
  add_graph(graph);

  graph->set_time_units(_time_units);
  graph->set_scroll_speed(_scroll_speed);
  graph->set_pause(_pause);
}

/**
 * Adds a new MenuDef to the monitor, or returns an existing one if there is
 * already one just like it.
 */
const GtkStatsMonitor::MenuDef *GtkStatsMonitor::
add_menu(const MenuDef &menu_def) {
  std::pair<Menus::iterator, bool> result = _menus.insert(menu_def);
  Menus::iterator mi = result.first;
  const GtkStatsMonitor::MenuDef &new_menu_def = (*mi);
  if (result.second) {
    // A new MenuDef was inserted.
    ((GtkStatsMonitor::MenuDef &)new_menu_def)._monitor = this;
  }
  return &new_menu_def;
}

/**
 * Called when the user selects a new time units from the monitor pulldown
 * menu, this should adjust the units for all graphs to the indicated mask if
 * it is a time-based graph.
 */
void GtkStatsMonitor::
set_time_units(int unit_mask) {
  _time_units = unit_mask;

  // First, change all of the open graphs appropriately.
  Graphs::iterator gi;
  for (gi = _graphs.begin(); gi != _graphs.end(); ++gi) {
    GtkStatsGraph *graph = (*gi);
    graph->set_time_units(_time_units);
  }
}

/**
 * Called when the user selects a new scroll speed from the monitor pulldown
 * menu, this should adjust the speeds for all graphs to the indicated value.
 */
void GtkStatsMonitor::
set_scroll_speed(double scroll_speed) {
  _scroll_speed = scroll_speed;

  // First, change all of the open graphs appropriately.
  Graphs::iterator gi;
  for (gi = _graphs.begin(); gi != _graphs.end(); ++gi) {
    GtkStatsGraph *graph = (*gi);
    graph->set_scroll_speed(_scroll_speed);
  }
}

/**
 * Called when the user selects a pause on or pause off option from the menu.
 */
void GtkStatsMonitor::
set_pause(bool pause) {
  _pause = pause;

  // First, change all of the open graphs appropriately.
  Graphs::iterator gi;
  for (gi = _graphs.begin(); gi != _graphs.end(); ++gi) {
    GtkStatsGraph *graph = (*gi);
    graph->set_pause(_pause);
  }
}

/**
 * Adds the newly-created graph to the list of managed graphs.
 */
void GtkStatsMonitor::
add_graph(GtkStatsGraph *graph) {
  _graphs.insert(graph);
}

/**
 * Deletes the indicated graph.
 */
void GtkStatsMonitor::
remove_graph(GtkStatsGraph *graph) {
  Graphs::iterator gi = _graphs.find(graph);
  if (gi != _graphs.end()) {
    _graphs.erase(gi);
    delete graph;
  }
}

/**
 * Creates the window for this monitor.
 */
void GtkStatsMonitor::
create_window() {
  if (_window != nullptr) {
    return;
  }

  _window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  g_signal_connect(G_OBJECT(_window), "delete_event",
       G_CALLBACK(window_delete_event), this);
  g_signal_connect(G_OBJECT(_window), "destroy",
       G_CALLBACK(window_destroy), this);

  _window_title = get_client_progname() + " on " + get_client_hostname();
  gtk_window_set_title(GTK_WINDOW(_window), _window_title.c_str());

  gtk_window_set_default_size(GTK_WINDOW(_window), 500, 360);

  // Set up the menu.
  GtkAccelGroup *accel_group = gtk_accel_group_new();
  _item_factory =
    gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<PStats>", accel_group);
  gtk_item_factory_create_items(_item_factory, num_menu_entries, menu_entries,
        this);
  gtk_window_add_accel_group(GTK_WINDOW(_window), accel_group);
  GtkWidget *menu_bar = gtk_item_factory_get_widget(_item_factory, "<PStats>");
  _next_chart_index = 2;

  setup_frame_rate_label();

  ChartMenus::iterator mi;
  for (mi = _chart_menus.begin(); mi != _chart_menus.end(); ++mi) {
    (*mi)->add_to_menu_bar(menu_bar, _next_chart_index);
    ++_next_chart_index;
  }

  // Pack the menu into the window.
  GtkWidget *main_vbox = gtk_vbox_new(FALSE, 1);
  gtk_container_add(GTK_CONTAINER(_window), main_vbox);
  gtk_box_pack_start(GTK_BOX(main_vbox), menu_bar, FALSE, TRUE, 0);

  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item(_item_factory, "/Speed/3")),
         TRUE);
  set_scroll_speed(3);

  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gtk_item_factory_get_item(_item_factory, "/Options/Units/ms")),
         TRUE);
  set_time_units(PStatGraph::GBU_ms);

  gtk_widget_show_all(_window);
  gtk_widget_show(_window);

  set_pause(false);
}

/**
 * Closes all the graphs associated with this monitor.
 */
void GtkStatsMonitor::
shutdown() {
  Graphs::iterator gi;
  for (gi = _graphs.begin(); gi != _graphs.end(); ++gi) {
    delete (*gi);
  }
  _graphs.clear();

  ChartMenus::iterator mi;
  for (mi = _chart_menus.begin(); mi != _chart_menus.end(); ++mi) {
    delete (*mi);
  }
  _chart_menus.clear();

  if (_window != nullptr) {
    gtk_widget_destroy(_window);
    _window = nullptr;
  }

#ifdef DEVELOP_GTKSTATS
  // For GtkStats developers, exit when the first monitor closes.
  gtk_main_quit();
#endif
}

/**
 * Callback when the window is closed by the user.
 */
gboolean GtkStatsMonitor::
window_delete_event(GtkWidget *widget, GdkEvent *event, gpointer data) {
  // Returning FALSE to indicate we should destroy the window when the user
  // selects "close".
  return FALSE;
}

/**
 * Callback when the window is destroyed by the system (or by delete_event).
 */
void GtkStatsMonitor::
window_destroy(GtkWidget *widget, gpointer data) {
  GtkStatsMonitor *self = (GtkStatsMonitor *)data;
  self->close();
}

/**
 * Creates the frame rate label on the right end of the menu bar.  This is
 * used as a text label to display the main thread's frame rate to the user,
 * although it is implemented as a right-justified toplevel menu item that
 * doesn't open to anything.
 */
void GtkStatsMonitor::
setup_frame_rate_label() {
  GtkWidget *menu_bar = gtk_item_factory_get_widget(_item_factory, "<PStats>");

  _frame_rate_menu_item = gtk_menu_item_new();
  _frame_rate_label = gtk_label_new("");
  gtk_container_add(GTK_CONTAINER(_frame_rate_menu_item), _frame_rate_label);

  gtk_widget_show(_frame_rate_menu_item);
  gtk_widget_show(_frame_rate_label);
  gtk_menu_item_right_justify(GTK_MENU_ITEM(_frame_rate_menu_item));

  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), _frame_rate_menu_item);
}

/**
 *
 */
void GtkStatsMonitor::
handle_menu_command(gpointer callback_data, guint menu_id, GtkWidget *widget) {
  GtkStatsMonitor *self = (GtkStatsMonitor *)callback_data;
  switch (menu_id) {
  case MI_none:
    break;

  case MI_time_ms:
    self->set_time_units(PStatGraph::GBU_ms);
    break;

  case MI_time_hz:
    self->set_time_units(PStatGraph::GBU_hz);
    break;

  case MI_speed_1:
    self->set_scroll_speed(1);
    break;

  case MI_speed_2:
    self->set_scroll_speed(2);
    break;

  case MI_speed_3:
    self->set_scroll_speed(3);
    break;

  case MI_speed_6:
    self->set_scroll_speed(6);
    break;

  case MI_speed_12:
    self->set_scroll_speed(12);
    break;

  case MI_pause:
    self->set_pause(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget)));
    break;
  }
}
