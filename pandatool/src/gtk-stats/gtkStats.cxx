// Filename: gtkStats.cxx
// Created by:  drose (14Jul00)
// 
////////////////////////////////////////////////////////////////////

#include "gtkStats.h"
#include "gtkStatsMainWindow.h"

#include <pStatServer.h>
#include <config_pstats.h>

#include <signal.h>

GtkStatsMainWindow *GtkStats::_main_window = NULL;

static bool user_interrupted = false;

// This simple signal handler lets us know when the user has pressed
// control-C, so we can clean up nicely.
static void signal_handler(int) {
  user_interrupted = true;
}

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
