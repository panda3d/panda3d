/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file functionWriter.cxx
 * @author drose
 * @date 2001-09-14
 */

#include "functionWriter.h"

/**
 *
 */
FunctionWriter::
FunctionWriter() {
}

/**
 *
 */
FunctionWriter::
~FunctionWriter() {
}

/**
 *
 */
const std::string &FunctionWriter::
get_name() const {
  return _name;
}

/**
 *
 */
int FunctionWriter::
compare_to(const FunctionWriter &other) const {
  // Lexicographical string comparison.

  std::string::const_iterator n1, n2;
  n1 = _name.begin();
  n2 = other._name.begin();
  while (n1 != _name.end() && n2 != other._name.end()) {
    if (*n1 != *n2) {
      return *n1 - *n2;
    }
    ++n1;
    ++n2;
  }
  if (n1 != _name.end()) {
    return -1;
  }
  if (n2 != other._name.end()) {
    return 1;
  }
  return 0;
}

/**
 * Outputs the prototype for the function.
 */
void FunctionWriter::
write_prototype(std::ostream &) {
}

/**
 * Outputs the code for the function.
 */
void FunctionWriter::
write_code(std::ostream &) {
}
