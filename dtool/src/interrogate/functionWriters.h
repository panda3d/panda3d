// Filename: functionWriters.h
// Created by:  drose (14Sep01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef FUNCTIONWRITERS_H
#define FUNCTIONWRITERS_H

#include "dtoolbase.h"
#include "functionWriter.h"

#include <set>

////////////////////////////////////////////////////////////////////
//       Class : FunctionWriters
// Description : A set of zero or more FunctionWriter pointers
//               accumulated by the various InterfaceMaker objects
//               that are generating code for one particular output
//               source file.
////////////////////////////////////////////////////////////////////
class FunctionWriters {
public:
  FunctionWriters();
  ~FunctionWriters();

  FunctionWriter *add_writer(FunctionWriter *writer);

  void write_prototypes(ostream &out);
  void write_code(ostream &out);

protected:
  class IndirectCompareTo {
  public:
    bool operator () (const FunctionWriter *a, const FunctionWriter *b) const {
      return a->compare_to(*b) < 0;
    }
  };

  typedef set<FunctionWriter *, IndirectCompareTo> Writers;
  Writers _writers;
};

#endif
