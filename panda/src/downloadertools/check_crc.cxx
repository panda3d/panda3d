/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file check_crc.cxx
 */

#include "download_utils.h"

int
main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: check_crc <file>" << std::endl;
    return 1;
  }

  Filename source_file = argv[1];

  std::cout << check_crc(source_file);

  return 0;
}
