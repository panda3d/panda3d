/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file gtkStats.cxx
 * @author drose
 * @date 2006-01-16
 */

#include "pandatoolbase.h"
#include "gtkStats.h"
#include "gtkStatsServer.h"
#include "config_pstatclient.h"

int
main(int argc, char *argv[]) {
  gtk_init(&argc, &argv);

  // Create the server window.
  GtkStatsServer *server = new GtkStatsServer;
  server->parse_command_line(argc, argv);

  // Now get lost in the message loop.
  gtk_main();

  return (0);
}
