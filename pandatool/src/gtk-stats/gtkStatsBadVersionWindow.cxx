// Filename: gtkStatsBadVersionWindow.cxx
// Created by:  drose (18May01)
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

#include "gtkStatsBadVersionWindow.h"
#include "gtkStatsMonitor.h"

#include "string_utils.h"

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsBadVersionWindow::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
GtkStatsBadVersionWindow::
GtkStatsBadVersionWindow(GtkStatsMonitor *monitor,
                         int client_major, int client_minor,
                         int server_major, int server_minor) {
  set_title("Bad client");

  Gtk::VBox *box1 = new Gtk::VBox;
  box1->show();
  box1->set_border_width(8);
  add(*manage(box1));

  string message =
    "Rejected connection by " +
    monitor->get_client_progname() + " from " +
    monitor->get_client_hostname() + ".\nClient uses PStats version " +
    format_string(client_major) + "." + format_string(client_minor) +
    ",\nwhile server expects PStats version " +
    format_string(server_major) + "." + format_string(server_minor) + ".";

  Gtk::Label *label = new Gtk::Label(message);
  label->show();
  box1->pack_start(*manage(label), true, false, 8);

  Gtk::HBox *box2 = new Gtk::HBox;
  box2->show();
  box1->pack_start(*manage(box2), false, false, 0);

  Gtk::Button *close = new Gtk::Button("Close");
  close->set_usize(80, 30);
  close->show();
  box2->pack_start(*manage(close), true, false, 0);
  close->clicked.connect(slot(this, &GtkStatsBadVersionWindow::close_clicked));

  setup();
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsBadVersionWindow::close_clicked
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void GtkStatsBadVersionWindow::
close_clicked() {
  destruct();
}
