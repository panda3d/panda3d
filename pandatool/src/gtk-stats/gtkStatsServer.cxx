/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file gtkStatsServer.cxx
 * @author drose
 * @date 2006-01-16
 */

#include "gtkStatsServer.h"
#include "gtkStatsMonitor.h"
#include "pandaVersion.h"
#include "pStatGraph.h"
#include "config_pstatclient.h"

#include <unistd.h>

/**
 *
 */
GtkStatsServer::
GtkStatsServer() : _port(pstats_port) {
  set_program_brief("GTK+3-based PStats client");
  set_program_description
    ("This is a GUI-based PStats server that listens on a TCP port for a "
     "connection from a PStatClient in a Panda3D application.  It offers "
     "various graphs for showing the timing information sent by the client."
     "\n\n"
     "The full documentation is available online:\n  "
#ifdef HAVE_PYTHON
     "https://docs.panda3d.org/" PANDA_ABI_VERSION_STR "/python/optimization/pstats"
#else
     "https://docs.panda3d.org/" PANDA_ABI_VERSION_STR "/cpp/optimization/pstats"
#endif
     "");

  add_option
    ("p", "port", 0,
     "Specify the TCP port to listen for connections on.  By default, this "
     "is taken from the pstats-port Config variable.",
     &ProgramBase::dispatch_int, nullptr, &_port);

  add_runline("[-p 5185]");
  add_runline("session.pstats");

#ifdef __APPLE__
  _last_session = Filename::expand_from(
    "$HOME/Library/Caches/Panda3D-" PANDA_ABI_VERSION_STR "/last-session.pstats");
#else
  _last_session = Filename::expand_from("$XDG_STATE_HOME/panda3d/last-session.pstats");
#endif
  _last_session.set_binary();

  create_window();
}

/**
 * Does something with the additional arguments on the command line (after all
 * the -options have been parsed).  Returns true if the arguments are good,
 * false otherwise.
 */
bool GtkStatsServer::
handle_args(ProgramBase::Args &args) {
  if (args.empty()) {
    new_session();
    return true;
  }
  else if (args.size() == 1) {
    Filename fn = Filename::from_os_specific(args[0]);
    fn.set_binary();
    GtkStatsMonitor *monitor = new GtkStatsMonitor(this);
    if (!monitor->read(fn)) {
      delete monitor;

      // If we're not running from the terminal, show a GUI message box.
      if (!isatty(STDERR_FILENO)) {
        GtkWidget *dialog =
          gtk_message_dialog_new(GTK_WINDOW(_window),
             GTK_DIALOG_DESTROY_WITH_PARENT,
             GTK_MESSAGE_ERROR,
             GTK_BUTTONS_CLOSE,
             "Failed to load session file: %s", fn.c_str());
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
      }
      return false;
    }
    _save_filename = fn;

    gtk_widget_set_sensitive(_new_session_menu_item, TRUE);
    gtk_widget_set_sensitive(_save_session_menu_item, TRUE);
    gtk_widget_set_sensitive(_close_session_menu_item, TRUE);
    gtk_widget_set_sensitive(_export_session_menu_item, TRUE);

    _monitor = monitor;
    return true;
  }
  else {
    nout << "At most one filename may be specified on the command-line.\n";
    return false;
  }
}

/**
 *
 */
PStatMonitor *GtkStatsServer::
make_monitor(const NetAddress &address) {
  // Enable the "New Session", "Save Session" and "Close Session" menu items.
  gtk_widget_set_sensitive(_new_session_menu_item, TRUE);
  gtk_widget_set_sensitive(_save_session_menu_item, TRUE);
  gtk_widget_set_sensitive(_close_session_menu_item, TRUE);
  gtk_widget_set_sensitive(_export_session_menu_item, TRUE);

  std::ostringstream strm;
  strm << "PStats Server (connected to " << address << ")";
  std::string title = strm.str();
  gtk_window_set_title(GTK_WINDOW(_window), title.c_str());

  if (_status_bar_label != nullptr) {
    gtk_container_remove(GTK_CONTAINER(_status_bar), _status_bar_label);
    g_object_unref(_status_bar_label);
    _status_bar_label = nullptr;
  }

  _monitor = new GtkStatsMonitor(this);
  return _monitor;
}

/**
 * Called when connection has been lost.
 */
void GtkStatsServer::
lost_connection(PStatMonitor *monitor) {
  if (_monitor != nullptr && !_monitor->_have_data) {
    // We didn't have any data yet.  Just silently restart the session.
    _monitor->close();
    _monitor = nullptr;
    if (new_session()) {
      return;
    }
  } else {
    // Store a backup now, in case PStats crashes or something.
    _last_session.make_dir();
    if (monitor->write(_last_session)) {
      nout << "Wrote to " << _last_session << "\n";
    } else {
      nout << "Failed to write to " << _last_session << "\n";
    }
  }

  stop_listening();

  gtk_window_set_title(GTK_WINDOW(_window), "PStats Server (disconnected)");
}

/**
 * Starts a new session.
 */
bool GtkStatsServer::
new_session() {
  if (!close_session()) {
    return false;
  }

  if (listen(_port)) {
    {
      std::ostringstream strm;
      strm << "PStats Server (listening on port " << _port << ")";
      std::string title = strm.str();
      gtk_window_set_title(GTK_WINDOW(_window), title.c_str());
    }
    {
      std::ostringstream strm;
      strm << "Waiting for client to connect on port " << _port << "...";
      std::string title = strm.str();
      // As workaround for https://gitlab.gnome.org/GNOME/gtk/-/merge_requests/2029
      // We manually create a GtkFlowBoxChild instance and attach the label to it
      // and increase its reference count so it it not prematurely destroyed when
      // removed from the GtkFlowBox, causing the app to segfault.
      GtkWidget * label = gtk_label_new(title.c_str());
      _status_bar_label = gtk_flow_box_child_new();
      gtk_container_add(GTK_CONTAINER(_status_bar_label), label);
      gtk_container_add(GTK_CONTAINER(_status_bar), _status_bar_label);
      gtk_widget_show(_status_bar_label);
      g_object_ref(_status_bar_label);
    }

    gtk_widget_set_sensitive(_new_session_menu_item, FALSE);
    gtk_widget_set_sensitive(_save_session_menu_item, FALSE);
    gtk_widget_set_sensitive(_close_session_menu_item, TRUE);
    gtk_widget_set_sensitive(_export_session_menu_item, FALSE);

    return true;
  }

  gtk_window_set_title(GTK_WINDOW(_window), "PStats Server");

  GtkWidget *dialog =
    gtk_message_dialog_new(GTK_WINDOW(_window),
       GTK_DIALOG_DESTROY_WITH_PARENT,
       GTK_MESSAGE_ERROR,
       GTK_BUTTONS_CLOSE,
       "Unable to open port %d.  Try specifying a different port number "
       "using pstats-port in your Config file or the -p option on the "
       "command-line.", _port);
  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);

  return false;
}

/**
 * Offers to open an existing session.
 */
bool GtkStatsServer::
open_session() {
  if (!close_session()) {
    return false;
  }

  GtkFileChooserNative *native = gtk_file_chooser_native_new(
    "Open Session",
    GTK_WINDOW(_window),
    GTK_FILE_CHOOSER_ACTION_SAVE,
    "_Open", "Cancel");
  GtkFileChooser *chooser = GTK_FILE_CHOOSER(native);

  GtkFileFilter *filter = gtk_file_filter_new();
  gtk_file_filter_set_name(filter, "PStats Session Files");
  gtk_file_filter_add_pattern(filter, "*.pstats");
  gtk_file_chooser_add_filter(chooser, filter);

  gint res = gtk_native_dialog_run(GTK_NATIVE_DIALOG(native));
  if (res == GTK_RESPONSE_ACCEPT) {
    char *buffer = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(chooser));
    Filename fn = Filename::from_os_specific(buffer);
    fn.set_binary();
    g_free(buffer);
    g_object_unref(native);

    GtkStatsMonitor *monitor = new GtkStatsMonitor(this);
    if (!monitor->read(fn)) {
      delete monitor;

      GtkWidget *dialog =
        gtk_message_dialog_new(GTK_WINDOW(_window),
           GTK_DIALOG_DESTROY_WITH_PARENT,
           GTK_MESSAGE_ERROR,
           GTK_BUTTONS_CLOSE,
           "Failed to load session file: %s", fn.c_str());
      gtk_dialog_run(GTK_DIALOG(dialog));
      gtk_widget_destroy(dialog);
      return false;
    }
    _save_filename = fn;

    gtk_widget_set_sensitive(_new_session_menu_item, TRUE);
    gtk_widget_set_sensitive(_save_session_menu_item, TRUE);
    gtk_widget_set_sensitive(_close_session_menu_item, TRUE);
    gtk_widget_set_sensitive(_export_session_menu_item, TRUE);

    _monitor = monitor;
    return true;
  }

  g_object_unref(native);
  return false;
}

/**
 * Opens the last session, if any.
 */
bool GtkStatsServer::
open_last_session() {
  if (!close_session()) {
    return false;
  }

  Filename fn = _last_session;
  GtkStatsMonitor *monitor = new GtkStatsMonitor(this);
  if (!monitor->read(fn)) {
    delete monitor;

    GtkWidget *dialog =
      gtk_message_dialog_new(GTK_WINDOW(_window),
         GTK_DIALOG_DESTROY_WITH_PARENT,
         GTK_MESSAGE_ERROR,
         GTK_BUTTONS_CLOSE,
         "Failed to load session file: %s", fn.c_str());
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

    return false;
  }
  _monitor = monitor;

  // Enable the "New Session", "Save Session" and "Close Session" menu items.
  gtk_widget_set_sensitive(_new_session_menu_item, TRUE);
  gtk_widget_set_sensitive(_save_session_menu_item, TRUE);
  gtk_widget_set_sensitive(_close_session_menu_item, TRUE);
  gtk_widget_set_sensitive(_export_session_menu_item, TRUE);

  // If the file contained no graphs, open the default graphs.
  if (monitor->_graphs.empty()) {
    monitor->open_default_graphs();
  }

  return true;
}

/**
 * Offers to save the current session.
 */
bool GtkStatsServer::
save_session() {
  nassertr_always(_monitor != nullptr, true);

  GtkFileChooserNative *native = gtk_file_chooser_native_new(
    "Save Session",
    GTK_WINDOW(_window),
    GTK_FILE_CHOOSER_ACTION_SAVE,
    "_Save", "Cancel");
  GtkFileChooser *chooser = GTK_FILE_CHOOSER(native);

  gtk_file_chooser_set_do_overwrite_confirmation(chooser, TRUE);

  GtkFileFilter *filter = gtk_file_filter_new();
  gtk_file_filter_set_name(filter, "PStats Session Files");
  gtk_file_filter_add_pattern(filter, "*.pstats");
  gtk_file_chooser_add_filter(chooser, filter);

  if (_save_filename.empty()) {
    gtk_file_chooser_set_current_name(chooser, "session.pstats");
  }
  else {
    gtk_file_chooser_set_filename(chooser, _save_filename.c_str());
  }

  gint res = gtk_native_dialog_run(GTK_NATIVE_DIALOG(native));
  if (res == GTK_RESPONSE_ACCEPT) {
    char *buffer = gtk_file_chooser_get_filename(chooser);
    Filename fn = Filename::from_os_specific(buffer);
    fn.set_binary();
    g_free(buffer);
    g_object_unref(native);

    if (!_monitor->write(fn)) {
      GtkWidget *dialog =
        gtk_message_dialog_new(GTK_WINDOW(_window),
           GTK_DIALOG_DESTROY_WITH_PARENT,
           GTK_MESSAGE_ERROR,
           GTK_BUTTONS_CLOSE,
           "Failed to save session file: %s", fn.c_str());
      gtk_dialog_run(GTK_DIALOG(dialog));
      gtk_widget_destroy(dialog);
      return false;
    }
    _save_filename = fn;
    _monitor->get_client_data()->clear_dirty();
    return true;
  }

  g_object_unref(native);
  return false;
}

/**
 * Offers to export the current session as a JSON file.
 */
bool GtkStatsServer::
export_session() {
  nassertr_always(_monitor != nullptr, true);

  GtkFileChooserNative *native = gtk_file_chooser_native_new(
    "Export Session",
    GTK_WINDOW(_window),
    GTK_FILE_CHOOSER_ACTION_SAVE,
    "_Export", "Cancel");
  GtkFileChooser *chooser = GTK_FILE_CHOOSER(native);

  gtk_file_chooser_set_do_overwrite_confirmation(chooser, TRUE);

  GtkFileFilter *filter = gtk_file_filter_new();
  gtk_file_filter_set_name(filter, "JSON files");
  gtk_file_filter_add_pattern(filter, "*.json");
  gtk_file_chooser_add_filter(chooser, filter);

  gtk_file_chooser_set_current_name(chooser, "session.json");

  gint res = gtk_native_dialog_run(GTK_NATIVE_DIALOG(native));
  if (res == GTK_RESPONSE_ACCEPT) {
    char *buffer = gtk_file_chooser_get_filename(chooser);
    Filename fn = Filename::from_os_specific(buffer);
    fn.set_text();
    g_free(buffer);
    g_object_unref(native);

    std::ofstream stream;
    if (!fn.open_write(stream)) {
      GtkWidget *dialog =
        gtk_message_dialog_new(GTK_WINDOW(_window),
           GTK_DIALOG_DESTROY_WITH_PARENT,
           GTK_MESSAGE_ERROR,
           GTK_BUTTONS_CLOSE,
           "Failed to open file for export: %s", fn.c_str());
      gtk_dialog_run(GTK_DIALOG(dialog));
      gtk_widget_destroy(dialog);
      return false;
    }

    int pid = _monitor->get_client_pid();
    _monitor->get_client_data()->write_json(stream, std::max(0, pid));
    stream.close();
    return true;
  }

  g_object_unref(native);
  return false;
}

/**
 * Closes the current session.
 */
bool GtkStatsServer::
close_session() {
  bool wrote_last_session = false;

  if (_monitor != nullptr) {
    const PStatClientData *client_data = _monitor->get_client_data();
    if (client_data != nullptr && client_data->is_dirty()) {
      if (!_monitor->has_read_filename()) {
        _last_session.make_dir();
        if (_monitor->write(_last_session)) {
          nout << "Wrote to " << _last_session << "\n";
          wrote_last_session = true;
        }
        else {
          nout << "Failed to write to " << _last_session << "\n";
        }
      }

      GtkWidget *dialog =
        gtk_message_dialog_new(GTK_WINDOW(_window),
           GTK_DIALOG_DESTROY_WITH_PARENT,
           GTK_MESSAGE_QUESTION,
           GTK_BUTTONS_YES_NO,
           "Would you like to save the currently open session?");
      gint response = gtk_dialog_run(GTK_DIALOG(dialog));
      gtk_widget_destroy(dialog);

      if (response == GTK_RESPONSE_CANCEL ||
          (response == GTK_RESPONSE_YES && !save_session())) {
        return false;
      }
    }

    _monitor->close();
    _monitor = nullptr;
  }

  _save_filename = Filename();
  stop_listening();

  gtk_window_set_title(GTK_WINDOW(_window), "PStats Server");

  if (_status_bar_label != nullptr) {
    gtk_container_remove(GTK_CONTAINER(_status_bar), _status_bar_label);
    g_object_unref(_status_bar_label);
    _status_bar_label = nullptr;
  }

  gtk_widget_set_sensitive(_new_session_menu_item, TRUE);
  if (wrote_last_session) {
    gtk_widget_set_sensitive(_open_last_session_menu_item, TRUE);
  }
  gtk_widget_set_sensitive(_save_session_menu_item, FALSE);
  gtk_widget_set_sensitive(_close_session_menu_item, FALSE);
  gtk_widget_set_sensitive(_export_session_menu_item, FALSE);
  return true;
}

/**
 * Returns the window handle to the server's window.
 */
GtkWidget *GtkStatsServer::
get_window() const {
  return _window;
}

/**
 * Returns the server window's accelerator group.
 */
GtkAccelGroup *GtkStatsServer::
get_accel_group() const {
  return _accel_group;
}

/**
 * Returns the menu handle to the server's menu bar.
 */
GtkWidget *GtkStatsServer::
get_menu_bar() const {
  return _menu_bar;
}

/**
 * Returns the window handle to the server's status bar.
 */
GtkWidget *GtkStatsServer::
get_status_bar() const {
  return _status_bar;
}

/**
 *
 */
int GtkStatsServer::
get_time_units() const {
  return _time_units;
}

/**
 * Called when the user selects a new time units from the monitor pulldown
 * menu, this should adjust the units for all graphs to the indicated mask if
 * it is a time-based graph.
 */
void GtkStatsServer::
set_time_units(int unit_mask) {
  _time_units = unit_mask;

  if (_monitor != nullptr) {
    _monitor->set_time_units(unit_mask);
  }
}

/**
 * Creates the window for this monitor.
 */
void GtkStatsServer::
create_window() {
  _window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(_window), "PStats Server");
  gtk_window_set_default_size(GTK_WINDOW(_window), 500, 360);

  // Connect the delete and destroy events, so the user can exit the
  // application by closing the main window.
  g_signal_connect(G_OBJECT(_window), "delete_event",
    G_CALLBACK(+[](GtkWidget *widget, GdkEvent *event, gpointer data) -> gboolean {
      GtkStatsServer *self = (GtkStatsServer *)data;
      return self->close_session() ? FALSE : TRUE;
    }), this);

  g_signal_connect(G_OBJECT(_window), "destroy",
    G_CALLBACK(+[](GtkWidget *widget, GdkEvent *event, gpointer data) -> gboolean {
      gtk_main_quit();
      return FALSE;
    }), this);

  // Set up the menu.
  _accel_group = gtk_accel_group_new();
  gtk_window_add_accel_group(GTK_WINDOW(_window), _accel_group);
  _menu_bar = gtk_menu_bar_new();

  setup_session_menu();
  setup_options_menu();

  // Pack the menu into the window.
  GtkWidget *main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
  gtk_container_add(GTK_CONTAINER(_window), main_vbox);
  gtk_box_pack_start(GTK_BOX(main_vbox), _menu_bar, FALSE, TRUE, 0);

  // Create the status bar.
  _status_bar = gtk_flow_box_new();
  gtk_flow_box_set_activate_on_single_click(GTK_FLOW_BOX(_status_bar), FALSE);
  gtk_flow_box_set_selection_mode(GTK_FLOW_BOX(_status_bar), GTK_SELECTION_NONE);
  g_signal_connect(G_OBJECT(_status_bar), "button_press_event",
    G_CALLBACK(status_bar_button_event), this);
  gtk_box_pack_end(GTK_BOX(main_vbox), _status_bar, FALSE, FALSE, 0);

  GtkWidget *sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
  gtk_box_pack_end(GTK_BOX(main_vbox), sep, FALSE, FALSE, 0);

  gtk_widget_show_all(_window);
  gtk_widget_show(_window);
  gtk_widget_realize(_window);

  // Set up a timer to poll the pstats every so often.
  g_timeout_add(200, +[](gpointer data) -> gboolean {
    GtkStatsServer *self = (GtkStatsServer *)data;
    self->poll();
    return TRUE;
  }, this);
}

/**
 * Creates the "Session" pulldown menu.
 */
void GtkStatsServer::
setup_session_menu() {
  _session_menu = gtk_menu_new();

  GtkWidget *item = gtk_menu_item_new_with_mnemonic("_Session");
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), _session_menu);
  gtk_menu_shell_append(GTK_MENU_SHELL(_menu_bar), item);

  item = gtk_menu_item_new_with_mnemonic("_New Session");
  _new_session_menu_item = item;
  gtk_menu_shell_append(GTK_MENU_SHELL(_session_menu), item);
  g_signal_connect(G_OBJECT(item), "activate",
    G_CALLBACK(+[](GtkMenuItem *item, gpointer data) {
      GtkStatsServer *self = (GtkStatsServer *)data;
      self->new_session();
    }), this);
  gtk_widget_add_accelerator(item, "activate", _accel_group,
                             GDK_KEY_n, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

  item = gtk_menu_item_new_with_mnemonic("_Open Session...");
  gtk_menu_shell_append(GTK_MENU_SHELL(_session_menu), item);
  g_signal_connect(G_OBJECT(item), "activate",
    G_CALLBACK(+[](GtkMenuItem *item, gpointer data) {
      GtkStatsServer *self = (GtkStatsServer *)data;
      self->open_session();
    }), this);
  gtk_widget_add_accelerator(item, "activate", _accel_group,
                             GDK_KEY_o, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

  item = gtk_menu_item_new_with_mnemonic("Open _Last Session");
  _open_last_session_menu_item = item;
  gtk_menu_shell_append(GTK_MENU_SHELL(_session_menu), item);
  g_signal_connect(G_OBJECT(item), "activate",
    G_CALLBACK(+[](GtkMenuItem *item, gpointer data) {
      GtkStatsServer *self = (GtkStatsServer *)data;
      self->open_last_session();
    }), this);

  if (!_last_session.exists()) {
    gtk_widget_set_sensitive(item, FALSE);
  }

  item = gtk_menu_item_new_with_mnemonic("_Save Session...");
  _save_session_menu_item = item;
  gtk_widget_set_sensitive(item, FALSE);
  gtk_menu_shell_append(GTK_MENU_SHELL(_session_menu), item);
  g_signal_connect(G_OBJECT(item), "activate",
    G_CALLBACK(+[](GtkMenuItem *item, gpointer data) {
      GtkStatsServer *self = (GtkStatsServer *)data;
      self->save_session();
    }), this);
  gtk_widget_add_accelerator(item, "activate", _accel_group,
                             GDK_KEY_s, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

  item = gtk_menu_item_new_with_mnemonic("_Close Session");
  _close_session_menu_item = item;
  gtk_widget_set_sensitive(item, FALSE);
  gtk_menu_shell_append(GTK_MENU_SHELL(_session_menu), item);
  g_signal_connect(G_OBJECT(item), "activate",
    G_CALLBACK(+[](GtkMenuItem *item, gpointer data) {
      GtkStatsServer *self = (GtkStatsServer *)data;
      self->close_session();
    }), this);
  gtk_widget_add_accelerator(item, "activate", _accel_group,
                             GDK_KEY_w, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

  GtkWidget *sep = gtk_separator_menu_item_new();
  gtk_menu_shell_append(GTK_MENU_SHELL(_session_menu), sep);

  item = gtk_menu_item_new_with_mnemonic("_Export as JSON...");
  _export_session_menu_item = item;
  gtk_widget_set_sensitive(item, FALSE);
  gtk_menu_shell_append(GTK_MENU_SHELL(_session_menu), item);
  g_signal_connect(G_OBJECT(item), "activate",
    G_CALLBACK(+[](GtkMenuItem *item, gpointer data) {
      GtkStatsServer *self = (GtkStatsServer *)data;
      self->export_session();
    }), this);

  sep = gtk_separator_menu_item_new();
  gtk_menu_shell_append(GTK_MENU_SHELL(_session_menu), sep);

  item = gtk_menu_item_new_with_mnemonic("E_xit");
  gtk_menu_shell_append(GTK_MENU_SHELL(_session_menu), item);
  g_signal_connect(G_OBJECT(item), "activate",
    G_CALLBACK(+[](GtkMenuItem *item, gpointer data) {
      GtkStatsServer *self = (GtkStatsServer *)data;
      if (self->close_session()) {
        gtk_main_quit();
      }
    }), this);
  gtk_widget_add_accelerator(item, "activate", _accel_group,
                             GDK_KEY_q, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

  gtk_widget_show_all(_session_menu);
}

/**
 * Creates the "Options" pulldown menu.
 */
void GtkStatsServer::
setup_options_menu() {
  _options_menu = gtk_menu_new();

  GtkWidget *item = gtk_menu_item_new_with_label("Options");
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), _options_menu);
  gtk_menu_shell_append(GTK_MENU_SHELL(_menu_bar), item);

  GtkWidget *units_menu = gtk_menu_new();
  item = gtk_menu_item_new_with_label("Units");
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), units_menu);
  gtk_menu_shell_append(GTK_MENU_SHELL(_options_menu), item);

  item = gtk_radio_menu_item_new_with_label(nullptr, "ms");
  gtk_menu_shell_append(GTK_MENU_SHELL(units_menu), item);
  g_signal_connect(G_OBJECT(item), "activate",
    G_CALLBACK(+[](GtkMenuItem *item, gpointer data) {
      GtkStatsServer *self = (GtkStatsServer *)data;
      self->set_time_units(PStatGraph::GBU_ms);
    }), this);

  item = gtk_radio_menu_item_new_with_label(
    gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(item)), "Hz");
  gtk_menu_shell_append(GTK_MENU_SHELL(units_menu), item);
  g_signal_connect(G_OBJECT(item), "activate",
    G_CALLBACK(+[](GtkMenuItem *item, gpointer data) {
      GtkStatsServer *self = (GtkStatsServer *)data;
      self->set_time_units(PStatGraph::GBU_hz);
    }), this);

  set_time_units(PStatGraph::GBU_ms);
}

/**
 * Handles clicks on a partion of the status bar.
 */
gboolean GtkStatsServer::
status_bar_button_event(GtkWidget *widget, GdkEventButton *event, gpointer data) {
  GtkStatsServer *server = (GtkStatsServer *)data;

  GtkFlowBoxChild *child = gtk_flow_box_get_child_at_pos(
    GTK_FLOW_BOX(server->_status_bar), event->x, event->y);
  if (child == nullptr) {
    return FALSE;
  }

  // Which child is this?
  GList *children = gtk_container_get_children(GTK_CONTAINER(server->_status_bar));
  int index = g_list_index(children, child);
  g_list_free(children);
  if (index < 0) {
    return FALSE;
  }

  if (event->type == GDK_2BUTTON_PRESS && event->button == 1) {
    server->_monitor->handle_status_bar_click(index);
    return TRUE;
  }
  else if (event->type == GDK_BUTTON_PRESS && event->button == 3 && index > 0) {
    server->_monitor->handle_status_bar_popup(index);
    return TRUE;
  }
  return FALSE;
}
