// Filename: gtkStatsMainWindow.cxx
// Created by:  drose (14Jul00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "gtkStatsMainWindow.h"
#include "gtkStats.h"
#include "gtkStatsServer.h"

#include "string_utils.h"

#include <signal.h>

static bool user_interrupted = false;

// This simple signal handler lets us know when the user has pressed
// control-C, so we can clean up nicely.
static void signal_handler(int) {
  user_interrupted = true;
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsMainWindow::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
GtkStatsMainWindow::
GtkStatsMainWindow(int port) : _port(port) {
  nassertv(GtkStats::_main_window == (GtkStatsMainWindow *)NULL);
  GtkStats::_main_window = this;

  // Set up a global signal handler to catch Interrupt (Control-C) so
  // we can clean up nicely if the user stops us.
  signal(SIGINT, &signal_handler);

  _server = new GtkStatsServer;
  if (!_server->listen(_port)) {
    nout << "Unable to open port.\n";
    exit(1);
  }

  layout_window();
  setup();

  Gtk::Main::timeout.
    connect(slot(this, &GtkStatsMainWindow::idle_callback), 200);
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsMainWindow::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
GtkStatsMainWindow::
~GtkStatsMainWindow() {
  nassertv(GtkStats::_main_window == this);
  GtkStats::_main_window = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsMainWindow::destruct
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
bool GtkStatsMainWindow::
destruct() {
  if (BasicGtkWindow::destruct()) {
    nassertr(_server != (GtkStatsServer *)NULL, false);
    delete _server;
    GtkStats::quit();
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsMainWindow::layout_window
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void GtkStatsMainWindow::
layout_window() {
  set_title("Gtk Stats");

  Gtk::VBox *box1 = new Gtk::VBox;
  box1->show();
  box1->set_border_width(8);
  add(*manage(box1));

  Gtk::Label *listening =
    new Gtk::Label("Listening on port " + format_string(_port));
  listening->show();
  box1->pack_start(*manage(listening), true, false, 8);

  Gtk::HBox *box2 = new Gtk::HBox;
  box2->show();
  box1->pack_start(*manage(box2), false, false, 0);

  Gtk::Button *close = new Gtk::Button("Close");
  close->set_usize(80, 30);
  close->show();
  box2->pack_start(*manage(close), true, false, 0);
  close->clicked.connect(slot(this, &GtkStatsMainWindow::close_clicked));
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsMainWindow::close_clicked
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void GtkStatsMainWindow::
close_clicked() {
  destruct();
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsMainWindow::idle_callback
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
gint GtkStatsMainWindow::
idle_callback() {
  if (user_interrupted) {
    destruct();
    return false;
  }
  _server->poll();
  return true;
}
