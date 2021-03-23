/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggToMayaBin.cxx
 * @author Derzsi Dániel
 * @date 2020-10-01
 */

#include "eggToMaya.h"

/**
 * Entrypoint for egg2maya.
 */
int main(int argc, char *argv[]) {
  EggToMaya prog;
  prog.parse_command_line(argc, argv);

  if (!prog.run()) {
    return 1;
  }

  return 0;
}
