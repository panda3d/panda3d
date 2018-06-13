/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file functionWriters.cxx
 * @author drose
 * @date 2001-09-14
 */

#include "functionWriters.h"

/**
 *
 */
FunctionWriters::
FunctionWriters() {
}

/**
 *
 */
FunctionWriters::
~FunctionWriters() {
  Writers::iterator wi;
  for (wi = _writers.begin(); wi != _writers.end(); ++wi) {
    delete (*wi);
  }
}

/**
 * Adds the indicated FunctionWriter to the set of functions to be written,
 * unless there is already a matching FunctionWriter.
 *
 * The return value is the FunctionWriter pointer that was added to the set,
 * which may be the same pointer or a previously-allocated (but equivalent)
 * pointer.
 */
FunctionWriter *FunctionWriters::
add_writer(FunctionWriter *writer) {
  std::pair<Writers::iterator, bool> result = _writers.insert(writer);
  if (!result.second) {
    // Already there; delete the pointer.
    delete writer;
  }

  // Return the pointer that is in the set.
  return *result.first;
}

/**
 * Generates prototypes for all of the functions.
 */
void FunctionWriters::
write_prototypes(std::ostream &out) {
  Writers::iterator wi;
  for (wi = _writers.begin(); wi != _writers.end(); ++wi) {
    FunctionWriter *writer = (*wi);
    writer->write_prototype(out);
  }
}

/**
 * Generates all of the functions.
 */
void FunctionWriters::
write_code(std::ostream &out) {
  Writers::iterator wi;
  for (wi = _writers.begin(); wi != _writers.end(); ++wi) {
    FunctionWriter *writer = (*wi);
    writer->write_code(out);
  }
}
