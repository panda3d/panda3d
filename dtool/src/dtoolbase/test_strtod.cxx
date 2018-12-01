/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_strtod.cxx
 * @author drose
 * @date 2009-06-14
 */

#include "pstrtod.h"

#ifndef _WIN32
#include <locale.h>
#endif

int
main(int argc, char *argv[]) {
#ifndef _WIN32
  setlocale(LC_ALL, "");
#endif

  for (int i = 1; i < argc; ++i) {
    char *endptr = nullptr;
    double result = pstrtod(argv[i], &endptr);
    std::cerr << "pstrtod - " << argv[i] << " : " << result << " : " << endptr << "\n";
    result = strtod(argv[i], &endptr);
    std::cerr << "strtod - " << argv[i] << " : " << result << " : " << endptr << "\n";
  }

  return 0;
}
