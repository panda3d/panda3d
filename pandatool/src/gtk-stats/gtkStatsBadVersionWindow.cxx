// Filename: gtkStatsBadVersionWindow.cxx
// Created by:  drose (18May01)
// 
////////////////////////////////////////////////////////////////////

#include "gtkStatsBadVersionWindow.h"
#include "gtkStatsMonitor.h"

#include <string_utils.h>

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
