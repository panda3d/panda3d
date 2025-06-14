/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file functionWriters.h
 * @author drose
 * @date 2001-09-14
 */

#ifndef FUNCTIONWRITERS_H
#define FUNCTIONWRITERS_H

#include "dtoolbase.h"
#include "functionWriter.h"

#include <set>

/**
 * A set of zero or more FunctionWriter pointers accumulated by the various
 * InterfaceMaker objects that are generating code for one particular output
 * source file.
 */
class FunctionWriters {
public:
  FunctionWriters();
  ~FunctionWriters();

  FunctionWriter *add_writer(FunctionWriter *writer);

  void write_prototypes(std::ostream &out);
  void write_code(std::ostream &out);

protected:
  class IndirectCompareTo {
  public:
    bool operator () (const FunctionWriter *a, const FunctionWriter *b) const {
      return a->compare_to(*b) < 0;
    }
  };

  typedef std::set<FunctionWriter *, IndirectCompareTo> Writers;
  Writers _writers;
};

#endif
