// Filename: gtkStatsWindow.h
// Created by:  drose (14Jul00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef GTKSTATSWINDOW_H
#define GTKSTATSWINDOW_H

#include "pandatoolbase.h"

#include "gtkStatsMonitor.h"

#include <basicGtkWindow.h>

#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : GtkStatsWindow
// Description : This is the base class for a family of windows that
//               are associated with one particular stats client.
//               Each window keeps a pointer back to the
//               GtkStatsMonitor object, which in turn knows about all
//               of the windows; when the last window is closed, the
//               monitor object goes away and ends the session.
////////////////////////////////////////////////////////////////////
class GtkStatsWindow : public BasicGtkWindow {
public:
  GtkStatsWindow(GtkStatsMonitor *monitor);
  virtual bool destruct();

  virtual void update_title();
  virtual void mark_dead();
  virtual void new_collector();
  virtual void idle();

protected:
  virtual void setup_menu();

  void menu_open_strip_chart();
  void menu_open_piano_roll();
  virtual void menu_new_window();
  void menu_close_window();
  void menu_close_all_windows();
  void menu_disconnect();

protected:
  PT(GtkStatsMonitor) _monitor;

  Gtk::VBox *_main_box;
  Gtk::MenuBar *_menu;
  Gtk::Menu *_file_menu;
};


#endif

