/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_egg.cxx
 * @author drose
 * @date 1999-01-16
 */

#include "eggData.h"
#include "pnotify.h"


int
main(int argc, char *argv[]) {
  if (argc != 2) {
    nout << "Specify an egg file to load.\n";
    exit(1);
  }
  const char *egg_filename = argv[1];

  EggData data;
  data.set_coordinate_system(CS_default);

  if (data.read(egg_filename)) {
    data.load_externals(DSearchPath(Filename("")));
    data.write_egg(std::cout);
  } else {
    nout << "Errors.\n";
  }
  return (0);
}
