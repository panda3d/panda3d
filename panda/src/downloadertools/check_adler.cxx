/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file check_adler.cxx
 */

#include "download_utils.h"
#include "pystub.h"

int
main(int argc, char *argv[]) {
  // A call to pystub() to force libpystub.so to be linked in.
  pystub();

  if (argc < 2) {
    cerr << "Usage: check_adler <file>" << endl;
    return 1;
  }

  Filename source_file = argv[1];

  cout << check_adler(source_file);

  return 0;
}
