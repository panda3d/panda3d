// Filename: gtkStatsMainWindow.h
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

#ifndef GTKSTATSMAINWINDOW_H
#define GTKSTATSMAINWINDOW_H

#include "pandatoolbase.h"

#include <basicGtkWindow.h>

class GtkStatsServer;

////////////////////////////////////////////////////////////////////
//       Class : GtkStatsMainWindow
// Description : This is the main window that's opened up and stays up
//               all the time when you run gtk-stats.  It just shows
//               that it's running.
////////////////////////////////////////////////////////////////////
class GtkStatsMainWindow : public BasicGtkWindow {
public:
  GtkStatsMainWindow(int port);
  virtual ~GtkStatsMainWindow();
  virtual bool destruct();

private:
  void layout_window();
  void close_clicked();
  gint idle_callback();

  int _port;
  GtkStatsServer *_server;
};


#endif

