// Filename: gtkStatsBadVersionWindow.h
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

#ifndef GTKSTATSBADVERSIONWINDOW_H
#define GTKSTATSBADVERSIONWINDOW_H

#include "pandatoolbase.h"

#include "basicGtkWindow.h"

class GtkStatsMonitor;

////////////////////////////////////////////////////////////////////
//       Class : GtkStatsBadVersionWindow
// Description : This window is popped up to indicated an attempted
//               connection from an invalid client.
////////////////////////////////////////////////////////////////////
class GtkStatsBadVersionWindow : public BasicGtkWindow {
public:
  GtkStatsBadVersionWindow(GtkStatsMonitor *monitor,
                           int client_major, int client_minor,
                           int server_major, int server_minor);

private:
  void close_clicked();
};


#endif

