// Filename: gtkStatsWindow.cxx
// Created by:  drose (14Jul00)
// 
////////////////////////////////////////////////////////////////////

#include "gtkStatsWindow.h"
#include "gtkStatsMonitor.h"
#include "gtkStatsStripWindow.h"
#include "gtkStatsPianoWindow.h"

using Gtk::Menu_Helpers::MenuElem;
using Gtk::Menu_Helpers::SeparatorElem;

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsWindow::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
GtkStatsWindow::
GtkStatsWindow(GtkStatsMonitor *monitor) : _monitor(monitor) {
  _monitor->add_window(this);
  update_title();
  setup();

  _main_box = new Gtk::VBox;
  _main_box->show();
  add(*manage(_main_box));

  _menu = manage(new Gtk::MenuBar);
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsWindow::destruct
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
bool GtkStatsWindow::
destruct() {
  if (BasicGtkWindow::destruct()) {
    _monitor->remove_window(this);
    _monitor.clear();
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsWindow::update_title
//       Access: Public, Virtual
//  Description: Sets the title bar appropriately, once the client's
//               information is known.
////////////////////////////////////////////////////////////////////
void GtkStatsWindow::
update_title() {
  if (_monitor->is_client_known()) {
    string title =
      _monitor->get_client_progname() + " from " + _monitor->get_client_hostname();
    if (!_monitor->is_alive()) {
      title += " (closed)";
    }
    set_title(title);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsWindow::new_collector
//       Access: Public, Virtual
//  Description: Called when a new collector has become known, in case
//               the window cares.
////////////////////////////////////////////////////////////////////
void GtkStatsWindow::
new_collector(int) {
}
////////////////////////////////////////////////////////////////////
//     Function: GtkStatsWindow::new_thread
//       Access: Public, Virtual
//  Description: Called when a new thread has become known, in case
//               the window cares.
////////////////////////////////////////////////////////////////////
void GtkStatsWindow::
new_thread(int) {
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsWindow::idle
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void GtkStatsWindow::
idle() {
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsWindow::setup_menu
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void GtkStatsWindow::
setup_menu() {
  _file_menu = new Gtk::Menu;

  _file_menu->items().push_back
    (MenuElem("New strip chart", 
	      slot(this, &GtkStatsWindow::menu_open_strip_chart)));
  _file_menu->items().push_back
    (MenuElem("New piano roll", 
	      slot(this, &GtkStatsWindow::menu_open_piano_roll)));

  /*
  _file_menu->items().push_back
    (MenuElem("New window", 
	      slot(this, &GtkStatsWindow::menu_new_window)));
  */

  _file_menu->items().push_back(SeparatorElem());

  _file_menu->items().push_back
    (MenuElem("Disconnect from client", 
	      slot(this, &GtkStatsWindow::menu_disconnect)));
  _file_menu->items().push_back
    (MenuElem("Close window", 
	      slot(this, &GtkStatsWindow::menu_close_window)));
  _file_menu->items().push_back
    (MenuElem("Close all windows this client", 
	      slot(this, &GtkStatsWindow::menu_close_all_windows)));

  _menu->items().push_back(MenuElem("File", *manage(_file_menu)));
  _menu->show();
  _main_box->pack_start(*_menu, false, false);
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsWindow::menu_open_strip_chart
//       Access: Protected
//  Description: Open up a new strip-chart style window for the main
//               thread.
////////////////////////////////////////////////////////////////////
void GtkStatsWindow::
menu_open_strip_chart() {
  new GtkStatsStripWindow(_monitor, 0, 0, 400, 100);
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsWindow::menu_open_piano_roll
//       Access: Protected
//  Description: Open up a new piano-roll style window for the main
//               thread.
////////////////////////////////////////////////////////////////////
void GtkStatsWindow::
menu_open_piano_roll() {
  new GtkStatsPianoWindow(_monitor, 0, 400, 100);
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsWindow::menu_new_window
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void GtkStatsWindow::
menu_new_window() {
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsWindow::menu_close_window
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
void GtkStatsWindow::
menu_close_window() {
  destruct();
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsWindow::menu_close_all_windows
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
void GtkStatsWindow::
menu_close_all_windows() {
  _monitor->close_all_windows();
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsWindow::menu_disconnect
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
void GtkStatsWindow::
menu_disconnect() {
  _monitor->close();
}
