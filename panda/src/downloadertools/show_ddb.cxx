/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file show_ddb.cxx
 * @author drose
 * @date 2002-11-02
 */

#include "pandabase.h"
#include "downloadDb.h"
#include "filename.h"

int
main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: show_ddb server.ddb client.ddb\n";
    return 1;
  }

  Filename server_ddb = Filename::from_os_specific(argv[1]);
  Filename client_ddb = Filename::from_os_specific(argv[2]);

  DownloadDb db(server_ddb, client_ddb);
  db.write(std::cout);

  return 0;
}
