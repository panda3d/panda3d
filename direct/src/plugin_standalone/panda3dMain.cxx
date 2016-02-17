/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file panda3dMain.cxx
 * @author drose
 * @date 2009-10-23
 */

#include "panda3d.h"

// The normal, "console" program.
int
main(int argc, char *argv[]) {
  Panda3D program(true);
  return program.run_command_line(argc, argv);
}
