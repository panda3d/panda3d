// Filename: gtkStats.cxx
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

#include "gtkStats.h"
#include "gtkStatsMainWindow.h"

#include "pStatServer.h"
#include "config_pstats.h"

#include <signal.h>

GtkStatsMainWindow *GtkStats::_main_window = NULL;

////////////////////////////////////////////////////////////////////
//     Function: GtkStats::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
GtkStats::
GtkStats() {
  set_program_description
    ("This is a fancy GUI PStats server that listens on a TCP port for a "
     "connection from a PStatClient in a Panda player.  It will then "
     "draw strip charts illustrating the performance stats as reported "
     "by the player.");

  add_option
    ("p", "port", 0,
     "Specify the TCP port to listen for connections on.  By default, this "
     "is taken from the pstats-host Config variable.",
     &GtkStats::dispatch_int, NULL, &_port);

  _port = pstats_port;
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStats::run
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void GtkStats::
run() {
  new GtkStatsMainWindow(_port);

  main_loop();
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStats::quit
//       Access: Public, Static
//  Description: Call this to cleanly shut down the program.
////////////////////////////////////////////////////////////////////
void GtkStats::
quit() {
  if (_main_window != (GtkStatsMainWindow *)NULL) {
    _main_window->destruct();
  }
  Gtk::Main::quit();
}


int main(int argc, char *argv[]) {
  GtkStats prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
