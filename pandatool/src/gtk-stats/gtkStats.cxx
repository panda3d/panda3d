/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file gtkStats.cxx
 * @author drose
 * @date 2006-01-16
 */

#include "pandatoolbase.h"
#include "gtkStats.h"
#include "gtkStatsServer.h"
#include "config_pstatclient.h"

GtkWidget *main_window;
static GtkStatsServer *server = nullptr;

static gboolean
delete_event(GtkWidget *widget,
       GdkEvent *event, gpointer data) {
  // Returning FALSE to indicate we should destroy the main window when the
  // user selects "close".
  return FALSE;
}

static void
destroy(GtkWidget *widget, gpointer data) {
  gtk_main_quit();
}

static gboolean
timer(gpointer data) {
  static int count = 0;
  server->poll();

  if (++count == 5) {
    count = 0;
    // Every once in a while, say once a second, we call this function, which
    // should force gdk to make all changes visible.  We do this in case we
    // are getting starved and falling behind, so that the user still gets a
    // chance to see *something* happen onscreen, even if it's just
    // increasingly old data.
    gdk_window_process_all_updates();
  }

  return TRUE;
}

int
main(int argc, char *argv[]) {
  gtk_init(&argc, &argv);

  main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  gtk_window_set_title(GTK_WINDOW(main_window), "PStats");

  // Connect the delete and destroy events, so the user can exit the
  // application by closing the main window.
  g_signal_connect(G_OBJECT(main_window), "delete_event",
       G_CALLBACK(delete_event), nullptr);

  g_signal_connect(G_OBJECT(main_window), "destroy",
       G_CALLBACK(destroy), nullptr);

  std::ostringstream stream;
  stream << "Listening on port " << pstats_port;
  std::string str = stream.str();
  GtkWidget *label = gtk_label_new(str.c_str());
  gtk_container_add(GTK_CONTAINER(main_window), label);
  gtk_widget_show(label);

  // Create the server object.
  server = new GtkStatsServer;
  if (!server->listen()) {
    std::ostringstream stream;
    stream
      << "Unable to open port " << pstats_port
      << ".  Try specifying a different\n"
      << "port number using pstats-port in your Config file.";
    std::string str = stream.str();

    GtkWidget *dialog =
      gtk_message_dialog_new(GTK_WINDOW(main_window),
           GTK_DIALOG_DESTROY_WITH_PARENT,
           GTK_MESSAGE_ERROR,
           GTK_BUTTONS_CLOSE,
           "%s", str.c_str());
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    exit(1);
  }

  gtk_widget_show(main_window);

  // Set up a timer to poll the pstats every so often.
  g_timeout_add(200, timer, nullptr);

  // Now get lost in the message loop.
  gtk_main();

  return (0);
}
