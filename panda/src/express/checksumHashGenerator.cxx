/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file checksumHashGenerator.cxx
 * @author drose
 * @date 2001-05-14
 */

#include "checksumHashGenerator.h"

/**
 * Adds a string to the hash, by breaking it down into a sequence of integers.
 */
void ChecksumHashGenerator::
add_string(const std::string &str) {
  add_int(str.length());
  std::string::const_iterator si;
  for (si = str.begin(); si != str.end(); ++si) {
    add_int(*si);
  }
}
