// Filename: functionWriters.h
// Created by:  drose (14Sep01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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
