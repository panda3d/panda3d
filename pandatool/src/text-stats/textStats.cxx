// Filename: textStats.cxx
// Created by:  drose (12Jul00)
// 
////////////////////////////////////////////////////////////////////

#include "textStats.h"
#include "textMonitor.h"

#include <pStatServer.h>
#include <config_pstats.h>

#include <signal.h>

static bool user_interrupted = false;

// This simple signal handler lets us know when the user has pressed
// control-C, so we can clean up nicely.
static void signal_handler(int) {
  user_interrupted = true;
}

////////////////////////////////////////////////////////////////////
//     Function: TextStats::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
TextStats::
TextStats() {
  set_program_description
    ("This is a simple PStats server that listens on a TCP port for a "
     "connection from a PStatClient in a Panda player.  It will then report "
     "frame rate and timing information sent by the player.");

  add_option
    ("p", "port", 0, 
     "Specify the TCP port to listen for connections on.  By default, this "
     "is taken from the pstats-host Config variable.",
     &TextStats::dispatch_int, NULL, &_port);

  _port = pstats_port;
}


////////////////////////////////////////////////////////////////////
//     Function: TextStats::make_monitor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PStatMonitor *TextStats::
make_monitor() {
  return new TextMonitor;
}
  

////////////////////////////////////////////////////////////////////
//     Function: TextStats::run
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void TextStats::
run() {
  // Set up a global signal handler to catch Interrupt (Control-C) so
  // we can clean up nicely if the user stops us.
  signal(SIGINT, &signal_handler);

  if (!listen(_port)) {
    nout << "Unable to open port.\n";
    exit(1);
  }

  nout << "Listening for connections.\n";

  main_loop(&user_interrupted);
  nout << "Exiting.\n";
}


int main(int argc, char *argv[]) {
  TextStats prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
