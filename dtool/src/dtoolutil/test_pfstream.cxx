/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_pfstream.cxx
 * @author drose
 * @date 2002-07-31
 */

#include "dtoolbase.h"
#include "pfstream.h"

int
main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cout << "test_pfstream command-line\n";
    return (1);
  }

  // Build one command out of the arguments.
  std::string cmd;
  cmd = argv[1];
  for (int i = 2; i < argc; i++) {
    cmd += " ";
    cmd += argv[i];
  }

  std::cout << "Executing command:\n" << cmd << "\n";

  IPipeStream in(cmd);

  char c;
  c = in.get();
  while (in && !in.fail() && !in.eof()) {
    std::cout.put(toupper(c));
    c = in.get();
  }

  return (0);
}
