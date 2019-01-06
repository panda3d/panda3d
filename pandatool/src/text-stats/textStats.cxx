/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file textStats.cxx
 * @author drose
 * @date 2000-07-12
 */

#include "textStats.h"
#include "textMonitor.h"

#include "pStatServer.h"
#include "config_pstatclient.h"

#include <signal.h>

static bool user_interrupted = false;

// This simple signal handler lets us know when the user has pressed
// control-C, so we can clean up nicely.
static void signal_handler(int) {
  user_interrupted = true;
}

/**
 *
 */
TextStats::
TextStats() {
  set_program_brief("text-based PStats client");
  set_program_description
    ("This is a simple PStats server that listens on a TCP port for a "
     "connection from a PStatClient in a Panda player.  It will then report "
     "frame rate and timing information sent by the player.");

  add_option
    ("p", "port", 0,
     "Specify the TCP port to listen for connections on.  By default, this "
     "is taken from the pstats-host Config variable.",
     &TextStats::dispatch_int, nullptr, &_port);

  add_option
    ("r", "", 0,
     "Show the raw frame data, in addition to boiling it down to a total "
     "time per collector.",
     &TextStats::dispatch_none, &_show_raw_data, nullptr);

  add_option
    ("o", "filename", 0,
     "Filename where to print. If not given then stderr is being used.",
     &TextStats::dispatch_string, &_got_outputFileName, &_outputFileName);

  _outFile = nullptr;
  _port = pstats_port;
}


/**
 *
 */
PStatMonitor *TextStats::
make_monitor() {

  return new TextMonitor(this, _outFile, _show_raw_data);
}


/**
 *
 */
void TextStats::
run() {
  // Set up a global signal handler to catch Interrupt (Control-C) so we can
  // clean up nicely if the user stops us.
  signal(SIGINT, &signal_handler);

  if (!listen(_port)) {
    nout << "Unable to open port.\n";
    exit(1);
  }

  nout << "Listening for connections.\n";

  if (_got_outputFileName) {
    _outFile = new std::ofstream(_outputFileName.c_str(), std::ios::out);
  } else {
    _outFile = &(nout);
  }

  main_loop(&user_interrupted);
  nout << "Exiting.\n";
}


int main(int argc, char *argv[]) {
  TextStats prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
